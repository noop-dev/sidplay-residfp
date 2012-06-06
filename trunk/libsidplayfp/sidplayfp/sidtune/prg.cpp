/*
 * C64 PRG file format support.
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "SidTuneCfg.h"
#include "sidplayfp/SidTune.h"
#include "SidTuneTools.h"
#include "SidTuneInfoImpl.h"

static const char _sidtune_format_prg[] = "Tape image file (PRG)";
static const char _sidtune_truncated[] = "ERROR: File is most likely truncated";


bool SidTune::PRG_fileSupport(const char *fileName,
                                             Buffer_sidtt<const uint_least8_t>& dataBuf)
{
    const char *ext = SidTuneTools::fileExtOfPath(const_cast<char *>(fileName));
    if ( (MYSTRICMP(ext,".prg")!=0) &&
         (MYSTRICMP(ext,".c64")!=0) )
    {
        return false;
    }

    if (dataBuf.len() < 2)
    {
        info->m_formatString = _sidtune_truncated;
        throw loadError();
    }

    info->m_formatString = _sidtune_format_prg;

    // Automatic settings
    info->m_songs         = 1;
    info->m_startSong     = 1;
    info->m_compatibility = SidTuneInfo::COMPATIBILITY_BASIC;
    info->m_numberOfInfoStrings = 0;

    // Create the speed/clock setting table.
    convertOldStyleSpeedToTables(~0, info->m_clockSpeed);
    return true;
}
