#ifndef MODULATION_FSKBURST_H_
#define MODULATION_FSKBURST_H_

#include <stdint.h>

#include "dma.hpp"
#include "gpio.hpp"

class fskburst: public bufferdma, public clkgpio, public pwmgpio, public pcmgpio {
protected:
    float freqdeviation;
    uint32_t Originfsel;
    bool syncwithpwm;
    dma_cb_t *lastcbp;
    size_t SR_upsample = 0;
    size_t Ramp = 0;
public:
    fskburst(uint64_t TuneFrequency, float SymbolRate, float Deviation, int Channel, uint32_t FifoSize, size_t upsample = 1, float RatioRamp = 0);
    ~fskburst();
    void SetDmaAlgo();
    void SetSymbols(unsigned char *Symbols, uint32_t Size);
};

#endif
