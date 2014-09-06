/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2014 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000-2001 Simon White
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


#ifndef PLAYER_H
#define PLAYER_H

#include <stdint.h>
#include <cstdio>

#include "siddefs.h"
#include "SidConfig.h"
#include "SidTuneInfo.h"
#include "SidInfoImpl.h"
#include "mixer.h"
#include "event.h"
#include "c64/c64.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef PC64_TESTSUITE
#  include <string>
#endif

#include <vector>

class SidTune;
class SidInfo;
class sidbuilder;


SIDPLAYFP_NAMESPACE_START

class Player
#ifdef PC64_TESTSUITE
  : public testEnv
#endif
{
private:
    /**
     * Configuration error exception.
     */
    class configError
    {
    private:
        const char* m_msg;

    public:
        configError(const char* msg) : m_msg(msg) {}
        const char* message() const { return m_msg; }
    };

private:
    /// Commodore 64 emulator
    c64 m_c64;

    /// Mixer
    Mixer m_mixer;

    /// Emulator info
    SidTune *m_tune;

    /// User Configuration Settings
    SidInfoImpl m_info;

    /// User Configuration Settings
    SidConfig m_cfg;

    /// Error message
    const char *m_errorString;

    volatile bool m_isPlaying;

    /// PAL/NTSC switch value
    uint8_t videoSwitch;

private:
    /**
     * Get the C64 model for the current loaded tune.
     *
     * @param defaultModel the default model
     * @param forced true if the default model shold be forced in spite of tune model
     */
    c64::model_t c64model(SidConfig::c64_model_t defaultModel, bool forced);

    /**
     * Initialize the emulation.
     *
     * @throw configError
     */
    void initialise();

    /**
     * Release the SID builders.
     */
    void sidRelease();

    /**
     * Create the SID emulation(s).
     *
     * @throw configError
     */
    void sidCreate(sidbuilder *builder, SidConfig::sid_model_t defaultModel,
                    bool forced, const std::vector<unsigned int> &extraSidAddresses);

    /**
     * Set the SID emulation parameters.
     *
     * @param cpuFreq the CPU clock frequency
     * @param frequency the output sampling frequency
     * @param sampling the sampling method to use
     * @param fastSampling true to enable fast low quality resampling (only for reSID)
     */
    void sidParams(double cpuFreq, int frequency,
                    SidConfig::sampling_method_t sampling, bool fastSampling);

#ifdef PC64_TESTSUITE
    void load(const char *file);
#endif

public:
    Player();
    ~Player() {}

    const SidConfig &config() const { return m_cfg; }

    const SidInfo &info() const { return m_info; }

    bool config(const SidConfig &cfg);

    bool fastForward(unsigned int percent);

    bool load(SidTune *tune);

    double cpuFreq() const { return m_c64.getMainCpuSpeed(); }

    uint_least32_t play(short *buffer, uint_least32_t samples);

    bool isPlaying() const { return m_isPlaying; }

    void stop();

    uint_least32_t time() const { return (uint_least32_t)(m_c64.getEventScheduler().getTime(EVENT_CLOCK_PHI1) / cpuFreq()); }

    void debug(const bool enable, FILE *out) { m_c64.debug(enable, out); }

    void mute(unsigned int sidNum, unsigned int voice, bool enable);

    const char *error() const { return m_errorString; }

    void setRoms(const uint8_t* kernal, const uint8_t* basic, const uint8_t* character);

    EventContext *getEventScheduler() { return m_c64.getEventScheduler(); }

    uint_least16_t getCia1TimerA() const { return m_c64.getCia1TimerA(); }
};

SIDPLAYFP_NAMESPACE_STOP

#endif // PLAYER_H
