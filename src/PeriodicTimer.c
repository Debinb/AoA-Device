/*
 * PeriodicTimer.c
 *
 *  Created on: Apr 19, 2024
 *      Author: debin
 */
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "nvic.h"


void PeriodicTimer1()
{
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                     // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;               // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;              // configure for periodic mode (count down)
    TIMER1_TAILR_R = (436000);               // Timer Ticks # = Seconds x Clock Freq.
    TIMER1_IMR_R = TIMER_IMR_TATOIM;                     // turn-on interrupts for timeout in timer module
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                      // turn-on timer
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);                  // turn-on interrupt 37 (TIMER1A)
}

void PeriodicTimer2()
{
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                     // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;               // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;              // configure for periodic mode (count down)
    TIMER2_TAILR_R = (43600);               // Timer Ticks # = Seconds x Clock Freq.
    TIMER2_IMR_R = TIMER_IMR_TATOIM;                     // turn-on interrupts for timeout in timer module
    TIMER2_CTL_R |= TIMER_CTL_TAEN;                      // turn-on timer
    //enableNvicInterrupt(INT_TIMER2A);
}

void PeriodicTimer3(uint16_t seconds)
{
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                     // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;               // configure as 32-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;              // configure for periodic mode (count down)
    TIMER3_TAILR_R = (seconds * 40000000);               // Timer Ticks # = Seconds x Clock Freq.
    TIMER3_IMR_R = TIMER_IMR_TATOIM;                     // turn-on interrupts for timeout in timer module
    TIMER3_CTL_R |= TIMER_CTL_TAEN;                      // turn-on timer
    enableNvicInterrupt(INT_TIMER3A);                 // turn-on interrupt 37 (TIMER1A)
}
