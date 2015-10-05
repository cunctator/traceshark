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

#ifndef TRACESHARK_H
#define TRACESHARK_H

#define TRACESHARK_VERSION_STRING "0.01-alpha"

#include <QtCore>
#include <cstdint>
#include "tstring.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui>
#else
#include <QtWidgets>
#endif

#define tsconnect(src, ssig, dest, dslot) \
	connect(src, SIGNAL(ssig), dest, SLOT(dslot))

#define sigconnect(src, ssig, dest, dsig) \
	connect(src, SIGNAL(ssig), dest, SIGNAL(dsig))

#define lastfunc(myint) ((double) myint)

#define DEFINE_CPUTASKMAP_ITERATOR(name) \
	QMap<unsigned int, CPUTask>::iterator name

#define DEFINE_TASKMAP_ITERATOR(name) \
	QMap<unsigned int, Task>::iterator name

#define DEFINE_COLORMAP_ITERATOR(name) \
	QMap<unsigned int, TColor>::iterator name

#define TSMAX(A, B) ((A) >= (B) ? A:B)
#define TSMIN(A, B) ((A) < (B) ? A:B)
#define TSABS(A) ((A) >= 0 ? A:-A)

/*  1024 CPUs ought to be enough for anybody */
#define HIGHEST_CPU_EVER (1023)

/* C++ syntax for calling the pointer to a member function for an object */
#define CALL_MEMBER_FN(ptrObject, ptrToMember) ((ptrObject)->*(ptrToMember))
/* C++ syntax for declaring a pointer to a member function */
#define DEFINE_MEMBER_FN(returntype, className, name) \
	returntype (className::* name)()

#define SPROL32(VALUE, N) \
	((VALUE << N) | (VALUE >> (32 - N)))

namespace TShark {

	enum CursorIdx {RED_CURSOR, BLUE_CURSOR, NR_CURSORS};

	/* This functions accepts ':' at the end of the value
	 * For example, 123.456: is ok. 123.456X is not ok if
	 * X is not a digit between 0-9 or a ':'
	 */
	static __always_inline double strToDouble(char* str, bool &ok)
	{
		char *c;
		double r;
		unsigned long long base = 0;
		bool isNeg = false;
		unsigned int d;
		unsigned long long divint;
		double div;

		ok = true;

		if (*str == '-') {
			str++;
			isNeg = true;
		}

		for (c = str; *c != '\0'; c++) {
			if (*c < '0' || *c > '9')
				break;
			d = *c - '0';
			base *= 10;
			base += d;
		}

		r = (double) base;

		if (*c == '.') {
			divint = 1;
			base = 0;
			for (c++; *c != '\0'; c++) {
				if (*c < '0' || *c > '9')
					break;
				d = *c - '0';
				base *= 10;
				base += d;
				divint *= 10;
			}
			div = (double) divint;
			r += base / div;
		}

		if (*c != ':' && *c != '\0')
			goto error;

		if (isNeg)
			return -r;
		else
			return r;
	error:
	        ok = false;
		return 0;
	}

	union value32 {
		uint32_t word32;
		uint8_t word8[4];
	};

	__always_inline uint32_t StrHash32(const TString *str)
	{
		union value32 uvalue;
		uvalue.word32 = 0;
		if (str->len < 1)
			return 0;
		uvalue.word8[0] = str->ptr[0];
		if (str->len < 4)
			return uvalue.word32;
		uvalue.word8[1] = str->ptr[1];
		uvalue.word8[2] = str->ptr[2];
		uvalue.word8[3] = str->ptr[3];
		return uvalue.word32;
	}
}

#endif /* TRACESHARK_H */
