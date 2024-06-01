///*
// * CrossCorrelate.h
// *
// *  Created on: Apr 30, 2024
// *      Author: debin
// */

#ifndef CROSSCORRELATE_H_
#define CROSSCORRELATE_H_

void ComputeCorrWindowStore(uint16_t *Data1, uint16_t *Data2, uint16_t *corr);
uint16_t calculate_tdoa( uint16_t *corr_array, uint16_t array_length) ;
double estAoA(double tdoa_xy, double tdoa_yz, double tdoa_xz);
uint16_t AngleEst(uint16_t Theta, uint16_t T1, uint16_t T2);

#endif /* CROSSCORRELATE_H_ */
