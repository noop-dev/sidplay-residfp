/*
 * This file is part of libsidplayfp, a SID player engine.
 *
 * Copyright 2011-2014 Leandro Nini <drfiemost@users.sourceforge.net>
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

#include "Spline.h"

#include <cassert>
#include <limits>

namespace reSIDfp
{

double Spline::slope(const Point &a, const Point &b)
{
    return (b.y - a.y) / (b.x - a.x);
}

Spline::Spline(const Point input[], int inputLength) :
    paramsLength(inputLength - 1),
    params(new double[paramsLength][6])
{
    assert(inputLength > 2);

    for (int i = 0; i < paramsLength; i++)
    {
        assert(input[i].x < input[i + 1].x);

        const Point *p0 = i != 0 ? &input[i - 1] : 0;
        const Point &p1 = input[i];
        const Point &p2 = input[i + 1];
        const Point *p3 = i != inputLength - 2 ? &input[i + 2] : 0;

        double k1, k2;

        if (p0 == 0)
        {
            k2 = slope(p1, *p3);
            k1 = (3. * slope(p1, p2) - k2) / 2.;
        }
        else if (p3 == 0)
        {
            k1 = slope(*p0, p2);
            k2 = (3. * slope(p1, p2) - k1) / 2.;
        }
        else
        {
            k1 = slope(*p0, p2);
            k2 = slope(p1, *p3);
        }

        const double x1 = p1.x;
        const double y1 = p1.y;
        const double x2 = p2.x;
        const double y2 = p2.y;

        const double dx = x2 - x1;
        const double dy = y2 - y1;

        const double a = ((k1 + k2) - 2. * dy / dx) / (dx * dx);
        const double b = ((k2 - k1) / dx - 3. * (x1 + x2) * a) / 2.;
        const double c = k1 - (3. * x1 * a + 2. * b) * x1;
        const double d = y1 - ((x1 * a + b) * x1 + c) * x1;

        params[i][0] = x1;
        params[i][1] = x2;
        params[i][2] = a;
        params[i][3] = b;
        params[i][4] = c;
        params[i][5] = d;
    }

    // Fix the value ranges, because we interpolate outside original bounds if necessary.
    params[0][0] = std::numeric_limits<double>::min();
    params[paramsLength - 1][1] = std::numeric_limits<double>::max();

    c = params[0];
}

void Spline::evaluate(double x, Point &out)
{
    if (x < c[0] || x > c[1])
    {
        for (int i = 0; i < paramsLength; i++)
        {
            if (x <= params[i][1])
            {
                c = params[i];
                break;
            }
        }
    }

    // y = a*x^3 + b*x^2 + c*x + d
    out.x = ((c[2] * x + c[3]) * x + c[4]) * x + c[5];

    // yd = 3*a*x^2 + 2*b*x + c
    out.y = (3. * c[2] * x + 2. * c[3]) * x + c[4];
}

} // namespace reSIDfp
