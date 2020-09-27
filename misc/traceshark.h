// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
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

#include "gitversion.h"

#define TRACESHARK_VERSION_STRING "0.9.11-beta" TRACESHARK_GIT_VERSION

#include <QtCore>
#include <cstdint>
#include <cstdio>
#include "misc/tstring.h"
#include "vtl/compiler.h"

#define DEFAULT_MIGRATION_WIDTH (1)
#define MAX_MIGRATION_WIDTH (8)
#define MIN_MIGRATION_WIDTH (1)
#define DEFAULT_FREQ_LINE_WIDTH (2)
#define MIN_FREQ_LINE_WIDTH (1)
#define MAX_FREQ_LINE_WIDTH (8)
#define DEFAULT_IDLE_LINE_WIDTH (1)
#define MIN_IDLE_LINE_WIDTH (1)
#define MAX_IDLE_LINE_WIDTH (8)
#define MIN_LINE_WIDTH_OPENGL (1)
#define DEFAULT_LINE_WIDTH_OPENGL (2)
#define MAX_LINE_WIDTH_OPENGL (8)
#define DEFAULT_LINE_WIDTH (1)
#define DEFAULT_MAX_VRT_LATENCY (20)
#define MIN_MAX_VRT_LATENCY (1)
#define MAX_MAX_VRT_LATENCY (1000)

#ifdef QCUSTOMPLOT_USE_OPENGL
#define has_opengl() (true)
#else
#define has_opengl() (false)
#endif /* QCUSTOMPLOT_USE_OPENGL */

#define EVENT_MAX_NR_ARGS (128)

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui>
#else
#include <QtWidgets>
#endif

#define MAX_NR_MIGRATIONS (200000)

typedef enum : int {
	TRACE_TYPE_FTRACE = 0,
	TRACE_TYPE_PERF,
	TRACE_TYPE_UNKNOWN,
	TRACE_TYPE_MAX
} tracetype_t;

#define arraylen(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

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
#define NR_CPUS_ALLOWED (9999)
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

	union value32 {
		uint32_t word32;
		uint8_t word8[4];
	};

	vtl_always_inline uint32_t StrHash32(const TString *str)
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

	vtl_always_inline bool cmp_timespec(const struct timespec &s1,
					    const struct timespec &s2) {
		return s1.tv_sec == s2.tv_sec && s1.tv_nsec == s2.tv_nsec;
	}
}

#endif /* TRACESHARK_H */
