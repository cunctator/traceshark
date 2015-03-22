/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRACESHARK_H

#include <cmath>

#define tsconnect(src, ssig, dest, dslot) \
	connect(src, SIGNAL(ssig), dest, SLOT(dslot))

namespace TraceShark {
	inline double strToDouble(char* str, bool &ok)
	{
		char *c;
		double r;
		unsigned long long base = 0;
		bool isNeg = false;
		unsigned int d, n;
		double div;

		ok = true;

		if (*str == '-') {
			str++;
			isNeg = true;
		}

		for (c = str; *c != '\0'; c++) {
			if (*c == '.')
				break;
			if (*c < '0' || *c > '9')
				goto error;
			d = *c - '0';
			base *= 10;
			base += d;
		}

		r = (double) base;

		if (*c == '.') {
			n = 0;
			base = 0;
			for (c++; *c != '\0'; c++) {
				if (*c < '0' || *c > '9')
					goto error;
				d = *c - '0';
				base *= 10;
				base += d;
				n++;
			}
			div = pow(10, n);
			r += base / div;
		}

		if (isNeg)
			return -r;
		else
			return r;
	error:
	        ok = false;
		return 0;
	}
}

#endif /* TRACESHARK_H */
