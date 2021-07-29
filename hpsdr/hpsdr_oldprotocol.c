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

#include <stdint.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include "hpsdr_main.h"
#include "hpsdr_definitions.h"
#include "hpsdr_functions.h"
#include "hpsdr_oldprotocol.h"
#include "hpsdr_debug.h"

// These variables store the state of the "old protocol" SDR.
// When every they are changed, this is reported.
struct old_protocol_t {
     int AlexTXrel;               //
     int alexRXout;               //
     int alexRXant;               //
     int MicTS;                   //
     int duplex;                  //
     int receivers;               //
     int rate;                    //
     int preamp;                  //
     int LTdither;                //
     int LTrandom;                //
     int ref10;                   //
     int src122;                  //
     int PMconfig;                //
     int MicSrc;                  //
     int txdrive;                 //
     int txatt;                   //
     int sidetone_volume;         //
     int cw_internal;             //
     int rx_att[2];               //
     int rx1_attE;                //
     int rx_preamp[4];            //
     int MerTxATT0;               //
     int MerTxATT1;               //
     int MetisDB9;                //
     int PeneSel;                 //
     int PureSignal;              //
     int LineGain;                //
     int MicPTT;                  //
     int tip_ring;                //
     int MicBias;                 //
     int ptt;                     //
     int AlexAtt;                 //
     int TX_class_E;              //
     int OpenCollectorOutputs;    //
    long tx_freq;                 //
    long rx_freq[7];              //
     int hermes_config;           //
     int alex_lpf;                //
     int alex_hpf;                //
     int alex_manual;             //
     int alex_bypass;             //
     int lna6m;                   //
     int alexTRdisable;           //
     int vna;                     //
     int c25_ext_board_i2c_data;  //
     int rx_adc[7];               //
     int cw_hang;                 //
     int cw_reversed;             //
     int cw_speed;                //
     int cw_mode;                 //
     int cw_weight;               //
     int cw_spacing;              //
     int cw_delay;                //
     int CommonMercuryFreq;       //
     int freq;                    //
};
struct old_protocol_t op_settings = {
        .AlexTXrel = -1,
        .alexRXout = -1,
        .alexRXant = -1,
        .MicTS = -1,
        .duplex = -1,
        .receivers = -1,
        .rate = -1,
        .preamp = -1,
        .LTdither = -1,
        .LTrandom = -1,
        .ref10 = -1,
        .src122 = -1,
        .PMconfig = -1,
        .MicSrc = -1,
        .txdrive = 0,
        .txatt = 0,
        .sidetone_volume = -1,
        .cw_internal = -1,
        .rx_att = { -1, -1 },
        .rx1_attE = -1,
        .rx_preamp = { -1, -1, -1, -1 },
        .MerTxATT0 = -1,
        .MerTxATT1 = -1,
        .MetisDB9 = -1,
        .PeneSel = -1,
        .PureSignal = -1,
        .LineGain = -1,
        .MicPTT = -1,
        .tip_ring = -1,
        .MicBias = -1,
        .ptt = 0,
        .AlexAtt = -1,
        .TX_class_E = -1,
        .OpenCollectorOutputs = -1,
        .tx_freq = -1,
        .rx_freq = { -1, -1, -1, -1, -1, -1, -1 },
        .hermes_config = -1,
        .alex_lpf = -1,
        .alex_hpf = -1,
        .alex_manual = -1,
        .alex_bypass = -1,
        .lna6m = -1,
        .alexTRdisable = -1,
        .vna = -1,
        .c25_ext_board_i2c_data = -1,
        .rx_adc = { -1, -1, -1, -1, -1, -1, -1 },
        .cw_hang = -1,
        .cw_reversed = -1,
        .cw_speed = -1,
        .cw_mode = -1,
        .cw_weight = -1,
        .cw_spacing = -1,
        .cw_delay = -1,
        .CommonMercuryFreq = -1,
        .freq = -1,
};

