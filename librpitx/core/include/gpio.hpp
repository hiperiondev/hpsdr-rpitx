#ifndef CORE_GPIO_H_
#define CORE_GPIO_H_

#include <gpio_registers.hpp>
#include "stdint.h"
#include <cstdio>

#define PHYSICAL_BUS 0x7E000000

class gpio {
public:
    bool pi_is_2711 = false;
    uint64_t XOSC_FREQUENCY = 19200000;
public:
    volatile uint32_t *gpioreg = NULL;
    uint32_t gpiolen;
    gpio(uint32_t base, uint32_t len);
    ~gpio();
    uint32_t GetPeripheralBase();
};

class dmagpio: public gpio {

public:
    dmagpio();
};

enum {
    fsel_input,
    fsel_output,
    fsel_alt5,
    fsel_alt4,
    fsel_alt0,
    fsel_alt1,
    fsel_alt2,
    fsel_alt3
};

class generalgpio: public gpio {

public:
    generalgpio();
    int setmode(uint32_t gpio, uint32_t mode);
    ~generalgpio();
    int setpulloff(uint32_t gpio);
};

//Parent PLL
enum {
    clk_gnd,
    clk_osc,
    clk_debug0,
    clk_debug1,
    clk_plla,
    clk_pllc,
    clk_plld,
    clk_hdmi
};

class clkgpio: public gpio {
protected:
    int pllnumber;
    int Mash;
    uint64_t Pllfrequency;
    bool ModulateFromMasterPLL = false;
    uint64_t CentralFrequency = 0;
    generalgpio gengpio;
    double clk_ppm = 0;

public:
    int PllFixDivider = 8; //Fix divider from the master clock in advanced mode

    clkgpio();
    ~clkgpio();
    int SetPllNumber(int PllNo, int MashType);
    uint64_t GetPllFrequency(int PllNo);
    void print_clock_tree(void);
    int SetFrequency(double Frequency);
    int SetClkDivFrac(uint32_t Div, uint32_t Frac);
    void SetPhase(bool inversed);
    void SetAdvancedPllMode(bool Advanced);
    int SetCenterFrequency(uint64_t Frequency, int Bandwidth);
    double GetFrequencyResolution();
    double GetRealFrequency(double Frequency);
    int ComputeBestLO(uint64_t Frequency, int Bandwidth);
    int SetMasterMultFrac(uint32_t Mult, uint32_t Frac);
    uint32_t GetMasterFrac(double Frequency);
    void enableclk(int gpio);
    void disableclk(int gpio);
    void Setppm(double ppm);
    void SetppmFromNTP();
    void SetPLLMasterLoop(int Ki, int Kp, int Ka);
};

enum pwmmode {
    pwm1pin,
    pwm2pin,
    pwm1pinrepeat
};

class pwmgpio: public gpio {
protected:
    clkgpio clk;
    int pllnumber;
    int Mash;
    int Prediv; // Range of PWM
    uint64_t Pllfrequency;
    bool ModulateFromMasterPLL = false;
    int ModePwm = pwm1pin;
    generalgpio gengpio;

public:
    pwmgpio();
    ~pwmgpio();
    int SetPllNumber(int PllNo, int MashType);
    uint64_t GetPllFrequency(int PllNo);
    int SetFrequency(uint64_t Frequency);
    int SetPrediv(int predivisor);
    void SetMode(int Mode);
    void enablepwm(int gpio, int PwmNumber);
    void disablepwm(int gpio);
};

class pcmgpio: public gpio {
protected:
    clkgpio clk;
    int pllnumber;
    int Mash;
    int Prediv; // Range of PCM
    uint64_t Pllfrequency;
    int SetPrediv(int predivisor);

public:
    pcmgpio();
    ~pcmgpio();
    int SetPllNumber(int PllNo, int MashType);
    uint64_t GetPllFrequency(int PllNo);
    int SetFrequency(uint64_t Frequency);
    int ComputePrediv(uint64_t Frequency);
};

class padgpio: public gpio {

public:
    padgpio();
    ~padgpio();
    int setlevel(int level);
};

enum dma_common_reg {
    dma_pllc_frac = 0x7E000000 + (PLLC_FRAC << 2)   + CLK_BASE,
    dma_pwm       = 0x7E000000 + (PWM_FIFO << 2)    + PWM_BASE,
    dma_pcm       = 0x7E000000 + (PCM_FIFO_A << 2)  + PCM_BASE,
    dma_fsel      = 0x7E000000 + (GPFSEL0 << 2)     + GENERAL_BASE,
    dma_pad       = 0x7E000000 + (PADS_GPIO_0 << 2) + PADS_GPIO
};

#endif
