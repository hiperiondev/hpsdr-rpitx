/*
 * hpsdr_newprotocol.h
 *
 *  Created on: 28 jul. 2021
 *      Author: Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 */

#ifndef HPSDR_NEWPROTOCOL_H_
#define HPSDR_NEWPROTOCOL_H_

 int new_protocol_running(void);
void new_protocol_general_packet(unsigned char *buffer);
void *ddc_specific_thread(void *data);
void *duc_specific_thread(void *data);
void *highprio_thread(void *data);
void *rx_thread(void *data);
void *tx_thread(void *data);
void *send_highprio_thread(void *data);
void *audio_thread(void *data);
void *mic_thread(void *data);


#endif /* HPSDR_NEWPROTOCOL_H_ */
