/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TRACESHARK_H
#define TRACESHARK_H

#define TRACESHARK_VERSION_STRING "0.3.1-alpha"
#define QCUSTOMPLOT_VERSION_STRING "2.0.0"

#include <QtCore>
#include <cstdint>
#include <cstdio>
#include "misc/tstring.h"

#define EVENT_MAX_NR_ARGS (128)

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui>
#else
#include <QtWidgets>
#endif

#ifdef __has_cpp_attribute
#define TS_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define TS_HAS_CPP_ATTRIBUTE(x) 0
#endif

#if TS_HAS_CPP_ATTRIBUTE(fallthrough)
#define ts_fallthrough [[fallthrough]]
#elif TS_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#define ts_fallthrough [[gnu::fallthrough]]
#else
#define ts_fallthrough (void)0
#endif

#ifdef __GNUC__

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/*
 * locality must be between [0, 3]
 *
 * 0 no temporal locality
 * 1 low temporal locality
 * 2 moderate degree of temporal locality
 * 3 high degree of temporal locality
 */
#define prefetch_read(addr, locality) \
	__builtin_prefetch(addr, 0, locality)
#define prefetch_write(addr, locality) \
	__builtin_prefetch(addr, 1, locality)
#define prefetch(addr) \
	__builtin_prefetch(addr)

#else /* __GNUC__ not defined */

#define likely(x)   (x)
#define unlikely(x) (x)
#define prefetch_read(addr, locality)
#define prefetch_write(addr, locality)
#define prefetch(addr)

#endif /* __GNUC__ */

typedef enum {
	TRACE_TYPE_FTRACE = 0,
	TRACE_TYPE_PERF,
	TRACE_TYPE_MAX
} tracetype_t;

#define TRACE_TYPE_NONE (TRACE_TYPE_MAX)

#define tsconnect(src, ssig, dest, dslot) \
	connect(src, SIGNAL(ssig), dest, SLOT(dslot))

#define sigconnect(src, ssig, dest, dsig) \
	connect(src, SIGNAL(ssig), dest, SIGNAL(dsig))

#define lastfunc(myint) ((double) myint)

#define DEFINE_CPUTASKMAP_ITERATOR(name) \
	vtl::AVLTree<int, CPUTask, \
		vtl::AVLBALANCE_USEPOINTERS>::iterator name

#define DEFINE_TASKMAP_ITERATOR(name) \
	vtl::AVLTree<int, TaskHandle>::iterator name

#define DEFINE_COLORMAP_ITERATOR(name) \
	vtl::AVLTree<int, TColor>::iterator name

#define DEFINE_FILTER_PIDMAP_ITERATOR(name) \
	QMap<int, int>::iterator name;

#define DEFINE_FILTER_EVENTMAP_ITERATOR(name) \
	QMap<event_t, event_t>::iterator name;

#define TSMAX(A, B) ((A) >= (B) ? A:B)
#define TSMIN(A, B) ((A) < (B) ? A:B)
#define TSABS(A) ((A) >= 0 ? A:-A)

/*  Don't increase this number, buy a system with fewer CPUs instead */
#define NR_CPUS_ALLOWED (256)
#define isValidCPU(CPU) (CPU < NR_CPUS_ALLOWED)

/* C++ syntax for calling the pointer to a member function for an object */
#define CALL_MEMBER_FN(ptrObject, ptrToMember) ((ptrObject)->*(ptrToMember))
/* C++ syntax for declaring a pointer to a member function */
#define DEFINE_MEMBER_FN(returntype, className, name) \
	returntype (className::* name)()

#define SPROL32(VALUE, N) \
	((VALUE << N) | (VALUE >> (32 - N)))

namespace TShark {

	enum CursorIdx {RED_CURSOR, BLUE_CURSOR, NR_CURSORS};

	/*
	 * This functions accepts ':' at the end of the value
	 * For example, 123.456: is ok. 123.456X is not ok if
	 * X is not a digit between 0-9 or a ':'
	 */
	static __always_inline double timeStrToDouble(char* str, bool &ok)
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

		if (*c != ':')
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
		unsigned int s = str->len;
		if (s < 1)
			return 0;
		uvalue.word8[3] = str->ptr[0];
		if (s < 4)
			return uvalue.word32;
		uvalue.word8[2] = str->ptr[s - 3];
		uvalue.word8[1] = str->ptr[s - 2];
		uvalue.word8[0] = str->ptr[s - 1];
		return uvalue.word32;
	}
}

#endif /* TRACESHARK_H */
