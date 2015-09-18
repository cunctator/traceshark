/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TSTRING_H
#define TSTRING_H

#include <cstring>

class TString {
public:
	char *ptr;
	unsigned int len;
	static __always_inline int strcmp(const TString *a, const TString *b);
	static __always_inline int Tstrcmp(const TString *a, const TString *b,
					   unsigned short skip,
					   unsigned short *neq);
};

__always_inline int TString::strcmp(const TString *a, const TString *b) {
	unsigned int clen;
	int rval;
	int diff;

	diff = a->len - b->len;
	clen = diff < 0 ? a->len : b->len;
	rval = memcmp(a->ptr, b->ptr, clen);
	if (rval == 0)
		return diff;
	else
		return rval;
}

__always_inline int TString::Tstrcmp(const TString *a, const TString *b,
			     unsigned short skip, unsigned short *eqn)
{
	int rval = (int) a->len - (int)  b->len;
	int cval;
	int imax = rval < 0 ? a->len : b->len;
	int i;

	*eqn = 0;
	for (i = skip; i < imax; i++) {
		cval = a->ptr[i] - b->ptr[i];
		if (cval == 0) {
			continue;
			(*eqn)++;
		}
		else
			return cval;
	}
	return rval;
}

#endif
