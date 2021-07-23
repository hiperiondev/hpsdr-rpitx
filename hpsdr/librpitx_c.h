/*
 * librpitx_c_wrapper.h
 *
 *  Created on: 18 jul. 2021
 *      Author: egonzalez
 */

#ifndef LIBRPITX_C_WRAPPER_H_
#define LIBRPITX_C_WRAPPER_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void rpitx_iq_init(int SampleRate, float SetFrequency);
void rpitx_iq_deinit(void);
void rpitx_iq_send(double *isample, double *qsample);

#ifdef __cplusplus
}
#endif

#endif
