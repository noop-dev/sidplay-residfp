/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef SIDBUILDER_H
#define SIDBUILDER_H

#include "sid2types.h"

class sidemu;
class EventContext;

class sidbuilder
{
private:
    const char * const m_name;

protected:
    bool m_status;

public:
    sidbuilder(const char * const name)
        : m_name(name), m_status (true) {;}
    virtual ~sidbuilder() {;}

    virtual  sidemu      *lock    (EventContext *env, const sid2_model_t model) = 0;
    virtual  void         unlock  (sidemu *device) = 0;
    const    char        *name    (void) const { return m_name; }
    virtual  const  char *error   (void) const = 0;
    virtual  const  char *credits (void) = 0;
    virtual  void         filter  (bool enable) = 0;

    // Determine current state of object (true = okay, false = error).
    bool     getStatus() const { return m_status; }
};

#endif // SIDBUILDER_H
