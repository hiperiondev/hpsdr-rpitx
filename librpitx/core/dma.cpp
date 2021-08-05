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

#include <stdio.h>
#include <unistd.h>

#include "dma.hpp"
#include "rpi.hpp"
#include "util.hpp"
#include "mailbox.hpp"
#include "raspberry_pi_revision.hpp"

#define BUS_TO_PHYS(x) ((x)&~0xC0000000)

dma::dma(int Channel, uint32_t CBSize, uint32_t UserMemSize) { // Fixme! Need to check to be 256 Aligned for UserMem
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    //Channel DMA is now hardcoded according to Raspi Model (DMA 7 for Pi4, DMA 14 for others)
    uint32_t BCM2708_PERI_BASE = bcm_host_get_peripheral_address();

    channel = Channel;
    if (BCM2708_PERI_BASE == 0xFE000000) {
        channel = 7; // Pi4
        librpitx_dbg_printf(1, "dma PI4 using channel %d\n", channel);
    } else {
        channel = 14; // Other Pi
        librpitx_dbg_printf(1, "dma (NOT a PI4)  using channel %d\n", channel);
    }

    librpitx_dbg_printf(1, "channel %d CBSize %u UsermemSize %u\n", channel, CBSize, UserMemSize);

    mbox.handle = mbox_open();
    if (mbox.handle < 0) {
        librpitx_dbg_printf(1, "Failed to open mailbox\n");

    }
    cbsize = CBSize;
    usermemsize = UserMemSize;

    GetRpiInfo(); // Fill mem_flag and dram_phys_base

    uint32_t MemoryRequired = CBSize * sizeof(dma_cb_t) + UserMemSize * sizeof(uint32_t);
    int NumPages = (MemoryRequired / PAGE_SIZE) + 1;
    librpitx_dbg_printf(1, "%d Size NUM PAGES %d PAGE_SIZE %d\n", MemoryRequired, NumPages, PAGE_SIZE);
    mbox.mem_ref = mem_alloc(mbox.handle, NumPages * PAGE_SIZE, PAGE_SIZE, mem_flag);
    // TODO: How do we know that succeeded?
    //dbg_printf(1,"mem_ref %x\n", mbox.mem_ref);
    mbox.bus_addr = mem_lock(mbox.handle, mbox.mem_ref);
    //dbg_printf(1,"bus_addr = %x\n", mbox.bus_addr);
    mbox.virt_addr = (uint8_t*) mapmem(BUS_TO_PHYS(mbox.bus_addr), NumPages * PAGE_SIZE);
    //dbg_printf(1,"virt_addr %p\n", mbox.virt_addr);
    virtbase = (uint8_t*) ((uint32_t*) mbox.virt_addr);
    //dbg_printf(1,"virtbase %p\n", virtbase);
    cbarray = (dma_cb_t*) virtbase; // We place DMA Control Blocks (CB) at beginning of virtual memory
    //dbg_printf(1,"cbarray %p\n", cbarray);
    usermem = (unsigned int*) (virtbase + CBSize * sizeof(dma_cb_t)); // user memory is placed after
    //dbg_printf(1,"usermem %p\n", usermem);

    dma_reg.gpioreg[DMA_CS + channel * 0x40] = BCM2708_DMA_RESET | DMA_CS_INT; // Remove int flag
    usleep(100);
    dma_reg.gpioreg[DMA_CONBLK_AD + channel * 0x40] = mem_virt_to_phys((void*) cbarray); // reset to beginning

    //get_clocks(mbox.handle);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
}

void dma::GetRpiInfo() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    dram_phys_base = bcm_host_get_sdram_address();

    mem_flag = MEM_FLAG_HINT_PERMALOCK | MEM_FLAG_NO_INIT; //0x0c;
    switch (dram_phys_base) {
    case 0x40000000:
        mem_flag |= MEM_FLAG_L1_NONALLOCATING;
        break;
    case 0xC0000000:
        mem_flag |= MEM_FLAG_DIRECT;
        break;
    default:
        librpitx_dbg_printf(0, "Unknown Raspberry architecture\n");
    }
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
}

