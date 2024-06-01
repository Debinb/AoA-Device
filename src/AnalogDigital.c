/*
 *  Created on: September 10, 2023
 *  Author: Debin Babykutty
/-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------
 * CO-:    Port C7
 * MIC 1:  Port E1 (Least Noise)
 * MIC 2:  Port D3 (Corner)
 * MIC 3:  Port E3
 * MIC 4:  Port E2 (Tower)
*/

// C Library
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <float.h>

//Local Library
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "gpio.h"
#include "uart0.h"
#include "adc0.h"
#include "wait.h"
#include "getInput.h"
#include "nvic.h"
#include "adc1.h"
#include "PeriodicTimer.h"
#include "CrossCorrelate.h"
#include "rgb_led.h"

//MASKS
#define MIC1 PORTE,1    //AIN 2 - Shortest
#define MIC2 PORTD,3    //AIN 4 - Corner
#define MIC3 PORTE,3    //AIN 0 - Farthest
#define MIC4 PORTE,2    //AIN 1 - Tower

#define BUFFER 128
#define MicReadingPerMic 32 //(128 readings / 4 mics) = 32 per mic

//GLOBAL VARIABLES
uint8_t threshold = 50;  //Global variable to set by user
uint8_t switched_buffer = 0;

#pragma DATA_SECTION(ChControl, ".ControlTable")
volatile uint32_t ChControl[];

volatile uint16_t ping[BUFFER];
volatile uint16_t pong[BUFFER];

uint16_t Mic1_PrimData[MicReadingPerMic];
uint16_t Mic2_PrimData[MicReadingPerMic];
uint16_t Mic3_PrimData[MicReadingPerMic];
uint16_t Mic4_PrimData[MicReadingPerMic];

uint16_t Mic1_AltData[MicReadingPerMic];
uint16_t Mic2_AltData[MicReadingPerMic];
uint16_t Mic3_AltData[MicReadingPerMic];
uint16_t Mic4_AltData[MicReadingPerMic];

uint16_t corr12[29];
uint16_t corr23[29];
uint16_t corr13[29];

uint16_t tdoa_12;
uint16_t tdoa_23;
uint16_t tdoa_13;
double estimated_angle = 0.0;

uint8_t validEvent = 0;
uint8_t TDOA_ON = 0;
uint8_t time = 0;

int Mic1Angle = 0;
int Mic2Angle = 0;
int Mic3Angle = 0;

uint8_t HoldOff_Flag = 0;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
void initHw()
{
    initSystemClockTo40Mhz();
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1 | SYSCTL_RCGCTIMER_R2 ;           //Timer 1 Clocking

    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);
    _delay_cycles(3);

    //Sets the MIC pins to analog input
    selectPinAnalogInput(MIC1);
    selectPinAnalogInput(MIC2);
    selectPinAnalogInput(MIC3);
    selectPinAnalogInput(MIC4);

}

//Digital Comparator ADC ISR
void ADC1isr()
{
    ADC1_ISC_R |= ADC_ISC_DCINSS2;      //Digital Comp SS2 Interrupt Status and Clear before interrupt is triggered
    ADC1_DCISC_R |= ADC_DCISC_DCINT0 | ADC_DCISC_DCINT1 | ADC_DCISC_DCINT2 | ADC_DCISC_DCINT3;      //Digital Comp Interrupt Status and Clear before interrupt is triggered

    PeriodicTimer1();
    if(HoldOff_Flag == 1)
    {
        disableNvicInterrupt(INT_ADC0SS2);
    }
    if(TDOA_ON == 1)
    {

        char tdoa[100];

        putsUart0("\n");
        snprintf(tdoa, sizeof(tdoa), "The time difference between Mic 1 and Mic 2 is %d\n microseconds", tdoa_12);
        putsUart0(tdoa);
        snprintf(tdoa, sizeof(tdoa), "The time difference between Mic 2 and Mic 3 is %d\n microseconds", tdoa_23);
        putsUart0(tdoa);
        snprintf(tdoa, sizeof(tdoa), "The time difference between Mic 1 and Mic 3 is %d\n microseconds", tdoa_13);
        putsUart0(tdoa);
    }

}

