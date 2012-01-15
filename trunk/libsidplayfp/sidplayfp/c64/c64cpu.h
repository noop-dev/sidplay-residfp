/*
 *  Copyright (C) 2012 Leandro Nini
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef C64CPU_H
#define C64CPU_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "sidplayfp/c64env.h"
#include "sidplayfp/mos6510/mos6510.h"

class c64cpu: public MOS6510
{
private:
    c64env &m_env;

public:
    c64cpu (c64env *env)
    :MOS6510(&(env->context ())),
     m_env(*env) {}

    uint8_t cpuRead  (const uint_least16_t addr) { return m_env.cpuRead (addr); }
    void    cpuWrite (const uint_least16_t addr, const uint8_t data) { m_env.cpuWrite (addr, data); }

#ifdef PC64_TESTSUITE
    void   loadFile (const char *file) { m_env.loadFile (file); }
#endif
};

#endif // C64CPU_H

