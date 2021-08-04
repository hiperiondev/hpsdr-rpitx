/*
 * Copyright 2021 Emiliano Gonzalez LU3VEA (lu3vea @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/hpsdr-rpitx *
 *
 * This is based on other projects:
 *    librpitx (https://github.com/F5OEO/librpitx)
 *    HPSDR simulator (https://github.com/g0orx/pihpsdr)
 *
 *    please contact their authors for more information.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <stdio.h>
#include <stdbool.h>

#include "librpitx.hpp"
#include "librpitx_c.h"
#include "hpsdr_main.h"

#define IQBURST 10
#define OLDRTXLEN 64512

static int Decimation = 1;
int Harmonic = 1;

iqdmasync *iqsender = NULL;
std::complex<float> CIQBuffer[IQBURST];

void rpitx_iq_init(uint32_t SampleRate, uint64_t SetFrequency) {
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
                                                    (float) iqsamples_tx->qsample[rp_txptr]
                                                );
                rp_txptr++;
            }
        }
        iqsender->SetIQSamples(CIQBuffer, CplxSampleNumber, Harmonic);

        if (!*enable) {
            break;
        }
    }
}