void Timer1isr()                                //3 sec periodic timer for setting the
{                                               //digital comparator ranges along with threshold

    TIMER1_ICR_R = TIMER_ICR_TATOCINT;          //Clears the timer 1
    //UDMA_ENACLR_R = (1 << 16);
    uint16_t i = 0;
    for (i = 0; i < MicReadingPerMic; i++)
    {
        // Extract ADC readings for mic1, mic2, and mic3 from ping buffer
        Mic1_PrimData[i] = ping[i * 4 + 0];  //  mic1 data is stored first in the buffer
        Mic2_PrimData[i] = ping[i * 4 + 1];
        Mic3_PrimData[i] = ping[i * 4 + 2];
        Mic4_PrimData[i] = ping[i * 4 + 3];
    }

    uint16_t j = 0;
    for (j = 0; j < MicReadingPerMic; j++)
    {
        // Extract ADC readings for mic1, mic2, and mic3 from ping buffer
        Mic1_AltData[j] = pong[j * 4 + 0];  //  mic1 data is stored first in the buffer
        Mic2_AltData[j] = pong[j * 4 + 1];
        Mic3_AltData[j] = pong[j * 4 + 2];
        Mic4_AltData[j] = pong[j * 4 + 3];
    }

    if(switched_buffer == 1)
    {
        ComputeCorrWindowStore(Mic1_PrimData, Mic2_PrimData, corr12);
        ComputeCorrWindowStore(Mic2_PrimData, Mic3_PrimData, corr23);
        ComputeCorrWindowStore(Mic1_PrimData, Mic3_PrimData, corr13);
    }
    else if(switched_buffer == 2)
    {
        ComputeCorrWindowStore(Mic1_AltData, Mic2_AltData, corr12);
        ComputeCorrWindowStore(Mic2_AltData, Mic3_AltData, corr23);
        ComputeCorrWindowStore(Mic1_AltData, Mic3_AltData, corr13);
    }

    tdoa_12 = calculate_tdoa(corr12, 29);
    tdoa_23 = calculate_tdoa(corr23, 29);
    tdoa_13 = calculate_tdoa(corr13, 29);
    estimated_angle = estAoA(tdoa_12, tdoa_23, tdoa_13);



    TIMER2_ICR_R = TIMER_ICR_TATOCINT;

    //UDMA_ENASET_R = (1 << 16);
//
//    uint16_t Mic1 = 0;                          //Read the mic values from the FIFO every 3 seconds
//    Mic1 = readAdc0Ss2();
//
//    uint16_t Mic2 = 0;
//    Mic2 = readAdc0Ss2();
//
//    uint16_t Mic3 = 0;
//    Mic3 = readAdc0Ss2();
//
//    uint16_t Mic4 = 0;
//    Mic4 = readAdc0Ss2();
//
//    uint32_t sum = Mic1 + Mic2 + Mic3 + Mic4;  //Get the average ADC values of the surrounding
//    uint16_t average = (sum/4);                //from all four mics to set the comparator values
//
//    uint16_t COMP1 = 0;                        //Sets the comparator ranges based on
//    uint16_t COMP0 = 0;                        //average and the threshold set.
//    COMP1 = average + threshold;
//    COMP0 = average - threshold;
//    if(COMP1 > COMP0)                          //Write to the comp. ranges only when comp1 > comp0
//    {
//        CompRanges(COMP1, COMP0);
//        char values[100];
//        snprintf(values, sizeof(values), "Sum: %d\t Average: %d\nCOMP1 value is:  %4"PRIu16"\t COMP0 value is:  %4"PRIu16"\n", sum, average, COMP1, COMP0);
//        putsUart0(values);
//    }
    //Debug----------------------------------
}

void Timer2isr()
{
    uint8_t casf = 0;
    if(casf == 0)
    {
        putsUart0("asf");
    }
}

