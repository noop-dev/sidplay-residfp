/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2013 Leandro Nini <drfiemost@users.sourceforge.net>
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

// References below are from:
//     The MOS 6567/6569 video controller (VIC-II)
//     and its application in the Commodore 64
//     http://www.uni-mainz.de/~bauec002/VIC-Article.gz

#include "mos656x.h"

#include <string.h>

#include "sidplayfp/sidendian.h"

typedef struct
{
    unsigned int cyclesPerLine;
    unsigned int rasterLines;
} model_data_t;

const model_data_t modelData[] =
{
    {262, 64}, // Old NTSC
    {263, 65}, // NTSC-M
    {312, 63}, // PAL-B
    {312, 65}, // PAL-N
};

const char *MOS656X::credit =
{   // Optional information
    "MOS6567/6569/6572 (VICII) Emulation:\n"
    "\tCopyright (C) 2001 Simon White\n"
    "\tCopyright (C) 2007-2010 Antti Lankila\n"
    "\tCopyright (C) 2011-2013 Leandro Nini\n"
};


MOS656X::MOS656X (EventContext *context) :
    Event("VIC Raster"),
    event_context(*context),
    sprite_enable(regs[0x15]),
    sprite_y_expansion(regs[0x17]),
    badLineStateChangeEvent("Update AEC signal", *this, &MOS656X::badLineStateChange)
{
    chip (MOS6569);
}

void MOS656X::reset ()
{
    irqFlags     = 0;
    irqMask      = 0;
    raster_irq   = 0;
    yscroll      = 0;
    rasterY      = maxRasters - 1;
    lineCycle    = 0;
    areBadLinesEnabled = false;
    rasterClk    = 0;
    vblanking    = lp_triggered = false;
    lpx          = 0;
    lpy          = 0;
    sprite_dma   = 0;
    memset(regs, 0, sizeof (regs));
    memset(sprite_mc_base, 0, sizeof (sprite_mc_base));
    memset(sprite_mc, 0, sizeof (sprite_mc));
    event_context.cancel(*this);
    event_context.schedule(*this, 0, EVENT_CLOCK_PHI1);
}

void MOS656X::chip (model_t model)
{
    maxRasters    = modelData[model].cyclesPerLine;
    cyclesPerLine = modelData[model].rasterLines;

    switch (model)
    {
    case MOS6567R56A:
        clock = &MOS656X::clockOldNTSC;
        break;
    case MOS6567R8:
    case MOS6572:
        clock = &MOS656X::clockNTSC;
        break;
    case MOS6569:
        clock = &MOS656X::clockPAL;
        break;
    }

    reset ();
}

uint8_t MOS656X::read (uint_least8_t addr)
{
    addr &= 0x3f;

    // Sync up timers
    sync();

    switch (addr)
    {
    case 0x11:
        // Control register 1
        return (regs[addr] & 0x7f) | ((rasterY & 0x100) >> 1);
    case 0x12:
        // Raster counter
        return rasterY & 0xFF;
    case 0x13:
        return lpx;
    case 0x14:
        return lpy;
    case 0x19:
        // Interrupt Pending Register
        return irqFlags | 0x70;
    case 0x1a:
        // Interrupt Mask Register
        return irqMask | 0xf0;
    default:
        // for addresses < $20 read from register directly, when < $2f set
        // bits of high nibble to 1, for >= $2f return $ff
        if (addr < 0x20)
            return regs[addr];
        if (addr < 0x2f)
            return regs[addr] | 0xf0;
        return 0xff;
    }
}

