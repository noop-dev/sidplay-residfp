/***************************************************************************
                          IniConfig.cpp  -  Sidplay2 config file reader.
                             -------------------
    begin                : Sun Mar 25 2001
    copyright            : (C) 2000 by Simon White
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
/***************************************************************************
 *  $Log: IniConfig.cpp,v $
 *  Revision 1.12  2004/02/26 18:17:27  s_a_white
 *  Use ini_readBool for boolean parameters rather than ini_readInt.
 *
 *  Revision 1.11  2002/02/18 20:05:52  s_a_white
 *  Fixed for new libini ini_open call.
 *
 *  Revision 1.10  2001/11/16 19:30:45  s_a_white
 *  Libsidutils support update.
 *
 *  Revision 1.9  2001/08/30 21:40:51  s_a_white
 *  libini-1.1.7 update.
 *
 *  Revision 1.8  2001/08/20 18:28:55  s_a_white
 *  MOS8580 fixed so that nothing means correct revision, 0 is 6581 and
 *  1 is 8580.
 *
 *  Revision 1.7  2001/07/14 16:52:56  s_a_white
 *  Removed warning.
 *
 *  Revision 1.6  2001/07/03 17:50:14  s_a_white
 *  External filter no longer supported.  This filter is needed internally by the
 *  library.
 *
 *  Revision 1.5  2001/04/09 17:11:03  s_a_white
 *  Added INI file version number so theres a possibility for automated updates
 *  should the keys/sections change names (or meaning).
 *
 *  Revision 1.4  2001/03/27 19:35:33  s_a_white
 *  Moved default record length for wav files from main.cpp to IniConfig.cpp.
 *
 *  Revision 1.3  2001/03/27 19:00:49  s_a_white
 *  Default record and play lengths can now be set in the sidplay2.ini file.
 *
 *  Revision 1.2  2001/03/26 18:13:07  s_a_white
 *  Support individual filters for 6581 and 8580.
 *
 ***************************************************************************/

#include <string>
#include <stdlib.h>
#include <string.h>
#include <sidplayfp/sidplay2.h>
#include "config.h"
#include "IniConfig.h"

#ifndef _WIN32
#   include <sys/types.h>
#   include <sys/stat.h>  /* mkdir */
#   include <dirent.h>    /* opendir */
    const char *IniConfig::DIR_NAME  = "sidplayfp";
#else
#   include <windows.h>
    const char *IniConfig::DIR_NAME  = "Application Data/sidplayfp";
#endif
const char *IniConfig::FILE_NAME = "sidplayfp.ini";

#define SAFE_FREE(p) { if(p) { free (p); (p)=NULL; } }

IniConfig::IniConfig ()
:status(true)
{   // Initialise everything else
    sidplay2_s.database    = NULL;
//     emulation_s.filter6581 = NULL;
//     emulation_s.filter8580 = NULL;
    clear ();
}


IniConfig::~IniConfig ()
{
    clear ();
}


void IniConfig::clear ()
{
    sidplay2_s.version      = 1;           // INI File Version
    SAFE_FREE (sidplay2_s.database);
    sidplay2_s.playLength   = 0;           // INFINITE
    sidplay2_s.recordLength = 3 * 60 + 30; // 3.5 minutes

    console_s.ansi          = false;
    console_s.topLeft       = '+';
    console_s.topRight      = '+';
    console_s.bottomLeft    = '+';
    console_s.bottomRight   = '+';
    console_s.vertical      = '|';
    console_s.horizontal    = '-';
    console_s.junctionLeft  = '+';
    console_s.junctionRight = '+';

    audio_s.frequency = SID2_DEFAULT_SAMPLING_FREQ;
    audio_s.playback  = sid2_mono;
    audio_s.precision = 16;

    emulation_s.clockSpeed    = SID2_CLOCK_CORRECT;
    emulation_s.clockForced   = false;
    emulation_s.sidModel      = SID2_MODEL_CORRECT;
    emulation_s.filter        = true;
    emulation_s.sidSamples    = true;

    emulation_s.bias            = 0.0;
    emulation_s.filterCurve6581 = 0.0;
    emulation_s.filterCurve8580 = 0;

//     SAFE_FREE (emulation_s.filter6581);
//     SAFE_FREE (emulation_s.filter8580);
}


