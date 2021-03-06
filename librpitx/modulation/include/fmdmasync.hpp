#ifndef MODULATION_FMDMASYNC_H_
#define MODULATION_FMDMASYNC_H_

#include <stdint.h>

#include "dma.hpp"

class fmdmasync: public dma {
public:
    fmdmasync(int Channel, uint32_t FifoSize);
    ~fmdmasync();
    void SetDmaAlgo();
    void FillMemory(uint32_t FreqDivider, uint32_t FreqFractionnal);
};

#endif
