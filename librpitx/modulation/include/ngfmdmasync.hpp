#ifndef MODULATION_NGFMDMASYNC_H_
#define MODULATION_NGFMDMASYNC_H_

#include <stdint.h>

#include "dma.hpp"
#include "gpio.hpp"

class ngfmdmasync: public bufferdma, public clkgpio, public pwmgpio, public pcmgpio {
protected:
    uint64_t tunefreq;
    bool syncwithpwm;
    uint32_t SampleRate;
public:
    ngfmdmasync(uint64_t TuneFrequency, uint32_t SR, int Channel, uint32_t FifoSize, bool UsePwm = false);
    ~ngfmdmasync();
    void SetDmaAlgo();

    void SetPhase(bool inversed);
    void SetFrequencySample(uint32_t Index, float Frequency);
    void SetFrequencySamples(float *sample, size_t Size);
};

#endif
