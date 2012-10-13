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

#ifndef C64ENV_H
#define C64ENV_H

#include "sidplayfp/event.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/** @internal
* An implementation of of this class can be created to perform the C64
* specifics.  A pointer to this child class can then be passed to
* each of the components so they can interact with it.
*/
class c64env
{
private:
    EventContext &m_context;

public:
    c64env (EventContext *context)
        :m_context (*context) {}

    EventContext &context (void) const { return m_context; }

    virtual uint8_t cpuRead(const uint_least16_t addr) =0;
    virtual void cpuWrite(const uint_least16_t addr, const uint8_t data) =0;

#ifdef PC64_TESTSUITE
    virtual void   loadFile (const char *file) =0;
#endif

    virtual void interruptIRQ (const bool state) = 0;
    virtual void interruptNMI () = 0;
    virtual void interruptRST () = 0;

    virtual void setBA        (const bool state) = 0;
    virtual void lightpen     () = 0;
};

#endif // C64ENV_H