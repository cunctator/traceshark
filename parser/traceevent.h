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

typedef enum {
	TASK_STATE_UNKNOWN = 0,
	TASK_STATE_RUNNABLE,
	TASK_STATE_NOT_RUNNABLE,
} taskstate_t;

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
