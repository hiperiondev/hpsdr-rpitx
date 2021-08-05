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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#include "hpsdr_main.h"
#include "hpsdr_debug.h"
#include "hpsdr_definitions.h"
#include "hpsdr_functions.h"
#include "hpsdr_newprotocol.h"

struct new_protocol_t np_settings = {
        .ddc_port = 0,
        .duc_port = 0,
        .hp_port = 0,
        .shp_port = 0,
        .audio_port = 0,
        .duc0_port = 0,
        .ddc0_port = 0,
        .mic_port = 0,
        .wide_port = 0,
        .wide_enable = 0,
        .wide_len = 0,
        .wide_size = 0,
        .wide_rate = 0,
        .wide_ppf = 0,
        .port_mm = 0,
        .port_smm = 0,
        .pwm_min = 0,
        .pwm_max = 0,
        .bits = 0,
        .hwtim = 0,
        .pa_enable = 0,
        .alex0_enable = 0,
        .alex1_enable = 0,
        .iqform = 0,
        .adc = 0,
        .dac = 0,
        .cwmode = 0,
        .sidelevel = 0,
        .sidefreq = 0,
        .speed = 0,
        .weight = 0,
        .hang = 0,
        .delay = 0,
        .txrate = 0,
        .ducbits = 0,
        .orion = 0,
        .gain = 0,
        .txatt = 0,
        .run = 0,
        .ptt = 0,
        .cwx = 0,
        .dot = 0,
        .dash = 0,
        .txfreq = 0,
        .txdrive = 0,
        .w1400 = 0,
        .ocout = 0,
        .db9 = 0,
        .mercury_atts = 0,
        .stepatt0 = 0,
        .stepatt1 = 0,
        .rxatt0_dbl = 1.0,
        .rxatt1_dbl = 1.0,
        .txatt_dbl = 1.0,
        .txdrv_dbl = 0.0
};
// End of state variables

static int txptr = 10000;

static pthread_t ddc_specific_thread_id;
static pthread_t duc_specific_thread_id;
static pthread_t rx_thread_id[NUMRECEIVERS];
static pthread_t tx_thread_id;
static pthread_t mic_thread_id;
static pthread_t audio_thread_id;
static pthread_t highprio_thread_id = 0;
static pthread_t send_highprio_thread_id;

void* np_ddc_thread(void*);
void* np_duc_thread(void*);
void* np_highprio_thread(void*);
void* np_send_highprio_thread(void*);
void* np_rx_thread(void*);
void* np_tx_thread(void*);
void* np_mic_thread(void*);
void* np_audio_thread(void*);

static double txlevel;

int np_running(void) {
    if (np_settings.run)
        return 1;
    else
        return 0;
}

