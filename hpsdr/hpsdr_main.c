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
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <termios.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include "hpsdr_main.h"
#include "hpsdr_debug.h"
#include "hpsdr_functions.h"
#include "hpsdr_definitions.h"
#include "hpsdr_oldprotocol.h"
#include "hpsdr_newprotocol.h"
#include "librpitx_c.h"

pthread_t tx_hardware_thread_id;

void* tx_hardware_thread(void *data);

static int oldnew = 3;    // 1: only P1, 2: only P2, 3: P1 and P2,

int main(int argc, char *argv[]) {
    int i, j, size;
    pthread_t thread;

    uint8_t reply[11] = { 0xef, 0xfe, 2, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0, 1 };

    uint8_t id[4] = { 0xef, 0xfe, 1, 6 };
    uint32_t code;
    //int16_t sample; //, l, r;

    struct sockaddr_in addr_udp;
    uint8_t buffer[1032];
    struct timeval tv;
    int yes = 1;
    //uint8_t *bp;
    unsigned long checksum;
    socklen_t lenaddr;
    struct sockaddr_in addr_from;
    unsigned int seed;

    uint32_t last_seqnum = 0xffffffff, seqnum;  // sequence number of received packet

    int udp_retries = 0;
    int bytes_read, bytes_left;
    uint32_t *code0 = (uint32_t*) buffer;  // fast access to code of first buffer

    double run, off, inc;

    checksum = 0;
    sock_TCP_Server = -1;
    sock_TCP_Client = -1;

    // Examples for METIS:     ATLAS bus with Mercury/Penelope boards
    // Examples for HERMES:    ANAN10, ANAN100
    // Examples for ANGELIA:   ANAN100D
    // Examples for ORION:     ANAN200D
    // Examples for ORION2:    ANAN7000, ANAN8000
    // Examples for C25:       RedPitaya based boards with fixed ADC connections

    // seed value for random number generator
    seed = ((uintptr_t) &seed) & 0xffffff;
    diversity = 0;
    OLDDEVICE = DEVICE_HERMES_LITE;
    NEWDEVICE = NEW_DEVICE_ORION2;

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-atlas", 6)) {
            OLDDEVICE = DEVICE_METIS;
            NEWDEVICE = NEW_DEVICE_ATLAS;
        }

        if (!strncmp(argv[i], "-hermes", 7)) {
            hpsdr_dbg_printf(1, "HERMES\n");
            OLDDEVICE = DEVICE_HERMES;
            NEWDEVICE = NEW_DEVICE_HERMES;
        }

        if (!strncmp(argv[i], "-griffin", 8)) {
            OLDDEVICE = DEVICE_GRIFFIN;
            NEWDEVICE = NEW_DEVICE_HERMES2;
        }

        if (!strncmp(argv[i], "-angelia", 8)) {
            OLDDEVICE = DEVICE_ANGELIA;
            NEWDEVICE = NEW_DEVICE_ANGELIA;
        }

        if (!strncmp(argv[i], "-orion", 6)) {
            OLDDEVICE = DEVICE_ORION;
            NEWDEVICE = NEW_DEVICE_ORION;
        }

        if (!strncmp(argv[i], "-orion2", 7)) {
            OLDDEVICE = DEVICE_ORION2;
            NEWDEVICE = NEW_DEVICE_ORION2;
        }

        if (!strncmp(argv[i], "-hermeslite", 11)) {
            OLDDEVICE = DEVICE_HERMES_LITE;
            NEWDEVICE = NEW_DEVICE_HERMES_LITE;
        }

        if (!strncmp(argv[i], "-hermeslite2", 12)) {
            OLDDEVICE = DEVICE_HERMES_LITE2;
            NEWDEVICE = NEW_DEVICE_HERMES_LITE2;
        }

        if (!strncmp(argv[i], "-c25", 4)) {
            OLDDEVICE = DEVICE_C25;
            NEWDEVICE = NEW_DEVICE_HERMES;
        }

        if (!strncmp(argv[i], "-diversity", 10)) {
            diversity = 1;
        }

        if (!strncmp(argv[i], "-P1", 3)) {
            oldnew = 1;
        }

        if (!strncmp(argv[i], "-P2", 3)) {
            oldnew = 2;
        }

        if (!strncmp(argv[i], "-debug", 3)) {
            hpsdr_dbg_setlevel(1);
        }

        /*
         if (!strncmp(argv[i], "-help", 3) || !strncmp(argv[i], "--help", 3)) {
         printf("Options:\n"
         "    -atlas: \n"
         "    -hermes: \n"
         "    -griffin: \n"
         "    -angelia: \n"
         "    -orion: \n"
         "    -orion2: \n"
         "    -hermeslite: \n"
         "    -hermeslite2: \n"
         "    -c25: \n"
         "    -diversity: \n"
         "    -P1: \n"
         "    -P2: \n"
         "    -debug: \n"
         "    -debugtx: \n"
         "    -debugrx: \n");
         exit(0);
         }
         */
    }

    switch (OLDDEVICE) {
    case DEVICE_METIS:
        hpsdr_dbg_printf(1, "DEVICE is METIS\n");
        c1 = 3.3;
        c2 = 0.090;
        break;
    case DEVICE_HERMES:
        hpsdr_dbg_printf(1, "DEVICE is HERMES\n");
        c1 = 3.3;
        c2 = 0.095;
        break;
    case DEVICE_GRIFFIN:
        hpsdr_dbg_printf(1, "DEVICE is GRIFFIN\n");
        c1 = 3.3;
        c2 = 0.095;
        break;
    case DEVICE_ANGELIA:
        hpsdr_dbg_printf(1, "DEVICE is ANGELIA\n");
        c1 = 3.3;
        c2 = 0.095;
        break;
    case DEVICE_HERMES_LITE:
        hpsdr_dbg_printf(1, "DEVICE is HermesLite V1\n");
        c1 = 3.3;
        c2 = 0.095;
        break;
    case DEVICE_HERMES_LITE2:
        hpsdr_dbg_printf(1, "DEVICE is HermesLite V2\n");
        c1 = 3.3;
        c2 = 0.095;
        break;
    case DEVICE_ORION:
        hpsdr_dbg_printf(1, "DEVICE is ORION\n");
        c1 = 5.0;
        c2 = 0.108;
        break;
    case DEVICE_ORION2:
        hpsdr_dbg_printf(1, "DEVICE is ORION MkII\n");
        c1 = 5.0;
        c2 = 0.108;
        break;
    case DEVICE_C25:
        hpsdr_dbg_printf(1, "DEVICE is STEMlab/C25\n");
        c1 = 3.3;
        c2 = 0.090;
        break;
    }

    // Initialize the data in the sample tables
    hpsdr_dbg_printf(1, ".... producing random noise\n");
    // Produce some noise
    j = RAND_MAX / 2;
    for (i = 0; i < LENNOISE; i++) {
        noiseItab[i] = ((double) rand_r(&seed) / j - 1.0) * 0.00003;
        noiseQtab[i] = ((double) rand_r(&seed) / j - 1.0) * 0.00003;
    }

    hpsdr_dbg_printf(1, ".... producing signals\n");
    // Produce an 800 Hz tone at 0 dBm
    run = 0.0;
    off = 0.0;
    for (i = 0; i < LENTONE; i++) {
        toneQtab[i] = cos(run) + cos(off);
        toneItab[i] = sin(run) + sin(off);
        run += 0.0032724923474893679567319201909161;
        off += 0.016362461737446839783659600954581;
    }

    if (diversity) {
        hpsdr_dbg_printf(1, "DIVERSITY testing activated!\n");
        hpsdr_dbg_printf(1, ".... producing some man-made noise\n");
        memset(divtab, 0, LENDIV * sizeof(double));
        for (j = 1; j <= 200; j++) {
            run = 0.0;
            off = 0.25 * j * j;
            inc = j * 0.00039269908169872415480783042290994;
            for (i = 0; i < LENDIV; i++) {
                divtab[i] += cos(run + off);
                run += inc;
            }
        }
        // normalize
        off = 0.0;
        for (i = 0; i < LENDIV; i++) {
            if (divtab[i] > off)
                off = divtab[i];
            if (-divtab[i] > off)
                off = -divtab[i];
        }
        off = 1.0 / off;
        hpsdr_dbg_printf(1, "(normalizing with %f)\n", off);
        for (i = 0; i < LENDIV; i++) {
            divtab[i] = divtab[i] * off;
        }
    }

    // clear TX fifo
    memset(iqsamples.isample, 0, OLDRTXLEN * sizeof(double));
    memset(iqsamples.qsample, 0, OLDRTXLEN * sizeof(double));

    if ((sock_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        hpsdr_dbg_printf(1, "socket");
        return EXIT_FAILURE;
    }

    setsockopt(sock_udp, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));
    setsockopt(sock_udp, SOL_SOCKET, SO_REUSEPORT, (void*) &yes, sizeof(yes));

    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(sock_udp, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    memset(&addr_udp, 0, sizeof(addr_udp));
    addr_udp.sin_family = AF_INET;
    addr_udp.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_udp.sin_port = htons(1024);

    if (bind(sock_udp, (struct sockaddr*) &addr_udp, sizeof(addr_udp)) < 0) {
        hpsdr_dbg_printf(1, "bind");
        return EXIT_FAILURE;
    }

    if ((sock_TCP_Server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        hpsdr_dbg_printf(1, "socket tcp");
        return EXIT_FAILURE;
    }

    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_REUSEADDR, (void*) &yes, sizeof(yes));

    int tcpmaxseg = 1032;
    setsockopt(sock_TCP_Server, IPPROTO_TCP, TCP_MAXSEG, (const char*) &tcpmaxseg, sizeof(int));

    int sndbufsize = 65535;
    int rcvbufsize = 65535;
    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_SNDBUF, (const char*) &sndbufsize, sizeof(int));
    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVBUF, (const char*) &rcvbufsize, sizeof(int));
    tv.tv_sec = 0;
    tv.tv_usec = 1000;
    setsockopt(sock_TCP_Server, SOL_SOCKET, SO_RCVTIMEO, (void*) &tv, sizeof(tv));

    if (bind(sock_TCP_Server, (struct sockaddr*) &addr_udp, sizeof(addr_udp)) < 0) {
        hpsdr_dbg_printf(1, "bind tcp");
        return EXIT_FAILURE;
    }

    listen(sock_TCP_Server, 1024);
    hpsdr_dbg_printf(1, "Listening for TCP client connection request\n");

    int flags = fcntl(sock_TCP_Server, F_GETFL, 0);
    fcntl(sock_TCP_Server, F_SETFL, flags | O_NONBLOCK);

    while (1) {
        memcpy(buffer, id, 4);

        if (sock_TCP_Client > -1) {
            // Using recvmmsg with a time-out should be used for a byte-stream protocol like TCP
            // (Each "packet" in the datagram may be incomplete). This is especially true if the
            // socket has a receive time-out, but this problem also occurs if the is no such
            // receive time-out.
            // Therefore we read a complete packet here (1032 bytes). Our TCP-extension to the
            // HPSDR protocol ensures that only 1032-byte packets may arrive here.
            bytes_read = 0;
            bytes_left = 1032;
            while (bytes_left > 0) {
                size = recvfrom(sock_TCP_Client, buffer + bytes_read, (size_t) bytes_left, 0, NULL, 0);
                if (size < 0 && errno == EAGAIN)
                    continue;
                if (size < 0)
                    break;
                bytes_read += size;
                bytes_left -= size;

            }

            bytes_read = size;
            if (size >= 0) {
                // 1032 bytes have successfully been read by TCP.
                // Let the downstream code know that there is a single packet, and its size
                bytes_read = 1032;

                // In the case of a METIS-discovery packet, change the size to 63
                if (*code0 == 0x0002feef) {
                    bytes_read = 63;
                }

                // In principle, we should check on (*code0 & 0x00ffffff) == 0x0004feef,
                // then we cover all kinds of start and stop packets.

                // In the case of a METIS-stop packet, change the size to 64
                if (*code0 == 0x0004feef) {
                    bytes_read = 64;
                }

                // In the case of a METIS-start TCP packet, change the size to 64
                // The special start code 0x11 has no function any longer, but we shall still support it.
                if (*code0 == 0x1104feef || *code0 == 0x0104feef) {
                    bytes_read = 64;
                }
            }
        } else {
            lenaddr = sizeof(addr_from);
            bytes_read = recvfrom(sock_udp, buffer, 1032, 0, (struct sockaddr*) &addr_from, &lenaddr);
            if (bytes_read > 0) {
                udp_retries = 0;
            } else {
                udp_retries++;
            }
        }

        if (bytes_read < 0 && errno != EAGAIN) {
            hpsdr_dbg_printf(1, "recvfrom");
            return EXIT_FAILURE;
        }

        // If nothing has arrived via UDP for some time, try to open TCP connection.
        // "for some time" means 10 subsequent un-successful UDP rcvmmsg() calls
        if (sock_TCP_Client < 0 && udp_retries > 10) {
            if ((sock_TCP_Client = accept(sock_TCP_Server, NULL, NULL)) > -1) {
                hpsdr_dbg_printf(1, "sock_TCP_Client: %d connected to sock_TCP_Server: %d\n", sock_TCP_Client, sock_TCP_Server);
            }
            // This avoids firing accept() too often if it constantly fails
            udp_retries = 0;
        }
        if (bytes_read <= 0)
            continue;
        memcpy(&code, buffer, 4);

        hpsdr_dbg_printf(2, "-- code received: %04x (%d)\n", code, code);

        switch (code) {
        // PC to SDR transmission via process_ep2
        case 0x0201feef:
            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 1032) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, (int) bytes_read);
                break;
            }

            // sequence number check
            seqnum = ((buffer[4] & 0xFF) << 24) + ((buffer[5] & 0xFF) << 16) + ((buffer[6] & 0xFF) << 8) + (buffer[7] & 0xFF);

            if (seqnum != last_seqnum + 1) {
                hpsdr_dbg_printf(1, "SEQ ERROR: last %ld, recvd %ld\n", (long) last_seqnum, (long) seqnum);
            }

            last_seqnum = seqnum;

            op_process_ep2(buffer + 11);
            op_process_ep2(buffer + 523);

            if (active_thread) {
                op_tx_samples(buffer);
            }
            break;

            // respond to an incoming Metis detection request
        case 0x0002feef:
            if (oldnew == 2) {
                hpsdr_dbg_printf(1, "OldProtocol detection request IGNORED.\n");
                break;  // swallow P1 detection requests
            }

            hpsdr_dbg_printf(1, "Respond to an incoming Metis detection request / code: 0x%08x\n", code);

            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 63) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, (int) bytes_read);
                break;
            }
            reply[2] = 2;
            if (active_thread || np_running()) {
                reply[2] = 3;
            }
            reply[9] = 31; // software version
            reply[10] = OLDDEVICE;
            if (OLDDEVICE == DEVICE_HERMES_LITE2) {
                // use HL1 device ID and new software version
                reply[9] = 41;
                reply[10] = DEVICE_HERMES_LITE;
            }
            memset(buffer, 0, 60);
            memcpy(buffer, reply, 11);

            if (sock_TCP_Client > -1) {
                // We will get into trouble if we respond via TCP while the radio is
                // running with TCP.
                // We simply suppress the response in this (very unlikely) case.
                if (!active_thread) {
                    if (send(sock_TCP_Client, buffer, 60, 0) < 0) {
                        hpsdr_dbg_printf(1, "TCP send error occurred when responding to an incoming Metis detection request!\n");
                    }
                    // close the TCP socket which was only used for the detection
                    close(sock_TCP_Client);
                    sock_TCP_Client = -1;
                }
            } else {
                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
            }

            break;

            // stop the SDR to PC transmission via handler_ep6
        case 0x0004feef:
            hpsdr_dbg_printf(1, "STOP the transmission via handler_ep6 / code: 0x%08x\n", code);

            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 64) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, bytes_read);
                break;
            }

            enable_thread = 0;
            while (active_thread)
                usleep(1000);

            if (sock_TCP_Client > -1) {
                close(sock_TCP_Client);
                sock_TCP_Client = -1;
            }
            break;

        case 0x0104feef:
        case 0x0204feef:
        case 0x0304feef:
            if (np_running()) {
                hpsdr_dbg_printf(1, "OldProtocol START command received but NewProtocol radio already running!\n");
                break;
            }
            // processing an invalid packet is too dangerous -- skip it!
            if (bytes_read != 64) {
                hpsdr_dbg_printf(1, "InvalidLength: RvcMsg Code=0x%08x Len=%d\n", code, bytes_read);
                break;
            }
            hpsdr_dbg_printf(1, "START the PC-to-SDR handler thread / code: 0x%08x\n", code);

            enable_thread = 0;
            while (active_thread)
                usleep(1000);
            memset(&addr_old, 0, sizeof(addr_old));
            addr_old.sin_family = AF_INET;
            addr_old.sin_addr.s_addr = addr_from.sin_addr.s_addr;
            addr_old.sin_port = addr_from.sin_port;

            //
            // The initial value of iqsamples.txptr defines the delay between
            // TX samples sent to the SDR and PURESIGNAL feedback
            // samples arriving
            //
            iqsamples.txptr = OLDRTXLEN / 2;
            memset(iqsamples.isample, 0, OLDRTXLEN * sizeof(double));
            memset(iqsamples.qsample, 0, OLDRTXLEN * sizeof(double));
            enable_thread = 1;
            active_thread = 1;

            if (pthread_create(&thread, NULL, op_handler_ep6, NULL) < 0) {
                hpsdr_dbg_printf(1, "ERROR: create old protocol thread");
                return EXIT_FAILURE;
            }
            pthread_detach(thread);

            if (pthread_create(&tx_hardware_thread_id, NULL, tx_hardware_thread, NULL) < 0) {
                hpsdr_dbg_printf(1, "ERROR: create tx_hardware_thread");
                return EXIT_FAILURE;
            }
            pthread_detach(thread);

            break;

        default:
            // Here we have to handle the following "non standard" cases:
            // OldProtocol "program"   packet
            // OldProtocol "erase"     packet
            // OldProtocol "Set IP"    packet
            // NewProtocol "Discovery" packet
            // NewProtocol "program"   packet
            // NewProtocol "erase"     packet
            // NewProtocol "Set IP"    packet
            // NewProtocol "General"   packet  ==> this starts NewProtocol radio

            if (bytes_read == 264 && buffer[0] == 0xEF && buffer[1] == 0xFE && buffer[2] == 0x03 && buffer[3] == 0x01) {
                static long cnt = 0;
                unsigned long blks = (buffer[4] << 24) + (buffer[5] << 16) + (buffer[6] << 8) + buffer[7];
                hpsdr_dbg_printf(1, "OldProtocol Program blks=%lu count=%ld\r", blks, ++cnt);

                op_program(buffer);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                if (blks == cnt)
                    hpsdr_dbg_printf(1, "\n\n Programming Done!\n");
                break;
            }

            if (bytes_read == 64 && buffer[0] == 0xEF && buffer[1] == 0xFE && buffer[2] == 0x03 && buffer[3] == 0x02) {
                hpsdr_dbg_printf(1, "OldProtocol Erase packet received:\n");

                op_erase_packet(buffer);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;

            }

            if (bytes_read == 63 && buffer[0] == 0xEF && buffer[1] == 0xFE && buffer[2] == 0x03) {
                hpsdr_dbg_printf(1, "OldProtocol SetIP packet received:\n");
                hpsdr_dbg_printf(1, "MAC address is %02x:%02x:%02x:%02x:%02x:%02x\n", buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
                hpsdr_dbg_printf(1, "IP  address is %03d:%03d:%03d:%03d\n", buffer[9], buffer[10], buffer[11], buffer[12]);

                op_set_ip(buffer);

                sendto(sock_udp, buffer, 63, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;
            }

            if (code == 0 && buffer[4] == 0x02) {
                if (oldnew == 1) {
                    hpsdr_dbg_printf(1, "NewProtocol discovery packet IGNORED.\n");
                    break;
                }
                hpsdr_dbg_printf(1, "NewProtocol discovery packet received\n");

                np_discovery(buffer, np_running(), NEWDEVICE);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;
            }

            if (code == 0 && buffer[4] == 0x04) {
                if (oldnew == 1) {
                    hpsdr_dbg_printf(1, "NewProtocol erase packet IGNORED.\n");
                    break;
                }
                hpsdr_dbg_printf(1, "NewProtocol erase packet received\n");

                np_erase_packet(buffer, active_thread, NEWDEVICE);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                sleep(5); // pretend erase takes 5 seconds
                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;
            }

            if (bytes_read == 265 && buffer[4] == 0x05) {
                if (oldnew == 1) {
                    hpsdr_dbg_printf(1, "NewProtocol program packet IGNORED.\n");
                    break;
                }
                unsigned long seq, blk;
                seq = (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3];
                blk = (buffer[5] << 24) + (buffer[6] << 16) + (buffer[7] << 8) + buffer[8];
                hpsdr_dbg_printf(1, "NewProtocol Program packet received: seq=%lu blk=%lu\r", seq, blk);
                if (seq == 0)
                    checksum = 0;
                for (j = 9; j <= 264; j++)
                    checksum += buffer[j];

                np_program(buffer, checksum, NEWDEVICE);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                if (seq + 1 == blk)
                    hpsdr_dbg_printf(1, "\n\nProgramming Done!\n");
                break;
            }

            if (bytes_read == 60 && code == 0 && buffer[4] == 0x06) {
                if (oldnew == 1) {
                    hpsdr_dbg_printf(1, "NewProtocol SetIP packet IGNORED.\n");
                    break;
                }
                hpsdr_dbg_printf(1, "NewProtocol SetIP packet received for MAC %2x:%2x:%2x:%2x%2x:%2x IP=%d:%d:%d:%d\n", buffer[5], buffer[6], buffer[7], buffer[8],
                        buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14]);

                // only respond if this is for OUR device
                if (buffer[5] != 0xAA)
                    break;
                if (buffer[6] != 0xBB)
                    break;
                if (buffer[7] != 0xCC)
                    break;
                if (buffer[8] != 0xDD)
                    break;
                if (buffer[9] != 0xEE)
                    break;
                if (buffer[10] != 0xFF)
                    break;

                np_set_ip(buffer, active_thread, NEWDEVICE);

                sendto(sock_udp, buffer, 60, 0, (struct sockaddr*) &addr_from, sizeof(addr_from));
                break;
            }

            if (bytes_read == 60 && buffer[4] == 0x00) {
                if (oldnew == 1) {
                    hpsdr_dbg_printf(1, "NewProtocol General packet IGNORED.\n");
                    break;
                }
                // handle "general packet" of the new protocol
                memset(&addr_new, 0, sizeof(addr_new));
                addr_new.sin_family = AF_INET;
                addr_new.sin_addr.s_addr = addr_from.sin_addr.s_addr;
                addr_new.sin_port = addr_from.sin_port;
                np_general_packet(buffer);
                break;
            } else {
                hpsdr_dbg_printf(1, "Invalid packet (len=%d) detected: ", bytes_read);
                for (i = 0; i < 16; i++)
                    hpsdr_dbg_printf(1, "%02x ", buffer[i]);
                hpsdr_dbg_printf(1, "\n");
            }

            break;
        }
    }

    close(sock_udp);

    if (sock_TCP_Client > -1) {
        close(sock_TCP_Client);
    }

    if (sock_TCP_Server > -1) {
        close(sock_TCP_Server);
    }

    return EXIT_SUCCESS;
}

void* tx_hardware_thread(void *data) {
    hpsdr_dbg_printf(1, "<Start tx_hardware_thread>\n");
    //struct samples_t *iqsamples_tx = (struct samples_t*) data;

    hpsdr_dbg_printf(1, " -- TX Frequency: %f\n", TX_Frequency);
    rpitx_iq_init(48000,  TX_Frequency);
    //rpitx_iq_init(48000, 147360); // only for test

    while (1) {
        if (!enable_thread)
            break;
        rpitx_iq_send(&iqsamples, &enable_thread);
    }

    rpitx_iq_deinit();
    hpsdr_dbg_printf(1, "<Stop tx_hardware_thread>\n");
    return NULL;
}
