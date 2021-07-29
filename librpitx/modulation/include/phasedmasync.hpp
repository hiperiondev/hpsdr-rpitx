#ifndef MODULATION_PHASEDMASYNC_H_
#define MODULATION_PHASEDMASYNC_H_

#include <stdint.h>

#include "dma.hpp"
#include "gpio.hpp"

class phasedmasync: public bufferdma, public clkgpio, public pwmgpio, public pcmgpio, public generalgpio {
protected:
    uint64_t tunefreq;
    int NumbPhase = 2;
    uint32_t SampleRate;
    uint32_t TabPhase[32]; //32 is Max Phase
public:
    phasedmasync(uint64_t TuneFrequency, uint32_t SampleRateIn, int NumberOfPhase, int Channel, uint32_t FifoSize);
    ~phasedmasync();
    void SetDmaAlgo();
    void SetPhase(uint32_t Index, int Phase);
    void SetPhaseSamples(int *sample, size_t Size);
};

#endif
