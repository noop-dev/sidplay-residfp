/***************************************************************************
                          psiddrv.cpp  -  PSID Driver Installtion
                             -------------------
    begin                : Fri Jul 27 2001
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


// --------------------------------------------------------
// The code here is use to support the PSID Version 2NG
// (proposal B) file format for player relocation support.
// --------------------------------------------------------
#include <string.h>

#include "sidendian.h"
#include "player.h"

#include "mos6510/opcodes.h"


#define PSIDDRV_MAX_PAGE 0xff

SIDPLAY2_NAMESPACE_START

const char Player::ERR_PSIDDRV_NO_SPACE[]  = "ERROR: No space to install psid driver in C64 ram";
const char Player::ERR_PSIDDRV_RELOC[]     = "ERROR: Failed whilst relocating psid driver";

extern "C" int reloc65(unsigned char** buf, int* fsize, int addr);

// Input: A 16-bit effective address
// Output: A default bank-select value for $01.
uint8_t Player::iomap (const uint_least16_t addr)
{
    // Force Real C64 Compatibility
    switch (m_tuneInfo.compatibility)
    {
    case SIDTUNE_COMPATIBILITY_R64:
    case SIDTUNE_COMPATIBILITY_BASIC:
        return 0;     // Special case, converted to 0x37 later
    }

    if (addr == 0)
        return 0;     // Special case, converted to 0x37 later
    if (addr < 0xa000)
        return 0x37;  // Basic-ROM, Kernal-ROM, I/O
    if (addr  < 0xd000)
        return 0x36;  // Kernal-ROM, I/O
    if (addr >= 0xe000)
        return 0x35;  // I/O only

    return 0x34;  // RAM only (special I/O in PlaySID mode)
}

int Player::psidDrvReloc (SidTuneInfo &tuneInfo, sid2_info_t &info)
{
    const int startlp = tuneInfo.loadAddr >> 8;
    const int endlp   = (tuneInfo.loadAddr + (tuneInfo.c64dataLen - 1)) >> 8;

    if (tuneInfo.compatibility == SIDTUNE_COMPATIBILITY_BASIC)
    {   // The psiddrv is only used for initialisation and to
        // autorun basic tunes as running the kernel falls
        // into a manual load/run mode
        tuneInfo.relocStartPage = 0x04;
        tuneInfo.relocPages     = 0x03;
    }

    // Check for free space in tune
    if (tuneInfo.relocStartPage == PSIDDRV_MAX_PAGE)
        tuneInfo.relocPages = 0;
    // Check if we need to find the reloc addr
    else if (tuneInfo.relocStartPage == 0)
    {   // Tune is clean so find some free ram around the
        // load image
        psidRelocAddr (tuneInfo, startlp, endlp);
    }
    else
    {   // Check reloc information mode
        //int startrp = tuneInfo.relocStartPage;
        //int endrp   = startrp + (tuneInfo.relocPages - 1);

        // New relocation implementation (exclude region)
        // to complement existing method rejected as being
        // unnecessary.  From tests in most cases this
        // method increases memory availibility.
        /*************************************************
        if ((startrp <= startlp) && (endrp >= endlp))
        {   // Is describing used space so find some free
            // ram outside this range
            psidRelocAddr (tuneInfo, startrp, endrp);
        }
        *************************************************/
    }

    if (tuneInfo.relocPages < 1)
    {
        m_errorString = ERR_PSIDDRV_NO_SPACE;
        return -1;
    }

    // Place psid driver into ram
    uint_least16_t relocAddr = tuneInfo.relocStartPage << 8;

    uint8_t psid_driver[] = {
#      include "psiddrv.bin"
    };
    uint8_t *reloc_driver = psid_driver;
    int      reloc_size   = sizeof (psid_driver);

    if (!reloc65 (&reloc_driver, &reloc_size, relocAddr - 10))
    {
        m_errorString = ERR_PSIDDRV_RELOC;
        return -1;
    }

    // Adjust size to not included initialisation data.
    reloc_size -= 10;
    info.driverAddr   = relocAddr;
    info.driverLength = (uint_least16_t) reloc_size;
    // Round length to end of page
    info.driverLength += 0xff;
    info.driverLength &= 0xff00;

    mmu.fillRom(0xfffc, &reloc_driver[0], 2); /* RESET */

    // If not a basic tune then the psiddrv must install
    // interrupt hooks and trap programs trying to restart basic
    if (tuneInfo.compatibility == SIDTUNE_COMPATIBILITY_BASIC)
    {   // Install hook to set subtune number for basic
        uint8_t prg[] = {LDAb, (uint8_t) (tuneInfo.currentSong-1),
                         STAa, 0x0c, 0x03, JSRw, 0x2c, 0xa8,
                         JMPw, 0xb1, 0xa7};
        mmu.fillRom (0xbf53, prg, sizeof (prg));
        mmu.writeRomByte(0xa7ae, JMPw);
        mmu.writeRomWord(0xa7af, 0xbf53);
    }
    else
    {   // Only install irq handle for RSID tunes
        mmu.fillRam(0x0314, &reloc_driver[2], tuneInfo.compatibility == SIDTUNE_COMPATIBILITY_R64 ? 2 : 6);

        // Experimental restart basic trap
        const uint_least16_t addr = endian_little16(&reloc_driver[8]);
        mmu.writeRomByte(0xa7ae, JMPw);
        mmu.writeRomWord(0xa7af, 0xffe1);
        mmu.writeMemWord(0x0328, addr);
    }

    int pos = relocAddr;

    // Install driver to ram
    mmu.fillRam(pos, &reloc_driver[10], reloc_size);

    // Tell C64 about song
    mmu.writeMemByte(pos, (uint8_t) (tuneInfo.currentSong-1));
    pos++;
    mmu.writeMemByte(pos, tuneInfo.songSpeed == SIDTUNE_SPEED_VBI ? 0 : 1);
    pos++;
    mmu.writeMemWord(pos, tuneInfo.compatibility == SIDTUNE_COMPATIBILITY_BASIC ?
                     0xbf55 /*Was 0xa7ae, see above*/ : tuneInfo.initAddr);
    pos += 2;
    mmu.writeMemWord(pos, tuneInfo.playAddr);
    pos += 2;
    // Initialise random number generator
    info.powerOnDelay = m_cfg.powerOnDelay;
    // Delays above MAX result in random delays
    if (info.powerOnDelay > SID2_MAX_POWER_ON_DELAY)
    {   // Limit the delay to something sensible.
        info.powerOnDelay = (uint_least16_t) (m_rand >> 3) &
                            SID2_MAX_POWER_ON_DELAY;
    }
    mmu.writeMemWord(pos, info.powerOnDelay);
    pos += 2;
    m_rand  = m_rand * 13 + 1;
    mmu.writeMemByte(pos, iomap (m_tuneInfo.initAddr));
    pos++;
    mmu.writeMemByte(pos, iomap (m_tuneInfo.playAddr));
    pos++;
    const uint8_t flag = mmu.readMemByte(0x02a6); // PAL/NTSC flag
    mmu.writeMemByte(pos, flag);
    pos++;

    // Add the required tune speed
    switch ((m_tune->getInfo()).clockSpeed)
    {
    case SIDTUNE_CLOCK_PAL:
        mmu.writeMemByte(pos, 1);
        break;
    case SIDTUNE_CLOCK_NTSC:
        mmu.writeMemByte(pos, 0);
        break;
    default: // UNKNOWN or ANY
        mmu.writeMemByte(pos, flag);
        break;
    }
    pos++;

    // Default processor register flags on calling init
    mmu.writeMemByte(pos, tuneInfo.compatibility >= SIDTUNE_COMPATIBILITY_R64 ? 0 : 1 << MOS6510::SR_INTERRUPT);

    return 0;
}