// floating-point represeners of TX att, RX att, and RX preamp settings
static double txatt_dbl = 1.0;
static double rxatt_dbl[4] = { 1.0, 1.0, 1.0, 1.0 };   // this reflects both ATT and PREAMP
static double last_i_sample = 0.0;
static double last_q_sample = 0.0;
static double txdrv_dbl = 0.99;
static double txlevel;

void op_tx_samples(uint8_t *buffer) {
    uint8_t *bp;
    int j;
    int16_t sample;

        // Put TX IQ samples into the ring buffer
        // In the old protocol, samples come in groups of 8 bytes L1 L0 R1 R0 I1 I0 Q1 Q0
        // Here, L1/L0 and R1/R0 are audio samples, and I1/I0 and Q1/Q0 are the TX iq samples
        // I1 contains bits 8-15 and I0 bits 0-7 of a signed 16-bit integer. We convert this
        // here to double. If the RX sample rate is larger than the TX on, we perform a
        // simple linear interpolation between the last and current sample.
        // Note that this interpolation causes weak "sidebands" at 48/96/... kHz distance (the
        // strongest ones at 48 kHz).
        double disample, dqsample, idelta, qdelta;
        double sum;
        bp = buffer + 16;  // skip 8 header and 8 SYNC/C&C bytes
        sum = 0.0;
        for (j = 0; j < 126; j++) {
            bp += 4;
            sample = (int) ((signed char) *bp++) << 8;
            sample |= (int) ((signed char) *bp++ & 0xFF);
            disample = (double) sample * 0.000030517578125;  // division by 32768
            sample = (int) ((signed char) *bp++) << 8;
            sample |= (int) ((signed char) *bp++ & 0xFF);
            dqsample = (double) sample * 0.000030517578125;
            sum += (disample * disample + dqsample * dqsample);

            switch (op_settings.rate) {
            case 0:  // RX sample rate = TX sample rate = 48000
                iqsamples.isample[iqsamples.txptr] = disample;
                iqsamples.qsample[iqsamples.txptr++] = dqsample;
                break;
            case 1:  // RX sample rate = 96000; TX sample rate = 48000
                idelta = 0.5 * (disample - last_i_sample);
                qdelta = 0.5 * (dqsample - last_q_sample);
                iqsamples.isample[iqsamples.txptr] = last_i_sample + idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + qdelta;
                iqsamples.isample[iqsamples.txptr] = disample;
                iqsamples.qsample[iqsamples.txptr++] = dqsample;
                break;
            case 2:  // RX sample rate = 192000; TX sample rate = 48000
                idelta = 0.25 * (disample - last_i_sample);
                qdelta = 0.25 * (dqsample - last_q_sample);
                iqsamples.isample[iqsamples.txptr] = last_i_sample + idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 2.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 2.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 3.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 3.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = disample;
                iqsamples.qsample[iqsamples.txptr++] = dqsample;
                break;
            case 3:  // RX sample rate = 384000; TX sample rate = 48000
                idelta = 0.125 * (disample - last_i_sample);
                qdelta = 0.125 * (dqsample - last_q_sample);
                iqsamples.isample[iqsamples.txptr] = last_i_sample + idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 2.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 2.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 3.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 3.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 4.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 4.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 5.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 5.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 6.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 6.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = last_i_sample + 7.0 * idelta;
                iqsamples.qsample[iqsamples.txptr++] = last_q_sample + 7.0 * qdelta;
                iqsamples.isample[iqsamples.txptr] = disample;
                iqsamples.qsample[iqsamples.txptr++] = dqsample;
                break;
            }

            last_i_sample = disample;
            last_q_sample = dqsample;

            if (j == 62)
                bp += 8;  // skip 8 SYNC/C&C bytes of second block
        }
        txlevel = txdrv_dbl * txdrv_dbl * sum * 0.0079365;
        // wrap-around of ring buffer
        if (iqsamples.txptr >= OLDRTXLEN)
            iqsamples.txptr = 0;
}

