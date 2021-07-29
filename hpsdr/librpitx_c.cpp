/*
 * librpitx_c_wrapper.c
 *
 *  Created on: 18 jul. 2021
 *      Author: egonzalez
 */
#include <librpitx_c.h>
#include <stdio.h>
#include <stdbool.h>
#include "librpitx.h"
#include "hpsdr_main.h"

#define IQBURST 10
#define OLDRTXLEN 64512

static int Decimation = 1;
int Harmonic = 1;

iqdmasync *iqsender = NULL;
std::complex<float> CIQBuffer[IQBURST];

void rpitx_iq_init(int SampleRate, float SetFrequency) {
	iqsender = new iqdmasync(SetFrequency, SampleRate, 14, IQBURST * 4, MODE_IQ);
	iqsender->Setppm(0);
}

void rpitx_iq_deinit(void) {
	if (iqsender != NULL)
		delete (iqsender);
}

void rpitx_iq_send(struct samples_t *iqsamples_tx, int *enable) {
	int CplxSampleNumber = 0;
	int rp_txptr = iqsamples_tx->txptr;
	while (1) {

		if (rp_txptr > iqsamples_tx->txptr)
			rp_txptr = iqsamples_tx->txptr;

		CplxSampleNumber = 0;

		for (int i = 0; i < IQBURST; i++) {
			if (i % Decimation == 0) {
				CIQBuffer[CplxSampleNumber++] = std::complex<float>(
						(float) iqsamples_tx->isample[rp_txptr],
						(float) iqsamples_tx->qsample[rp_txptr]);
				rp_txptr++;
			}
		}
		iqsender->SetIQSamples(CIQBuffer, CplxSampleNumber, Harmonic);

		if (!*enable) {
			break;
		}
	}
}