void Player::psidRelocAddr (SidTuneInfo &tuneInfo, int startp, int endp)
{   // Used memory ranges.
    bool pages[256];
    int  used[] = {0x00,   0x03,
                   0xa0,   0xbf,
                   0xd0,   0xff,
                   startp, (startp <= endp) &&
                   (endp <= 0xff) ? endp : 0xff};

    // Mark used pages in table.
    memset(pages, false, sizeof(pages));
    for (size_t i = 0; i < sizeof(used)/sizeof(*used); i += 2)
    {
        for (int page = used[i]; page <= used[i + 1]; page++)
            pages[page] = true;
    }

    {   // Find largest free range.
        int relocPages, lastPage = 0;
        tuneInfo.relocPages = 0;
        for (size_t page = 0; page < sizeof(pages)/sizeof(*pages); page++)
        {
            if (pages[page] == false)
                continue;
            relocPages = page - lastPage;
            if (relocPages > tuneInfo.relocPages)
            {
                tuneInfo.relocStartPage = lastPage;
                tuneInfo.relocPages     = relocPages;
            }
            lastPage = page + 1;
        }
    }

    if (tuneInfo.relocPages    == 0)
        tuneInfo.relocStartPage = PSIDDRV_MAX_PAGE;
}

SIDPLAY2_NAMESPACE_STOP