void MOS656X::write (uint_least8_t addr, uint8_t data)
{
    addr &= 0x3f;

    regs[addr] = data;

    // Sync up timers
    sync();

    switch (addr)
    {
    case 0x11: // Control register 1
    {
        endian_16hi8(raster_irq, data >> 7);
        yscroll = data & 7;

        if (lineCycle < 11)
            break;

        /* display enabled at any cycle of line 48 enables badlines */
        if (rasterY == FIRST_DMA_LINE)
            areBadLinesEnabled |= readDEN();

        const bool oldBadLine = isBadLine;

        /* Re-evaluate badline condition */
        isBadLine = evaluateIsBadLine();

        // Start bad dma line now
        if ((isBadLine != oldBadLine) && (lineCycle < 53))
            event_context.schedule(badLineStateChangeEvent, 0, EVENT_CLOCK_PHI1);
        break;
    }

    case 0x12: // Raster counter
        endian_16lo8(raster_irq, data);
        break;

    case 0x17:
    {
        uint8_t mask = 1;
        for (unsigned int i=0; i<8; i++, mask<<=1)
        {
            if ((sprite_enable & mask) && !(sprite_y_expansion & mask))
            {
                if (lineCycle == 14)
                {
                    sprite_mc[i] = (0x2a & (sprite_mc_base[i] & sprite_mc[i])) | (0x15 & (sprite_mc_base[i] | sprite_mc[i]));
                }
                sprite_y_expansion |= mask;
            }
        }
    }
        break;

    case 0x19:
        // VIC Interrupt Flag Register
        irqFlags &= (~data & 0x0f) | 0x80;
        handleIrqState();
        break;

    case 0x1a:
        // IRQ Mask Register
        irqMask = data & 0x0f;
        handleIrqState();
        break;
    }
}

void MOS656X::handleIrqState()
{
    /* signal an IRQ unless we already signaled it */
    if ((irqFlags & irqMask & 0x0f) != 0)
    {
        if ((irqFlags & 0x80) == 0)
        {
            interrupt(true);
            irqFlags |= 0x80;
        }
    }
    else if ((irqFlags & 0x80) != 0)
    {
        interrupt(false);
        irqFlags &= 0x7f;
    }
}

void MOS656X::event ()
{
    const event_clock_t cycles = event_context.getTime(rasterClk, event_context.phase());

    event_clock_t delay;

    if (cycles)
    {
        // Update x raster
        rasterClk += cycles;
        lineCycle += cycles;
        lineCycle %= cyclesPerLine;

        delay = (this->*clock)();
    }
    else
        delay = 1;

    event_context.schedule(*this, delay - event_context.phase(), EVENT_CLOCK_PHI1);
}

event_clock_t MOS656X::clockPAL()
{
    event_clock_t delay = 1;

    switch (lineCycle)
    {
    case 0:
        checkVblank();

        // End DMA for sprite 2
        if (!(sprite_dma & 0x18))
            setBA(true);
        break;

    case 1:
        vblank();

        // Start DMA for sprite 5
        if (sprite_dma & 0x20)
            setBA(false);
        // No sprites before next compulsory cycle
        else if (!(sprite_dma & 0xf8))
           delay = 10;
        break;

    case 2:
        // End DMA for sprite 3
        if (!(sprite_dma & 0x30))
            setBA(true);
        break;

    case 3:
        // Start DMA for sprite 6
        if (sprite_dma & 0x40)
            setBA(false);
        break;

    case 4:
        // End DMA for sprite 4
         if (!(sprite_dma & 0x60))
            setBA(true);
        break;

    case 5:
        // Start DMA for sprite 7
        if (sprite_dma & 0x80)
            setBA(false);
        break;

    case 6:
        // End DMA for sprite 5
        if (!(sprite_dma & 0xc0))
        {
            setBA(true);
            delay = 5;
        } else
            delay = 2;
        break;

    case 7:
        break;

    case 8:
        // End DMA for sprite 6
        if (!(sprite_dma & 0x80))
        {
            setBA(true);
            delay = 3;
        } else
            delay = 2;
        break;

    case 9:
        break;

    case 10:
        updateMc();

        // End DMA for sprite 7
        setBA(true);
        break;

    case 11:
        // Start bad line
        if (isBadLine)
        {   // DMA starts on cycle 15
            setBA(false);
        }

        delay = 4;
        break;

    case 12:
        break;

    case 13:
        break;

    case 14:
        break;

    case 15:
        updateMcBase();

        delay = 39;
        break;

    case 54:
        checkSpriteDma();

        // Start DMA for sprite 0
        setBA (!(sprite_dma & 0x01));
        break;

    case 55:
        checkSpriteDmaExp();

        // Start DMA for sprite 0
        if (sprite_dma & 0x01)
        {
            setBA (false);
        }
        else
        {
            setBA (true);
            // No sprites before next compulsory cycle
            if (!(sprite_dma & 0x1f))
                delay = 8;
        }
        break;

    case 56:
        // Start DMA for sprite 1
        if (sprite_dma & 0x02)
            setBA(false);
        break;

    case 57:
        checkSpriteDisplay();
        break;

    case 58:
        // Start DMA for sprite 2
        if (sprite_dma & 0x04)
            setBA(false);
        break;

    case 59:
        // End DMA for sprite 0
        if (!(sprite_dma & 0x06))
            setBA(true);
        break;

    case 60:
        // Start DMA for sprite 3
        if (sprite_dma & 0x08)
            setBA(false);
        break;

    case 61:
        // End DMA for sprite 1
        if (!(sprite_dma & 0x0c))
            setBA(true);
        break;

    case 62:
        // Start DMA for sprite 4
        if (sprite_dma & 0x10)
            setBA(false);
        break;

    default:
        delay = 54 - lineCycle;
    }

    return delay;
}

