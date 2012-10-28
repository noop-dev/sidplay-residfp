/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2001 Simon White
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

#include <cstring>
#include <stdio.h>

#include "resid/siddefs.h"
#include "resid/spline.h"
#include "resid.h"
#include "resid-emu.h"

#include <sstream>

std::string ReSID::m_credit;

const char* ReSID::getCredits()
{
    if (m_credit.empty())
    {
        // Setup credits
        std::ostringstream ss;
        ss << "ReSID V" << VERSION << " Engine:\n";
        ss << "\t(C) 1999-2002 Simon White\n";
        ss << "MOS6581 (SID) Emulation (ReSID V" << resid_version_string << "):\n";
        ss << "\t(C) 1999-2002 Dag Lem\n";
        m_credit = ss.str();
    }

    return m_credit.c_str();
}

ReSID::ReSID (sidbuilder *builder)
:sidemu(builder),
 m_context(0),
 m_sid(*(new RESID_NS::SID)),
 m_status(true),
 m_locked(false),
 m_voiceMask(0x07)
{
    m_error = "N/A";

    m_buffer = new short[OUTPUTBUFFERSIZE];
    m_bufferpos = 0;
    reset (0);
}

ReSID::~ReSID ()
{
    delete &m_sid;
    delete[] m_buffer;
}

void ReSID::bias (const double dac_bias)
{
    m_sid.adjust_filter_bias(dac_bias);
}

// Standard component options
void ReSID::reset (uint8_t volume)
{
    m_accessClk = 0;
    m_sid.reset ();
    m_sid.write (0x18, volume);
}

uint8_t ReSID::read (uint_least8_t addr)
{
    clock();
    return m_sid.read (addr);
}

void ReSID::write (uint_least8_t addr, uint8_t data)
{
    clock();
    m_sid.write (addr, data);
}

void ReSID::clock()
{
    RESID_NS::cycle_count cycles = m_context->getTime(m_accessClk, EVENT_CLOCK_PHI1);
    m_accessClk += cycles;
    m_bufferpos += m_sid.clock(cycles, (short *) m_buffer + m_bufferpos, OUTPUTBUFFERSIZE - m_bufferpos, 1);
}

void ReSID::filter (bool enable)
{
    m_sid.enable_filter (enable);
}

void ReSID::sampling (float systemclock, float freq,
        const SidConfig::sampling_method_t method, const bool fast)
{
    RESID_NS::sampling_method sampleMethod;
    switch (method)
    {
    case SidConfig::INTERPOLATE:
        sampleMethod = fast ? RESID_NS::SAMPLE_FAST : RESID_NS::SAMPLE_INTERPOLATE;
        break;
    case SidConfig::RESAMPLE_INTERPOLATE:
        sampleMethod = fast ? RESID_NS::SAMPLE_RESAMPLE_FASTMEM : RESID_NS::SAMPLE_RESAMPLE;
        break;
    default:
        m_status = false;
        m_error = "Invalid sampling method.";
        return;
    }

    if (! m_sid.set_sampling_parameters (systemclock, sampleMethod, freq)) {
        m_status = false;
        m_error = "Unable to set desired output frequency.";
    }
}

void ReSID::voice (const unsigned int num, const bool mute)
{
    if (mute)
        m_voiceMask &= ~(1<<num);
    else
        m_voiceMask |= 1<<num;

    m_sid.set_voice_mask(m_voiceMask);
}

// Set execution environment and lock sid to it
bool ReSID::lock (EventContext *env)
{
    if (m_locked)
        return false;

    m_locked  = true;
    m_context = env;

    return true; 
}

// Unlock sid
void ReSID::unlock ()
{
    m_locked  = false;
    m_context = 0;
}

// Set the emulated SID model
void ReSID::model (SidConfig::model_t model)
{
    if (model == SidConfig::MOS8580)
        m_sid.set_chip_model (RESID_NS::MOS8580);
    else
        m_sid.set_chip_model (RESID_NS::MOS6581);
/* MOS8580 + digi boost
        m_sid.set_chip_model (RESID_NS::MOS8580);
        m_sid.set_voice_mask(0x0f);
        m_sid.input(-32768);
*/
}