void Timer3isr()
{
    enableNvicInterrupt(INT_ADC0SS2);
    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
}

void initDMA()
{
    //DMA Initialization
    SYSCTL_RCGCDMA_R |= SYSCTL_RCGCDMA_R0;             //Enable DMA system clocking
    _delay_cycles(10);
    UDMA_CFG_R |= UDMA_STAT_MASTEN;                    //Enable uDMA controller
    UDMA_CTLBASE_R |= (uint32_t)ChControl;             //Location of channel control table

    //Channel Configuration
    UDMA_ALTCLR_R |= (1 << 16);                        //Clear Alternative Control Structure
    UDMA_REQMASKCLR_R |= (1 << 16);                    //Channel Request Clear

    UDMA_CHMAP2_R &= ~UDMA_CHMAP2_CH16SEL_M;;          //Assigns ADC0 SS2 to Channel 16

    //Primary control structure
    ChControl[16 * 4 + 0x00] = (uint32_t)&ADC0_SSFIFO2_R;       //Source
    ChControl[16 * 4 + 0x01] = (uint32_t)&ping[BUFFER - 1];     //Destination
    ChControl[16 * 4 + 0x02] = UDMA_CHCTL_DSTINC_16 | UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_ARBSIZE_2 | UDMA_CHCTL_XFERMODE_PINGPONG | (BUFFER - 1) << UDMA_CHCTL_XFERSIZE_S;
    ChControl[16 * 4 + 0x03] = 0;                               //Unused

    //Alternate control structure
    ChControl[16 * 4 + 0x80] = (uint32_t)&ADC0_SSFIFO2_R;       //Source
    ChControl[16 * 4 + 0x81] = (uint32_t)&pong[BUFFER - 1];     //Destination
    ChControl[16 * 4 + 0x82] = UDMA_CHCTL_DSTINC_16 | UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_ARBSIZE_2 | UDMA_CHCTL_XFERMODE_PINGPONG | (BUFFER - 1) << UDMA_CHCTL_XFERSIZE_S;
    ChControl[16 * 4 + 0x83] = 0;                               //Unused

    enableNvicInterrupt(INT_ADC0SS2);
    UDMA_ENASET_R = (1 << 16);      //Enable DMA channel 16
}


