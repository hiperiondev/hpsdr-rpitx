#ifndef MODULATION_SERIALDMASYNC_H_
#define MODULATION_SERIALDMASYNC_H_

#include <dma.hpp>
#include <gpio.hpp>
#include "stdint.h"

class serialdmasync: public bufferdma, public clkgpio, public pwmgpio {
protected:
    uint64_t tunefreq;
    bool syncwithpwm;
public:
    serialdmasync(uint32_t SampleRate, int Channel, uint32_t FifoSize, bool dualoutput);
    ~serialdmasync();
    void SetDmaAlgo();

    void SetSample(uint32_t Index, int Sample);
};

#endif