bool  IniConfig::readDouble (ini_fd_t ini, const char *key, double &value)
{
    double d = value;
    if (ini_locateKey (ini, key) < 0)
    {   // Dosen't exist, add it
        (void) ini_writeString (ini, "");
    }
    if (ini_readDouble (ini, &d) < 0)
        return false;
    value = d;
    return true;
}


bool IniConfig::readInt (ini_fd_t ini, const char *key, int &value)
{
    int i = value;
    if (ini_locateKey (ini, key) < 0)
    {   // Dosen't exist, add it
        (void) ini_writeString (ini, "");
    }
    if (ini_readInt (ini, &i) < 0)
        return false;
    value = i;
    return true;
}


bool IniConfig::readString (ini_fd_t ini, const char *key, char *&str)
{
    char  *ret;
    size_t length;

    if (ini_locateKey (ini, key) < 0)
    {   // Dosen't exist, add it
        (void) ini_writeString (ini, "");
    }

    length = (size_t) ini_dataLength (ini);
    if (!length)
        return 0;

    ret = (char *) malloc (++length);
    if (!ret)
        return false;

    if (ini_readString (ini, ret, (uint) length) < 0)
    {
        free (ret);
        return false;
    }

    str = ret;
return true;
}


bool IniConfig::readBool (ini_fd_t ini, const char *key, bool &boolean)
{
    int b = boolean;
    if (ini_locateKey (ini, key) < 0)
    {   // Dosen't exist, add it
        (void) ini_writeString (ini, "");
    }
    if (ini_readBool (ini, &b) < 0)
        return false;
    boolean = (b != 0);
    return true;
}


bool IniConfig::readChar (ini_fd_t ini, const char *key, char &ch)
{
    char *str, c = 0;
    bool  ret = readString (ini, key, str);
    if (!ret)
        return false;

    // Check if we have an actual chanracter
    if (str[0] == '\'')
    {
        if (str[2] != '\'')
            ret = false;
        else
            c = str[1];
    } // Nope is number
    else
        c = (char) atoi (str);

    // Clip off special characters
    if ((unsigned) c >= 32)
        ch = c;

    free (str);
    return ret;
}


bool IniConfig::readTime (ini_fd_t ini, const char *key, int &value)
{
    char *str, *sep;
    int   time;
    bool  ret = readString (ini, key, str);
    if (!ret)
        return false;

    if (!*str)
        return false;

    sep = strstr (str, ":");
    if (!sep)
    {   // User gave seconds
        time = atoi (str);
    }
    else
    {   // Read in MM:SS format
        int val;
        *sep = '\0';
        val  = atoi (str);
        if (val < 0 || val > 99)
            goto IniCofig_readTime_error;
        time = val * 60;
        val  = atoi (sep + 1);
        if (val < 0 || val > 59)
            goto IniCofig_readTime_error;
        time += val;
    }

    value = time;
    free (str);
return ret;

IniCofig_readTime_error:
    free (str);
    return false;
}


bool IniConfig::readSidplay2 (ini_fd_t ini)
{
    bool ret = true;
    int  time, version = sidplay2_s.version;

    (void) ini_locateHeading (ini, "SIDPlay2");
    ret &= readInt (ini, "Version", version);
    if (version > 0)
        sidplay2_s.version = version;

    ret &= readString (ini, "Songlength Database", sidplay2_s.database);
    if (readTime (ini, "Default Play Length", time))
        sidplay2_s.playLength   = (uint_least32_t) time;
    if (readTime (ini, "Default Record Length", time))
        sidplay2_s.recordLength = (uint_least32_t) time;

    return ret;
}


bool IniConfig::readConsole (ini_fd_t ini)
{
    bool ret = true;
    (void) ini_locateHeading (ini, "Console");
    ret &= readBool (ini, "Ansi",                console_s.ansi);
    ret &= readChar (ini, "Char Top Left",       console_s.topLeft);
    ret &= readChar (ini, "Char Top Right",      console_s.topRight);
    ret &= readChar (ini, "Char Bottom Left",    console_s.bottomLeft);
    ret &= readChar (ini, "Char Bottom Right",   console_s.bottomRight);
    ret &= readChar (ini, "Char Vertical",       console_s.vertical);
    ret &= readChar (ini, "Char Horizontal",     console_s.horizontal);
    ret &= readChar (ini, "Char Junction Left",  console_s.junctionLeft);
    ret &= readChar (ini, "Char Junction Right", console_s.junctionRight);
    return ret;
}


