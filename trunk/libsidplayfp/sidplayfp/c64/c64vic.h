/***************************************************************************
                          c64vic.h  -  C64 VIC
                             -------------------
    begin                : Fri Apr 4 2001
    copyright            : (C) 2001 by Simon White
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

#ifndef _c64vic_h_
#define _c64vic_h_

// The VIC emulation is very generic and here we need to effectively
// wire it into the computer (like adding a chip to a PCB).

#include "Bank.h"
#include "sidplayfp/c64/c64env.h"
#include "sidplayfp/sidendian.h"
#include "VIC_II/mos656x.h"

class c64vic: public MOS656X, public Bank
{
private:
    c64env &m_env;

protected:
    void write(const uint_least16_t address, const uint8_t value)
    {
        MOS656X::write(endian_16lo8(address), value);
    }

    uint8_t read(const uint_least16_t address)
    {
        return MOS656X::read(endian_16lo8(address));
    }

    void interrupt (const bool state)
    {
        m_env.interruptIRQ (state);
    }

    void setBA (const bool state)
    {
        m_env.setBA (state);
    }

public:
    c64vic (c64env *env)
    :MOS656X(&(env->context ())),
     m_env(*env) {}
    const char *error (void) const {return "";}
};

#endif // _c64vic_h_
