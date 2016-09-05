/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TRACEEVENT_H
#define TRACEEVENT_H

#include "parser/traceline.h"
#include "misc/tstring.h"

#include <cstdint>

/*
 * The definitions of the char values below comes from how the Linux kernel
 * outputs the task state when sched_switch events are emitted, see how the
 * sched_switch event is define in include/trace/events/sched.h in the Linux
 * kernel sources, here I am referring to version 4.7.2
 */

#define TASK_STATE_RUNNABLE		0
#define TASK_SCHAR_RUNNABLE		'R'

#define TASK_FLAG_INTERRUPTIBLE		1
#define TASK_CHAR_INTERRUPTIBLE		'S'

#define TASK_FLAG_UNINTERRUPTIBLE	2
#define TASK_CHAR_UNINTERRUPTIBLE	'D'

#define TASK_FLAG_STOPPED		4 /* Use with care */
#define TASK_CHAR_STOPPED		'T'

#define TASK_FLAG_TRACED		8 /* Use with care */
#define TASK_CHAR_TRACED		't'

#define TASK_FLAG_EXIT_DEAD		16
#define TASK_CHAR_EXIT_DEAD		'Z'

#define TASK_FLAG_EXIT_ZOMBIE		32
#define TASK_CHAR_EXIT_ZOMBIE		'X'

/* I guess this well never be used but include it for completeness sake */
#define TASK_FLAG_EXIT_TRACE		(16 | 32)

#define TASK_FLAG_DEAD			64
#define TASK_CHAR_DEAD			'x'

#define TASK_FLAG_WAKEKILL		128
#define TASK_CHAR_WAKEKILL		'K'

#define TASK_FLAG_WAKING		256
#define TASK_CHAR_WAKING		'W'

#define TASK_FLAG_PARKED		512
#define TASK_CHAR_PARKED		'P'

#define TASK_FLAG_NOLOAD		1024
#define TASK_CHAR_NOLOAD		'N'

#define TASK_STATE_PARSER_ERROR         2048

#define TASK_FLAG_MAX			4096
#define TASK_FLAG_PREEMPT      		TASK_FLAG_MAX
#define TASK_CHAR_PREEMPT		'+'

#define TASK_FLAG_MASK			(TASK_FLAG_MAX - 1)

#define TASK_CHAR_SEPARATOR		'|'

typedef uint32_t taskstate_t;

/*
 * This works because TASK_FLAG_MASK will mask out the preemption flag
 */
static __always_inline bool task_state_is_runnable(taskstate_t state)
{
	return ((state & TASK_FLAG_MASK) == TASK_STATE_RUNNABLE);
}

static __always_inline bool task_state_is_flag_set(taskstate_t state,
						   taskstate_t flag)
{
	return ((state & flag) != 0);
}

typedef enum {
	EVENT_ERROR = -1,
	CPU_FREQUENCY,
	CPU_IDLE,
	SCHED_MIGRATE_TASK,
	SCHED_SWITCH,
	SCHED_WAKEUP,
	SCHED_WAKEUP_NEW,
	SCHED_PROCESS_FORK,
	SCHED_PROCESS_EXIT,
	IRQ_HANDLER_ENTRY,
	IRQ_HANDLER_EXIT,
	NR_EVENTS,
} event_t;

#define EVENT_UNKNOWN (NR_EVENTS)

class StringTree;

class TraceEvent {
public:
	TString *taskName;
	unsigned int pid;
	unsigned int cpu;
	double time;
	int intArg;
	event_t type;
	TString **argv;
	unsigned int argc;
	/* postEventInfo most likely will contain a backtrace that will occur
	 * in perf traces after the event. Note that this TString will have a
	 * pointer to the read-only mapping of the trace file and thus it
	 * cannot be null terminated, instead we will have to rely on the len
	 * field to determine the length when using  this TString */
	TString *postEventInfo;
	TString *getEventName();
	static void setStringTree(StringTree *sTree);
private:
	/* This is supposed to be set to the stringtree that was involved in
	 * the parsing of the events, so that it can used to translate from
	 * event_t to event name */
	static StringTree *stringTree;
};

extern char *eventstrings[];

#endif
