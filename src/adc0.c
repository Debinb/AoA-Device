// ADC0 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// ADC0 SS3

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "adc0.h"

//Dithering improves resolution of signal quality
#define ADC_CTL_DITHER          0x00000040

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initAdc0Ss2()
{
    // Enable clocks
    SYSCTL_RCGCADC_R |= SYSCTL_RCGCADC_R0;
    _delay_cycles(16);

    // Configure ADC for sampling
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN2;                // disable sample sequencer 2 (SS2) for programming
    ADC0_CC_R = ADC_CC_CS_SYSPLL;                    // select PLL as the time base (not needed, since default value)
    ADC0_PC_R = ADC_PC_SR_1M;                        // select 1Msps rate
    ADC0_EMUX_R = ADC_EMUX_EM2_ALWAYS;               // Continuous Sampling
    ADC0_SSCTL2_R = ADC_SSCTL2_END3 | ADC_SSCTL2_IE3;                 // mark fourth sample as the end ???
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN2;                 // enable SS2 for operation
}

// Set SS3 analog input
void setAdc0Ss2Mux(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4)
{
    ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN2;                // disable sample sequencer 3 (SS3) for programming
    ADC0_SSMUX2_R |= in1 << 0;                       // Set analog input for single sample
    ADC0_SSMUX2_R |= in2 << 4;
    ADC0_SSMUX2_R |= in3 << 8;
    ADC0_SSMUX2_R |= in4 << 12;
    ADC0_ACTSS_R |= ADC_ACTSS_ASEN2;                 // enable SS3 for operation
}

// Request and read one sample from SS3
int16_t readAdc0Ss2()
{
    ADC0_PSSI_R |= ADC_PSSI_SS2 | ADC_PSSI_SYNCWAIT;                     // set start bit

    //while (ADC0_ACTSS_R & ADC_ACTSS_BUSY);           // wait until SS3 is not busy
    while (ADC0_SSFSTAT2_R & ADC_SSFSTAT2_EMPTY);
    return ADC0_SSFIFO2_R;                           // get single result from the FIFO
}