void np_general_packet(unsigned char *buffer) {
    hpsdr_dbg_printf(1, "-- new protocol packet received\n");
    static unsigned long seqnum = 0;
    unsigned long seqold;
    int rc;

    seqold = seqnum;
    seqnum = (buffer[0] >> 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
    if (seqnum != 0 && seqnum != seqold + 1) {
        hpsdr_dbg_printf(1, "GP: SEQ ERROR, old=%lu new=%lu\n", seqold, seqnum);
    }

    rc = (buffer[5] << 8) + buffer[6];
    if (rc == 0)
        rc = 1025;
    if (rc != np_settings.ddc_port || !np_settings.run) {
        np_settings.ddc_port = rc;
        hpsdr_dbg_printf(1, "GP: RX specific rcv        port is  %4d\n", rc);
    }
    rc = (buffer[7] << 8) + buffer[8];
    if (rc == 0)
        rc = 1026;
    if (rc != np_settings.duc_port || !np_settings.run) {
        np_settings.duc_port = rc;
        hpsdr_dbg_printf(1, "GP: TX specific rcv        port is  %4d\n", rc);
    }
    rc = (buffer[9] << 8) + buffer[10];
    if (rc == 0)
        rc = 1027;
    if (rc != np_settings.hp_port || !np_settings.run) {
        np_settings.hp_port = rc;
        hpsdr_dbg_printf(1, "GP: HighPrio Port rcv      port is  %4d\n", rc);
    }
    rc = (buffer[11] << 8) + buffer[12];
    if (rc == 0)
        rc = 1025;
    if (rc != np_settings.shp_port || !np_settings.run) {
        np_settings.shp_port = rc;
        hpsdr_dbg_printf(1, "GP: HighPrio Port snd      port is  %4d\n", rc);
    }
    rc = (buffer[13] << 8) + buffer[14];
    if (rc == 0)
        rc = 1028;
    if (rc != np_settings.audio_port || !np_settings.run) {
        np_settings.audio_port = rc;
        hpsdr_dbg_printf(1, "GP: Audio rcv              port is  %4d\n", rc);
    }
    rc = (buffer[15] << 8) + buffer[16];
    if (rc == 0)
        rc = 1029;
    if (rc != np_settings.duc0_port || !np_settings.run) {
        np_settings.duc0_port = rc;
        hpsdr_dbg_printf(1, "GP: TX data rcv base       port is  %4d\n", rc);
    }
    rc = (buffer[17] << 8) + buffer[18];
    if (rc == 0)
        rc = 1035;
    if (rc != np_settings.ddc0_port || !np_settings.run) {
        np_settings.ddc0_port = rc;
        hpsdr_dbg_printf(1, "GP: RX data snd base       port is  %4d\n", rc);
    }
    rc = (buffer[19] << 8) + buffer[20];
    if (rc == 0)
        rc = 1026;
    if (rc != np_settings.mic_port || !np_settings.run) {
        np_settings.mic_port = rc;
        hpsdr_dbg_printf(1, "GP: Microphone data snd    port is  %4d\n", rc);
    }
    rc = (buffer[21] << 8) + buffer[22];
    if (rc == 0)
        rc = 1027;
    if (rc != np_settings.wide_port || !np_settings.run) {
        np_settings.wide_port = rc;
        hpsdr_dbg_printf(1, "GP: Wideband data snd      port is  %4d\n", rc);
    }
    rc = buffer[23];
    if (rc != np_settings.wide_enable || !np_settings.run) {
        np_settings.wide_enable = rc;
        hpsdr_dbg_printf(1, "GP: Wideband Enable Flag is %d\n", rc);
    }
    rc = (buffer[24] << 8) + buffer[25];
    if (rc == 0)
        rc = 512;
    if (rc != np_settings.wide_len || !np_settings.run) {
        np_settings.wide_len = rc;
        hpsdr_dbg_printf(1, "GP: WideBand Length is %d\n", rc);
    }
    rc = buffer[26];
    if (rc == 0)
        rc = 16;
    if (rc != np_settings.wide_size || !np_settings.run) {
        np_settings.wide_size = rc;
        hpsdr_dbg_printf(1, "GP: Wideband sample size is %d\n", rc);
    }
    rc = buffer[27];
    if (rc != np_settings.wide_rate || !np_settings.run) {
        np_settings.wide_rate = rc;
        hpsdr_dbg_printf(1, "GP: Wideband sample rate is %d\n", rc);
    }
    rc = buffer[28];
    if (rc != np_settings.wide_ppf || !np_settings.run) {
        np_settings.wide_ppf = rc;
        hpsdr_dbg_printf(1, "GP: Wideband PPF is %d\n", rc);
    }
    rc = (buffer[29] << 8) + buffer[30];
    if (rc != np_settings.port_mm || !np_settings.run) {
        np_settings.port_mm = rc;
        hpsdr_dbg_printf(1, "MemMapped Registers rcv port is %d\n", rc);
    }
    rc = (buffer[31] << 8) + buffer[32];
    if (rc != np_settings.port_smm || !np_settings.run) {
        np_settings.port_smm = rc;
        hpsdr_dbg_printf(1, "MemMapped Registers snd port is %d\n", rc);
    }
    rc = (buffer[33] << 8) + buffer[34];
    if (rc != np_settings.pwm_min || !np_settings.run) {
        np_settings.pwm_min = rc;
        hpsdr_dbg_printf(1, "GP: PWM Min value is %d\n", rc);
    }
    rc = (buffer[35] << 8) + buffer[36];
    if (rc != np_settings.pwm_max || !np_settings.run) {
        np_settings.pwm_max = rc;
        hpsdr_dbg_printf(1, "GP: PWM Max value is %d\n", rc);
    }
    rc = buffer[37];
    if (rc != np_settings.bits || !np_settings.run) {
        np_settings.bits = rc;
        hpsdr_dbg_printf(1, "GP: ModeBits=x%02x\n", rc);
    }
    rc = buffer[38];
    if (rc != np_settings.hwtim || !np_settings.run) {
        np_settings.hwtim = rc;
        hpsdr_dbg_printf(1, "GP: Hardware Watchdog enabled=%d\n", rc);
    }

    np_settings.iqform = buffer[39];
    if (np_settings.iqform == 0)
        np_settings.iqform = 3;
    if (np_settings.iqform != 3)
        hpsdr_dbg_printf(1, "GP: Wrong IQ Format requested: %d\n", np_settings.iqform);

    rc = (buffer[58] & 0x01);
    if (rc != np_settings.pa_enable || !np_settings.run) {
        np_settings.pa_enable = rc;
        hpsdr_dbg_printf(1, "GP: PA enabled=%d\n", rc);
    }

    rc = buffer[59] & 0x01;
    if (rc != np_settings.alex0_enable || !np_settings.run) {
        np_settings.alex0_enable = rc;
        hpsdr_dbg_printf(1, "GP: ALEX0 register enable=%d\n", rc);
    }
    rc = (buffer[59] & 0x02) >> 1;
    if (rc != np_settings.alex1_enable || !np_settings.run) {
        np_settings.alex1_enable = rc;
        hpsdr_dbg_printf(1, "GP: ALEX1 register enable=%d\n", rc);
    }

    // Start HighPrio thread if we arrive here for the first time
    // The HighPrio thread keeps running all the time.
    if (!highprio_thread_id) {
        if (pthread_create(&highprio_thread_id, NULL, np_highprio_thread, NULL) < 0) {
            hpsdr_dbg_printf(1, "***** ERROR: Create HighPrio thread");
        }
        pthread_detach(highprio_thread_id);

        // init state arrays to zero for the first time
        memset(np_settings.adcdither, -1, 8 * sizeof(int));
        memset(np_settings.adcrandom, -1, 8 * sizeof(int));
        memset(np_settings.ddcenable, -1, NUMRECEIVERS * sizeof(int));
        memset(np_settings.adcmap, -1, NUMRECEIVERS * sizeof(int));
        memset(np_settings.syncddc, -1, NUMRECEIVERS * sizeof(int));

        memset(np_settings.rxfreq, -1, NUMRECEIVERS * sizeof(unsigned long));
        memset(np_settings.alex0, 0, 32 * sizeof(int));
        memset(np_settings.alex1, 0, 32 * sizeof(int));
    }
}

void* np_ddc_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start ddc_specific_thread port: %d\n", np_settings.ddc_port);
    int sock;
    struct sockaddr_in addr;
    socklen_t lenaddr = sizeof(addr);
    unsigned long seqnum, seqold;
    struct timeval tv;
    unsigned char buffer[2000];
    int yes = 1;
    int rc;
    int i, j;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: RX specific: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.ddc_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: RX specific: bind\n");
        close(sock);
        return NULL;
    }

    seqnum = 0;
    while (np_settings.run) {
        rc = recvfrom(sock, buffer, 1444, 0, (struct sockaddr*) &addr, &lenaddr);
        if (rc < 0 && errno != EAGAIN) {
            hpsdr_dbg_printf(1, "***** ERROR: DDC specific thread: recvmsg\n");
            break;
        }
        if (rc < 0)
            continue;
        if (rc != 1444) {
            hpsdr_dbg_printf(1, "RXspec: Received DDC specific packet with incorrect length\n");
            break;
        }
        seqold = seqnum;
        seqnum = (buffer[0] >> 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        if (seqnum != 0 && seqnum != seqold + 1) {
            hpsdr_dbg_printf(1, "RXspec: SEQ ERROR, old=%lu new=%lu\n", seqold, seqnum);
        }
        if (np_settings.adc != buffer[4]) {
            np_settings.adc = buffer[4];
            hpsdr_dbg_printf(1, "RX: Number of ADCs: %d\n", np_settings.adc);
        }
        for (i = 0; i < np_settings.adc; i++) {
            rc = (buffer[5] >> i) & 0x01;
            if (rc != np_settings.adcdither[i]) {
                np_settings. adcdither[i] = rc;
                hpsdr_dbg_printf(1, "RX: ADC%d dither=%d\n", i, rc);
            }
        }
        for (i = 0; i < np_settings.adc; i++) {
            rc = (buffer[6] >> i) & 0x01;
            if (rc != np_settings.adcrandom[i]) {
                np_settings.adcrandom[i] = rc;
                hpsdr_dbg_printf(1, "RX: ADC%d random=%d\n", i, rc);
            }
        }

        for (i = 0; i < NUMRECEIVERS; i++) {
            int modified = 0;

            rc = buffer[17 + 6 * i];
            if (rc != np_settings.adcmap[i]) {
                modified = 1;
                np_settings.adcmap[i] = rc;
            }

            rc = (buffer[18 + 6 * i] << 8) + buffer[19 + 6 * i];
            if (rc != np_settings.rxrate[i]) {
                modified = 1;
                np_settings.rxrate[i] = rc;
                modified = 1;
            }

            if (np_settings.syncddc[i] != buffer[1363 + i]) {
                np_settings.syncddc[i] = buffer[1363 + i];
                modified = 1;
            }
            rc = (buffer[7 + (i / 8)] >> (i % 8)) & 0x01;
            if (rc != np_settings.ddcenable[i]) {
                modified = 1;
                np_settings.ddcenable[i] = rc;
            }
            if (modified) {
                hpsdr_dbg_printf(1, "RX: DDC%d Enable=%d ADC%d Rate=%d SyncMap=%02x\n", i, np_settings.ddcenable[i], np_settings.adcmap[i],
                        np_settings.rxrate[i], np_settings.syncddc[i]);
                rc = 0;
                for (j = 0; j < 8; j++) {
                    rc += (np_settings.syncddc[i] >> i) & 0x01;
                }
                if (rc > 1) {
                    hpsdr_dbg_printf(1, "WARNING:\n");
                    hpsdr_dbg_printf(1, "WARNING:\n");
                    hpsdr_dbg_printf(1, "WARNING:\n");
                    hpsdr_dbg_printf(1, "WARNING: more than two DDC sync'ed\n");
                    hpsdr_dbg_printf(1, "WARNING: this system is not prepared to handle this case\n");
                    hpsdr_dbg_printf(1, "WARNING: so are most of SDRs around!\n");
                    hpsdr_dbg_printf(1, "WARNING:\n");
                    hpsdr_dbg_printf(1, "WARNING:\n");
                    hpsdr_dbg_printf(1, "WARNING:\n");
                }
            }
        }
    }
    close(sock);
    return NULL;
}