dma::~dma() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    stop();
    mem_unlock(mbox.handle, mbox.mem_ref);
    mem_free(mbox.handle, mbox.mem_ref);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
}

uint32_t dma::mem_virt_to_phys(volatile void *virt) {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    //MBOX METHOD
    uint32_t offset = (uint8_t*) virt - mbox.virt_addr;
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return mbox.bus_addr + offset;
}

uint32_t dma::mem_phys_to_virt(volatile uint32_t phys) {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    //MBOX METHOD
    uint32_t offset = phys - mbox.bus_addr;
    uint32_t result = (size_t) ((uint8_t*) mbox.virt_addr + offset);
    //printf("MemtoVirt:Offset=%lx phys=%lx -> %lx\n",offset,phys,result);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return result;
}

int dma::start() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    dma_reg.gpioreg[DMA_CS + channel * 0x40] = BCM2708_DMA_RESET;
    usleep(100);
    dma_reg.gpioreg[DMA_CONBLK_AD + channel * 0x40] = mem_virt_to_phys((void*) cbarray); // reset to beginning
    dma_reg.gpioreg[DMA_DEBUG + channel * 0x40] = 7; // clear debug error flags
    usleep(100);
    dma_reg.gpioreg[DMA_CS + channel * 0x40] = DMA_CS_PRIORITY(7) | DMA_CS_PANIC_PRIORITY(7) | DMA_CS_DISDEBUG | DMA_CS_ACTIVE;
    Started = true;
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return 0;
}

int dma::stop() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    dma_reg.gpioreg[DMA_CS + channel * 0x40] = BCM2708_DMA_RESET;
    usleep(1000);
    dma_reg.gpioreg[DMA_CS + channel * 0x40] =
    BCM2708_DMA_INT | BCM2708_DMA_END;
    usleep(100);
    dma_reg.gpioreg[DMA_CONBLK_AD + channel * 0x40] = mem_virt_to_phys((void*) cbarray);
    usleep(100);
    dma_reg.gpioreg[DMA_DEBUG + channel * 0x40] = 7; // clear debug error flags
    usleep(100);
    Started = false;
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return 0;
}

int dma::getcbposition() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    volatile uint32_t dmacb = (uint32_t) (dma_reg.gpioreg[DMA_CONBLK_AD + channel * 0x40]);
    //dbg_printf(1,"cb=%x\n",dmacb);
    if (dmacb > 0) {
        librpitx_dbg_printf(2, "< func: %s -a|\n", __func__);
        return mem_phys_to_virt(dmacb) - (size_t) virtbase;
    } else {
        librpitx_dbg_printf(2, "< func: %s -b|\n", __func__);
        return -1;
    }
    // dma_reg.gpioreg[DMA_CONBLK_AD+channel*0x40]-mem_virt_to_phys((void *)cbarray );
}

bool dma::isrunning() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    return ((dma_reg.gpioreg[DMA_CS + channel * 0x40] & DMA_CS_ACTIVE) > 0);
}

bool dma::isunderflow() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    //if((dma_reg.gpioreg[DMA_CS+channel*0x40]&DMA_CS_INT)>0)  dbg_printf(1,"Status:%x\n",dma_reg.gpioreg[DMA_CS+channel*0x40]);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return ((dma_reg.gpioreg[DMA_CS + channel * 0x40] & DMA_CS_INT) > 0);
}

bool dma::SetCB(dma_cb_t *cbp, uint32_t dma_flag, uint32_t src, uint32_t dst, uint32_t repeat) {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    cbp->info = dma_flag;
    cbp->src = src;
    cbp->dst = dst;
    cbp->length = 4 * repeat;
    cbp->stride = 0;
    cbp->next = mem_virt_to_phys(cbp + 1);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return true;
}

