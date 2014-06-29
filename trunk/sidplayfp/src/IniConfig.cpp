/*
 * This file is part of sidplayfp, a console SID player.
 *
 * Copyright 2011-2014 Leandro Nini
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

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "IniConfig.h"

#include <string>

#include <cstring>
#include <cstdlib>
#include <cstdio>

#ifndef _WIN32
#  include <sys/types.h>
#  include <sys/stat.h>  /* mkdir */
#  include <dirent.h>    /* opendir */
#else
#  include <windows.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "utils.h"
#include "ini/dataParser.h"

const TCHAR *IniConfig::DIR_NAME  = TEXT("sidplayfp");
const TCHAR *IniConfig::FILE_NAME = TEXT("sidplayfp.ini");

#define SAFE_FREE(p) { if(p) { free (p); (p)=NULL; } }

IniConfig::IniConfig() :
    status(true)
{   // Initialise everything else
    clear();
}


IniConfig::~IniConfig()
{
    clear();
}


void IniConfig::clear()
{
    sidplay2_s.version      = 1;           // INI File Version
    sidplay2_s.database.clear();
    sidplay2_s.playLength   = 0;           // INFINITE
    sidplay2_s.recordLength = 3 * 60 + 30; // 3.5 minutes
    sidplay2_s.kernalRom.clear();
    sidplay2_s.basicRom.clear();
    sidplay2_s.chargenRom.clear();

    console_s.ansi          = false;
    console_s.topLeft       = '+';
    console_s.topRight      = '+';
    console_s.bottomLeft    = '+';
    console_s.bottomRight   = '+';
    console_s.vertical      = '|';
    console_s.horizontal    = '-';
    console_s.junctionLeft  = '+';
    console_s.junctionRight = '+';

    audio_s.frequency = SidConfig::DEFAULT_SAMPLING_FREQ;
    audio_s.playback  = SidConfig::MONO;
    audio_s.precision = 16;

    emulation_s.modelDefault  = SidConfig::PAL;
    emulation_s.modelForced   = false;
    emulation_s.sidModel      = SidConfig::MOS6581;
    emulation_s.forceModel    = false;
    emulation_s.filter        = true;
    emulation_s.engine.clear();

    emulation_s.bias            = 0.0;
    emulation_s.filterCurve6581 = 0.0;
    emulation_s.filterCurve8580 = 0;
}


bool IniConfig::readDouble(iniHandler &ini, const TCHAR *key, double &result)
{
    const TCHAR* value = ini.getValue(key);
    if (value == 0)
    {   // Doesn't exist, add it
        ini.addValue(key, TEXT(""));
        return false;
    }

    try
    {
        result = dataParser::parseDouble(value);
    }
    catch (dataParser::parseError const &e)
    {
        return false;
    }

    return true;
}


bool IniConfig::readInt(iniHandler &ini, const TCHAR *key, int &result)
{
    const TCHAR* value = ini.getValue(key);
    if (value == 0)
    {   // Doesn't exist, add it
        ini.addValue(key, TEXT(""));
        return false;
    }

    try
    {
        result = dataParser::parseInt(value);
    }
    catch (dataParser::parseError const &e)
    {
        return false;
    }

    return true;
}


bool IniConfig::readString(iniHandler &ini, const TCHAR *key, SID_STRING &result)
{
    const TCHAR* value = ini.getValue(key);
    if (value == 0)
    {   // Doesn't exist, add it
        ini.addValue(key, TEXT(""));
        return false;
    }

    result.assign(value);
    return true;
}


bool IniConfig::readBool(iniHandler &ini, const TCHAR *key, bool &result)
{
    const TCHAR* value = ini.getValue(key);
    if (value == 0)
    {   // Doesn't exist, add it
        ini.addValue(key, TEXT(""));
        return false;
    }

    try
    {
        result = dataParser::parseBool(value);
    }
    catch (dataParser::parseError const &e)
    {
        return false;
    }

    return true;
}


bool IniConfig::readChar(iniHandler &ini, const TCHAR *key, char &ch)
{
    SID_STRING str;
    bool ret = readString(ini, key, str);
    if (!ret)
        return false;

    TCHAR c = 0;

    // Check if we have an actual chanracter
    if (str[0] == '\'')
    {
        if (str[2] != '\'')
            ret = false;
        else
            c = str[1];
    } // Nope is number
    else
    {
        try
        {
            c = dataParser::parseInt(str.c_str());
        }
        catch (dataParser::parseError const &e)
        {
            return false;
        }
    }

    // Clip off special characters
    if ((unsigned) c >= 32)
        ch = c;

    return ret;
}


bool IniConfig::readTime(iniHandler &ini, const TCHAR *key, int &value)
{
    SID_STRING str;
    bool ret = readString(ini, key, str);
    if (!ret)
        return false;

    int time;
    size_t sep = str.find_first_of(':');
    try
    {
        if (sep == SID_STRING::npos)
        {   // User gave seconds
            time = dataParser::parseInt(str.c_str());
        }
        else
        {   // Read in MM:SS format
            str.replace(sep, 1, '\0');
            int val = dataParser::parseInt(str.c_str());
            if (val < 0 || val > 99)
                goto IniCofig_readTime_error;
            time = val * 60;
            val  = dataParser::parseInt(str.c_str()+sep + 1);
            if (val < 0 || val > 59)
                goto IniCofig_readTime_error;
            time += val;
        }
    }
    catch (dataParser::parseError const &e)
    {
        return false;
    }

    value = time;
    return ret;

IniCofig_readTime_error:
    return false;
}


