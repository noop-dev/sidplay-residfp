/*
 * This file is part of sidplayfp, a console SID player.
 *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef AUDIO_OSS_H
#define AUDIO_OSS_H

#include "config.h"

#ifdef HAVE_OSS

#ifndef AudioDriver
#  define AudioDriver Audio_OSS
#endif

#include <sys/ioctl.h>

#if defined(HAVE_SYS_SOUNDCARD_H)
#  include <sys/soundcard.h>
#elif defined(HAVE_LINUX_SOUNDCARD_H)
#  include <linux/soundcard.h>
#elif defined(HAVE_MACHINE_SOUNDCARD_H)
#  include <machine/soundcard.h>
#elif defined(HAVE_SOUNDCARD_H)
#  include <soundcard.h>
#else
#  error Audio driver not supported.
#endif

#include "../AudioBase.h"

/*
 * Open Sound System (OSS) specific audio driver interface.
 */
class Audio_OSS: public AudioBase
{
private:  // ------------------------------------------------------- private
    static   const char AUDIODEVICE[];
    volatile int   _audiofd;

    bool _swapEndian;
    void outOfOrder ();

public:  // --------------------------------------------------------- public
    Audio_OSS();
    ~Audio_OSS();

    short *open  (AudioConfig &cfg, const char *name);
    void  close ();
    // Rev 1.2 (saw) - Changed, see AudioBase.h
    short *reset ()
    {
        if (_audiofd != (-1))
        {
            if (ioctl (_audiofd, SNDCTL_DSP_RESET, 0) != (-1))
                return _sampleBuffer;
        }
        return NULL;
    }
    short *write ();
    void  pause () {;}
};

#endif // HAVE_OSS
#endif // AUDIO_OSS_H
