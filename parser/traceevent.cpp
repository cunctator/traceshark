// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2017, 2019
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "parser/traceevent.h"
#include "mm/stringtree.h"

/* Do not change the order of these without updating the enum in
 * traceevent.h */

static char cpufreqstr[] = "cpu_frequency";
static char cpuidlestr[] = "cpu_idle";
static char migratestr[] = "sched_migrate_task";
static char sswitchstr[] = "sched_switch";
static char swakeupstr[] = "sched_wakeup";
static char swaknewstr[] = "sched_wakeup_new";
static char swakingstr[] = "sched_waking";
static char sprforkstr[] = "sched_process_fork";
static char sprexitstr[] = "sched_process_exit";
static char irqhdlrent[] = "irq_handler_entry";
static char irqhdlrext[] = "irq_handler_exit";

char *eventstrings[NR_EVENTS] = {
	cpufreqstr,
	cpuidlestr,
	migratestr,
	sswitchstr,
	swakeupstr,
	swaknewstr,
	swakingstr,
	sprforkstr,
	sprexitstr,
	irqhdlrent,
	irqhdlrext
};

StringTree<> *TraceEvent::stringTree = nullptr;

void TraceEvent::setStringTree(StringTree<> *sTree)
{
	stringTree = sTree;
}

const StringTree<> *TraceEvent::getStringTree()
{
	return stringTree;
}

const TString *TraceEvent::getEventName() const
{
	return stringTree->stringLookup(TraceEvent::type);
}

const TString *TraceEvent::getEventName(event_t event)
{
	return stringTree->stringLookup(event);
}

int TraceEvent::getNrEvents()
{
	return stringTree->getMaxEvent() + 1;
}
