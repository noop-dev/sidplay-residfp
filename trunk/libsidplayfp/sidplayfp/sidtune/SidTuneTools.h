/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright (C) Michael Schwendt <mschwendt@yahoo.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SIDTUNETOOLS_H
#define SIDTUNETOOLS_H

#include <string.h>

#include "SidTuneCfg.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef HAVE_STRCASECMP
#   define MYSTRICMP strcasecmp
#elif HAVE_STRICMP
#   define MYSTRICMP stricmp
#else
#  error Neither strcasecmp nor stricmp is available.
#endif

#ifdef HAVE_STRNCASECMP
#   define MYSTRNICMP strncasecmp
#elif HAVE_STRNICMP
#   define MYSTRNICMP strnicmp
#else
#  error Neither strncasecmp nor strnicmp is available.
#endif

class SidTuneTools
{
 public:
    /** Return pointer to file name position in complete path. */
    static size_t fileNameWithoutPath(const char* s);

    /**
    * Return pointer to file name position in complete path.
    * Special version: file separator = forward slash.
    */
    static size_t slashedFileNameWithoutPath(const char* s);

    /**
    * Return pointer to file name extension in path.
    * Searching backwards until first dot is found.
    */
    static const char* fileExtOfPath(const char* s);
};

#endif  /* SIDTUNETOOLS_H */
