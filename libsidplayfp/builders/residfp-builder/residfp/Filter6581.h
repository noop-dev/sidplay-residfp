#ifndef FILTER6581_H
#define FILTER6581_H

#include "siddefs-fp.h"

#include "Filter.h"
#include "FilterModelConfig.h"

namespace reSIDfp
{

class Integrator;

/** @internal
 * Filter based on Dag Lem's 6581 filter from reSID 1.0 prerelease. See original
 * source for discussion about theory of operation.
 *
 * Java port by Antti S. Lankila
 *
 * @author Ken Händel
 * @author Dag Lem
 * @author Antti Lankila
 * @author Leandro Nini
 */
class Filter6581 : public Filter {

private:
	/** Filter highpass state. */
	int Vhp;

	/** Filter bandpass state. */
	int Vbp;

	/** Filter lowpass state. */
	int Vlp;

	/** Filter external input. */
	int ve;

	const int voiceScaleS14, voiceDC, vo_T16;

	/** Current volume amplifier setting. */
	unsigned short* currentGain;

	/** Current filter/voice mixer setting. */
	unsigned short* currentMixer;

	/** Filter input summer setting. */
	unsigned short* currentSummer;

	/** Filter resonance value. */
	unsigned short* currentResonance;

	/** VCR + associated capacitor connected to highpass output. */
	Integrator* hpIntegrator;

	/** VCR + associated capacitor connected to lowpass output. */
	Integrator* bpIntegrator;

	const unsigned int* f0_dac;

	unsigned short** mixer;
	unsigned short** summer;
	unsigned short** gain;

public:
	Filter6581() :
		Vhp(0),
		Vbp(0),
		Vlp(0),
		ve(0),
		voiceScaleS14(FilterModelConfig::getInstance()->getVoiceScaleS14()),
		voiceDC(FilterModelConfig::getInstance()->getVoiceDC()),
		vo_T16(FilterModelConfig::getInstance()->getVO_T16()),
		currentGain(0),
		currentMixer(0),
		currentSummer(0),
		currentResonance(0),
		hpIntegrator(FilterModelConfig::getInstance()->buildIntegrator()),
		bpIntegrator(FilterModelConfig::getInstance()->buildIntegrator()),
		f0_dac(FilterModelConfig::getInstance()->getDAC(FilterModelConfig::getInstance()->getDacZero(0.5))),
		mixer(FilterModelConfig::getInstance()->getMixer()),
		summer(FilterModelConfig::getInstance()->getSummer()),
		gain(FilterModelConfig::getInstance()->getGain()) {

		input(0);
	}

	~Filter6581();

	int clock(const int voice1, const int voice2, const int voice3);

	void input(const int sample);

	/**
	 * Switch to new distortion curve.
	 */
	void updatedCenterFrequency();

	/**
	 * Resonance tuned by ear, based on a few observations:
	 *
	 * - there's a small notch even in allpass mode - size of resonance hump is
	 * about 8 dB
	 */
	void updatedResonance();

	void updatedMixing();

public:
	/**
	 * Set filter curve type based on single parameter.
	 *
	 * @param curvePosition 0 .. 1, where 0 sets center frequency high ("light") and 1 sets it low ("dark")
	 */
	void setFilterCurve(const double curvePosition);
};

} // namespace reSIDfp

#if RESID_INLINING || defined(FILTER6581_CPP)

#include "Integrator.h"

namespace reSIDfp
{

RESID_INLINE
int Filter6581::clock(const int voice1, const int voice2, const int voice3) {

	const int v1 = (voice1 * voiceScaleS14 >> 18) + voiceDC;
	const int v2 = (voice2 * voiceScaleS14 >> 18) + voiceDC;
	const int v3 = (voice3 * voiceScaleS14 >> 18) + voiceDC;

	int Vi = 0;
	int Vo = 0;

	if (filt1) {
		Vi += v1;
	} else {
		Vo += v1;
	}
	if (filt2) {
		Vi += v2;
	} else {
		Vo += v2;
	}
	// NB! Voice 3 is not silenced by voice3off if it is routed
	// through the filter.
	if (filt3) {
		Vi += v3;
	} else if (!voice3off) {
		Vo += v3;
	}
	if (filtE) {
		Vi += ve;
	} else {
		Vo += ve;
	}

	const int oldVhp = Vhp;
	Vhp = currentSummer[currentResonance[Vbp] + Vlp + Vi];
	Vlp = bpIntegrator->solve(Vbp + vo_T16) - vo_T16;
	Vbp = hpIntegrator->solve(oldVhp + vo_T16) - vo_T16;

	if (lp) {
		Vo += Vlp;
	}
	if (bp) {
		Vo += Vbp;
	}
	if (hp) {
		Vo += Vhp;
	}
	return currentGain[currentMixer[Vo]] - (1 << 15);
}

} // namespace reSIDfp

#endif

#endif