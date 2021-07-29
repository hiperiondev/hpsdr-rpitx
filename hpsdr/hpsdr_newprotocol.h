/*
 * hpsdr_newprotocol.h
 *
 *  Created on: 28 jul. 2021
 *      Author: Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 */

#ifndef HPSDR_NEWPROTOCOL_H_
#define HPSDR_NEWPROTOCOL_H_

 int np_running(void);
void np_general_packet(unsigned char *buffer);
void *np_ddc_thread(void *data);
void *np_duc_thread(void *data);
void *np_highprio_thread(void *data);
void *np_rx_thread(void *data);
void *np_tx_thread(void *data);
void *np_send_highprio_thread(void *data);
void *np_audio_thread(void *data);
void *np_mic_thread(void *data);

#endif /* HPSDR_NEWPROTOCOL_H_ */
