/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 1998, 2002 by LaLa <LaLa@C64.org>
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

//
// STIL - Common defines
//

#ifndef _STILDEFS_H
#define _STILDEFS_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

/* DLL building support on win32 hosts */
#ifndef STIL_EXTERN
#   ifdef DLL_EXPORT      /* defined by libtool (if required) */
#       define STIL_EXTERN __declspec(dllexport)
#   endif
#   ifdef STIL_DLL_IMPORT  /* define if linking with this dll */
#       define STIL_EXTERN __declspec(dllimport)
#   endif
#   ifndef STIL_EXTERN     /* static linking or !_WIN32 */
#     if defined(__GNUC__) && (__GNUC__ >= 4)
#       define STIL_EXTERN __attribute__ ((visibility("default")))
#     else
#       define STIL_EXTERN
#     endif
#   endif
#endif

/* Deprecated attributes */
#if defined(_MSCVER)
#  define STIL_DEPRECATED __declspec(deprecated)
#elif defined(__GNUC__)
#  define STIL_DEPRECATED __attribute__ ((deprecated))
#else
#  define STIL_DEPRECATED
#endif

#if defined(__linux__) || defined(__FreeBSD__) || defined(solaris2) || defined(sun) || defined(sparc) || defined(sgi)
#  define UNIX
#endif

#if defined(__MACOS__)
#  define MAC
#endif

#if defined(__amigaos__)
#  define AMIGA
#endif

//
// Here you should define:
// - what the pathname separator is on your system (attempted to be defined
//   automatically),
// - what function compares strings case-insensitively,
// - what function compares portions of strings case-insensitively.
//

#ifdef UNIX
#  define SLASH '/'
#elif MAC
#  define SLASH ':'
#elif AMIGA
#  define SLASH '/'
#else // WinDoze
#  define SLASH '\\'
#endif

// Define which one of the following two is supported on your system.
//#define HAVE_STRCASECMP 1
// #define HAVE_STRICMP 1

// Define which one of the following two is supported on your system.
//#define HAVE_STRNCASECMP 1
// #define HAVE_STRNICMP 1

// Now let's do the actual definitions.

#ifdef HAVE_STRCASECMP
#  define MYSTRICMP strcasecmp
#elif HAVE_STRICMP
#  define MYSTRICMP stricmp
#else
#  error Neither strcasecmp nor stricmp is available.
#endif

#ifdef HAVE_STRNCASECMP
#  define MYSTRNICMP strncasecmp
#elif HAVE_STRNICMP
#  define MYSTRNICMP strnicmp
#else
#  error Neither strncasecmp nor strnicmp is available.
#endif

// These are the hardcoded STIL/BUG field names.
const char    _NAME_STR[]="   NAME: ";
const char  _AUTHOR_STR[]=" AUTHOR: ";
const char   _TITLE_STR[]="  TITLE: ";
const char  _ARTIST_STR[]=" ARTIST: ";
const char _COMMENT_STR[]="COMMENT: ";
const char     _BUG_STR[]="BUG: ";

// Maximum size of a single line in STIL - also accounts for some extra
// padding, just in case.
#define STIL_MAX_LINE_SIZE 91

// Maximum size of a single STIL entry (in bytes).
#define STIL_MAX_ENTRY_SIZE STIL_MAX_LINE_SIZE*100

// HVSC path to STIL.
const char DEFAULT_PATH_TO_STIL[]="/DOCUMENTS/STIL.txt";

// HVSC path to BUGlist.
const char DEFAULT_PATH_TO_BUGLIST[]="/DOCUMENTS/BUGlist.txt";

#endif // _STILDEFS_H
