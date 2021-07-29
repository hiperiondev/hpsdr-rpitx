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

#ifndef _HPSDRSIM_H_
#define _HPSDRSIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <netinet/in.h>

// These two variables monitor whether the TX thread is active
extern int enable_thread;
extern int active_thread;

// Socket for communicating with the "PC side"
extern int sock_TCP_Server;
extern int sock_TCP_Client;
extern int sock_udp;

// Address where to send packets from the old and new protocol
// to the PC
extern struct sockaddr_in addr_new;
extern struct sockaddr_in addr_old;

extern int OLDDEVICE;
extern int NEWDEVICE;

// A table of (random) noise with about -90 dBm on the whole spectrum
// This is a very long table such that there is no audible "beating"
// pattern even at very high sample rates.
#define LENNOISE 1536000
#define NOISEDIV (RAND_MAX / 768000)

extern double noiseItab[LENNOISE];
extern double noiseQtab[LENNOISE];

// A table of (man made) noise fed to the I samples of ADC0
// and to the Q samples of ADC1, such that it can be eliminated
// using DIVERSITY
extern int diversity;

#define LENDIV 16000
extern double divtab[LENDIV];

// An 800-Hz tone with 0 dBm
#define LENTONE 15360
extern double toneItab[LENTONE];
extern double toneQtab[LENTONE];

// TX fifo (needed for PURESIGNAL)

// RTXLEN must be an sixteen-fold multiple of 63
// because we have 63 samples per 512-byte METIS packet,
// and two METIS packets per TCP/UDP packet,
// and two/four/eight-fold up-sampling if the TX sample
// rate is 96000/192000/384000
//
// In the new protocol, TX samples come in bunches of
// 240 samples. So NEWRTXLEN is defined as a multiple of
// 240 not exceeding RTXLEN
#define OLDRTXLEN 64512 // must be larger than NEWRTXLEN
#define NEWRTXLEN 64320

struct samples_t {
	double isample[OLDRTXLEN];
	double qsample[OLDRTXLEN];
	int txptr;
};
extern struct samples_t iqsamples;

extern float TX_Frequency;

// Constants for conversion of TX power
extern double c1, c2;

pthread_t tx_hardware_thread_id;
void* tx_hardware_thread(void*);

// Forward declaration for the debug data
void data_print(char* prfx, double l, double r);

// Forward declarations for new protocol stuff
void np_general_packet(unsigned char *buffer);
 int np_running(void);

// Using clock_nanosleep of librt
extern int clock_nanosleep(clockid_t __clock_id, int __flags, __const struct timespec *__req, struct timespec *__rem);

// Constants defining the distortion of the TX signal
// These give about -24 dBc at full drive, that is
// about the value a reasonable amp gives.
#define IM3a  0.60
#define IM3b  0.20

#ifdef __cplusplus
}
#endif

#endif
