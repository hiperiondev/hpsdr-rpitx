//
// hpsdrsim.h, define global data
//
// From the main program, this is included with EXTERN="", while
// other modules include is with "EXTERN=extern".
//
///////////////////////////////////////////////////////////////////////////
//
// The 800-Hz tone and the "man made noise" are for a sample rate of
// 1536 kHz, and must be decimated when using smaller sample rates
//
///////////////////////////////////////////////////////////////////////////

#ifndef _HPSDRSIM_H_
#define _HPSDRSIM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <netinet/in.h>

// These two variables monitor whether the TX thread is active
int enable_thread;
int active_thread;

// Socket for communicating with the "PC side"
int sock_TCP_Server;
int sock_TCP_Client;
int sock_udp;

// Address where to send packets from the old and new protocol
// to the PC
struct sockaddr_in addr_new;
struct sockaddr_in addr_old;

int OLDDEVICE;
int NEWDEVICE;

// A table of (random) noise with about -90 dBm on the whole spectrum
// This is a very long table such that there is no audible "beating"
// pattern even at very high sample rates.
#define LENNOISE 1536000
#define NOISEDIV (RAND_MAX / 768000)

double noiseItab[LENNOISE];
double noiseQtab[LENNOISE];

// A table of (man made) noise fed to the I samples of ADC0
// and to the Q samples of ADC1, such that it can be eliminated
// using DIVERSITY
int diversity;

#define LENDIV 16000
double divtab[LENDIV];

// An 800-Hz tone with 0 dBm
#define LENTONE 15360
double toneItab[LENTONE];
double toneQtab[LENTONE];

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
struct samples_t iqsamples;

// Constants for conversion of TX power
double c1, c2;

pthread_t tx_hardware_thread_id;
void* tx_hardware_thread(void*);

// Forward declaration for the debug data
void data_print(char* prfx, double l, double r);

// Forward declarations for new protocol stuff
void new_protocol_general_packet(unsigned char *buffer);
int new_protocol_running(void);

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