bool dma::SetEasyCB(dma_cb_t *cbp, uint32_t index, dma_common_reg dst, uint32_t repeat) {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    uint32_t flag = BCM2708_DMA_NO_WIDE_BURSTS | BCM2708_DMA_WAIT_RESP;
    uint32_t src = mem_virt_to_phys(&usermem[index]);
    switch (dst) {
    case dma_pllc_frac:
        break;
    case dma_fsel:
        break;
    case dma_pad:
        break;
    case dma_pwm:
        flag |= BCM2708_DMA_D_DREQ | BCM2708_DMA_PER_MAP(DREQ_PWM);
        break;
    case dma_pcm:
        flag |= BCM2708_DMA_D_DREQ | BCM2708_DMA_PER_MAP(DREQ_PCM_TX);
        break;
    }
    SetCB(cbp, flag, src, dst, repeat);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return true;
}

///*************************************** BUFFER DMA ********************************************************
bufferdma::bufferdma(int Channel, uint32_t tbuffersize, uint32_t tcbbysample, uint32_t tregisterbysample) :
        dma(Channel, tbuffersize * tcbbysample, tbuffersize * tregisterbysample) {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    buffersize = tbuffersize;
    cbbysample = tcbbysample;
    registerbysample = tregisterbysample;
    librpitx_dbg_printf(1, "BufferSize %d , cb %d user %d\n", buffersize, buffersize * cbbysample, buffersize * registerbysample);

    current_sample = 0;
    last_sample = 0;
    sample_available = buffersize;

    sampletab = usermem;
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
}

void bufferdma::SetDmaAlgo() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
}

int bufferdma::GetBufferAvailable() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    int diffsample = 0;
    if (Started) {
        int CurrenCbPos = getcbposition();
        if (CurrenCbPos != -1) {
            current_sample = CurrenCbPos / (sizeof(dma_cb_t) * cbbysample);
        } else {
            librpitx_dbg_printf(1, "DMA WEIRD STATE\n");
            current_sample = 0;
        }
        //dbg_printf(1,"CurrentCB=%d\n",current_sample);
        diffsample = current_sample - last_sample;
        if (diffsample < 0)
            diffsample += buffersize;

        //dbg_printf(1,"cur %d last %d diff%d\n",current_sample,last_sample,diffsample);
    } else {
        //last_sample=buffersize-1;
        diffsample = buffersize;
        current_sample = 0;
        //dbg_printf(1,"Warning DMA stopped \n");
        //dbg_printf(1,"S:cur %d last %d diff%d\n",current_sample,last_sample,diffsample);
    }

    //
    if (isunderflow()) {
        librpitx_dbg_printf(1, "cur %d last %d \n", current_sample, last_sample);
        librpitx_dbg_printf(1, "Underflow\n");
    }
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return diffsample;

}

int bufferdma::GetUserMemIndex() {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);

    int IndexAvailable = -1;
    //dbg_printf(1,"Avail=%d\n",GetBufferAvailable());
    if (GetBufferAvailable() > 0) {
        IndexAvailable = last_sample + 1;
        if (IndexAvailable >= (int) buffersize)
            IndexAvailable = 0;
    }
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return IndexAvailable;
}

int bufferdma::PushSample(int Index) {
    librpitx_dbg_printf(2, "> func: %s (file %s | line %d)\n", __func__, __FILE__, __LINE__);
    if (Index < 0)
        return -1; // No buffer available

    //
    dma_cb_t *cbp;
    cbp = &cbarray[last_sample * cbbysample + cbbysample - 1];
    cbp->info = cbp->info & (~BCM2708_DMA_SET_INT);

    last_sample = Index;
    //
    cbp = &cbarray[Index * cbbysample + cbbysample - 1];
    cbp->info = cbp->info | (BCM2708_DMA_SET_INT);

    if (Started == false) {
        if (last_sample > buffersize / 4)
            start(); // 1/4 Fill buffer before starting DMA
    }
    librpitx_dbg_printf(2, "< func: %s |\n", __func__);
    return 0;
}
