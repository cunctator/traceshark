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

#include "traceevent.h"

/* Do not change the order of these without updating the enum in
 * traceevent.h */

static char cpufreqstr[] = "cpu_frequency";
static char cpuidlestr[] = "cpu_idle";
static char migratestr[] = "sched_migrate_task";
static char sswitchstr[] = "sched_switch";
static char swakeupstr[] = "sched_wakeup";
static char swaknewstr[] = "sched_wakeup_new";
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
	sprforkstr,
	sprexitstr,
	irqhdlrent,
	irqhdlrext
};
