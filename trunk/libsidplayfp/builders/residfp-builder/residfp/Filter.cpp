/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
 * Copyright (C) 2004  Dag Lem <resid@nimrod.no>
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

#include "Filter.h"

namespace reSIDfp
{

void Filter::enable(const bool enable) {
	enabled = enable;
	if (enabled) {
		writeRES_FILT(filt);
	} else {
		filt1 = filt2 = filt3 = filtE = false;
	}
}

void Filter::setClockFrequency(const double clock) {
	clockFrequency = clock;
	updatedCenterFrequency();
}

void Filter::reset() {
	writeFC_LO(0);
	writeFC_HI(0);
	writeMODE_VOL(0);
	writeRES_FILT(0);
}

void Filter::writeFC_LO(const unsigned char fc_lo) {
	fc = (fc & 0x7f8) | (fc_lo & 0x007);
	updatedCenterFrequency();
}

void Filter::writeFC_HI(const unsigned char fc_hi) {
	fc = (fc_hi << 3 & 0x7f8) | (fc & 0x007);
	updatedCenterFrequency();
}

void Filter::writeRES_FILT(const unsigned char res_filt) {
	filt = res_filt;

	res = res_filt >> 4 & 0x0f;
	updatedResonance();

	if (enabled) {
		filt1 = (filt & 1) != 0;
		filt2 = (filt & 2) != 0;
		filt3 = (filt & 4) != 0;
		filtE = (filt & 8) != 0;
	}

	updatedMixing();
}

void Filter::writeMODE_VOL(const unsigned char mode_vol) {
	vol = mode_vol & 0xf;
	lp = (mode_vol & 0x10) != 0;
	bp = (mode_vol & 0x20) != 0;
	hp = (mode_vol & 0x40) != 0;
	voice3off = (mode_vol & 0x80) != 0;

	updatedMixing();
}

} // namespace reSIDfp