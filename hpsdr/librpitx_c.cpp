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

#include "util.hpp"
#include "librpitx.hpp"
#include "librpitx_c.h"
#include "hpsdr_main.h"
#include "hpsdr_oldprotocol.h"
#include "hpsdr_newprotocol.h"

static int Decimation = 1;
int Harmonic = 1;

iqdmasync *iqsender = NULL;

void rpitx_iq_init(int rpitx_SampleRate, int rpitx_SetFrequency) {
    dbg_printf(0, " >> rpitx_iq_init  SampleRate:%d SetFrequency:%d <<\n", rpitx_SampleRate, rpitx_SetFrequency);
    iqsender = new iqdmasync((uint64_t)rpitx_SetFrequency, (uint32_t)rpitx_SampleRate, 14, IQBURST * 4, MODE_IQ);
    iqsender->Setppm(0);
}

void rpitx_iq_deinit(void) {
    dbg_printf(0, " >> rpitx_iq_deinit <<\n");
    if (iqsender != NULL)
        delete (iqsender);
}

void rpitx_iq_send(struct samples_t *iqsamples_tx, int *enable) {
    dbg_printf(0, " >> Start rpitx iq send | IQBURST: %d <<\n", IQBURST);
    int *ext_txdrive;

    if (np_running()) {
        ext_txdrive = &(np_settings.txdrive);
    } else {
        ext_txdrive = &(op_settings.txdrive);
    }

    std::complex<float> CIQBuffer[IQBURST];
    int CplxSampleNumber = 0;
    int rp_txptr = iqsamples_tx->txptr;
    float tx_drive = 0;

    while (1) {
        if (*ext_txdrive != tx_drive) {
            tx_drive = *ext_txdrive;
            dbg_printf(0, "                TX DRIVE= %d%\n", (int)((tx_drive / 255) * 100));
        }

        if (rp_txptr > iqsamples_tx->txptr)
            rp_txptr = iqsamples_tx->txptr;

        CplxSampleNumber = 0;

        for (int i = 0; i < IQBURST; i++) {
            if (i % Decimation == 0) {
                CIQBuffer[CplxSampleNumber++] = std::complex<float>(
                                                    ((float) iqsamples_tx->isample[rp_txptr]) * (tx_drive / 255),
                                                    ((float) iqsamples_tx->qsample[rp_txptr]) * (tx_drive / 255)
                                                );
                rp_txptr++;
            }
        }
        iqsender->SetIQSamples(CIQBuffer, CplxSampleNumber, Harmonic);

        if (!*enable) {
            break;
        }
    }
    dbg_printf(0, " >> Stop rpitx iq send <<\n");
}
