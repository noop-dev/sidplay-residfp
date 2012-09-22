/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2001-2001 by Jarno Paananen
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

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "hardsid-emu.h"

// Move these to common header file
#define HSID_IOCTL_RESET     _IOW('S', 0, int)
#define HSID_IOCTL_FIFOSIZE  _IOR('S', 1, int)
#define HSID_IOCTL_FIFOFREE  _IOR('S', 2, int)
#define HSID_IOCTL_SIDTYPE   _IOR('S', 3, int)
#define HSID_IOCTL_CARDTYPE  _IOR('S', 4, int)
#define HSID_IOCTL_MUTE      _IOW('S', 5, int)
#define HSID_IOCTL_NOFILTER  _IOW('S', 6, int)
#define HSID_IOCTL_FLUSH     _IO ('S', 7)
#define HSID_IOCTL_DELAY     _IOW('S', 8, int)
#define HSID_IOCTL_READ      _IOWR('S', 9, int*)

bool       HardSID::m_sidFree[16] = {0};
const uint HardSID::voices = HARDSID_VOICES;
uint       HardSID::sid = 0;
char       HardSID::credit[];

HardSID::HardSID (sidbuilder *builder)
:sidemu(builder),
 Event("HardSID Delay"),
 m_handle(0),
 m_eventContext(0),
 m_instance(sid++),
 m_status(false),
 m_locked(false)
{
    uint num = 16;
    for ( uint i = 0; i < 16; i++ )
    {
        if(m_sidFree[i] == 0)
        {
            m_sidFree[i] = 1;
            num = i;
            break;
        }
    }

    // All sids in use?!?
    if ( num == 16 )
        return;

    m_instance = num;

    {
        char device[20];
        *m_errorBuffer = '\0';
        sprintf (device, "/dev/sid%u", m_instance);
        m_handle = open (device, O_RDWR);
        if (m_handle < 0)
        {
            if (m_instance == 0)
            {
                m_handle = open ("/dev/sid", O_RDWR);
                if (m_handle < 0)
                {
                    sprintf (m_errorBuffer, "HARDSID ERROR: Cannot access \"/dev/sid\" or \"%s\"", device);
                    return;
                }
            }
            else
            {
                sprintf (m_errorBuffer, "HARDSID ERROR: Cannot access \"%s\"", device);
                return;
            }
        }
    }

    m_status = true;
    reset ();
}

HardSID::~HardSID()
{
    sid--;
    m_sidFree[m_instance] = 0;
    if (m_handle)
        close (m_handle);
}

void HardSID::reset (uint8_t volume)
{
    for (uint i= 0; i < voices; i++)
        muted[i] = false;
    ioctl(m_handle, HSID_IOCTL_RESET, volume);
    m_accessClk = 0;
    if (m_eventContext != 0)
        m_eventContext->schedule (*this, HARDSID_DELAY_CYCLES, EVENT_CLOCK_PHI1);
}

void HardSID::clock()
{
    if (!m_handle)
        return;

    event_clock_t cycles = m_eventContext->getTime (m_accessClk, EVENT_CLOCK_PHI1);
    m_accessClk += cycles;

    while (cycles > 0xffff) {
        ioctl(m_handle, HSID_IOCTL_DELAY, 0xffff);
        cycles -= 0xffff;
    }
    if (cycles)
        ioctl(m_handle, HSID_IOCTL_DELAY, cycles);
}

uint8_t HardSID::read (uint_least8_t addr)
{
    if (!m_handle)
        return 0;

    event_clock_t cycles = m_eventContext->getTime (m_accessClk, EVENT_CLOCK_PHI1);
    m_accessClk += cycles;

    while ( cycles > 0xffff )
    {
        /* delay */
        ioctl(m_handle, HSID_IOCTL_DELAY, 0xffff);
        cycles -= 0xffff;
    }

    uint packet = (( cycles & 0xffff ) << 16 ) | (( addr & 0x1f ) << 8 );
    ioctl(m_handle, HSID_IOCTL_READ, &packet);

    cycles = 0;
    return (uint8_t) (packet & 0xff);
}

void HardSID::write (uint_least8_t addr, uint8_t data)
{
    if (!m_handle)
        return;

    event_clock_t cycles = m_eventContext->getTime (m_accessClk, EVENT_CLOCK_PHI1);
    m_accessClk += cycles;

    while ( cycles > 0xffff )
    {
        /* delay */
        ioctl(m_handle, HSID_IOCTL_DELAY, 0xffff);
        cycles -= 0xffff;
    }

    uint packet = (( cycles & 0xffff ) << 16 ) | (( addr & 0x1f ) << 8 )
        | (data & 0xff);
    cycles = 0;
    ::write (m_handle, &packet, sizeof (packet));
}

void HardSID::voice (const unsigned int num, const bool mute)
{
    // Only have 3 voices!
    if (num >= voices)
        return;
    muted[num] = mute;

    int cmute = 0;
    for ( uint i = 0; i < voices; i++ )
        cmute |= (muted[i] << i);
    ioctl (m_handle, HSID_IOCTL_MUTE, cmute);
}

void HardSID::event (void)
{
    event_clock_t cycles = m_eventContext->getTime (m_accessClk, EVENT_CLOCK_PHI1);
    if (cycles < HARDSID_DELAY_CYCLES)
    {
        m_eventContext->schedule (*this, HARDSID_DELAY_CYCLES - cycles,
                  EVENT_CLOCK_PHI1);
    }
    else
    {
        m_accessClk += cycles;
        ioctl(m_handle, HSID_IOCTL_DELAY, (uint) cycles);
        m_eventContext->schedule (*this, HARDSID_DELAY_CYCLES, EVENT_CLOCK_PHI1);
    }
}

void HardSID::filter(bool enable)
{
    ioctl (m_handle, HSID_IOCTL_NOFILTER, !enable);
}

void HardSID::flush(void)
{
    ioctl(m_handle, HSID_IOCTL_FLUSH);
}

bool HardSID::lock(EventContext* env)
{
    if( !env )
    {
        if (!m_locked)
            return false;
        m_eventContext->cancel (*this);
        m_locked = false;
        m_eventContext = 0;
    }
    else
    {
        if (m_locked)
            return false;
        m_locked = true;
        m_eventContext = env;
        m_eventContext->schedule (*this, HARDSID_DELAY_CYCLES, EVENT_CLOCK_PHI1);
    }
    return true;
}
