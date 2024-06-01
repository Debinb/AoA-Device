// ADC1 Library

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "adc1.h"
#include "nvic.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initAdc1()
{
    // Enable ADC 1 clocking
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R1;
    _delay_cycles(16);

    //Enable Hardware Averaging
    ADC1_SAC_R = ADC_SAC_AVG_64X;

    //Enable Digital Comparator for ADC 1 --> MID BAND, HYSTERSIS ONCE or ALWAYS,
    ADC1_ACTSS_R &= ~ADC_ACTSS_ASEN2;                // Disable Sample Sequencer 2 for configuration
    ADC1_CC_R = ADC_CC_CS_SYSPLL;                    // select PLL as the time base (not needed, since default value)
    ADC1_PC_R = ADC_PC_SR_1M;                        // select 1Msps rate
    ADC1_EMUX_R = ADC_EMUX_EM2_ALWAYS;               // ALWAYS Sampling
    ADC1_SSCTL2_R = ADC_SSCTL2_END3;                 // mark fourth sample as the end ???

    ADC1_SSMUX2_R |= 2 << 0;                         // Set analog input for single sample
    ADC1_SSMUX2_R |= 4 << 4;
    ADC1_SSMUX2_R |= 0 << 8;
    ADC1_SSMUX2_R |= 1 << 12;

    ADC1_SSOP2_R |= ADC_SSOP2_S3DCOP | ADC_SSOP2_S2DCOP | ADC_SSOP2_S1DCOP | ADC_SSOP2_S0DCOP;   //1st-4th Samples of SS2 sent to Digital Comparator
    ADC1_SSDC2_R |= 0x0 | 0x1 | 0x2 | 0x3;                                                       //Digital Comparators 1-4 selected

    ADC1_ISC_R |= ADC_ISC_DCINSS2;      //Digital Comp Interrupt Status and Clear before interrupt is triggered
    ADC1_DCISC_R |= ADC_DCISC_DCINT0 | ADC_DCISC_DCINT1 | ADC_DCISC_DCINT2 | ADC_DCISC_DCINT3;

    ADC1_DCCTL0_R |= ADC_DCCTL0_CIE | ADC_DCCTL0_CIM_ONCE | ADC_DCCTL0_CIC_MID;                 //Digital Comparator Configs
    ADC1_DCCTL1_R |= ADC_DCCTL1_CIE | ADC_DCCTL1_CIM_ONCE | ADC_DCCTL1_CIC_MID;                 //CIE - Interrupt Enable
    ADC1_DCCTL2_R |= ADC_DCCTL2_CIE | ADC_DCCTL2_CIM_ONCE | ADC_DCCTL2_CIC_MID;                 //HONCE - Hysteresis Once
    ADC1_DCCTL3_R |= ADC_DCCTL3_CIE | ADC_DCCTL3_CIM_ONCE | ADC_DCCTL3_CIC_MID;                 //MID - Mid Band

    ADC1_RIS_R |= ADC_RIS_INRDC;
    ADC1_IM_R |= ADC_IM_DCONSS2;        //Interrupt Mask Enable
    enableNvicInterrupt(INT_ADC1SS2);

    ADC1_ACTSS_R |= ADC_ACTSS_ASEN2;                                                             //Enable Sample Sequencer 2 for sampling
}

void CompRanges(uint16_t COMP1, uint16_t COMP0)
{
    ADC1_DCCMP0_R |= (COMP1 << ADC_DCCMP0_COMP1_S) | (COMP0 << ADC_DCCMP0_COMP0_S);     //Digital Comparator Ranges
    ADC1_DCCMP1_R |= (COMP1 << ADC_DCCMP0_COMP1_S) | (COMP0 << ADC_DCCMP0_COMP0_S);     //Comp1 and comp0 gets values from the timer
    ADC1_DCCMP2_R |= (COMP1 << ADC_DCCMP0_COMP1_S) | (COMP0 << ADC_DCCMP0_COMP0_S);
    ADC1_DCCMP3_R |= (COMP1 << ADC_DCCMP0_COMP1_S) | (COMP0 << ADC_DCCMP0_COMP0_S);
}