void* np_duc_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start duc_specific_thread port: %d\n", np_settings.duc_port);
    int sock;
    struct sockaddr_in addr;
    socklen_t lenaddr = sizeof(addr);
    unsigned long seqnum, seqold;
    struct timeval tv;
    unsigned char buffer[100];
    int yes = 1;
    int rc;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: TX specific: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.duc_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: TXspec: bind\n");
        close(sock);
        return NULL;
    }

    seqnum = 0;
    while (np_settings.run) {
        rc = recvfrom(sock, buffer, 60, 0, (struct sockaddr*) &addr, &lenaddr);
        if (rc < 0 && errno != EAGAIN) {
            hpsdr_dbg_printf(1, "***** ERROR: TXspec: recvmsg\n");
            break;
        }
        if (rc < 0)
            continue;
        if (rc != 60) {
            hpsdr_dbg_printf(1, "TX: wrong length\n");
            break;
        }
        seqold = seqnum;
        seqnum = (buffer[0] >> 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        if (seqnum != 0 && seqnum != seqold + 1) {
            hpsdr_dbg_printf(1, "TX: SEQ ERROR, old=%lu new=%lu\n", seqold, seqnum);
        }
        if (np_settings.dac != buffer[4]) {
            np_settings.dac = buffer[4];
            hpsdr_dbg_printf(1, "TX: Number of DACs: %d\n", np_settings.dac);
        }
        if (np_settings.cwmode != buffer[5]) {
            np_settings.cwmode = buffer[5];
            hpsdr_dbg_printf(1, "TX: CW mode bits = %x\n", np_settings.cwmode);
        }
        if (np_settings.sidelevel != buffer[6]) {
            np_settings.sidelevel = buffer[6];
            hpsdr_dbg_printf(1, "TX: CW side tone level: %d\n", np_settings.sidelevel);
        }
        rc = (buffer[7] << 8) + buffer[8];
        if (rc != np_settings.sidefreq) {
            np_settings.sidefreq = rc;
            hpsdr_dbg_printf(1, "TX: CW sidetone freq: %d\n", np_settings.sidefreq);
        }
        if (np_settings.speed != buffer[9]) {
            np_settings.speed = buffer[9];
            hpsdr_dbg_printf(1, "TX: CW keyer speed: %d wpm\n", np_settings.speed);
        }
        if (np_settings.weight != buffer[10]) {
            np_settings.weight = buffer[10];
            hpsdr_dbg_printf(1, "TX: CW weight: %d\n", np_settings.weight);
        }
        rc = (buffer[11] << 8) + buffer[12];
        if (np_settings.hang != rc) {
            np_settings.hang = rc;
            hpsdr_dbg_printf(1, "TX: CW hang time: %d msec\n", np_settings.hang);
        }
        if (np_settings.delay != buffer[13]) {
            np_settings.delay = buffer[13];
            hpsdr_dbg_printf(1, "TX: RF delay: %d msec\n", np_settings.delay);
        }
        rc = (buffer[14] << 8) + buffer[15];
        if (np_settings.txrate != rc) {
            np_settings.txrate = rc;
            hpsdr_dbg_printf(1, "TX: DUC sample rate: %d\n", rc);
        }
        if (np_settings.ducbits != buffer[16]) {
            np_settings. ducbits = buffer[16];
            hpsdr_dbg_printf(1, "TX: DUC sample width: %d bits\n", np_settings.ducbits);
        }
        if (np_settings.orion != buffer[50]) {
            np_settings.orion = buffer[50];
            hpsdr_dbg_printf(1, "TX: ORION bits (mic etc): %x\n", np_settings.orion);
        }
        if (np_settings.gain != buffer[51]) {
            np_settings.gain = buffer[51];
            hpsdr_dbg_printf(1, "TX: LineIn Gain (dB): %f\n", 12.0 - 1.5 * np_settings.gain);
        }
        if (np_settings.txatt != buffer[59]) {
            np_settings.txatt = buffer[59];
            np_settings.txatt_dbl = pow(10.0, -0.05 * np_settings.txatt);
            hpsdr_dbg_printf(1, "TX: ATT DUC0/ADC0: %d\n", np_settings.txatt);
        }
    }
    close(sock);
    return NULL;
}