bool IniConfig::readSidplay2(iniHandler &ini)
{
    if (!ini.setSection (TEXT("SIDPlayfp")))
        return false;

    bool ret = true;

    int version = sidplay2_s.version;
    ret &= readInt (ini, TEXT("Version"), version);
    if (version > 0)
        sidplay2_s.version = version;

    ret &= readString(ini, TEXT("Songlength Database"), sidplay2_s.database);

#if !defined _WIN32 && defined HAVE_UNISTD_H
            if (sidplay2_s.database.empty())
            {
            char buffer[PATH_MAX];
                snprintf(buffer, PATH_MAX, "%sSonglengths.txt", PKGDATADIR);
                if (::access(buffer, R_OK) == 0)
                    sidplay2_s.database.assign(buffer);
            }
#endif

    int time;
    if (readTime (ini, TEXT("Default Play Length"), time))
        sidplay2_s.playLength   = (uint_least32_t) time;
    if (readTime (ini, TEXT("Default Record Length"), time))
        sidplay2_s.recordLength = (uint_least32_t) time;

    ret &= readString(ini, TEXT("Kernal Rom"), sidplay2_s.kernalRom);
    ret &= readString(ini, TEXT("Basic Rom"), sidplay2_s.basicRom);
    ret &= readString(ini, TEXT("Chargen Rom"), sidplay2_s.chargenRom);

    return ret;
}


bool IniConfig::readConsole(iniHandler &ini)
{
    if (!ini.setSection (TEXT("Console")))
        return false;

    bool ret = true;

    ret &= readBool (ini, TEXT("Ansi"),                console_s.ansi);
    ret &= readChar (ini, TEXT("Char Top Left"),       console_s.topLeft);
    ret &= readChar (ini, TEXT("Char Top Right"),      console_s.topRight);
    ret &= readChar (ini, TEXT("Char Bottom Left"),    console_s.bottomLeft);
    ret &= readChar (ini, TEXT("Char Bottom Right"),   console_s.bottomRight);
    ret &= readChar (ini, TEXT("Char Vertical"),       console_s.vertical);
    ret &= readChar (ini, TEXT("Char Horizontal"),     console_s.horizontal);
    ret &= readChar (ini, TEXT("Char Junction Left"),  console_s.junctionLeft);
    ret &= readChar (ini, TEXT("Char Junction Right"), console_s.junctionRight);
    return ret;
}


bool IniConfig::readAudio(iniHandler &ini)
{
    if (!ini.setSection (TEXT("Audio")))
        return false;

    bool ret = true;

    {
        int frequency = (int) audio_s.frequency;
        ret &= readInt (ini, TEXT("Frequency"), frequency);
        audio_s.frequency = (unsigned long) frequency;
    }

    {
        int channels = 0;
        ret &= readInt (ini, TEXT("Channels"),  channels);
        if (channels)
        {
            audio_s.playback = (channels == 1) ? SidConfig::MONO : SidConfig::STEREO;
        }
    }

    ret &= readInt (ini, TEXT("BitsPerSample"), audio_s.precision);
    return ret;
}


bool IniConfig::readEmulation(iniHandler &ini)
{
    if (!ini.setSection (TEXT("Emulation")))
        return false;

    bool ret = true;

    ret &= readString (ini, TEXT("Engine"), emulation_s.engine);

    {
        SID_STRING str;
        const bool res = readString (ini, TEXT("C64Model"), str);
        if (res)
        {
            if (str.compare(TEXT("PAL")) == 0)
                emulation_s.modelDefault = SidConfig::PAL;
            else if (str.compare(TEXT("NTSC")) == 0)
                emulation_s.modelDefault = SidConfig::NTSC;
            else if (str.compare(TEXT("OLD_NTSC")) == 0)
                emulation_s.modelDefault = SidConfig::OLD_NTSC;
            else if (str.compare(TEXT("DREAN")) == 0)
                emulation_s.modelDefault = SidConfig::DREAN;
        }
        ret &= res;
    }

    ret &= readBool (ini, TEXT("ForceC64Model"), emulation_s.modelForced);

    {
        SID_STRING str;
        const bool res = readString (ini, TEXT("SidModel"), str);
        if (res)
        {
            if (str.compare(TEXT("MOS6581")) == 0)
                emulation_s.sidModel = SidConfig::MOS6581;
            else if (str.compare(TEXT("MOS8580")) == 0)
                emulation_s.sidModel = SidConfig::MOS8580;
        }
        ret &= res;
    }

    ret &= readBool (ini, TEXT("ForceSidModel"), emulation_s.forceModel);

    ret &= readBool (ini, TEXT("UseFilter"), emulation_s.filter);

    ret &= readDouble (ini, TEXT("FilterBias"), emulation_s.bias);
    ret &= readDouble (ini, TEXT("FilterCurve6581"), emulation_s.filterCurve6581);
    ret &= readInt (ini, TEXT("FilterCurve8580"), emulation_s.filterCurve8580);

    return ret;
}

void IniConfig::read()
{
    iniHandler ini;

    SID_STRING configPath;

    try
    {
        configPath = utils::getConfigPath();
    }
    catch (utils::error const &e)
    {
        goto IniConfig_read_error;
    }

    configPath.append(TEXT("/")).append(DIR_NAME);

#ifndef _WIN32
    // Make sure the config path exists
    if (!opendir(configPath.c_str()))
        mkdir(configPath.c_str(), 0755);
#else
    CreateDirectory(configPath.c_str(), NULL);
#endif

    configPath.append(TEXT("/")).append(FILE_NAME);

    // Opens an existing file or creates a new one
    if (!ini.open(configPath.c_str()))
        goto IniConfig_read_error;

    clear ();

    // This may not exist here...
    status &= readSidplay2  (ini);
    status &= readConsole   (ini);
    status &= readAudio     (ini);
    status &= readEmulation (ini);
    ini.close ();

    return;

IniConfig_read_error:
    clear ();
    status = false;
}
