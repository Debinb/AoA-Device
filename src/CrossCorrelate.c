//// CrossCorrelate
//
////-----------------------------------------------------------------------------
//// Device includes, defines, and assembler directives
////-----------------------------------------------------------------------------
//
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "tm4c123gh6pm.h"
#include "CrossCorrelate.h"
#include "uart0.h"
#include "rgb_led.h"
////-----------------------------------------------------------------------------
//// Global variables
////-----------------------------------------------------------------------------
#define length 28  // Adjust this based on expected window size and sequence lengths
#define m 29 //N - L+1
#define SAMPLING_RATE 1000000.0
////-----------------------------------------------------------------------------
//// Function
////-----------------------------------------------------------------------------

void ComputeCorrWindowStore(uint16_t *Data1, uint16_t *Data2, uint16_t *corr)
{
    uint8_t i = 0;
    uint8_t j = 0;

    for(i = 0; i < m; i++)
    {
        corr[i] = 0;
        for(j = 0; j < length; j++)
        {
            if((j+i) < length)
            {
                corr[i] += Data1[j] * Data2[j+i];
            }
        }
    }
}

// Function to calculate Time Difference of Arrival (TDOA) between two cross-correlated arrays
uint16_t calculate_tdoa(uint16_t *corr_array, uint16_t array_length)
{
    uint8_t max_index = 0;
    uint32_t max_value = 0;
    uint8_t i = 0;

    // Find the peak in the correlation array
    for ( i = 0; i < array_length; i++)
    {
        if (corr_array[i] > max_value)
        {
            max_value = corr_array[i];
            max_index = i;
        }
    }

    // Calculate time delay (in seconds) based on peak index and sampling rate
    uint16_t tdoa_seconds = max_index;

    return tdoa_seconds;
}

double estAoA(double tdoa_xy, double tdoa_yz, double tdoa_xz)
{
    // Calculate ratios of TDOAs
    double ratio_xy_yz = tdoa_xy / tdoa_yz;
    double ratio_xy_xz = tdoa_xy / tdoa_xz;

    double estimated_angle = (ratio_xy_yz - ratio_xy_xz) * (180.0 / 3.14159); // Convert radians to degrees

    while (estimated_angle < 0)
    {
          estimated_angle += 360.0; // Add 360 degrees to negative angles
      }
      while (estimated_angle >= 360.0)
      {
          estimated_angle -= 360.0; // Subtract 360 degrees from angles greater than or equal to 360
      }

    return estimated_angle;
}


uint16_t AngleEst(uint16_t Theta, uint16_t T1, uint16_t T2)
{
    uint16_t angle = 0;
    uint16_t K1 = 1;
    uint16_t K2 = 0.5;

    angle = Theta + K1 * (T1 - T2) + K2 * ((T1 - T2) * (T1 - T2));
    return angle;
}
