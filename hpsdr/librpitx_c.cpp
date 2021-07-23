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
#include "circular_queue.h"

#define IQBURST 4000

static int Decimation = 1;
int Harmonic = 1;

iqdmasync *iqsender = NULL;
std::complex<float> CIQBuffer[IQBURST];

void rpitx_iq_init(int SampleRate, float SetFrequency) {
    iqsender = new iqdmasync(SetFrequency, SampleRate, 14, IQBURST * 4, MODE_IQ);
}

void rpitx_iq_deinit(void) {
    if (iqsender != NULL)
        delete (iqsender);
}

void rpitx_iq_send(double *isample, double *qsample) {
    int CplxSampleNumber = 0;

    static double IBuffer[IQBURST];
    static double QBuffer[IQBURST];

    int nbread = cq_dequeue_buf(isample, IBuffer, IQBURST);
    cq_dequeue_buf(qsample, QBuffer, IQBURST);

    if (nbread > 0) {
        for (int i = 0; i < nbread ; i++) {
            if (i % Decimation == 0) {
                CIQBuffer[CplxSampleNumber++] = std::complex<float>((float)IBuffer[i], (float)QBuffer[i]);

            }
        }
    }

    iqsender->SetIQSamples(CIQBuffer, CplxSampleNumber, Harmonic);
}
