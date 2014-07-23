/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2014 Leandro Nini <drfiemost@users.sourceforge.net>
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

#ifndef MD5_INTERNAL_H
#define MD5_INTERNAL_H

#include "iMd5.h"

#include "MD5/MD5.h"

class md5Internal : public iMd5
{
private:
    MD5 hd;

public:
    void append(const void* data, int nbytes) { hd.append(data, nbytes); }

    void finish() { hd.finish(); }

    const unsigned char* getDigest() { hd.getDigest(0); }

    void reset() { hd.reset(); }
};

#endif
