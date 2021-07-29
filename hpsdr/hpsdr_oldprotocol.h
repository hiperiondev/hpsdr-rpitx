/*
 * hpsdr_oldprotocol.h
 *
 *  Created on: 29 jul. 2021
 *      Author: Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 */

#ifndef HPSDR_OLDPROTOCOL_H_
#define HPSDR_OLDPROTOCOL_H_

void op_tx_samples(uint8_t *buffer);
void op_process_ep2(uint8_t *frame);
void *op_handler_ep6(void *arg);

#endif
