/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2013 Leandro Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright 2001-2002 by Jarno Paananen
 * Copyright 2000-2002 Simon White
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

#ifndef HARDSID_EMU_H
#define HARDSID_EMU_H

#include <string>

#include "sidplayfp/event.h"
#include "sidplayfp/sidemu.h"
#include "sidplayfp/EventScheduler.h"
#include "sidplayfp/siddefs.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef _WIN32

#include <windows.h>

#define HSID_VERSION_MIN (WORD) 0x0200
#define HSID_VERSION_204 (WORD) 0x0204
#define HSID_VERSION_207 (WORD) 0x0207

//**************************************************************************
// Version 2 Interface
typedef void (CALLBACK* HsidDLL2_Delay_t)   (BYTE deviceID, WORD cycles);
typedef BYTE (CALLBACK* HsidDLL2_Devices_t) ();
typedef void (CALLBACK* HsidDLL2_Filter_t)  (BYTE deviceID, BOOL filter);
typedef void (CALLBACK* HsidDLL2_Flush_t)   (BYTE deviceID);
typedef void (CALLBACK* HsidDLL2_Mute_t)    (BYTE deviceID, BYTE channel, BOOL mute);
typedef void (CALLBACK* HsidDLL2_MuteAll_t) (BYTE deviceID, BOOL mute);
typedef void (CALLBACK* HsidDLL2_Reset_t)   (BYTE deviceID);
typedef BYTE (CALLBACK* HsidDLL2_Read_t)    (BYTE deviceID, WORD cycles, BYTE SID_reg);
typedef void (CALLBACK* HsidDLL2_Sync_t)    (BYTE deviceID);
typedef void (CALLBACK* HsidDLL2_Write_t)   (BYTE deviceID, WORD cycles, BYTE SID_reg, BYTE data);
typedef WORD (CALLBACK* HsidDLL2_Version_t) ();

// Version 2.04 Extensions
typedef BOOL (CALLBACK* HsidDLL2_Lock_t)    (BYTE deviceID);
typedef void (CALLBACK* HsidDLL2_Unlock_t)  (BYTE deviceID);
typedef void (CALLBACK* HsidDLL2_Reset2_t)  (BYTE deviceID, BYTE volume);

// Version 2.07 Extensions
typedef void (CALLBACK* HsidDLL2_Mute2_t)   (BYTE deviceID, BYTE channel, BOOL mute, BOOL manual);

struct HsidDLL2
{
    HINSTANCE          Instance;
    HsidDLL2_Delay_t   Delay;
    HsidDLL2_Devices_t Devices;
    HsidDLL2_Filter_t  Filter;
    HsidDLL2_Flush_t   Flush;
    HsidDLL2_Lock_t    Lock;
    HsidDLL2_Unlock_t  Unlock;
    HsidDLL2_Mute_t    Mute;
    HsidDLL2_Mute2_t   Mute2;
    HsidDLL2_MuteAll_t MuteAll;
    HsidDLL2_Reset_t   Reset;
    HsidDLL2_Reset2_t  Reset2;
    HsidDLL2_Read_t    Read;
    HsidDLL2_Sync_t    Sync;
    HsidDLL2_Write_t   Write;
    WORD               Version;
};

#endif // _WIN32

#define HARDSID_VOICES 3
// Approx 60ms
#define HARDSID_DELAY_CYCLES 60000

/***************************************************************************
 * HardSID SID Specialisation
 ***************************************************************************/
class HardSID : public sidemu, private Event
{
private:
    friend class HardSIDBuilder;

    // HardSID specific data
#ifndef _WIN32
    static         bool m_sidFree[16];
    int            m_handle;
#endif

    static const unsigned int voices;
    static       unsigned int sid;

    static std::string m_credit;


    // Generic variables
    EventContext *m_eventContext;
    event_clock_t m_accessClk;
    std::string   m_errorBuffer;

    // Must stay in this order
    bool           muted[HARDSID_VOICES];
    unsigned int   m_instance;
    bool           m_status;
    bool           m_locked;

public:
    static const char* getCredits();

public:
    HardSID(sidbuilder *builder);
    ~HardSID();

    // Standard component functions
    const char *credits () const { return getCredits(); }

    void reset() { sidemu::reset (); }
    void reset(uint8_t volume);

    uint8_t read(uint_least8_t addr);
    void write(uint_least8_t addr, uint8_t data);

    void clock();
    const char *error() const { return m_errorBuffer.c_str(); }
    bool getStatus() const { return m_status; }

    // Standard SID functions
    void filter(bool enable);
    void model(SidConfig::sid_model_t model SID_UNUSED) {;}
    void voice(unsigned int num, bool mute);
    // HardSID specific
    void flush();

    // Must lock the SID before using the standard functions.
    bool lock(EventContext *env);
    void unlock();

private:
    // Fixed interval timer delay to prevent sidplay2
    // shoot to 100% CPU usage when song nolonger
    // writes to SID.
    void event();
};

#endif // HARDSID_EMU_H