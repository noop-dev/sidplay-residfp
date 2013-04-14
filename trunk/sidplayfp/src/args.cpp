/***************************************************************************
                          args.cpp  -  Command Line Parameters
                             -------------------
    begin                : Sun Oct 7 2001
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

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cstdlib>

using std::cout;
using std::cerr;
using std::endl;
#include "player.h"

#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif

#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
#   include <sidplayfp/builders/hardsid.h>
#endif

#if defined(HAVE_SGI)
#   define DISALLOW_16BIT_SOUND
#   define DISALLOW_STEREO_SOUND
#endif


// Convert time from integer
bool ConsolePlayer::parseTime (const char *str, uint_least32_t &time)
{
    char *sep;
    uint_least32_t _time;

    // Check for empty string
    if (*str == '\0')
        return false;

    sep = (char *) strstr (str, ":");
    if (!sep)
    {   // User gave seconds
        _time = atoi (str);
    }
    else
    {   // Read in MM:SS format
        int val;
        *sep = '\0';
        val  = atoi (str);
        if (val < 0 || val > 99)
            return false;
        _time = (uint_least32_t) val * 60;
        val   = atoi (sep + 1);
        if (val < 0 || val > 59)
            return false;
        _time += (uint_least32_t) val;
    }

    time = _time;
    return true;
}

bool ConsolePlayer::parseAddress (const char *str, uint_least16_t &address)
{
    if (*str == '\0')
        return false;

    long x = strtol(str, 0, 0);

    address = x;
    return true;
}

// Parse command line arguments
int ConsolePlayer::args (int argc, const char *argv[])
{
    int  infile = 0;
    int  i      = 0;
    bool err    = false;

    if (argc == 0) // at least one argument required
    {
        displayArgs ();
        return -1;
    }

    // default arg options
    m_driver.output = OUT_SOUNDCARD;
    m_driver.file   = false;
    m_driver.sid    = EMU_RESIDFP;

    v1mute = v2mute = v3mute = false;
    v4mute = v5mute = v6mute = false;

    // parse command line arguments
    while ((i < argc) && (argv[i] != NULL))
    {
        if ((argv[i][0] == '-') && (argv[i][1] != '\0'))
        {
            // help options
            if ((argv[i][1] == 'h') || !strcmp(&argv[i][1], "-help"))
            {
                displayArgs ();
                return 0;
            }
            else if (!strcmp(&argv[i][1], "-help-debug"))
            {
                displayDebugArgs ();
                return 0;
            }

            else if (argv[i][1] == 'b')
            {
                if (!parseTime (&argv[i][2], m_timer.start))
                    err = true;
            }
            else if (strncmp (&argv[i][1], "ds", 2) == 0)
            {   // Override sidTune and enable the second sid
                if (!parseAddress (&argv[i][3], m_engCfg.secondSidAddress))
                    err = true;
            }
            else if (argv[i][1] == 'f')
            {
                if (argv[i][2] == '\0')
                    err = true;
                m_engCfg.frequency = (uint_least32_t) atoi (argv[i]+2);
            }

            // No filter options
            else if (strncmp (&argv[i][1], "nf", 2) == 0)
            {
                if (argv[i][3] == '\0')
                    m_filter.enabled = false;
            }

            // Track options
            else if (strncmp (&argv[i][1], "ols", 3) == 0)
            {
                m_track.loop   = true;
                m_track.single = true;
                m_track.first  = atoi(&argv[i][4]);
            }
            else if (strncmp (&argv[i][1], "ol", 2) == 0)
            {
                m_track.loop  = true;
                m_track.first = atoi(&argv[i][3]);
            }
            else if (strncmp (&argv[i][1], "os", 2) == 0)
            {
                m_track.single = true;
                m_track.first  = atoi(&argv[i][3]);
            }
            else if (argv[i][1] == 'o')
            {   // User forgot track number ?
                if (argv[i][2] == '\0')
                    err = true;
                m_track.first = atoi(&argv[i][2]);
            }

            // Channel muting
            else if (argv[i][1] == 'u')
            {
                if (argv[i][2] == '\0')
                    err = true;
                else
                {
                    const int voice = atoi(&argv[i][2]);
                    switch (voice)
                    {
                    case 1: v1mute = true; break;
                    case 2: v2mute = true; break;
                    case 3: v3mute = true; break;
                    case 4: v4mute = true; break;
                    case 5: v5mute = true; break;
                    case 6: v6mute = true; break;
                    }
                }
            }

            else if (argv[i][1] == 'p')
            {   // User forgot precision
                if (argv[i][2] == '\0')
                    err = true;
                {
                    uint_least8_t precision = atoi(&argv[i][2]);
                    if (precision <= 16)
                        m_precision = 16;
                    else
                        m_precision = 32;
                }
            }

            else if (argv[i][1] == 'q')
            {
                if (argv[i][2] == '\0')
                    m_quietLevel = 1;
                else
                    m_quietLevel = atoi(&argv[i][2]);
            }

            else if (argv[i][1] == 's')
            {   // Stereo Playback
                m_engCfg.playback = SidConfig::STEREO;
            }

            else if (argv[i][1] == 't')
            {
                if (!parseTime (&argv[i][2], m_timer.length))
                    err = true;
                m_timer.valid = true;
            }

            // Resampling Options ----------
            else if (strcmp (&argv[i][1], "rif") == 0)
            {
                m_engCfg.samplingMethod = SidConfig::INTERPOLATE;
                m_engCfg.fastSampling = true;
            }
            else if (strcmp (&argv[i][1], "rrf") == 0)
            {
                m_engCfg.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
                m_engCfg.fastSampling = true;
            }
            else if (strcmp (&argv[i][1], "ri") == 0)
            {
                m_engCfg.samplingMethod = SidConfig::INTERPOLATE;
            }
            else if (strcmp (&argv[i][1], "rr") == 0)
            {
                m_engCfg.samplingMethod = SidConfig::RESAMPLE_INTERPOLATE;
            }

            // SID model options
            else if (strcmp (&argv[i][1], "mof") == 0)
            {
                m_engCfg.defaultSidModel = SidConfig::MOS6581;
                m_engCfg.forceSidModel = true;
            }
            else if (strcmp (&argv[i][1], "mnf") == 0)
            {
                m_engCfg.defaultSidModel = SidConfig::MOS8580;
                m_engCfg.forceSidModel = true;
            }
            else if (strcmp (&argv[i][1], "mo") == 0)
            {
                m_engCfg.defaultSidModel = SidConfig::MOS6581;
            }
            else if (strcmp (&argv[i][1], "mn") == 0)
            {
                m_engCfg.defaultSidModel = SidConfig::MOS8580;
            }

            // Video/Verbose Options
            else if (strcmp (&argv[i][1], "vnf") == 0)
            {
                m_engCfg.forceC64Model = true;
                m_engCfg.defaultC64Model  = SidConfig::NTSC;
            }
            else if (strcmp (&argv[i][1], "vpf") == 0)
            {
                m_engCfg.forceC64Model = true;
                m_engCfg.defaultC64Model  = SidConfig::PAL;
            }
            else if (strcmp (&argv[i][1], "vf") == 0)
            {
                m_engCfg.forceC64Model = true;
            }
            else if (strcmp (&argv[i][1], "vn") == 0)
            {
                m_engCfg.defaultC64Model  = SidConfig::NTSC;
            }
            else if (strcmp (&argv[i][1], "vp") == 0)
            {
                m_engCfg.defaultC64Model  = SidConfig::PAL;
            }
            else if (argv[i][1] == 'v')
            {
                if (argv[i][2] == '\0')
                    m_verboseLevel = 1;
                else
                    m_verboseLevel = atoi(&argv[i][2]);
            }
            else if (strncmp (&argv[i][1], "-delay=", 7) == 0)
            {
                m_engCfg.powerOnDelay = (uint_least16_t) atoi(&argv[i][8]);
            }

            // File format conversions
            else if (argv[i][1] == 'w')
            {
                m_driver.output = OUT_WAV;
                m_driver.file   = true;
                if (argv[i][2] != '\0')
                    m_outfile = &argv[i][2];
            }
            else if (strncmp (&argv[i][1], "-wav", 4) == 0)
            {
                m_driver.output = OUT_WAV;
                m_driver.file   = true;
                if (argv[i][5] != '\0')
                    m_outfile = &argv[i][5];
            }
            else if (strncmp (&argv[i][1], "-au", 3) == 0)
            {
                m_driver.output = OUT_AU;
                m_driver.file   = true;
                if (argv[i][4] != '\0')
                    m_outfile = &argv[i][4];
            }

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
            else if (strcmp (&argv[i][1], "-residfp") == 0)
            {
                m_driver.sid    = EMU_RESIDFP;
            }
#endif // HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
            else if (strcmp (&argv[i][1], "-resid") == 0)
            {
                m_driver.sid    = EMU_RESID;
            }
#endif // HAVE_SIDPLAYFP_BUILDERS_RESID_H

            // Hardware selection
#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
            else if (strcmp (&argv[i][1], "-hardsid") == 0)
            {
                m_driver.sid    = EMU_HARDSID;
                m_driver.output = OUT_NULL;
            }
#endif // HAVE_SIDPLAYFP_BUILDERS_HARDSID_H

            // These are for debug
            else if (strcmp (&argv[i][1], "-none") == 0)
            {
                m_driver.sid    = EMU_NONE;
                m_driver.output = OUT_NULL;
            }
            else if (strcmp (&argv[i][1], "-nosid") == 0)
            {
                m_driver.sid = EMU_NONE;
            }
            else if (strcmp (&argv[i][1], "-cpu-debug") == 0)
            {
                m_cpudebug = true;
            }

            else
            {
                err = true;
            }

        }
        else
        {   // Reading file name
            if (infile == 0)
                infile = i;
            else
                err = true;
        }

        if (err)
        {
            displayArgs (argv[i]);
            return -1;
        }

        i++;  // next index
    }

    // Load the tune
    m_filename = argv[infile];
    m_tune.load (m_filename);
    if (!m_tune.getStatus())
    {
        displayError (m_tune.statusString());
        return -1;
    }

    // If filename specified we can only convert one song
    if (m_outfile != NULL)
        m_track.single = true;

    // Can only loop if not creating audio files
    if (m_driver.output > OUT_SOUNDCARD)
        m_track.loop = false;

    // Check to see if we are trying to generate an audio file
    // whilst using a hardware emulation
    if (m_driver.file && (m_driver.sid >= EMU_HARDSID))
    {
        displayError ("ERROR: Cannot generate audio files using hardware emulations");
        return -1;
    }

    // Select the desired track
    m_track.first    = m_tune.selectSong (m_track.first);
    m_track.selected = m_track.first;
    if (m_track.single)
        m_track.songs = 1;

    // If user provided no time then load songlength database
    // and set default lengths incase it's not found in there.
    {
        if (m_driver.file && m_timer.valid && !m_timer.length)
        {   // Time of 0 provided for wav generation
            displayError ("ERROR: -t0 invalid in record mode");
            return -1;
        }
        if (!m_timer.valid)
        {
#if !defined _WIN32 && defined HAVE_UNISTD_H
            char buffer[PATH_MAX];
#endif
            const char *database = (m_iniCfg.sidplay2()).database;
            m_timer.length = (m_iniCfg.sidplay2()).playLength;
            if (m_driver.file)
                m_timer.length = (m_iniCfg.sidplay2()).recordLength;
#if !defined _WIN32 && defined HAVE_UNISTD_H
            if (!database || *database == '\0') {
                snprintf(buffer, PATH_MAX, "%sSonglengths.txt", PKGDATADIR);
                if (::access(buffer, R_OK) == 0)
                   database = buffer;
            }
#endif
            if (database && (*database != '\0'))
            {   // Try loading the database specificed by the user
                if (!m_database.open (database))
                {
                    displayError (m_database.error ());
                    return -1;
                }
            }
        }
    }

#if HAVE_TSID == 1
    // Set TSIDs base directory
    if (!m_tsid.setBaseDir(true))
    {
        displayError (m_tsid.getError ());
        return -1;
    }
#endif

    // Configure engine with settings
    if (m_engine.config (m_engCfg) < 0)
    {   // Config failed
        displayError (m_engine.error ());
        return -1;
    }
    return 1;
}


void ConsolePlayer::displayArgs (const char *arg)
{
    std::ostream &out = arg ? cerr : cout;

    if (arg)
        out << "Option Error: " << arg << endl;
    else
        out << "Syntax: " << m_name << " [-<option>...] <datafile>" << endl;

    out << "Options:" << endl
        << " --help|-h    display this screen" << endl
        << " --help-debug debug help menu" << endl
        << " -b<num>      set start time in [m:]s format (default 0)" << endl

        << " -f<num>      set frequency in Hz (default: "
        << SidConfig::DEFAULT_SAMPLING_FREQ << ")" << endl
        << " -ds<addr>    set second sid address (e.g. -dsd420)" << endl

        << " -u<num>      mute voice <num> (e.g. -u1 -u2)" << endl

        << " -nf          no SID filter emulation" << endl

        << " -o<l|s>      looping and/or single track" << endl
        << " -o<num>      start track (default: preset)" << endl

        << " -p<num>      set format for wav output (16 = signed 16 bit, 32 = 32 bit float)"
        << "(default: " << 16 << ")" << endl

#if !defined(DISALLOW_STEREO_SOUND)
        << " -s[l|r]      stereo sid support or [left/right] channel only" << endl
#endif

        << " -t<num>      set play length in [m:]s format (0 is endless)" << endl

        << " -<v|q>       verbose or quiet (no time display) output" << endl
        << " -v[p|n][f]   set VIC PAL/NTSC clock speed (default: defined by song)" << endl
        << "              Use 'f' to force the clock by preventing speed fixing" << endl

        << " -m<o|n>[f]   set SID new/old chip model (default: old)" << endl
        << "              Use 'f' to force the model" << endl

        << " -r[i|r][f]   set resampling method (default: resample interpolate)" << endl
        << "              Use 'f' to enable fast resampling (only for reSID)" << endl

        << " -w[name]     create wav file (default: <datafile>[n].wav)" << endl;

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESIDFP_H
    out << " --residfp    use reSIDfp emulation (default)" << endl;
#endif

#ifdef HAVE_SIDPLAYFP_BUILDERS_RESID_H
    out << " --resid      use reSID emulation" << endl;
#endif

#ifdef HAVE_SIDPLAYFP_BUILDERS_HARDSID_H
    {
        HardSIDBuilder hs("");
        if (hs.availDevices ())
            out << " --hardsid    enable hardsid support" << endl;
    }
#endif
    out << endl
        // Changed to new homepage address
        << "Home Page: http://sourceforge.net/projects/sidplay-residfp/" << endl;
//        << "Mail comments, bug reports, or contributions to <sidplay2@email.com>." << endl;
}


void ConsolePlayer::displayDebugArgs ()
{
    std::ostream &out = cout;

    out << "Debug Options:" << endl
        << " --cpu-debug   display cpu register and assembly dumps" << endl
        << " --delay=<num> simulate c64 power on delay" << endl

        << " --none        no audio output device" << endl
        << " --nosid       no sid emulation" << endl;
}
