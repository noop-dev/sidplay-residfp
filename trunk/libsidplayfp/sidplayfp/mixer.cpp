/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2013 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2000 Simon White
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

#include "mixer.h"

#include <algorithm> 

#include "sidplayfp/sidemu.h"

/**
* Scheduling time for next sample mixing event.
*/
const int MIXER_EVENT_RATE = OUTPUTBUFFERSIZE;

void clockChip(sidemu *s) { s->clock(); }

class bufferPos
{
public:
    bufferPos(int i) : pos(i) {}
    void operator()(sidemu *s) { s->bufferpos(pos); }

private:
    int pos;
};

void Mixer::event()
{
    /* this clocks the SIDs to the present moment, if they aren't already. */
    std::for_each(m_chips.begin(), m_chips.end(), clockChip);

    if (m_sampleBuffer && m_chips.size())
    {
        short *buf = m_sampleBuffer + m_sampleIndex;

        /* extract buffer info now that the SID is updated.
         * clock() may update bufferpos. */
        std::vector<short*> buffers;
        for(std::vector<sidemu*>::iterator it = m_chips.begin(); it != m_chips.end(); ++it)
        {
            buffers.push_back((*it)->buffer());
        }
        /* NB: if chip2 exists, its bufferpos is identical to chip1's. */
        const int sampleCount = m_chips[0]->bufferpos();

        int i = 0;
        while (i < sampleCount)
        {
            /* Handle whatever output the sid has generated so far */
            if (m_sampleIndex >= m_sampleCount)
            {
                break;
            }
            /* Are there enough samples to generate the next one? */
            if (i + m_fastForwardFactor >= sampleCount)
            {
                break;
            }

            /* This is a crude boxcar low-pass filter to
             * reduce aliasing during fast forward. */
            int sample1 = 0;
            int sample2 = 0;
            int j;
            for (j = 0; j < m_fastForwardFactor; j++)
            {
                sample1 += buffers[0][i + j];
                if (buffers.size() > 1)
                    sample2 += buffers[1][i + j];
            }
            /* increment i to mark we ate some samples, finish the boxcar thing. */
            const int dither = triangularDithering();
            i += j;
            sample1 = (sample1 * m_leftVolume + dither) / VOLUME_MAX;
            sample1 /= j;
            sample2 = (sample2 * m_rightVolume + dither) / VOLUME_MAX;
            sample2 /= j;

            /* mono mix. */
            if (m_chips.size() > 1 && !m_stereo)
                sample1 = (sample1 + sample2) / 2;

            /* stereo clone, for people who keep stereo on permanently. */
            if (m_chips.size() == 1 && m_stereo)
                sample2 = sample1;

            *buf++ = (short int)sample1;
            m_sampleIndex ++;
            if (m_stereo)
            {
                *buf++ = (short int)sample2;
                m_sampleIndex ++;
            }
        }

        /* move the unhandled data to start of buffer, if any. */
        const int samplesLeft = sampleCount - i;
        for(std::vector<short*>::iterator it = buffers.begin(); it != buffers.end(); ++it)
        {
            for (int j = 0; j < samplesLeft; j++)
            {
                (*it)[j] = (*it)[i + j];
            }
        }

        std::for_each(m_chips.begin(), m_chips.end(), bufferPos(samplesLeft - 1));
    }
    else
    {
        // FIXME this sucks
        m_sampleIndex++;
        std::for_each(m_chips.begin(), m_chips.end(), bufferPos(0));
    }

    /* Post a callback to ourselves. */
    event_context.schedule(*this, MIXER_EVENT_RATE, EVENT_CLOCK_PHI1);
}

void Mixer::reset()
{
    event_context.schedule(*this, MIXER_EVENT_RATE, EVENT_CLOCK_PHI1);
}

void Mixer::begin(short *buffer, uint_least32_t count)
{
    m_sampleIndex  = 0;
    m_sampleCount  = count;
    m_sampleBuffer = buffer;
}

void Mixer::setSids(sidemu *chip1, sidemu *chip2)
{
    m_chips.clear();
    if (chip1) m_chips.push_back(chip1);
    if (chip2) m_chips.push_back(chip2);
}

bool Mixer::setFastForward(int ff)
{
    if (ff < 1 || ff > 32)
        return false;

    m_fastForwardFactor = ff;
    return true;
}

void Mixer::setVolume(int_least32_t left, int_least32_t right)
{
    m_leftVolume = left;
    m_rightVolume = right;
}
