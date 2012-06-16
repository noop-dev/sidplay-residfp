/***************************************************************************
                          mixer.cpp  -  Sids Mixer Routines
                             -------------------
    begin                : Sun Jul 9 2000
    copyright            : (C) 2000 by Simon White
    email                : s_a_white@email.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "mixer.h"

#include "sidplayfp/sidemu.h"

void Mixer::event()
{
    /* this clocks the SID to the present moment, if it isn't already. */
    m_chip1->clock();
    if (m_chip2)
        m_chip2->clock();

    short *buf = m_sampleBuffer + m_sampleIndex;
    if (buf)
    {
    /* extract buffer info now that the SID is updated.
     * clock() may update bufferpos. */
    short *buf1 = m_chip1->buffer();
    short *buf2 = m_chip2 ? m_chip2->buffer() : 0;
    int samples = m_chip1->bufferpos();
    /* NB: if chip2 exists, its bufferpos is identical to chip1's. */

    int i = 0;
    while (i < samples) {
        /* Handle whatever output the sid has generated so far */
        if (m_sampleIndex >= m_sampleCount) {
            break;
        }
        /* Are there enough samples to generate the next one? */
        if (i + m_fastForwardFactor >= samples)
            break;

        /* This is a crude boxcar low-pass filter to
         * reduce aliasing during fast forward, something I commonly do. */
        int sample1 = 0;
        int sample2 = 0;
        int j;
        for (j = 0; j < m_fastForwardFactor; j++) {
            sample1 += buf1[i + j];
            if (buf2)
                sample2 += buf2[i + j];
        }
        /* increment i to mark we ate some samples, finish the boxcar thing. */
        const int dither = triangularDithering();
        i += j;
        sample1 = (sample1 * m_leftVolume + dither) / VOLUME_MAX;
        sample1 /= j;
        sample2 = (sample2 * m_rightVolume + dither) / VOLUME_MAX;
        sample2 /= j;

        /* mono mix. */
        if (buf2 && !m_stereo)
            sample1 = (sample1 + sample2) / 2;

        /* stereo clone, for people who keep stereo on permanently. */
        if (!buf2 && m_stereo)
            sample2 = sample1;

        *buf++ = (short int)sample1;
        m_sampleIndex ++;
        if (m_stereo) {
            *buf++ = (short int)sample2;
            m_sampleIndex ++;
        }
    }

    /* move the unhandled data to start of buffer, if any. */
    int j = 0;
    for (j = 0; j < samples - i; j++) {
        buf1[j] = buf1[i + j];
        if (buf2)
            buf2[j] = buf2[i + j];
    }
    m_chip1->bufferpos(j);
    if (m_chip2)
        m_chip2->bufferpos(j);
    }
    else
        m_sampleIndex++;

    /* Post a callback to ourselves. */
    event_context.schedule(*this, MIXER_EVENT_RATE, EVENT_CLOCK_PHI1);
}

void Mixer::begin(short *buffer, const uint_least32_t count)
{
    m_sampleIndex  = 0;
    m_sampleCount  = count;
    m_sampleBuffer = buffer;
}

void Mixer::setSids(sidemu *chip1, sidemu *chip2)
{
    m_chip1 = chip1;
    m_chip2 = chip2;
}

bool Mixer::setFastForward(const int ff)
{
    if (ff < 1 || ff > 32)
        return false;

    m_fastForwardFactor = ff;
    return true;
}

void Mixer::setVolume(const int_least32_t left, const int_least32_t right)
{
    m_leftVolume = left;
    m_rightVolume = right;
}
