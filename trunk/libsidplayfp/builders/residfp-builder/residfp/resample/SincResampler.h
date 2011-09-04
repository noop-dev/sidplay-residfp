/*
 * This file is part of reSID, a MOS6581 SID emulator engine.
 * Copyright (C) 2004  Dag Lem <resid@nimrod.no>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * @author Ken Händel
 *
 */

#ifndef SINCRESAMPLER_H
#define SINCRESAMPLER_H

#include "Resampler.h"

#include "../array.h"

#include <string>
#include <map>

namespace reSIDfp
{

/** @internal
 * This is the theoretically correct (and computationally intensive) audio sample generation. The samples are generated by resampling to the specified sampling frequency. The work rate is
 * inversely proportional to the percentage of the bandwidth allocated to the filter transition band.
 * <P>
 * This implementation is based on the paper "A Flexible Sampling-Rate Conversion Method", by J. O. Smith and P. Gosset, or rather on the expanded tutorial on the
 * "Digital Audio Resampling Home Page": http:*www-ccrma.stanford.edu/~jos/resample/
 * <P>
 * By building shifted FIR tables with samples according to the sampling frequency, this implementation dramatically reduces the computational effort in the filter convolutions, without any loss
 * of accuracy. The filter convolutions are also vectorizable on current hardware.
 * <P>
 * Further possible optimizations are:
 * <OL>
 * <LI>An equiripple filter design could yield a lower filter order, see http://www.mwrf.com/Articles/ArticleID/7229/7229.html
 * <LI>The Convolution Theorem could be used to bring the complexity of convolution down from O(n*n) to O(n*log(n)) using the Fast Fourier Transform, see
 * http://en.wikipedia.org/wiki/Convolution_theorem
 * <LI>Simply resampling in two steps can also yield computational savings, since the transition band will be wider in the first step and the required filter order is thus lower in this step.
 * Laurent Ganier has found the optimal intermediate sampling frequency to be (via derivation of sum of two steps):<BR>
 * <CODE>2 * pass_freq + sqrt [ 2 * pass_freq * orig_sample_freq
 *       * (dest_sample_freq - 2 * pass_freq) / dest_sample_freq ]</CODE>
 * </OL>
 *
 * @author Dag Lem
 * @author Antti Lankila
 */
class SincResampler : public Resampler {

private:
	static const int RINGSIZE = 2048;

	static const int BITS = 16;

	short sample[RINGSIZE * 2];

	array<short>* firTable;

	int sampleIndex;

	int firRES, firN;

	const int cyclesPerSample;

	int sampleOffset;

	int outputValue;

	/**
	* Cache for the expensive FIR table computation results.
	*/
	static std::map<std::string, array<short> > FIR_CACHE;

	/** Maximum error acceptable in I0 is 1e-6, or ~96 dB. */
	static const double I0E;

	/**
	 * I0() computes the 0th order modified Bessel function of the first kind.
	 * This function is originally from resample-1.5/filterkit.c by J. O. Smith.
	 * It is used to build the Kaiser window for resampling.
	 *
	 * @param x evaluate I0 at x
	 * @return value of I0 at x.
	 */
	static double I0(const double x);

	/**
	 * Calculate convolution with sample and sinc.
	 *
	 * @param a sample buffer input
	 * @param aPos offset in sample buffer
	 * @param b sinc
	 * @return convolved result
	 */
	static int convolve(const short* a, const short* b, const int bLength);

	int fir(const int subcycle);

public:
	/**
	 * Use a clock freqency of 985248Hz for PAL C64, 1022730Hz for NTSC C64. The default end of passband frequency is pass_freq = 0.9*sample_freq/2 for sample frequencies up to ~ 44.1kHz, and 20kHz
	 * for higher sample frequencies.
	 * <P>
	 * For resampling, the ratio between the clock frequency and the sample frequency is limited as follows: 125*clock_freq/sample_freq < 16384 E.g. provided a clock frequency of ~ 1MHz, the sample
	 * frequency can not be set lower than ~ 8kHz. A lower sample frequency would make the resampling code overfill its 16k sample ring buffer.
	 * <P>
	 * The end of passband frequency is also limited: pass_freq <= 0.9*sample_freq/2
	 * <P>
	 * E.g. for a 44.1kHz sampling rate the end of passband frequency is limited to slightly below 20kHz. This constraint ensures that the FIR table is not overfilled.
	 *
	 * @param clockFrequency System clock frequency at Hz
	 * @param method sampling method to use
	 * @param samplingFrequency Desired output sampling rate
	 * @return success
	 */
	SincResampler(const double clockFrequency, const double samplingFrequency, const double highestAccurateFrequency);

	bool input(const int input);

	int output() const { return outputValue; }

	void reset();
#if 0
	/**
	 * Simple sin waveform in, power output measurement function.
	 * It would be far better to use FFT.
	 *
	 * @param args
	 */
	static void main(String[] args);
#endif
};

} // namespace reSIDfp

#endif