void ADCforDMAisr()
{
    //If stop mode for primary structure is valid
    if ((ChControl[16 * 4 + 0x02] & UDMA_CHCTL_XFERMODE_M) == 0)
    {
        switched_buffer = 1;
        // Reset alternate table
        ChControl[16 * 4 + 0x02] = UDMA_CHCTL_DSTINC_16 | UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_ARBSIZE_2 | (BUFFER - 1) << UDMA_CHCTL_XFERSIZE_S | UDMA_CHCTL_XFERMODE_PINGPONG;
        ChControl[16 * 4 + 0x01] = (uint32_t)&pong[BUFFER - 1];     //Destination
    }

    //if stop mode for alternate structure is valid
    if ((ChControl[16 * 4 + 0x82] & UDMA_CHCTL_XFERMODE_M) == 0)
    {
        switched_buffer = 2;
        // Reset alternate table
        ChControl[16 * 4 + 0x82] = UDMA_CHCTL_DSTINC_16 | UDMA_CHCTL_DSTSIZE_16 | UDMA_CHCTL_SRCINC_NONE | UDMA_CHCTL_SRCSIZE_16 | UDMA_CHCTL_ARBSIZE_2 | (BUFFER - 1) << UDMA_CHCTL_XFERSIZE_S | UDMA_CHCTL_XFERMODE_PINGPONG;
        ChControl[16 * 4 + 0x81] = (uint32_t)&ping[BUFFER - 1];     //Destination
    }
    ADC0_ISC_R = ADC_ISC_IN2;
    UDMA_CHIS_R = (1 << 16);
    UDMA_ENASET_R = (1 << 16);      //Enable DMA channel 16
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    USER_DATA data;
    initHw();

    initUart0();
    setUart0BaudRate(115200, 40e6);
    initRgb();
    putsUart0("Starting Audio Analysis \n");
    initDMA();
    initAdc0Ss2();
    setAdc0Ss2Mux(2, 4, 0, 1);
    initAdc1();
    CompRanges(2000, 1000);
    PeriodicTimer2();
    while(true)
    {
        //ProcessData();
        bool valid = false;
        getsUart0(&data);
        putcUart0('\n');
        parseFields(&data);

        if(isCommand(&data, "reset", 0))
        {
            valid = true;
            putsUart0("The system will be reset in 3 seconds\n");
            waitMicrosecond(3000000);
            NVIC_APINT_R = (0X05FA0000 | NVIC_APINT_SYSRESETREQ);      //System Reset Request
        }

        else if(isCommand(&data, "average", 0))
        {
            valid = true;
            uint16_t Mic1 = 0;                          //Read the mic values from the FIFO every 3 seconds
            Mic1 = readAdc0Ss2();
            uint16_t Mic2 = 0;
            Mic2 = readAdc0Ss2();
            uint16_t Mic3 = 0;
            Mic3 = readAdc0Ss2();
            uint16_t Mic4 = 0;
            Mic4 = readAdc0Ss2();

            uint8_t DAC1, DAC2, DAC3, DAC4;
            DAC1 = ((Mic1/4096)*3.3);
            DAC2 = ((Mic2/4096)*3.3);
            DAC3 = ((Mic3/4096)*3.3);
            DAC4 = ((Mic4/4096)*3.3);

            char avg[100];
            snprintf(avg, sizeof(avg), "The average value in DAC units are \n Mic 1: %d\n Mic 2: %d\n Mic 3: %d\n Mic 4: %d\n", DAC1, DAC2, DAC3, DAC4);
            putsUart0(avg);

        }

        else if(isCommand(&data, "level", 0))
        {
            valid = true;
            PeriodicTimer1(3);          //Starts the periodic timer for grabbing COMP0 & COMP1 based on surrounding values.

        }

        else if(isCommand(&data, "backoff", 1))
        {
            valid = true;


        }

        else if(isCommand(&data, "holdoff", 1))
        {
            valid = true;
            uint16_t time = getFieldInteger(&data, 1);
            PeriodicTimer3(time);
            HoldOff_Flag = 1;
        }

        else if(isCommand(&data, "hysteresis", 1))
        {
            valid = true;

        }

        else if(isCommand(&data, "aoa", 0))
        {
            valid = true;

            char aoa[100];
            snprintf(aoa, sizeof(aoa), "The angle of arrival is %.2f\n",estimated_angle);
            putsUart0(aoa);
        }

        else if(isCommand(&data, "aoa", 1))
        {
            valid = true;
            char* mode  = getFieldString(&data, 1);

            if(mode != NULL && cmpStr(mode, "always") == 0)
            {

            }

        }

        else if(isCommand(&data, "tdoa", 1))
        {
            valid = true;
            char* tdoaState = getFieldString(&data, 1);

            if((tdoaState != NULL && cmpStr(tdoaState, "ON") == 0) || (tdoaState != NULL && cmpStr(tdoaState, "on") == 0))
            {
                TDOA_ON = 1;
            }
            else if((tdoaState != NULL && cmpStr(tdoaState, "OFF") == 0) || (tdoaState != NULL && cmpStr(tdoaState, "off") == 0))
            {
                TDOA_ON = 0;
            }
            else
            {
                putsUart0("Wrong use of command. Syntax: tdoa ON|OFF\n");
            }
        }

        if(!valid)
        {
            putsUart0("Invalid Command. Please try again.\n");
            putsUart0("Command List: \n- reset \n- average \n- level \n- backoff [amount] \n- holdoff [duration] \n- hysteresis [setting] \n- aoa \n- aoa always \n- tdoa[ON|OFF]\n");
        }

//        waitMicrosecond(time);
    }
}
