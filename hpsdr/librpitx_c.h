/*
 * librpitx_c_wrapper.h
 *
 *  Created on: 18 jul. 2021
 *      Author: Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 */

#ifndef LIBRPITX_C_WRAPPER_H_
#define LIBRPITX_C_WRAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

void rpitx_iq_init(int SampleRate, float SetFrequency);
void rpitx_iq_deinit(void);
void rpitx_iq_send(struct samples_t *iqsamples_tx, int *enable);

#ifdef __cplusplus
}
#endif

#endif
