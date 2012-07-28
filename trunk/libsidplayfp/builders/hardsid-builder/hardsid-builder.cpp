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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <cstring>
#include <string.h>

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_EXCEPTIONS
#   include <new>
#endif

#include "hardsid.h"
#include "hardsid-emu.h"


#ifdef _WIN32
//**************************************************************************
// Version 1 Interface
typedef BYTE (CALLBACK* HsidDLL1_InitMapper_t) (void);

HsidDLL2 hsid2 = {0};
#endif

bool HardSIDBuilder::m_initialised = false;
#ifndef _WIN32
uint HardSIDBuilder::m_count = 0;
#endif

HardSIDBuilder::HardSIDBuilder (const char * const name)
:sidbuilder (name)
{
    strcpy (m_errorBuffer, "N/A");

    if (!m_initialised)
    {   // Setup credits
        sprintf (HardSID::credit, "HardSID V" VERSION " Engine:\n"
#ifdef _WIN32
            "\t(C) 1999-2002 Simon White\n");
#else
            "\t(C) 2001-2002 Jarno Paanenen\n");
#endif

        if (init () < 0)
            return;
        m_initialised = true;
    }
}

HardSIDBuilder::~HardSIDBuilder (void)
{   // Remove all are SID emulations
    remove ();
}

// Create a new sid emulation.  Called by libsidplay2 only
uint HardSIDBuilder::create (uint sids)
{
    HardSID *sid = 0;
    m_status     = true;

    // Check available devices
    uint count = devices (false);
    if (!m_status)
        goto HardSIDBuilder_create_error;
    if (count && (count < sids))
        sids = count;

    for (count = 0; count < sids; count++)
    {
#   ifdef HAVE_EXCEPTIONS
        sid = new(std::nothrow) HardSID(this);
#   else
        sid = new HardSID(this);
#   endif

        // Memory alloc failed?
        if (!sid)
        {
            sprintf (m_errorBuffer, "%s ERROR: Unable to create HardSID object", name ());
            goto HardSIDBuilder_create_error;
        }

        // SID init failed?
        if (!sid->getStatus())
        {
            strcpy (m_errorBuffer, sid->error ());
            goto HardSIDBuilder_create_error;
        }
        sidobjs.push_back (sid);
    }
    return count;

HardSIDBuilder_create_error:
    m_status = false;
    delete sid;
    return count;
}

uint HardSIDBuilder::devices (const bool created)
{
    m_status = true;
    if (created)
        return sidobjs.size ();

    // Available devices
    // @FIXME@ not yet supported on Linux
#ifdef _WIN32
    if (hsid2.Instance)
    {
        uint count = hsid2.Devices ();
        if (count == 0)
        {
            sprintf (m_errorBuffer, "HARDSID ERROR: No devices found (run HardSIDConfig)");
            m_status = false;
        }
        return count;
    }
    return 0;
#else
    return m_count;
#endif
}

const char *HardSIDBuilder::credits ()
{
    m_status = true;
    return HardSID::credit;
}

void HardSIDBuilder::flush(void)
{
    const int size = sidobjs.size ();
    for (int i = 0; i < size; i++)
        static_cast<HardSID*>(sidobjs[i])->flush();
}

void HardSIDBuilder::filter (const bool enable)
{
    const int size = sidobjs.size ();
    m_status = true;
    for (int i = 0; i < size; i++)
    {
        HardSID *sid = static_cast<HardSID*>(sidobjs[i]);
        sid->filter (enable);
    }
}

// Find a free SID of the required specs
sidemu *HardSIDBuilder::lock (EventContext *env, const sid2_model_t model)
{
    const int size = sidobjs.size ();
    m_status = true;

    for (int i = 0; i < size; i++)
    {
        HardSID *sid = static_cast<HardSID*>(sidobjs[i]);
        if (sid->lock (env))
        {
            sid->model (model);
            return sid;
        }
    }
    // Unable to locate free SID
    m_status = false;
    sprintf (m_errorBuffer, "%s ERROR: No available SIDs to lock", name ());
    return 0;
}

// Allow something to use this SID
void HardSIDBuilder::unlock (sidemu *device)
{
    const int size = sidobjs.size ();
    // Make sure this is our SID
    for (int i = 0; i < size; i++)
    {
        HardSID *sid = static_cast<HardSID*>(sidobjs[i]);
        if (sid == device)
        {   // Unlock it
            sid->lock (0);
            break;
        }
    }
}

// Remove all SID emulations.
void HardSIDBuilder::remove ()
{
    const int size = sidobjs.size ();
    for (int i = 0; i < size; i++)
        delete sidobjs[i];
    sidobjs.clear();
}

#ifdef _WIN32

