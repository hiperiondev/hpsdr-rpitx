#ifndef MODULATION_AMDMASYNC_H_
#define MODULATION_AMDMASYNC_H_

#include <stdint.h>
#include <stdio.h>

#include "dma.hpp"
#include "gpio.hpp"

class amdmasync: public bufferdma, public clkgpio, public pwmgpio, public pcmgpio {
protected:
    uint64_t tunefreq;
    bool syncwithpwm;
    uint32_t Originfsel;
    uint32_t SampleRate;
public:
    amdmasync(uint64_t TuneFrequency, uint32_t SR, int Channel, uint32_t FifoSize);
    ~amdmasync();
    void SetDmaAlgo();
    void SetAmSample(uint32_t Index, float Amplitude);
    void SetAmSamples(float *sample, size_t Size);
};

#endif