void* np_highprio_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start highprio_thread port: %d\n", np_settings.hp_port);
    int sock;
    struct sockaddr_in addr;
    socklen_t lenaddr = sizeof(addr);
    unsigned long seqnum, seqold;
    unsigned char buffer[2000];
    struct timeval tv;
    int yes = 1;
    int rc;
    unsigned long freq;
    int i;

    seqnum = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: HP: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.hp_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: HP: bind\n");
        close(sock);
        return NULL;
    }

    while (1) {
        rc = recvfrom(sock, buffer, 1444, 0, (struct sockaddr*) &addr, &lenaddr);
        if (rc < 0 && errno != EAGAIN) {
            hpsdr_dbg_printf(1, "***** ERROR: HighPrio thread: recvmsg\n");
            break;
        }
        if (rc < 0)
            continue;
        if (rc != 1444) {
            hpsdr_dbg_printf(1, "Received HighPrio packet with incorrect length\n");
            break;
        }
        seqold = seqnum;
        seqnum = (buffer[0] >> 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        if (seqnum != 0 && seqnum != seqold + 1) {
            hpsdr_dbg_printf(1, "HP: SEQ ERROR, old=%lu new=%lu\n", seqold, seqnum);
        }
        rc = (buffer[4] >> 0) & 0x01;
        if (rc != np_settings.run) {
            np_settings.run = rc;
            hpsdr_dbg_printf(1, "HP: Run=%d\n", rc);
            // if run=0, wait for threads to complete, otherwise spawn them off
            if (np_settings.run) {
                if (pthread_create(&ddc_specific_thread_id, NULL, np_ddc_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create DDC specific thread\n");
                }
                if (pthread_create(&duc_specific_thread_id, NULL, np_duc_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create DUC specific thread\n");
                }
                for (i = 0; i < NUMRECEIVERS; i++) {
                    if (pthread_create(&rx_thread_id[i], NULL, np_rx_thread, (void*) (uintptr_t) i) < 0) {
                        hpsdr_dbg_printf(1, "***** ERROR: Create RX thread\n");
                    }
                }
                if (pthread_create(&tx_thread_id, NULL, np_tx_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create TX thread\n");
                }
                if (pthread_create(&tx_hardware_thread_id, NULL, tx_hardware_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create TX Hardware thread\n");
                }
                if (pthread_create(&send_highprio_thread_id, NULL, np_send_highprio_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create SendHighPrio thread\n");
                }
                if (pthread_create(&mic_thread_id, NULL, np_mic_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create Mic thread\n");
                }
                if (pthread_create(&audio_thread_id, NULL, np_audio_thread, NULL) < 0) {
                    hpsdr_dbg_printf(1, "***** ERROR: Create Audio thread\n");
                }
            } else {
                pthread_join(ddc_specific_thread_id, NULL);
                pthread_join(duc_specific_thread_id, NULL);
                for (i = 0; i < NUMRECEIVERS; i++) {
                    pthread_join(rx_thread_id[i], NULL);
                }
                pthread_join(send_highprio_thread_id, NULL);
                pthread_join(tx_thread_id, NULL);
                pthread_join(mic_thread_id, NULL);
                pthread_join(audio_thread_id, NULL);
                highprio_thread_id = 0;
                hpsdr_dbg_printf(1, "HP thread terminated.\n");
                close(sock);
                return NULL;
            }
        }
        rc = (buffer[4] >> 1) & 0x01;
        if (rc != np_settings.ptt) {
            np_settings.ptt = rc;
            hpsdr_dbg_printf(1, "HP: PTT=%d\n", rc);
            if (np_settings.ptt == 0) {
                memset(iqsamples.isample, 0, sizeof(float) * NEWRTXLEN);
                memset(iqsamples.qsample, 0, sizeof(float) * NEWRTXLEN);
            }
        }
        rc = (buffer[5] >> 0) & 0x01;
        if (rc != np_settings.cwx) {
            np_settings.cwx = rc;
            hpsdr_dbg_printf(1, "HP: CWX=%d\n", rc);
        }
        rc = (buffer[5] >> 1) & 0x01;
        if (rc != np_settings.dot) {
            np_settings.dot = rc;
            hpsdr_dbg_printf(1, "HP: DOT=%d\n", rc);
        }
        rc = (buffer[5] >> 2) & 0x01;
        if (rc != np_settings.dash) {
            np_settings.dash = rc;
            hpsdr_dbg_printf(1, "HP: DASH=%d\n", rc);
        }
        for (i = 0; i < NUMRECEIVERS; i++) {
            freq = (buffer[9 + 4 * i] << 24) + (buffer[10 + 4 * i] << 16) + (buffer[11 + 4 * i] << 8) + buffer[12 + 4 * i];
            if (np_settings.bits & 0x08) {
                freq = round(122880000.0 * (double) freq / 4294967296.0);
            }
            if (freq != np_settings.rxfreq[i]) {
                np_settings.rxfreq[i] = freq;
                hpsdr_dbg_printf(1, "HP: DDC%d freq: %lu\n", i, freq);
            }
        }
        freq = (buffer[329] << 24) + (buffer[330] << 16) + (buffer[331] << 8) + buffer[332];
        if (np_settings.bits & 0x08) {
            freq = round(122880000.0 * (double) freq / 4294967296.0);
        }
        if (freq != np_settings.txfreq) {
            np_settings.txfreq = freq;
            TX_Frequency = freq;
            hpsdr_dbg_printf(1, "HP: DUC freq: %lu\n", freq);
        }
        rc = buffer[345];
        if (rc != np_settings.txdrive) {
            np_settings.txdrive = rc;
            np_settings.txdrv_dbl = (double) np_settings.txdrive * 0.003921568627;
            hpsdr_dbg_printf(1, "HP: TX drive= %d (%f)\n", np_settings.txdrive, np_settings.txdrv_dbl);
        }
        rc = buffer[1400];
        if (rc != np_settings.w1400) {
            np_settings.w1400 = rc;
            hpsdr_dbg_printf(1, "HP: Xvtr/Audio enable=%x\n", rc);
        }
        rc = buffer[1401];
        if (rc != np_settings.ocout) {
            np_settings.ocout = rc;
            hpsdr_dbg_printf(1, "HP: OC outputs=%x\n", rc);
        }
        rc = buffer[1402];
        if (rc != np_settings.db9) {
            np_settings.db9 = rc;
            hpsdr_dbg_printf(1, "HP: Outputs DB9=%x\n", rc);
        }
        rc = buffer[1403];
        if (rc != np_settings.mercury_atts) {
            np_settings.mercury_atts = rc;
            hpsdr_dbg_printf(1, "HP: MercuryAtts=%x\n", rc);
        }
        // Store Alex0 and Alex1 bits in separate ints
        freq = (buffer[1428] << 24) + (buffer[1429] << 16) + (buffer[1430] << 8) + buffer[1431];
        for (i = 0; i < 32; i++) {
            rc = (freq >> i) & 0x01;
            if (rc != np_settings.alex1[i]) {
                np_settings.alex1[i] = rc;
                hpsdr_dbg_printf(1, "HP: ALEX1 bit%d set to %d\n", i, rc);
            }
        }
        freq = (buffer[1432] << 24) + (buffer[1433] << 16) + (buffer[1434] << 8) + buffer[1435];
        for (i = 0; i < 32; i++) {
            rc = (freq >> i) & 0x01;
            if (rc != np_settings.alex0[i]) {
                np_settings.alex0[i] = rc;
                hpsdr_dbg_printf(1, "HP: ALEX0 bit%d set to %d\n", i, rc);
            }
        }
        rc = buffer[1442];
        if (rc != np_settings.stepatt1) {
            np_settings.stepatt1 = rc;
            np_settings.rxatt1_dbl = pow(10.0, -0.05 * np_settings.stepatt1);
            hpsdr_dbg_printf(1, "HP: StepAtt1 = %d\n", rc);
        }
        rc = buffer[1443];
        if (rc != np_settings.stepatt0) {
            np_settings.stepatt0 = rc;
            hpsdr_dbg_printf(1, "HP: StepAtt0 = %d\n", np_settings.stepatt0);
        }
        // rxatt0 depends both on ALEX att and Step Att, so re-calc. it each time
        if (NEWDEVICE == NEW_DEVICE_ORION2) {
            // There is no step attenuator on ANAN7000
            np_settings.rxatt0_dbl = pow(10.0, -0.05 * np_settings.stepatt0);
        } else {
            np_settings.rxatt0_dbl = pow(10.0, -0.05 * (np_settings.stepatt0 + 10 * np_settings.alex0[14] + 20 * np_settings.alex0[13]));
        }
    }
    close(sock);
    return NULL;
}

void* np_rx_thread(void *data) {
    int sock;
    struct sockaddr_in addr;
    // One instance of this thread is started for each DDC
    unsigned long seqnum;
    unsigned char buffer[1444];
    int yes = 1;
    int i;
    long wait;
    double i0sample, q0sample;
    double i1sample, q1sample;
    double irsample, qrsample;
    double fac;
    int sample;
    unsigned char *p;
    int noisept, tonept;
    int myddc;
    int sync, size;
    int myadc, syncadc;
    int rxptr;
    int divptr;
    int decimation;
    unsigned int seed;

    struct timespec delay;

    syncadc = 0;

    myddc = (int) (uintptr_t) data;
    if (myddc < 0 || myddc >= NUMRECEIVERS)
        return NULL;

    hpsdr_dbg_printf(1, "-- Start rx_thread %d port: %d\n", myddc, np_settings.ddc0_port + myddc);

    seqnum = 0;
    // unique seed value for random number generator
    seed = ((uintptr_t) &seed) & 0xffffff;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: RXthread: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.ddc0_port + myddc);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: RXthread: bind\n");
        close(sock);
        return NULL;
    }

    tonept = noisept = 0;
    clock_gettime(CLOCK_MONOTONIC, &delay);
    hpsdr_dbg_printf(1, "RX thread %d, enabled=%d\n", myddc, np_settings.ddcenable[myddc]);
    rxptr = txptr - 4096;
    if (rxptr < 0)
        rxptr += NEWRTXLEN;
    divptr = 0;

    while (np_settings.run) {
        if (np_settings.ddcenable[myddc] <= 0 || np_settings.rxrate[myddc] == 0 || np_settings.rxfreq[myddc] == 0) {
            usleep(5000);
            clock_gettime(CLOCK_MONOTONIC, &delay);
            rxptr = txptr - 4096;
            if (rxptr < 0)
                rxptr += NEWRTXLEN;
            continue;
        }
        decimation = 1536 / np_settings.rxrate[myddc];
        myadc = np_settings.adcmap[myddc];
        // for simplicity, we only allow for a single "synchronized" DDC,
        // this well covers the PURESIGNAL and DIVERSITY cases
        sync = 0;
        i = np_settings.syncddc[myddc];
        while (i) {
            sync++;
            i = i >> 1;
        }
        // sync == 0 means no synchronizatsion
        // sync == 1,2,3  means synchronization with DDC0,1,2
        // Usually we send 238 samples per buffer, but with synchronization
        // we send 119 sample *pairs*.
        if (sync) {
            size = 119;
            wait = 119000000L / np_settings.rxrate[myddc]; // time for these samples in nano-secs
            syncadc = np_settings.adcmap[sync - 1];
        } else {
            size = 238;
            wait = 238000000L / np_settings.rxrate[myddc]; // time for these samples in nano-secs
        }

        // ADC0 RX: noise + 800Hz signal at -73 dBm
        // ADC0 TX: noise + distorted TX signal
        // ADC1 RX: noise
        // ADC1 TX: HERMES only: original TX signal
        // ADC2   : original TX signal
        p = buffer;
        *p++ = (seqnum >> 24) & 0xFF;
        *p++ = (seqnum >> 16) & 0xFF;
        *p++ = (seqnum >> 8) & 0xFF;
        *p++ = (seqnum >> 0) & 0xFF;
        seqnum += 1;
        // do not use time stamps
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        // 24 bits per sample *ALWAYS*
        *p++ = 0;
        *p++ = 24;
        *p++ = 0;
        *p++ = sync ? 2 * size : size;  // should be 238 in either case
        for (i = 0; i < size; i++) {
            // produce noise depending on the ADC
            i1sample = i0sample = noiseItab[noisept];
            q1sample = q0sample = noiseQtab[noisept++];
            if (noisept == LENNOISE)
                noisept = rand_r(&seed) / NOISEDIV;
            // PS: produce sample PAIRS,
            // a) distorted TX data (with Drive and Attenuation)
            // b) original TX data (normalized)
            //
            // DIV: produce sample PAIRS,
            // a) add man-made-noise on I-sample of RX channel
            // b) add man-made-noise on Q-sample of "synced" channel
            if (sync && (np_settings.rxrate[myadc] == 192) && np_settings.ptt && (syncadc == np_settings.adc)) {
                irsample = iqsamples.isample[rxptr];
                qrsample = iqsamples.qsample[rxptr++];
                if (rxptr >= NEWRTXLEN)
                    rxptr = 0;
                fac = np_settings.txatt_dbl * np_settings.txdrv_dbl * (IM3a + IM3b * (irsample * irsample + qrsample * qrsample) * np_settings.txdrv_dbl * np_settings.txdrv_dbl);
                if (myadc == 0) {
                    i0sample += irsample * fac;
                    q0sample += qrsample * fac;
                }
                i1sample = irsample * 0.2899;
                q1sample = qrsample * 0.2899;
            } else if (myadc == 0) {
                i0sample += toneItab[tonept] * 0.0002239 * np_settings.rxatt0_dbl;
                q0sample += toneQtab[tonept] * 0.0002239 * np_settings.rxatt0_dbl;
                tonept += decimation;
                if (tonept >= LENTONE)
                    tonept = 0;
            }
            if (diversity && !sync && myadc == 0) {
                i0sample += 0.0001 * np_settings.rxatt0_dbl * divtab[divptr];
                divptr += decimation;
                if (divptr >= LENDIV)
                    divptr = 0;
            }
            if (diversity && !sync && myadc == 1) {
                q0sample += 0.0002 * np_settings.rxatt1_dbl * divtab[divptr];
                divptr += decimation;
                if (divptr >= LENDIV)
                    divptr = 0;
            }
            if (diversity && sync && !np_settings.ptt) {
                if (myadc == 0)
                    i0sample += 0.0001 * np_settings.rxatt0_dbl * divtab[divptr];
                if (syncadc == 1)
                    q1sample += 0.0002 * np_settings.rxatt1_dbl * divtab[divptr];
                divptr += decimation;
                if (divptr >= LENDIV)
                    divptr = 0;
            }
            if (sync) {
                sample = i0sample * 8388607.0;
                *p++ = (sample >> 16) & 0xFF;
                *p++ = (sample >> 8) & 0xFF;
                *p++ = (sample >> 0) & 0xFF;
                sample = q0sample * 8388607.0;
                *p++ = (sample >> 16) & 0xFF;
                *p++ = (sample >> 8) & 0xFF;
                *p++ = (sample >> 0) & 0xFF;
                sample = i1sample * 8388607.0;
                *p++ = (sample >> 16) & 0xFF;
                *p++ = (sample >> 8) & 0xFF;
                *p++ = (sample >> 0) & 0xFF;
                sample = q1sample * 8388607.0;
                *p++ = (sample >> 16) & 0xFF;
                *p++ = (sample >> 8) & 0xFF;
                *p++ = (sample >> 0) & 0xFF;
            } else {
                sample = i0sample * 8388607.0;
                *p++ = (sample >> 16) & 0xFF;
                *p++ = (sample >> 8) & 0xFF;
                *p++ = (sample >> 0) & 0xFF;
                sample = q0sample * 8388607.0;
                *p++ = (sample >> 16) & 0xFF;
                *p++ = (sample >> 8) & 0xFF;
                *p++ = (sample >> 0) & 0xFF;
            }
        }
        delay.tv_nsec += wait;
        while (delay.tv_nsec >= 1000000000) {
            delay.tv_nsec -= 1000000000;
            delay.tv_sec++;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &delay, NULL);

        if (sendto(sock, buffer, 1444, 0, (struct sockaddr*) &addr_new, sizeof(addr_new)) < 0) {
            hpsdr_dbg_printf(1, "***** ERROR: RX thread sendto\n");
            break;
        }
    }
    close(sock);
    return NULL;
}

// This thread receives data (TX samples) from the PC
void* np_tx_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start tx_thread port: %d\n", np_settings.duc0_port);
    int sock;
    struct sockaddr_in addr;
    socklen_t lenaddr = sizeof(addr);
    unsigned long seqnum, seqold;
    unsigned char buffer[1444];
    int yes = 1;
    int rc;
    int i;
    unsigned char *p;
    int sample;
    double di, dq;
    double sum;
    struct timeval tv;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: TX: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.duc0_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: TX: bind\n");
        close(sock);
        return NULL;
    }

    seqnum = 0;

    while (np_settings.run) {
        rc = recvfrom(sock, buffer, 1444, 0, (struct sockaddr*) &addr, &lenaddr);
        if (rc < 0 && errno != EAGAIN) {
            hpsdr_dbg_printf(1, "***** ERROR: TX thread: recvmsg\n");
            break;
        }
        if (rc < 0)
            continue;
        if (rc != 1444) {
            hpsdr_dbg_printf(1, "Received TX packet with incorrect length\n");
            break;
        }
        seqold = seqnum;
        seqnum = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        if (seqnum != 0 && seqnum != seqold + 1) {
            hpsdr_dbg_printf(1, "TXthread: SEQ ERROR, old=%lu new=%lu\n", seqold, seqnum);
        }
        p = buffer + 4;
        sum = 0.0;
        for (i = 0; i < 240; i++) {
            // process 240 TX iq samples
            sample = (int) ((signed char) (*p++)) << 16;
            sample |= (int) ((((unsigned char) (*p++)) << 8) & 0xFF00);
            sample |= (int) ((unsigned char) (*p++) & 0xFF);
            di = (double) sample / 8388608.0;
            sample = (int) ((signed char) (*p++)) << 16;
            sample |= (int) ((((unsigned char) (*p++)) << 8) & 0xFF00);
            sample |= (int) ((unsigned char) (*p++) & 0xFF);
            dq = (double) sample / 8388608.0;

            // I don't know why (perhaps the CFFIR in the SDR program)
            // but somehow I must multiply the samples to get the correct
            // strength
            di *= 1.118;
            dq *= 1.118;

            // put TX samples into ring buffer
            iqsamples.isample[txptr] = di;
            iqsamples.qsample[txptr++] = dq;
            if (txptr >= NEWRTXLEN)
                txptr = 0;

            // accumulate TX power
            sum += (di * di + dq * dq);
        }
        txlevel = sum * np_settings.txdrv_dbl * np_settings.txdrv_dbl * 0.0041667;
    }
    close(sock);
    return NULL;
}

void* np_send_highprio_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start send_highprio_thread port: %d\n", np_settings.shp_port);
    int sock;
    struct sockaddr_in addr;
    unsigned long seqnum;
    unsigned char buffer[60];
    int yes = 1;
    int rc;
    unsigned char *p;

    seqnum = 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: SendHighPrio thread: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.shp_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: SendHighPrio thread: bind\n");
        close(sock);
        return NULL;
    }

    seqnum = 0;
    while (1) {
        if (!np_settings.run) {
            close(sock);
            break;
        }
        // prepare buffer
        memset(buffer, 0, 60);
        p = buffer;
        *p++ = (seqnum >> 24) & 0xFF;
        *p++ = (seqnum >> 16) & 0xFF;
        *p++ = (seqnum >> 8) & 0xFF;
        *p++ = (seqnum >> 0) & 0xFF;
        *p++ = 0;  // no PTT and CW attached
        *p++ = 0;  // no ADC overload
        *p++ = 0;
        *p++ = np_settings.txdrive;

        p += 6;

        rc = (int) ((4095.0 / c1) * sqrt(100.0 * txlevel * c2));
        *p++ = (rc >> 8) & 0xFF;
        *p++ = (rc) & 0xFF;

        buffer[49] = 63;  // about 13 volts supply

        if (sendto(sock, buffer, 60, 0, (struct sockaddr*) &addr_new, sizeof(addr_new)) < 0) {
            hpsdr_dbg_printf(1, "***** ERROR: HP send thread sendto\n");
            break;
        }
        seqnum++;
        usleep(50000);  // wait 50 msec then send again
    }
    close(sock);
    return NULL;
}

