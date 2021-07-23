/*
 Copyright (C) 2018  Evariste COURJAUD F5OEO

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CORE_DMA_REGISTERS_H_
#define CORE_DMA_REGISTERS_H_

// Memory allocating defines
// https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface
#define MEM_FLAG_DISCARDABLE       (1 << 0)                              // can be resized to 0 at any time. Use for cached data
#define MEM_FLAG_NORMAL            (0 << 2)                              // normal allocating alias. Don't use from ARM
#define MEM_FLAG_DIRECT            (1 << 2)                              // 0xC alias uncached
#define MEM_FLAG_COHERENT          (2 << 2)                              // 0x8 alias. Non-allocating in L2 but coherent
#define MEM_FLAG_L1_NONALLOCATING  (MEM_FLAG_DIRECT | MEM_FLAG_COHERENT) // Allocating in L2
#define MEM_FLAG_ZERO              (1 << 4)                              // initialise buffer to all zeros
#define MEM_FLAG_NO_INIT           (1 << 5)                              // don't initialise (default is initialise to all ones
#define MEM_FLAG_HINT_PERMALOCK    (1 << 6)                              // Likely to be locked for long periods of time.

#define BCM2708_DMA_SRC_IGNOR      (1 << 11)
#define BCM2708_DMA_SRC_INC        (1 << 8)
#define BCM2708_DMA_DST_IGNOR      (1 << 7)
#define BCM2708_DMA_NO_WIDE_BURSTS (1 << 26)
#define BCM2708_DMA_WAIT_RESP      (1 << 3)

#define BCM2708_DMA_D_DREQ         (1 << 6)
#define BCM2708_DMA_PER_MAP(x)     ((x) << 16)
#define BCM2708_DMA_END            (1 << 1)
#define BCM2708_DMA_RESET          (1 << 31)
#define BCM2708_DMA_INT            (1 << 2)

#define DMA_CS                     (0x00 / 4)
#define DMA_CONBLK_AD              (0x04 / 4)
#define DMA_DEBUG                  (0x20 / 4)

//Page 61
#define DREQ_PCM_TX                2
#define DREQ_PCM_RX                3
#define DREQ_SMI                   4
#define DREQ_PWM                   5
#define DREQ_SPI_TX                6
#define DREQ_SPI_RX                7
#define DREQ_SPI_SLAVE_TX          8
#define DREQ_SPI_SLAVE_RX          9

#endif