event_clock_t MOS656X::clockNTSC()
{
    event_clock_t delay = 1;

    switch (lineCycle)
    {
    case 0:
        checkVblank();

        // Start DMA for sprite 5
        if (sprite_dma & 0x20)
            setBA(false);
        break;

    case 1:
        vblank();

        // End DMA for sprite 3
        if (!(sprite_dma & 0x30))
        {
            setBA(true);
            // No sprites before next compulsory cycle
            if (!(sprite_dma & 0xf8))
                delay = 10;
        }
        break;

    case 2:
        // Start DMA for sprite 6
        if (sprite_dma & 0x40)
            setBA(false);
        break;

    case 3:
        // End DMA for sprite 4
         if (!(sprite_dma & 0x60))
            setBA(true);
        break;

    case 4:
        // Start DMA for sprite 7
        if (sprite_dma & 0x80)
            setBA(false);
        break;

    case 5:
        // End DMA for sprite 5
        if (!(sprite_dma & 0xc0))
        {
            setBA(true);
            delay = 5;
        } else
            delay = 2;
        break;

    case 6:
        break;

    case 7:
        // End DMA for sprite 6
        if (!(sprite_dma & 0x80))
        {
            setBA(true);
            delay = 3;
        } else
            delay = 2;
        break;

    case 8:
        break;

    case 9:
        updateMc();

        // End DMA for sprite 7
        setBA(true);

        delay = 2;
        break;

    case 10:
        break;

    case 11:
        // Start bad line
        if (isBadLine)
        {   // DMA starts on cycle 15
            setBA(false);
        }

        delay = 4;
        break;

    case 12:
        break;

    case 13:
        break;

    case 14:
        break;

    case 15:
        updateMcBase();

        delay = 40;
        break;

    case 55:
        checkSpriteDmaExp();

        // Start DMA for sprite 0
        setBA (!(sprite_dma & 0x01));
        break;

    case 56:
        checkSpriteDma();

        // Start DMA for sprite 0
        if (sprite_dma & 0x01)
        {
            setBA (false);
        }
        else
        {
            setBA (true);
            // No sprites before next compulsory cycle
            if (!(sprite_dma & 0x1f))
                delay = 8;
        }
        break;

    case 57:
        // Start DMA for sprite 1
        if (sprite_dma & 0x02)
            setBA(false);
        break;

    case 58:
        checkSpriteDisplay();
        break;

    case 59:
        // Start DMA for sprite 2
        if (sprite_dma & 0x04)
            setBA(false);
        break;

    case 60:
        // End DMA for sprite 0
        if (!(sprite_dma & 0x06))
            setBA(true);
        break;

    case 61:
        // Start DMA for sprite 3
        if (sprite_dma & 0x08)
            setBA(false);
        break;

    case 62:
        // End DMA for sprite 1
        if (!(sprite_dma & 0x0c))
            setBA(true);
        break;

    case 63:
        // Start DMA for sprite 4
        if (sprite_dma & 0x10)
            setBA(false);
        break;

    case 64:
        // End DMA for sprite 2
        if (!(sprite_dma & 0x18))
            setBA(true);
        break;

    default:
        delay = 55 - lineCycle;
    }

    return delay;
}

