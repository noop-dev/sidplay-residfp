/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
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

#include "c64.h"

const double c64::CLOCK_FREQ_NTSC = 1022727.14;
const double c64::CLOCK_FREQ_PAL  = 985248.4;

c64::c64()
:c64env  (&m_scheduler),
 m_cpuFreq(CLOCK_FREQ_PAL),
 cpu     (this),
 cia1    (this),
 cia2    (this),
 vic     (this),
 mmu     (&m_scheduler, &ioBank)
{
    ioBank.setBank(0x0, &vic);
    ioBank.setBank(0x1, &vic);
    ioBank.setBank(0x2, &vic);
    ioBank.setBank(0x3, &vic);
    ioBank.setBank(0x4, &sidBank);
    ioBank.setBank(0x5, &sidBank);
    ioBank.setBank(0x6, &sidBank);
    ioBank.setBank(0x7, &sidBank);
    ioBank.setBank(0x8, &colorRAMBank);
    ioBank.setBank(0x9, &colorRAMBank);
    ioBank.setBank(0xa, &colorRAMBank);
    ioBank.setBank(0xb, &colorRAMBank);
    ioBank.setBank(0xc, &cia1);
    ioBank.setBank(0xd, &cia2);
    ioBank.setBank(0xe, &disconnectedBusBank);
    ioBank.setBank(0xf, &disconnectedBusBank);
}

void c64::reset()
{
    m_scheduler.reset();

    //cpu.reset  ();
    cia1.reset ();
    cia2.reset ();
    vic.reset  ();
    sidBank.reset();
    colorRAMBank.reset();
    mmu.reset();

    irqCount = 0;
    oldBAState = true;
}

void c64::setMainCpuSpeed(double cpuFreq)
{
    m_cpuFreq = cpuFreq;
    if (m_cpuFreq == CLOCK_FREQ_PAL)
    {
        vic.chip   (MOS6569);
    }
    else
    {
        vic.chip   (MOS6567R8);
    }

    const unsigned int rate = vic.getCyclesPerLine() * vic.getRasterLines();
    cia1.setDayOfTimeRate (rate);
    cia2.setDayOfTimeRate (rate);
}
