/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <memory>

#include "sidplayfp/sidplayfp.h"
#include "sidplayfp/SidTune.h"
#include "sidplayfp/sidbuilder.h"
#include "sidplayfp/sidemu.h"

/*
* Adjust these paths to point to existing ROM dumps
*/
#define KERNAL_PATH "/usr/local/lib/vice/C64/kernal"
#define BASIC_PATH "/usr/local/lib/vice/C64/basic"
#define CHARGEN_PATH "/usr/local/lib/vice/C64/chargen"

class NullSID: public sidemu
{
public:
    NullSID () : sidemu(0) { m_buffer = 0; }

    void    reset (uint8_t) {}
    uint8_t read  (uint_least8_t) { return 0; }
    void    write (uint_least8_t, uint8_t) {}
    void    clock() {}
    const   char *credits (void) const { return ""; }
    void    voice (const unsigned int num, const bool mute) {}
    void    model    (SidConfig::model_t model)  {}
    const   char *error   (void) const { return ""; }
    bool    lock     (EventContext *env) { return true; }
    void    unlock   () {}
};

class FakeBuilder: public sidbuilder
{
private:
    NullSID sidobj;

public:
    FakeBuilder  (const char * const name)
      : sidbuilder(name) { sidobjs.push_back (new NullSID()); }
    ~FakeBuilder (void) {}

    const char *credits (void) const { return ""; }
    void        filter (const bool enable) {}
};

void loadRom(const char* path, char* buffer)
{
    std::ifstream is(path, std::ios::binary);
    is.read(buffer, 8192);
    is.close();
}

int main(int argc, char* argv[])
{
    sidplayfp m_engine;

    char kernal[8192];
    char basic[8192];
    char chargen[4096];

    loadRom(KERNAL_PATH, kernal);
    loadRom(BASIC_PATH, basic);
    loadRom(CHARGEN_PATH, chargen);

    m_engine.setRoms((const uint8_t*)kernal, (const uint8_t*)basic, (const uint8_t*)chargen);

    std::auto_ptr<FakeBuilder> rs(new FakeBuilder("Test"));

    char name[0x100] = PC64_TESTSUITE;

    if (argc > 1)
    {
        strcat (name, argv[1]);
        strcat (name, ".prg");
    }
    else
    {
        strcat (name, " start.prg");
    }

    std::auto_ptr<SidTune> tune(new SidTune(name));

    if (!tune->getStatus())
    {
        printf("Error: %s\n", tune->statusString());
        return -1;
    }

    SidConfig cfg;
    cfg.clockForced = false;
    cfg.frequency = 48000;
    cfg.samplingMethod = SidConfig::INTERPOLATE;
    cfg.fastSampling = false;
    cfg.playback = SidConfig::STEREO;
    cfg.sidEmulation = rs.get();
    cfg.sidDefault = SidConfig::MOS6581;
    m_engine.config(cfg);

    tune->selectSong(0);

    if (!m_engine.load(tune.get()))
    {
        printf("%s\n", m_engine.error());
        return -1;
    }

    for (;;)
    {
        m_engine.play(0, 48000);
    }
}