#define chk_data(a,b,c,FUNCTION) if ((a) != b) {FUNCTION(frame, &b, a, c);}
void op_process_ep2(uint8_t *frame) {
    uint16_t data;
    int rc;
    int mod;

    chk_data(frame[0] & 1, op_settings.ptt, "PTT", ep2_ptt);
    switch (frame[0]) {
    case 0:
    case 1:
        chk_data((frame[1] & 0x03) >> 0, op_settings.rate, "SampleRate", ep2_samplerate);
        chk_data((frame[1] & 0x0C) >> 3, op_settings.ref10, "Ref10MHz", ep2_ref10mhz);
        chk_data((frame[1] & 0x10) >> 4, op_settings.src122, "Source122MHz", ep2_src122mhz);
        chk_data((frame[1] & 0x60) >> 5, op_settings.PMconfig, "Penelope/Mercury config", ep2_pm_config);
        chk_data((frame[1] & 0x80) >> 7, op_settings.MicSrc, "MicSource", ep2_micsrc);
        chk_data(frame[2] & 1, op_settings.TX_class_E, "TX CLASS-E", ep2_txclasse);
        chk_data((frame[2] & 0xfe) >> 1, op_settings.OpenCollectorOutputs, "OpenCollector", ep2_opencollector);
        chk_data(((frame[4] >> 3) & 7) + 1, op_settings.receivers, "RECEIVERS", ep2_receivers);
        chk_data(((frame[4] >> 6) & 1), op_settings.MicTS, "TimeStampMic", ep2_timestampmic);
        chk_data(((frame[4] >> 7) & 1), op_settings.CommonMercuryFreq, "Common Mercury Freq", ep2_commonmercuryfreq);

        mod = 0;
        rc = frame[3] & 0x03;
        if (rc != op_settings.AlexAtt) {
            mod = 1;
            op_settings.AlexAtt = rc;
        }
        rc = (frame[3] & 0x04) >> 2;
        if (rc != op_settings.preamp) {
            mod = 1;
            op_settings.preamp = rc;
        }
        rc = (frame[3] & 0x08) >> 3;
        if (rc != op_settings.LTdither) {
            mod = 1;
            op_settings.LTdither = rc;
        }
        rc = (frame[3] & 0x10) >> 4;
        if (rc != op_settings.LTrandom) {
            mod = 1;
            op_settings.LTrandom = rc;
        }
        if (mod)
            hpsdr_dbg_printf(1, "AlexAtt=%d Preamp=%d Dither=%d Random=%d\n", op_settings.AlexAtt, op_settings.preamp, op_settings.LTdither, op_settings.LTrandom);

        mod = 0;
        rc = (frame[3] & 0x60) >> 5;
        if (rc != op_settings.alexRXant) {
            mod = 1;
            op_settings.alexRXant = rc;
        }
        rc = (frame[3] & 0x80) >> 7;
        if (rc != op_settings.alexRXout) {
            mod = 1;
            op_settings.alexRXout = rc;
        }
        rc = (frame[4] >> 0) & 3;
        if (rc != op_settings.AlexTXrel) {
            mod = 1;
            op_settings.AlexTXrel = rc;
        }
        rc = (frame[4] >> 2) & 1;
        if (rc != op_settings.duplex) {
            mod = 1;
            op_settings.duplex = rc;
        }
        if (mod)
            hpsdr_dbg_printf(1, "RXout=%d RXant=%d TXrel=%d Duplex=%d\n", op_settings.alexRXout, op_settings.alexRXant, op_settings.AlexTXrel, op_settings.duplex);

        if (OLDDEVICE == DEVICE_C25) {
            // Charly25: has two 18-dB preamps that are switched with "preamp" and "dither"
            //           and two attenuators encoded in Alex-ATT
            //           Both only applies to RX1!
            rxatt_dbl[0] = pow(10.0, -0.05 * (12 * op_settings.AlexAtt - 18 * op_settings.LTdither - 18 * op_settings.preamp));
            rxatt_dbl[1] = 1.0;
        } else {
            // Assume that it has ALEX attenuators in addition to the Step Attenuators
            rxatt_dbl[0] = pow(10.0, -0.05 * (10 * op_settings.AlexAtt + op_settings.rx_att[0]));
            rxatt_dbl[1] = 1.0;
        }
        break;

    case 2:
    case 3:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.tx_freq, "TX FREQ", ep2_txfreq);
        break;

    case 4:
    case 5:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[0], "RX FREQ1", ep2_rxfreq1);
        break;

    case 6:
    case 7:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[1], "RX FREQ2", ep2_rxfreq2);
        break;

    case 8:
    case 9:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[2], "RX FREQ3", ep2_rxfreq3);
        break;

    case 10:
    case 11:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[3], "RX FREQ4", ep2_rxfreq4);
        break;

    case 12:
    case 13:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[4], "RX FREQ5", ep2_rxfreq5);
        break;

    case 14:
    case 15:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[5], "RX FREQ6", ep2_rxfreq6);
        break;

    case 16:
    case 17:
        chk_data(frame[4] | (frame[3] << 8) | (frame[2] << 16) | (frame[1] << 24), op_settings.rx_freq[6], "RX FREQ7", ep2_rxfreq7);
        break;

    case 18:
    case 19:
        chk_data(frame[1], op_settings.txdrive, "TX DRIVE", ep2_txdrive);
        chk_data(frame[2] & 0x3F, op_settings.hermes_config, "HERMES CONFIG", ep2_hermesconfig);
        chk_data((frame[2] >> 6) & 0x01, op_settings.alex_manual, "ALEX manual HPF/LPF", ep2_alexmanhpflpf);
        chk_data((frame[2] >> 7) & 0x01, op_settings.vna, "VNA mode", ep2_vnamode);
        chk_data(frame[3] & 0x1F, op_settings.alex_hpf, "ALEX HPF", ep2_alexhpf);
        chk_data((frame[3] >> 5) & 0x01, op_settings.alex_bypass, "ALEX Bypass HPFs", ep2_alexbyphpfs);
        chk_data((frame[3] >> 6) & 0x01, op_settings.lna6m, "ALEX 6m LNA", ep2_alex6mlna);
        chk_data((frame[3] >> 7) & 0x01, op_settings.alexTRdisable, "ALEX T/R disable", ep2_alextrdis);
        chk_data(frame[4], op_settings.alex_lpf, "ALEX LPF", ep2_alexlpf);
        // reset TX level. Leve a little head-room for noise
        txdrv_dbl = (double) op_settings.txdrive * 0.00390625;  // div. by. 256
        break;

    case 20:
    case 21:
        chk_data((frame[1] & 0x01) >> 0, op_settings.rx_preamp[0], "ADC1 preamp", ep2_adc1preamp);
        chk_data((frame[1] & 0x02) >> 1, op_settings.rx_preamp[1], "ADC2 preamp", ep2_adc2preamp);
        chk_data((frame[1] & 0x04) >> 2, op_settings.rx_preamp[2], "ADC3 preamp", ep2_adc3preamp);
        chk_data((frame[1] & 0x08) >> 3, op_settings.rx_preamp[3], "ADC4 preamp", ep2_adc4preamp);
        chk_data((frame[1] & 0x10) >> 4, op_settings.tip_ring, "TIP/Ring", ep2_tipring);
        chk_data((frame[1] & 0x20) >> 5, op_settings.MicBias, "MicBias", ep2_micbias);
        chk_data((frame[1] & 0x40) >> 6, op_settings.MicPTT, "MicPTT", ep2_micptt);
        chk_data((frame[2] & 0x1F) >> 0, op_settings.LineGain, "LineGain", ep2_linegain);
        chk_data((frame[2] & 0x20) >> 5, op_settings.MerTxATT0, "Mercury Att on TX/0", ep2_mercuryattontx0);
        chk_data((frame[2] & 0x40) >> 6, op_settings.PureSignal, "PureSignal", ep2_puresignal);
        chk_data((frame[2] & 0x80) >> 7, op_settings.PeneSel, "PenelopeSelect", ep2_penelopeselect);
        chk_data((frame[3] & 0x0F) >> 0, op_settings.MetisDB9, "MetisDB9", ep2_metisdb9);
        chk_data((frame[3] & 0x10) >> 4, op_settings.MerTxATT1, "Mercury Att on TX/1", ep2_mercuryattontx1);

        if (frame[4] & 0x40) {
            // Some firmware/emulators use bit6 to indicate a 6-bit format
            // for a combined attenuator/preamplifier with the AD9866 chip.
            // The value is between 0 and 60 and formally correspondes to
            // to an RX gain of -12 to +48 dB. However, we set here that
            // a value of +16 (that is, 28 on the 0-60 scale) corresponds to
            // "zero attenuation"
            chk_data(37 - (frame[4] & 0x3F), op_settings.rx_att[0], "RX1 HL ATT/GAIN", ep2_rx1hlattgain);
        } else {
            chk_data((frame[4] & 0x1F) >> 0, op_settings.rx_att[0], "RX1 ATT", ep2_rx1att);
            chk_data((frame[4] & 0x20) >> 5, op_settings.rx1_attE, "RX1 ATT enable", ep2_rx1attenable);
            //
            // Some hardware emulates "switching off ATT and preamp" by setting ATT
            // to 20 dB, because the preamp cannot be switched.
            // if (!rx1_attE) rx_att[0]=20;
        }
        if (OLDDEVICE != DEVICE_C25) {
            // Set RX amplification factors. No switchable preamps available normally.
            rxatt_dbl[0] = pow(10.0, -0.05 * (10 * op_settings.AlexAtt + op_settings.rx_att[0]));
            rxatt_dbl[1] = pow(10.0, -0.05 * (op_settings.rx_att[1]));
            rxatt_dbl[2] = 1.0;
            rxatt_dbl[3] = 1.0;
        }
        break;

    case 22:
    case 23:
        chk_data(frame[1] & 0x1f, op_settings.rx_att[1], "RX2 ATT", ep2_rx2att);
        chk_data((frame[2] >> 6) & 1, op_settings.cw_reversed, "CW REV", ep2_cwrev);
        chk_data(frame[3] & 63, op_settings.cw_speed, "CW SPEED", ep2_cwspeed);
        chk_data((frame[3] >> 6) & 3, op_settings.cw_mode, "CW MODE", ep2_cw_mode);
        chk_data(frame[4] & 127, op_settings.cw_weight, "CW WEIGHT", ep2_cwweight);
        chk_data((frame[4] >> 7) & 1, op_settings.cw_spacing, "CW SPACING", ep2_cwspacing);

        // Set RX amplification factors.
        rxatt_dbl[1] = pow(10.0, -0.05 * (op_settings.rx_att[1]));
        break;

    case 24:
    case 25:
        data = frame[1];
        data |= frame[2] << 8;
        chk_data((frame[2] << 8) | frame[1], op_settings.c25_ext_board_i2c_data, "C25 EXT BOARD DATA", ep2_c25extboarddata);
        break;

    case 28:
    case 29:
        chk_data((frame[1] & 0x03) >> 0, op_settings.rx_adc[0], "RX1 ADC", ep2_rx1adc);
        chk_data((frame[1] & 0x0C) >> 2, op_settings.rx_adc[1], "RX2 ADC", ep2_rx2adc);
        chk_data((frame[1] & 0x30) >> 4, op_settings.rx_adc[2], "RX3 ADC", ep2_rx3adc);
        chk_data((frame[1] & 0xC0) >> 6, op_settings.rx_adc[3], "RX4 ADC", ep2_rx4adc);
        chk_data((frame[2] & 0x03) >> 0, op_settings.rx_adc[4], "RX5 ADC", ep2_rx5adc);
        chk_data((frame[2] & 0x0C) >> 2, op_settings.rx_adc[5], "RX6 ADC", ep2_rx6adc);
        chk_data((frame[2] & 0x30) >> 4, op_settings.rx_adc[6], "RX7 ADC", ep2_rx7adc);
        chk_data((frame[3] & 0x1f), op_settings.txatt, "TX ATT", ep2_txatt);
        txatt_dbl = pow(10.0, -0.05 * (double) op_settings.txatt);
        if (OLDDEVICE == DEVICE_C25) {
            // RedPitaya: Hard-wired ADC settings.
            op_settings.rx_adc[0] = 0;
            op_settings.rx_adc[1] = 1;
            op_settings.rx_adc[2] = 1;
        }
        break;

    case 30:
    case 31:
        chk_data(frame[1] & 1, op_settings.cw_internal, "CW INT", ep2_cwint);
        chk_data(frame[2], op_settings.sidetone_volume, "SIDE TONE VOLUME", ep2_sidetonevolume);
        chk_data(frame[3], op_settings.cw_delay, "CW DELAY", ep2_cwdelay);
        op_settings.cw_delay = frame[3];
        break;

    case 32:
    case 33:
        chk_data((frame[1] << 2) | (frame[2] & 3), op_settings.cw_hang, "CW HANG", ep2_cwhang);
        chk_data((frame[3] << 4) | (frame[4] & 255), op_settings.freq, "SIDE TONE FREQ", ep2_sidetonefreq);
        break;
    }
}

