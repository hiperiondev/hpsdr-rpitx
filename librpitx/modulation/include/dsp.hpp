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

#ifndef CORE_DSP_H_
#define CORE_DSP_H_

#include <stdint.h>
#include <iostream>
#include <math.h>
#include <complex>

class dsp {
protected:
    double prev_phase = 0;
    double constrainAngle(double x);
    double angleConv(double angle);
    double angleDiff(double a, double b);
    double unwrap(double previousAngle, double newAngle);
    int arctan2(int y, int x);

public:
    uint32_t samplerate;
    // double phase;
    double amplitude;
    double frequency;
    dsp();
    dsp(uint32_t samplerate);
    void pushsample(std::complex<float> sample);
};

#endif