bool IniConfig::readAudio (ini_fd_t ini)
{
    bool ret = true;
    (void) ini_locateHeading (ini, "Audio");

    {
        int frequency = (int) audio_s.frequency;
        ret &= readInt (ini, "Frequency", frequency);
        audio_s.frequency = (unsigned long) frequency;
    }

    {
        int channels = 0;
        ret &= readInt (ini, "Channels",  channels);
        if (channels)
        {
            audio_s.playback = sid2_mono;
            if (channels != 1)
                audio_s.playback = sid2_stereo;
        }
    }

    ret &= readInt (ini, "BitsPerSample", audio_s.precision);
    return ret;
}


bool IniConfig::readEmulation (ini_fd_t ini)
{
    bool ret = true;
    (void) ini_locateHeading (ini, "Emulation");

    {
        int clockSpeed = -1;
        ret &= readInt (ini, "ClockSpeed", clockSpeed);
        if (clockSpeed != -1)
        {
            if (clockSpeed < 0 || clockSpeed > 1)
                ret = false;
            emulation_s.clockSpeed = SID2_CLOCK_PAL;
            if (clockSpeed)
                emulation_s.clockSpeed = SID2_CLOCK_NTSC;
        }
    }

    ret &= readBool (ini, "ForceSongSpeed", emulation_s.clockForced);

    {
        int mos8580 = -1;
        ret &= readInt (ini, "MOS8580", mos8580);
        if (mos8580 != -1)
        {
            if (mos8580 < 0 || mos8580 > 1)
                ret = false;
            emulation_s.sidModel = SID2_MOS6581;
            if (mos8580)
                emulation_s.sidModel = SID2_MOS8580;
        }
    }

    ret &= readBool (ini, "UseFilter", emulation_s.filter);

//     ret &= readString (ini, "Filter6581", emulation_s.filter6581);
//     ret &= readString (ini, "Filter8580", emulation_s.filter8580);
    ret &= readDouble (ini, "FilterBias", emulation_s.bias);
    ret &= readDouble (ini, "FilterCurve6581", emulation_s.filterCurve6581);
    ret &= readInt (ini, "FilterCurve8580", emulation_s.filterCurve8580);
    ret &= readBool   (ini, "SidSamples", emulation_s.sidSamples);

    return ret;
}

/*
bool IniConfig::readFilters (const char *ini_file)
{
    bool ret = true;
    if (emulation_s.filter6581)
    {   // Try to load the filter
        filter6581.read (ini_file, emulation_s.filter6581);
        if (!filter6581)
        {
            ret = false;
        }
    }

    if (emulation_s.filter8580)
    {   // Try to load the filter
        filter8580.read (ini_file, emulation_s.filter8580);
        if (!filter8580)
        {
            ret = false;
        }
    }

    return ret;
}
*/

void IniConfig::read ()
{
    char *path = NULL;
    ini_fd_t ini  = 0;
    size_t  length;
    std::string configPath;

#ifdef _WIN32
    path = getenv("USERPROFILE");
    if (!path)
        goto IniConfig_read_error;
    configPath.append(path);
#else
    path = getenv("XDG_CONFIG_HOME");
    if (!path) {
        path = getenv("HOME");
        if (!path)
            goto IniConfig_read_error;
        configPath.append(path).append("/.config");
    } else
        configPath.append(path);
#endif

    configPath.append("/").append(DIR_NAME);

#ifndef _WIN32
    // Make sure the config path exists
    if (!opendir(configPath.c_str()))
        mkdir(configPath.c_str(), 0755);
#else
    CreateDirectoryA(configPath.c_str(), NULL);
#endif

    /* sprintf on top of itself fails nowadays. */
    configPath.append("/").append(FILE_NAME);

    // Opens an existing file or creates a new one
    ini = ini_open (configPath.c_str(), "w", ";");

    // Unable to open file?
    if (!ini)
        goto IniConfig_read_error;

    clear ();

    // This may not exist here...
    status &= readSidplay2  (ini);
    status &= readConsole   (ini);
    status &= readAudio     (ini);
    status &= readEmulation (ini);
    ini_close (ini);

//     status &= readFilters (configPath);

    return;

IniConfig_read_error:
    if (ini)
        ini_close (ini);
    clear ();
    status = false;
}
/*
const sid_filterfp_t* IniConfig::filter (sid2_model_t model)
{
    if (model == SID2_MOS8580)
        return filter8580.providefp ();
    return filter6581.providefp ();
}
*/
