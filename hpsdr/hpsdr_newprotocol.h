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

 int np_running(void);
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
