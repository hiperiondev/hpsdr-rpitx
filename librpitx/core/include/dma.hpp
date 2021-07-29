#ifndef CORE_DMA_H_
#define CORE_DMA_H_

#include <dma_registers.hpp>
#include <gpio.hpp>
#include <util.hpp>
#include "stdint.h"

class dma {
protected:
    struct {
        int handle;         // From mbox_open()
        unsigned mem_ref;   // From mem_alloc()
        unsigned bus_addr;  // From mem_lock()
        uint8_t *virt_addr; // From mapmem()
    } mbox;

    typedef struct {
        uint32_t info, src, dst, length, stride, next, pad[2];
    } dma_cb_t; // 8*4=32 bytes

    typedef struct {
        uint8_t *virtaddr;
        uint32_t physaddr;
    } page_map_t;

    page_map_t *page_map;

    uint8_t *virtbase;
    int NumPages = 0;
    int channel; // DMA Channel
    dmagpio dma_reg;

    uint32_t mem_flag; // Cache or not depending on Rpi1 or 2/3
    uint32_t dram_phys_base;

public:
    dma_cb_t *cbarray;
    uint32_t cbsize;
    uint32_t *usermem;
    uint32_t usermemsize;
    bool Started = false;

    dma(int Channel, uint32_t CBSize, uint32_t UserMemSize);
    ~dma();
    uint32_t mem_virt_to_phys(volatile void *virt);
    uint32_t mem_phys_to_virt(volatile uint32_t phys);
    void GetRpiInfo();
    int start();
    int stop();
    int getcbposition();
    bool isrunning();
    bool isunderflow();
    bool SetCB(dma_cb_t *cbp, uint32_t dma_flag, uint32_t src, uint32_t dst, uint32_t repeat);
    bool SetEasyCB(dma_cb_t *cbp, uint32_t index, dma_common_reg dst, uint32_t repeat);
};

class bufferdma: public dma {
protected:
    uint32_t current_sample;
    uint32_t last_sample;
    uint32_t sample_available;

public:
    uint32_t buffersize;
    uint32_t cbbysample;
    uint32_t registerbysample;
    uint32_t *sampletab;

public:
    bufferdma(int Channel, uint32_t tbuffersize, uint32_t tcbbysample, uint32_t tregisterbysample);
    void SetDmaAlgo();
    int GetBufferAvailable();
    int GetUserMemIndex();
    int PushSample(int Index);
};

#endif
