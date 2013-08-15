/*
 * This file is part of sidplayfp, a console SID player.
 *
 * Copyright 2013 Leandro Nini
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

#ifndef AUDIOBASE_H
#define AUDIOBASE_H

#include <string.h>

#include "IAudio.h"
#include "AudioConfig.h"

class AudioBase : public IAudio
{
protected:
    class error
    {
    private:
        const char* m_msg;

    public:
        error(const char* msg) : m_msg(msg) {}
        const char* message() const { return m_msg; }
    };

protected:
    AudioConfig _settings;
    const char *_errorString;
    short      *_sampleBuffer;

public:
    AudioBase ()
    {
        _errorString  = "None";
        _sampleBuffer = NULL;
    }
    virtual ~AudioBase() {}

    short *buffer() { return _sampleBuffer; }

    void getConfig(AudioConfig &cfg) const {
        cfg = _settings;
    }

    const char *getErrorString() const {
        return _errorString;
    }
};

#endif // AUDIOBASE_H
