/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
	static __always_inline int cmp(const TString *a, const TString *b);
	static __always_inline int strcmp(const TString *a, const TString *b);
	static __always_inline int strcmp(const TString *a, const TString *b,
					  unsigned short skip,
					  unsigned short *neq);
	__always_inline bool merge(const TString *s, unsigned int maxlen);
	__always_inline bool set(const TString *s, unsigned int maxlen);
};

__always_inline int TString::cmp(const TString *a, const TString *b) {
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

__always_inline int TString::strcmp(const TString *a, const TString *b,
				    unsigned short skip, unsigned short *eqn)
{
	int rval = (int) a->len - (int)  b->len;
	int cval;
	int imax = rval < 0 ? a->len : b->len;
	int i;

	*eqn = skip;
	for (i = skip; i < imax; i++) {
		cval = a->ptr[i] - b->ptr[i];
		if (cval == 0) {
			(*eqn)++;
			continue;
		} else
			return cval;
	}
	return rval;
}

/* This seems to be the fastest now, at least for stringpool */
__always_inline int TString::strcmp(const TString *a, const TString *b)
{
	int rval = (int) a->len - (int)  b->len;
	int cval;
	int imax = rval < 0 ? a->len : b->len;
	int i;

	for (i = 0; i < imax; i++) {
		cval = a->ptr[i] - b->ptr[i];
		if (cval == 0)
			continue;
		else
			return cval;
	}
	return rval;
}

__always_inline bool TString::merge(const TString *s, unsigned int maxlen)
{
	unsigned int newlen;

	newlen = len + 1 + s->len;
	if (newlen > maxlen)
		return false;
	ptr[len] = ' ';
	strncpy(ptr + len + 1, s->ptr, s->len);
	len = newlen;
	ptr[len] = '\0';
	return true;
}

__always_inline bool TString::set(const TString *s, unsigned int maxlen)
{
	if (s->len > maxlen)
		return false;
	strncpy(ptr, s->ptr, s->len);
	len = s->len;
	ptr[len] = '\0';
	return true;
}

#endif