event_clock_t MOS656X::clockOldNTSC()
{
    event_clock_t delay = 1;

    switch (lineCycle)
    {
    case 0:
        checkVblank();

        // End DMA for sprite 2
        if (!(sprite_dma & 0x18))
            setBA(true);
        break;

    case 1:
        vblank();

        // Start DMA for sprite 5
        if (sprite_dma & 0x20)
            setBA(false);
        // No sprites before next compulsory cycle
        else if (!(sprite_dma & 0xf8))
           delay = 10;
        break;

    case 2:
        // End DMA for sprite 3
        if (!(sprite_dma & 0x30))
            setBA(true);
        break;

    case 3:
        // Start DMA for sprite 6
        if (sprite_dma & 0x40)
            setBA(false);
        break;

    case 4:
        // End DMA for sprite 4
         if (!(sprite_dma & 0x60))
            setBA(true);
        break;

    case 5:
        // Start DMA for sprite 7
        if (sprite_dma & 0x80)
            setBA(false);
        break;

    case 6:
        // End DMA for sprite 5
        if (!(sprite_dma & 0xc0))
        {
            setBA(true);
            delay = 5;
        } else
            delay = 2;
        break;

    case 7:
        break;

    case 8:
        // End DMA for sprite 6
        if (!(sprite_dma & 0x80))
        {
            setBA(true);
            delay = 3;
        } else
            delay = 2;
        break;

    case 9:
        break;

    case 10:
        updateMc();

        // End DMA for sprite 7
        setBA(true);
        break;

    case 11:
        // Start bad line
        if (isBadLine)
        {   // DMA starts on cycle 15
            setBA(false);
        }

        delay = 4;
        break;

    case 12:
        break;

    case 13:
        break;

    case 14:
        break;

    case 15:
        updateMcBase();

        delay = 40;
        break;

    case 55:
        checkSpriteDmaExp();

        // Start DMA for sprite 0
        setBA (!(sprite_dma & 0x01));
        break;

    case 56:
        checkSpriteDma();

        // Start DMA for sprite 0
        if (sprite_dma & 0x01)
        {
            setBA (false);
        }
        else
        {
            setBA (true);
            // No sprites before next compulsory cycle
            if (!(sprite_dma & 0x1f))
                delay = 8;
        }
        break;

    case 57:
        checkSpriteDisplay();

        // Start DMA for sprite 1
        if (sprite_dma & 0x02)
            setBA(false);

            delay = 2;
        break;

    case 58:
        break;

    case 59:
        // Start DMA for sprite 2
        if (sprite_dma & 0x04)
            setBA(false);
        break;

    case 60:
        // End DMA for sprite 0
        if (!(sprite_dma & 0x06))
            setBA(true);
        break;

    case 61:
        // Start DMA for sprite 3
        if (sprite_dma & 0x08)
            setBA(false);
        break;

    case 62:
        // End DMA for sprite 1
        if (!(sprite_dma & 0x0c))
            setBA(true);
        break;

    case 63:
        // Start DMA for sprite 4
        if (sprite_dma & 0x10)
            setBA(false);
        break;

    default:
        delay = 55 - lineCycle;
    }

    return delay;
}

// Handle light pen trigger
void MOS656X::lightpen ()
{   // Synchronise simulation
    sync();

    if (!lp_triggered)
    {   // Latch current coordinates
        lpx = lineCycle << 2;
        lpy = (uint8_t)rasterY & 0xff;
        activateIRQFlag(IRQ_LIGHTPEN);
    }
}
