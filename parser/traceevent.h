// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2021, 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TRACEEVENT_H
#define TRACEEVENT_H

#include "mm/stringtree.h"
#include "parser/traceline.h"
#include "misc/tstring.h"
#include "misc/types.h"
#include "vtl/compiler.h"
#include "vtl/time.h"

#include <cstdint>

/*
 * The definitions of the char values below comes from how the Linux kernel
 * outputs the task state when sched_switch events are emitted, see how the
 * sched_switch event is defined in include/trace/events/sched.h in the Linux
 * kernel sources, here I am referring to versions 4.7 - 6.7-rc7
 *
 * You can also checkout include/linux/sched.h
 */

#define TASK_STATE_RUNNABLE			0x0000
#define TASK_SCHAR_RUNNABLE			'R'

#define TASK_FLAG_INTERRUPTIBLE			0x0001
#define TASK_CHAR_INTERRUPTIBLE			'S'

#define TASK_FLAG_UNINTERRUPTIBLE		0x0002
#define TASK_CHAR_UNINTERRUPTIBLE		'D'

#define TASK_FLAG_STOPPED			0x0004 /* Use with care */
#define TASK_CHAR_STOPPED			'T'

#define TASK_FLAG_TRACED			0x0008 /* Use with care */
#define TASK_CHAR_TRACED			't'

/*
 * The meaning of 'Z' and 'X' has been swapped somewhere between v4.7 and
 * 6.7-rc7
 */
#define TASK_FLAG_EXIT_DEAD_OR_ZOMBIE		0x0010
#define TASK_CHAR_EXIT_DEAD_OR_ZOMBIE		'Z'

#define TASK_FLAG_EXIT_ZOMBIE_OR_DEAD	       0x0020
#define TASK_CHAR_EXIT_ZOMBIE_OR_DEAD		'X'

/* I guess this well never be used but include it for completeness sake */
#define TASK_FLAG_EXIT_TRACE			(0x0010 | 0x0020)

#define TASK_FLAG_DEAD_V1			0x0040
#define TASK_CHAR_DEAD_V1			'x'

#define TASK_FLAG_DEAD_V2			0x0080
#define TASK_CHAR_DEAD_V2			'I'

/* This is apparently deprecated */
#define TASK_FLAG_WAKEKILL_OLD			0x0100
#define TASK_CHAR_WAKEKILL_OLD			'K'

/* This too is deprecated */
#define TASK_FLAG_WAKING_OLD			0x0200
#define TASK_CHAR_WAKING_OLD			'W'

#define TASK_FLAG_PARKED			0x0400
#define TASK_CHAR_PARKED			'P'

/* This too is deprecated */
#define TASK_FLAG_NOLOAD_OLD			0x0800
#define TASK_CHAR_NOLOAD_OLD			'N'

#define TASK_FLAG_UNKNOWN 		        0x1000

#define TASK_STATE_PARSER_ERROR			0x2000

#define TASK_FLAG_MAX				0x4000
#define TASK_FLAG_PREEMPT    	  		TASK_FLAG_MAX
#define TASK_CHAR_PREEMPT			'+'

#define TASK_FLAG_MASK				(TASK_FLAG_MAX - 1)

#define TASK_CHAR_SEPARATOR			'|'

/*
 * This works because TASK_FLAG_MASK will mask out the preemption flag
 */
static vtl_always_inline bool task_state_is_runnable(taskstate_t state)
{
	return ((state & TASK_FLAG_MASK) == TASK_STATE_RUNNABLE);
}

static vtl_always_inline bool task_state_is_flag_set(taskstate_t state,
						     taskstate_t flag)
{
	return ((state & flag) != 0);
}

#define EVENT_UNKNOWN (NR_EVENTS)

class Chunk;

class TraceEvent {
public:
	const TString *taskName;
	int pid;
	unsigned int cpu;
	const TString *flagstr;
	vtl::Time time;
	int intArg;
	event_t type;
	const TString **argv;
	int argc;

	/*
	 * postEventInfo most likely will contain a backtrace that will occur
	 * in perf traces after the event. Note that this TString will have a
	 * pointer to the read-only mapping of the trace file and thus it
	 * cannot be null terminated, instead we will have to rely on the len
	 * field to determine the length when using  this TString.
	 */
	Chunk *postEventInfo;

	const TString *getEventName() const;
	void clear();
	static const TString *getEventName(event_t event);
	static void setStringTree(StringTree<> *sTree);
	static const StringTree<> *getStringTree();
	static int getNrEvents();
private:
	/* This is supposed to be set to the stringtree that was involved in
	 * the parsing of the events, so that it can used to translate from
	 * event_t to event name */
	static StringTree<> *stringTree;
};

extern const char * const eventstrings[];

#endif /* TRACEEVENT_H */