void* op_handler_ep6(void *arg) {
    hpsdr_dbg_printf(1, "< Start handler_ep6 >\n");
    int i, j, k, n, size;
    int header_offset;
    uint32_t counter;
    uint8_t buffer[1032];
    uint8_t *pointer;
    uint8_t id[4] = { 0xef, 0xfe, 1, 6 };
    uint8_t header[40] = {
         // C0   C1   C2   C3   C4
            127, 127, 127, 0,   0,
            33,  17,  21,  127, 127,
            127, 8,   0,   0,   0,
            0,   127, 127, 127, 16,
            0,   0,   0,   0,   127,
            127, 127, 24,  0,   0,
            0,   0,   127, 127, 127,
            32,  66,  66,  66,  66
    };
    int32_t adc1isample, adc1qsample;
    int32_t adc2isample, adc2qsample;
    int32_t dacisample, dacqsample;
    int32_t myisample, myqsample;

    struct timespec delay;
    long wait;
    int noiseIQpt, toneIQpt, divpt, rxptr;
    double i1, q1, fac1, fac2, fac3, fac4;
    int decimation;  // for converting 1536 kHz samples to 48, 192, 384, ....
    unsigned int seed;
    unsigned int tx_fifo_count;

    fac2 = 0;
    fac4 = 0;

    seed = ((uintptr_t) &seed) & 0xffffff;

    memcpy(buffer, id, 4);

    header_offset = 0;
    counter = 0;

    noiseIQpt = 0;
    toneIQpt = 0;
    divpt = 0;

    // The rxptr should never "overtake" the iqsamples.txptr, but
    // it also must not lag behind by too much. Let's take
    // the typical TX FIFO size
    rxptr = iqsamples.txptr - 4096;
    if (rxptr < 0)
        rxptr += OLDRTXLEN;

    clock_gettime(CLOCK_MONOTONIC, &delay);
    while (1) {
        if (!enable_thread)
            break;

        size = op_settings.receivers * 6 + 2;
        n = 504 / size;  // number of samples per 512-byte-block
        // Time (in nanosecs) to "collect" the samples sent in one sendmsg
        if ((48 << op_settings.rate) == 0) {
            wait = (2 * n * 1000000L);
        } else {
            wait = (2 * n * 1000000L) / (48 << op_settings.rate);

        }

        // plug in sequence numbers
        *(uint32_t*) (buffer + 4) = htonl(counter);
        ++counter;

        // This defines the distortion as well as the amplification
        // Use PA settings such that there is full drive at full power (39 dB)

        //  48 kHz   decimation = 32
        //  96 kHz   decimation = 16
        // 192 kHz   decimation =  8
        // 384 kHz   decimation =  4
        decimation = 32 >> op_settings.rate;
        for (i = 0; i < 2; ++i) {
            pointer = buffer + i * 516 - i % 2 * 4 + 8;
            memcpy(pointer, header + header_offset, 8);

            switch (header_offset) {
            case 0:
                // do not set PTT and CW in C0
                // do not set ADC overflow in C1
                if (OLDDEVICE == DEVICE_HERMES_LITE2) {
                    // C2/C3 is TX FIFO count
                    tx_fifo_count = iqsamples.txptr - rxptr;
                    if (tx_fifo_count < 0)
                        tx_fifo_count += OLDRTXLEN;
                    *(pointer + 5) = (tx_fifo_count >> 8) & 0x7F;
                    *(pointer + 6) = tx_fifo_count & 0xFF;
                }
                header_offset = 8;
                break;
            case 8:
                if (OLDDEVICE == DEVICE_HERMES_LITE2) {
                    // HL2: temperature
                    *(pointer + 4) = 0;
                    *(pointer + 5) = tx_fifo_count & 0x7F;  // pseudo random number
                } else {
                    // AIN5: Exciter power
                    *(pointer + 4) = 0;  // about 500 mW
                    *(pointer + 5) = op_settings.txdrive;
                }
                // AIN1: Forward Power
                j = (int) ((4095.0 / c1) * sqrt(100.0 * txlevel * c2));
                *(pointer + 6) = (j >> 8) & 0xFF;
                *(pointer + 7) = (j) & 0xFF;
                header_offset = 16;
                break;
            case 16:
                // AIN2: Reverse power
                // AIN3:
                header_offset = 24;
                break;
            case 24:
                // AIN4:
                // AIN5: supply voltage
                *(pointer + 6) = 0;
                *(pointer + 7) = 63;
                header_offset = 32;
                break;
            case 32:
                header_offset = 0;
                break;
            }

            pointer += 8;
            memset(pointer, 0, 504);
            fac1 = rxatt_dbl[0] * 0.0002239;   // Amplitude of 800-Hz-signal to ADC1
            if (diversity) {
                fac2 = 0.0001 * rxatt_dbl[0];  // Amplitude of broad "man-made" noise to ADC1
                fac4 = 0.0002 * rxatt_dbl[1];  // Amplitude of broad "man-made" noise to ADC2
                // (phase shifted 90 deg., 6 dB stronger)
            }
            for (j = 0; j < n; j++) {
                // ADC1: noise + weak tone on RX, feedback sig. on TX (except STEMlab)
                if (op_settings.ptt && (OLDDEVICE != DEVICE_C25)) {
                    i1 = iqsamples.isample[rxptr] * txdrv_dbl;
                    q1 = iqsamples.qsample[rxptr] * txdrv_dbl;
                    fac3 = IM3a + IM3b * (i1 * i1 + q1 * q1);
                    adc1isample = (txatt_dbl * i1 * fac3 + noiseItab[noiseIQpt]) * 8388607.0;
                    adc1qsample = (txatt_dbl * q1 * fac3 + noiseItab[noiseIQpt]) * 8388607.0;
                } else if (diversity) {
                    // man made noise only to I samples
                    adc1isample = (noiseItab[noiseIQpt] + toneItab[toneIQpt] * fac1 + divtab[divpt] * fac2) * 8388607.0;
                    adc1qsample = (noiseQtab[noiseIQpt] + toneQtab[toneIQpt] * fac1) * 8388607.0;
                } else {
                    adc1isample = (noiseItab[noiseIQpt] + toneItab[toneIQpt] * fac1) * 8388607.0;
                    adc1qsample = (noiseQtab[noiseIQpt] + toneQtab[toneIQpt] * fac1) * 8388607.0;
                }
                // ADC2: noise RX, feedback sig. on TX (only STEMlab)
                if (op_settings.ptt && (OLDDEVICE == DEVICE_C25)) {
                    i1 = iqsamples.isample[rxptr] * txdrv_dbl;
                    q1 = iqsamples.qsample[rxptr] * txdrv_dbl;
                    fac3 = IM3a + IM3b * (i1 * i1 + q1 * q1);
                    adc2isample = (txatt_dbl * i1 * fac3 + noiseItab[noiseIQpt]) * 8388607.0;
                    adc2qsample = (txatt_dbl * q1 * fac3 + noiseItab[noiseIQpt]) * 8388607.0;
                } else if (diversity) {
                    // man made noise to Q channel only
                    adc2isample = noiseItab[noiseIQpt] * 8388607.0;  // Noise
                    adc2qsample = (noiseQtab[noiseIQpt] + divtab[divpt] * fac4) * 8388607.0;
                } else {
                    adc2isample = noiseItab[noiseIQpt] * 8388607.0;  // Noise
                    adc2qsample = noiseQtab[noiseIQpt] * 8388607.0;
                }

                // TX signal with peak=0.407
                if (OLDDEVICE == DEVICE_HERMES_LITE2) {
                    dacisample = iqsamples.isample[rxptr] * 0.230 * 8388607.0;
                    dacqsample = iqsamples.qsample[rxptr] * 0.230 * 8388607.0;
                } else {
                    dacisample = iqsamples.isample[rxptr] * 0.407 * 8388607.0;
                    dacqsample = iqsamples.qsample[rxptr] * 0.407 * 8388607.0;
                }

                for (k = 0; k < op_settings.receivers; k++) {
                    myisample = 0;
                    myqsample = 0;
                    switch (op_settings.rx_adc[k]) {
                    case 0: // ADC1
                        myisample = adc1isample;
                        myqsample = adc1qsample;
                        break;
                    case 1: // ADC2
                        myisample = adc2isample;
                        myqsample = adc2qsample;
                        break;
                    default:
                        myisample = 0;
                        myqsample = 0;
                        break;
                    }
                    if ((OLDDEVICE == DEVICE_METIS || OLDDEVICE == DEVICE_HERMES_LITE) && op_settings.ptt && (k == 1)) {
                        // METIS: TX DAC signal goes to RX2 when TXing
                        myisample = dacisample;
                        myqsample = dacqsample;
                    }
                    if ((OLDDEVICE == DEVICE_HERMES || OLDDEVICE == DEVICE_GRIFFIN || OLDDEVICE == DEVICE_C25 || OLDDEVICE == DEVICE_HERMES_LITE2)
                            && op_settings.ptt && (k == 3)) {
                        // HERMES: TX DAC signal goes to RX4 when TXing
                        myisample = dacisample;
                        myqsample = dacqsample;
                    }
                    if ((OLDDEVICE == DEVICE_ANGELIA || OLDDEVICE == DEVICE_ORION || OLDDEVICE == DEVICE_ORION2) && op_settings.ptt && (k == 4)) {
                        // ANGELIA and beyond: TX DAC signal goes to RX5 when TXing
                        myisample = dacisample;
                        myqsample = dacqsample;
                    }
                    *pointer++ = (myisample >> 16) & 0xFF;
                    *pointer++ = (myisample >> 8) & 0xFF;
                    *pointer++ = (myisample >> 0) & 0xFF;
                    *pointer++ = (myqsample >> 16) & 0xFF;
                    *pointer++ = (myqsample >> 8) & 0xFF;
                    *pointer++ = (myqsample >> 0) & 0xFF;
                }
                // Microphone samples: silence
                pointer += 2;
                rxptr++;
                if (rxptr >= OLDRTXLEN)
                    rxptr = 0;
                noiseIQpt++;
                if (noiseIQpt >= LENNOISE)
                    noiseIQpt = rand_r(&seed) / NOISEDIV;
                toneIQpt += decimation;
                if (toneIQpt >= LENTONE)
                    toneIQpt = 0;
                divpt += decimation;
                if (divpt >= LENDIV)
                    divpt = 0;
            }
        }

        // Wait until the time has passed for all these samples
        delay.tv_nsec += wait;
        while (delay.tv_nsec >= 1000000000) {
            delay.tv_nsec -= 1000000000;
            delay.tv_sec++;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &delay, NULL);

        if (sock_TCP_Client > -1) {
            if (sendto(sock_TCP_Client, buffer, 1032, 0, (struct sockaddr*) &addr_old, sizeof(addr_old)) < 0) {
                hpsdr_dbg_printf(1, "TCP sendmsg error occurred at sequence number: %u !\n", counter);
            }
        } else {
            sendto(sock_udp, buffer, 1032, 0, (struct sockaddr*) &addr_old, sizeof(addr_old));
        }
    }
    active_thread = 0;
    hpsdr_dbg_printf(1, "<Stop handler_ep6 >\n");
    return NULL;
}
