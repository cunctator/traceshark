/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2014, 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "traceline.h"
#include "tstring.h"

typedef enum {
	CPU_FREQUENCY = 0,
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

class TraceEvent {
public:
	TString *taskName;
	unsigned int pid;
	unsigned int cpu;
	double time;
	TString *eventName;
	event_t type;
	TString **argv;
	unsigned int argc;
};

#endif