// This thread receives the audio samples and plays them
void* np_audio_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start audio_thread port: %d\n", np_settings.audio_port);
    int sock;
    struct sockaddr_in addr;
    socklen_t lenaddr = sizeof(addr);
    unsigned long seqnum, seqold;
    unsigned char buffer[260];
    int yes = 1;
    int rc;
    int i;
    unsigned char *p;
    int16_t lsample, rsample;
    struct timeval tv;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: Audio: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));
    tv.tv_sec = 0;
    tv.tv_usec = 10000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.audio_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: Audio: bind\n");
        close(sock);
        return NULL;
    }

    seqnum = 0;
    while (np_settings.run) {
        rc = recvfrom(sock, buffer, 260, 0, (struct sockaddr*) &addr, &lenaddr);
        if (rc < 0 && errno != EAGAIN) {
            hpsdr_dbg_printf(1, "***** ERROR: Audio thread: recvmsg\n");
            break;
        }
        if (rc < 0)
            continue;
        if (rc != 260) {
            hpsdr_dbg_printf(1, "Received Audio packet with incorrect length\n");
            break;
        }
        seqold = seqnum;
        seqnum = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
        if (seqnum != 0 && seqnum != seqold + 1) {
            hpsdr_dbg_printf(1, "Audio thread: SEQ ERROR, old=%lu new=%lu\n", seqold, seqnum);
        }
        p = buffer + 4;
        for (i = 0; i < 64; i++) {
            lsample = ((signed char) *p++) << 8;
            lsample |= (*p++ & 0xff);
            rsample = ((signed char) *p++) << 8;
            rsample |= (*p++ & 0xff);
        }
    }
    close(sock);
    return NULL;
}

