/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2013 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2004 Dag Lem <resid@nimrod.no>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SINCRESAMPLER_H
#define SINCRESAMPLER_H

#include "Resampler.h"

#include <string>
#include <map>

#include "../array.h"

namespace reSIDfp
{

/**
 * This is the theoretically correct (and computationally intensive) audio sample generation.
 * The samples are generated by resampling to the specified sampling frequency.
 * The work rate is inversely proportional to the percentage of the bandwidth
 * allocated to the filter transition band.
 * <p>
 * This implementation is based on the paper "A Flexible Sampling-Rate Conversion Method",
 * by J. O. Smith and P. Gosset, or rather on the expanded tutorial on the
 * "Digital Audio Resampling Home Page": http:*www-ccrma.stanford.edu/~jos/resample/
 * <p>
 * By building shifted FIR tables with samples according to the sampling frequency,
 * this implementation dramatically reduces the computational effort in the
 * filter convolutions, without any loss of accuracy.
 * The filter convolutions are also vectorizable on current hardware.
 */
class SincResampler : public Resampler
{
private:
    static const int RINGSIZE = 2048;

private:
    matrix_t* firTable;

    int sampleIndex;

    int firRES, firN;

    const int cyclesPerSample;

    int sampleOffset;

    int outputValue;

    short sample[RINGSIZE * 2];

private:
    int fir(int subcycle);

public:
    /**
     * Use a clock freqency of 985248Hz for PAL C64, 1022730Hz for NTSC C64.
     * The default end of passband frequency is pass_freq = 0.9*sample_freq/2
     * for sample frequencies up to ~ 44.1kHz, and 20kHz for higher sample frequencies.
     * <p>
     * For resampling, the ratio between the clock frequency and the sample frequency
     * is limited as follows: 125*clock_freq/sample_freq < 16384
     * E.g. provided a clock frequency of ~ 1MHz, the sample frequency
     * can not be set lower than ~ 8kHz.
     * A lower sample frequency would make the resampling code overfill its 16k sample ring buffer.
     * <p>
     * The end of passband frequency is also limited: pass_freq <= 0.9*sample_freq/2
     * <p>
     * E.g. for a 44.1kHz sampling rate the end of passband frequency is limited
     * to slightly below 20kHz. This constraint ensures that the FIR table is not overfilled.
     *
     * @param clockFrequency System clock frequency at Hz
     * @param samplingFrequency Desired output sampling rate
     * @param highestAccurateFrequency
     */
    SincResampler(double clockFrequency, double samplingFrequency, double highestAccurateFrequency);

    bool input(int input);

    int output() const { return outputValue; }

    void reset();
};

} // namespace reSIDfp

#endif
