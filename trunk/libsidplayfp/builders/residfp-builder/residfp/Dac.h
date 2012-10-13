/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2012 Leando Nini <drfiemost@users.sourceforge.net>
 * Copyright 2007-2010 Antti Lankila
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

#ifndef DAC_H
#define DAC_H

namespace reSIDfp
{

/** @internal
* Estimate DAC nonlinearity. The SID contains R-2R ladder, and some likely errors
* in the resistor lengths which result in errors depending on the bits chosen.
*
* This model was derived by Dag Lem, and is port of the upcoming reSID version.
* In average, it shows a value higher than the target by a value that depends
* on the _2R_div_R parameter. It differs from the version written by Antti Lankila
* chiefly in the emulation of the lacking termination of the 2R ladder, which
* destroys the output with respect to the low bits of the DAC.
*
* @param input digital value to convert to analog
* @param _2R_div_R nonlinearity parameter, 1.0 for perfect linearity.
* @param bits highest bit that may be set in input.
* @param termi is the dac terminated by a 2R resistor? (6581 DACs are not)
*
* @return the analog value as modeled from the R-2R network.
*/
class Dac {

public:
	static void kinkedDac(double* dac, const int dacLength, const double _2R_div_R, const bool term);
};

} // namespace reSIDfp

#endif