// The microphone thread just sends silence
void* np_mic_thread(void *data) {
    hpsdr_dbg_printf(1, "-- Start mic_thread port: %d\n", np_settings.mic_port);
    int sock;
    struct sockaddr_in addr;
    unsigned long seqnum;
    unsigned char buffer[132];
    unsigned char *p;
    int yes = 1;
    struct timespec delay;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: Mic thread: socket\n");
        return NULL;
    }

    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(np_settings.mic_port);

    if (bind(sock, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        hpsdr_dbg_printf(1, "***** ERROR: Mic thread: bind\n");
        close(sock);
        return NULL;
    }

    seqnum = 0;
    memset(buffer, 0, 132);
    clock_gettime(CLOCK_MONOTONIC, &delay);
    while (np_settings.run) {
        // update seq number
        p = buffer;
        *p++ = (seqnum >> 24) & 0xFF;
        *p++ = (seqnum >> 16) & 0xFF;
        *p++ = (seqnum >> 8) & 0xFF;
        *p++ = (seqnum >> 0) & 0xFF;
        seqnum++;
        // 64 samples with 48000 kHz, makes 1333333 nsec
        delay.tv_nsec += 1333333;
        while (delay.tv_nsec >= 1000000000) {
            delay.tv_nsec -= 1000000000;
            delay.tv_sec++;
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &delay, NULL);

        if (sendto(sock, buffer, 132, 0, (struct sockaddr*) &addr_new, sizeof(addr_new)) < 0) {
            hpsdr_dbg_printf(1, "***** ERROR: Mic thread sendto\n");
            break;
        }
    }
    close(sock);
    return NULL;
}