// Load the library and initialise the hardsid
int HardSIDBuilder::init ()
{
    HINSTANCE dll;

    if (hsid2.Instance)
        return 0;

    m_status = false;
#ifdef UNICODE
    dll = LoadLibrary(L"HARDSID.DLL");
#else
    dll = LoadLibrary("HARDSID.DLL");
#endif
    if (!dll)
    {
        DWORD err = GetLastError();
        if (err == ERROR_DLL_INIT_FAILED)
            sprintf (m_errorBuffer, "HARDSID ERROR: hardsid.dll init failed!");
        else
            sprintf (m_errorBuffer, "HARDSID ERROR: hardsid.dll not found!");
        goto HardSID_init_error;
    }

    {   // Export Needed Version 1 Interface
        HsidDLL1_InitMapper_t mapper;
        mapper = (HsidDLL1_InitMapper_t) GetProcAddress(dll, "InitHardSID_Mapper");

        if (mapper)
            mapper();
        else
        {
            sprintf (m_errorBuffer, "HARDSID ERROR: hardsid.dll is corrupt!");
            goto HardSID_init_error;
        }
    }

    {   // Check for the Version 2 interface
        HsidDLL2_Version_t version;
        version = (HsidDLL2_Version_t) GetProcAddress(dll, "HardSID_Version");
        if (!version)
        {
            sprintf (m_errorBuffer, "HARDSID ERROR: hardsid.dll not V2");
            goto HardSID_init_error;
        }
        hsid2.Version = version ();
    }

    {
        WORD version = hsid2.Version;
        if ((version >> 8) != (HSID_VERSION_MIN >> 8))
        {
            sprintf (m_errorBuffer, "HARDSID ERROR: hardsid.dll not V%d", HSID_VERSION_MIN >> 8);
            goto HardSID_init_error;
        }

        if (version < HSID_VERSION_MIN)
        {
            sprintf (m_errorBuffer, "HARDSID ERROR: hardsid.dll must be V%02u.%02u or greater",
                     HSID_VERSION_MIN >> 8, HSID_VERSION_MIN & 0xff);
            goto HardSID_init_error;
        }
    }

    // Export Needed Version 2 Interface
    hsid2.Delay    = (HsidDLL2_Delay_t)   GetProcAddress(dll, "HardSID_Delay");
    hsid2.Devices  = (HsidDLL2_Devices_t) GetProcAddress(dll, "HardSID_Devices");
    hsid2.Filter   = (HsidDLL2_Filter_t)  GetProcAddress(dll, "HardSID_Filter");
    hsid2.Flush    = (HsidDLL2_Flush_t)   GetProcAddress(dll, "HardSID_Flush");
    hsid2.MuteAll  = (HsidDLL2_MuteAll_t) GetProcAddress(dll, "HardSID_MuteAll");
    hsid2.Read     = (HsidDLL2_Read_t)    GetProcAddress(dll, "HardSID_Read");
    hsid2.Sync     = (HsidDLL2_Sync_t)    GetProcAddress(dll, "HardSID_Sync");
    hsid2.Write    = (HsidDLL2_Write_t)   GetProcAddress(dll, "HardSID_Write");

    if (hsid2.Version < HSID_VERSION_204)
        hsid2.Reset  = (HsidDLL2_Reset_t)  GetProcAddress(dll, "HardSID_Reset");
    else
    {
        hsid2.Lock   = (HsidDLL2_Lock_t)   GetProcAddress(dll, "HardSID_Lock");
        hsid2.Unlock = (HsidDLL2_Unlock_t) GetProcAddress(dll, "HardSID_Unlock");
        hsid2.Reset2 = (HsidDLL2_Reset2_t) GetProcAddress(dll, "HardSID_Reset2");
    }

    if (hsid2.Version < HSID_VERSION_207)
        hsid2.Mute   = (HsidDLL2_Mute_t)   GetProcAddress(dll, "HardSID_Mute");
    else
        hsid2.Mute2  = (HsidDLL2_Mute2_t)  GetProcAddress(dll, "HardSID_Mute2");

    hsid2.Instance = dll;
    m_status       = true;
    return 0;

HardSID_init_error:
    if (dll)
        FreeLibrary (dll);
    return -1;
}

#else

#include <ctype.h>
#include <dirent.h>

// Find the number of sid devices.  We do not care about
// stuppid device numbering or drivers not loaded for the
// available nodes.
int HardSIDBuilder::init ()
{
    DIR    *dir = opendir("/dev");
    if (!dir)
        return -1;

    m_count = 0;
    dirent *entry;

    while ( (entry=readdir(dir)) )
    {
        // SID device
        if (strncmp ("sid", entry->d_name, 3))
            continue;

        // if it is truely one of ours then it will be
        // followed by numerics only
        const char *p = entry->d_name+3;
        unsigned int index = 0;
        while (*p)
        {
            if (!isdigit (*p))
                continue;
            index = index * 10 + (*p++ - '0');
        }
        index++;
        if (m_count < index)
            m_count = index;
    }
    closedir (dir);
    return 0;
}

#endif // _WIN32
