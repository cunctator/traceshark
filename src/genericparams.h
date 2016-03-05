/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef GENERICPARAMS_H
#define GENERICPARAMS_H

#include "ftraceparams.h"
#include "perfparams.h"

typedef enum {
	TRACE_TYPE_FTRACE = 0,
	TRACE_TYPE_PERF,
	TRACE_TYPE_MAX
} tracetype_t;

#define TRACE_TYPE_NONE (TRACE_TYPE_MAX)

static __always_inline bool tracetype_is_valid(tracetype_t ttype)
{
	return ttype == TRACE_TYPE_FTRACE || ttype == TRACE_TYPE_PERF;		
}

#define DECLARE_GENERIC_TRACEFN(FNAME, RETTYPE)			   \
static __always_inline RETTYPE FNAME(tracetype_t tt,	  	   \
				     const TraceEvent &event)	   \
{							           \
	if (tt == TRACE_TYPE_FTRACE)				   \
		return ftrace_##FNAME(event);			   \
	else /* (tt == TRACE_TYPE_PERF) */			   \
		return perf_##FNAME(event);			   \
}

#define DECLARE_GENERIC_TRACEFN_POOL(FNAME, RETTYPE)		   \
static __always_inline RETTYPE FNAME(tracetype_t tt,	  	   \
				     const TraceEvent &event,	   \
				     MemPool *pool)		   \
{							           \
	if (tt == TRACE_TYPE_FTRACE)				   \
		return ftrace_##FNAME(event, pool);		   \
	else /* (tt == TRACE_TYPE_PERF) */			   \
		return perf_##FNAME(event, pool);		   \
}

DECLARE_GENERIC_TRACEFN(cpufreq_args_ok, bool)
DECLARE_GENERIC_TRACEFN(cpufreq_cpu, unsigned int)
DECLARE_GENERIC_TRACEFN(cpufreq_freq, unsigned int)

DECLARE_GENERIC_TRACEFN(cpuidle_args_ok, bool)
DECLARE_GENERIC_TRACEFN(cpuidle_cpu, unsigned int)
DECLARE_GENERIC_TRACEFN(cpuidle_state, int32_t)

DECLARE_GENERIC_TRACEFN(sched_migrate_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_migrate_destCPU, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_migrate_origCPU, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_migrate_prio, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_migrate_pid, unsigned int)

DECLARE_GENERIC_TRACEFN(sched_switch_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_switch_newprio, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_switch_newpid, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_switch_state, taskstate_t)
DECLARE_GENERIC_TRACEFN(sched_switch_oldprio, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_switch_oldpid, unsigned int)
DECLARE_GENERIC_TRACEFN_POOL(sched_switch_oldname_strdup, char *)
DECLARE_GENERIC_TRACEFN_POOL(sched_switch_newname_strdup, char *)

DECLARE_GENERIC_TRACEFN(sched_wakeup_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_wakeup_cpu, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_wakeup_success, bool)
DECLARE_GENERIC_TRACEFN(sched_wakeup_prio, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_wakeup_pid, unsigned int)
DECLARE_GENERIC_TRACEFN_POOL(sched_wakeup_name_strdup, char *)

DECLARE_GENERIC_TRACEFN(sched_process_fork_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_process_fork_childpid, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_process_fork_parent_pid, unsigned int)
DECLARE_GENERIC_TRACEFN_POOL(sched_process_fork_childname_strdup, char *)

DECLARE_GENERIC_TRACEFN(sched_process_exit_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_process_exit_pid, unsigned int)

DECLARE_GENERIC_TRACEFN(irq_handler_entry_args_ok, bool)
DECLARE_GENERIC_TRACEFN(irq_handler_entry_irq, unsigned int)

DECLARE_GENERIC_TRACEFN(irq_handler_exit_args_ok, bool)
DECLARE_GENERIC_TRACEFN(irq_handler_exit_irq, unsigned int)

#endif /* GENERICPARAMS_H */
