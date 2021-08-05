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

#ifndef HPSDR_NEWPROTOCOL_H_
#define HPSDR_NEWPROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NUMRECEIVERS 4

// These variables represent the state of the machine
// data from general packet
struct new_protocol_t {
              int ddc_port;                 //
              int duc_port;                 //
              int hp_port;                  //
              int shp_port;                 //
              int audio_port;               //
              int duc0_port;                //
              int ddc0_port;                //
              int mic_port;                 //
              int wide_port;                //
              int wide_enable;              //
              int wide_len;                 //
              int wide_size;                //
              int wide_rate;                //
              int wide_ppf;                 //
              int port_mm;                  //
              int port_smm;                 //
              int pwm_min;                  //
              int pwm_max;                  //
              int bits;                     //
              int hwtim;                    //
              int pa_enable;                //
              int alex0_enable;             //
              int alex1_enable;             //
              int iqform;                   //

// data from rx specific packet
              int adc;                      //
              int adcdither[8];             //
              int adcrandom[8];             //
              int ddcenable[NUMRECEIVERS];  //
              int adcmap[NUMRECEIVERS];     //
              int rxrate[NUMRECEIVERS];     //
              int syncddc[NUMRECEIVERS];    //

// data from tx specific packet
              int dac;                      //
              int cwmode;                   //
              int sidelevel;                //
              int sidefreq;                 //
              int speed;                    //
              int weight;                   //
              int hang;                     //
              int delay;                    //
              int txrate;                   //
              int ducbits;                  //
              int orion;                    //
              int gain;                     //
              int txatt;                    //

// stat from high-priority packet
              int run;                      //
              int ptt;                      //
              int cwx;                      //
              int dot;                      //
              int dash;                     //
    unsigned long rxfreq[NUMRECEIVERS];     //
    unsigned long txfreq;                   //
              int txdrive;                  //
              int w1400;                    // Xvtr and Audio enable
              int ocout;                    //
              int db9;                      //
              int mercury_atts;             //
              int alex0[32];                //
              int alex1[32];                //
              int stepatt0;                 //
              int stepatt1;                 //

// floating point representation of TX-Drive and ADC0-Attenuator
           double rxatt0_dbl;               //
           double rxatt1_dbl;               //
           double txatt_dbl;                //
           double txdrv_dbl;                //
};
extern struct new_protocol_t np_settings;

int np_running(void);

#ifdef __cplusplus
}
#endif

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
