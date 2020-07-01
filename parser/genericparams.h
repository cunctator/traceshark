// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef GENERICPARAMS_H
#define GENERICPARAMS_H

#include "parser/ftrace/ftraceparams.h"
#include "parser/perf/perfparams.h"
#include "misc/traceshark.h"
#include "vtl/compiler.h"

static vtl_always_inline bool tracetype_is_valid(tracetype_t ttype)
{
	return ttype == TRACE_TYPE_FTRACE || ttype == TRACE_TYPE_PERF;		
}

#define DECLARE_GENERIC_TRACEFN(FNAME, RETTYPE)			   \
static vtl_always_inline RETTYPE FNAME(tracetype_t tt,	  	   \
				       const TraceEvent &event)	   \
{							           \
	if (tt == TRACE_TYPE_FTRACE)				   \
		return ftrace_##FNAME(event);			   \
	else /* (tt == TRACE_TYPE_PERF) */			   \
		return perf_##FNAME(event);			   \
}

#define DECLARE_GENERIC_TRACEFN_POOL(FNAME, RETTYPE)		   \
static vtl_always_inline RETTYPE FNAME(tracetype_t tt,	  	   \
				       const TraceEvent &event,	   \
				       StringPool<> *pool)	   \
{							           \
	if (tt == TRACE_TYPE_FTRACE)				   \
		return ftrace_##FNAME(event, pool);		   \
	else /* (tt == TRACE_TYPE_PERF) */			   \
		return perf_##FNAME(event, pool);		   \
}

#define DECLARE_GENERIC_TRACEFN_HANDLE(FNAME, RETTYPE, HANDLETYPE)	\
	static vtl_always_inline RETTYPE FNAME(tracetype_t tt,		\
					       const TraceEvent &event,	\
					       HANDLETYPE handle	\
		)							\
	{								\
		if (tt == TRACE_TYPE_FTRACE)				\
			return ftrace_##FNAME(event, handle);		\
		else /* (tt == TRACE_TYPE_PERF) */			\
			return perf_##FNAME(event, handle);		\
	}


#define DECLARE_GENERIC_TRACEFN_POOL_HANDLE(FNAME, RETTYPE, HANDLETYPE)	\
	static vtl_always_inline RETTYPE FNAME(tracetype_t tt,		\
					       const TraceEvent &event,	\
					       StringPool<> *pool,	\
					       HANDLETYPE handle)	\
	{								\
		if (tt == TRACE_TYPE_FTRACE)				\
			return ftrace_##FNAME(event, pool, handle);	\
		else /* (tt == TRACE_TYPE_PERF) */			\
			return perf_##FNAME(event, pool, handle);	\
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
DECLARE_GENERIC_TRACEFN(sched_migrate_pid, int)

DECLARE_GENERIC_TRACEFN_HANDLE(sched_switch_parse,		\
			       bool,				\
			       sched_switch_handle&)
DECLARE_GENERIC_TRACEFN_HANDLE(sched_switch_handle_newpid, int, \
			       const sched_switch_handle&)
DECLARE_GENERIC_TRACEFN_HANDLE(sched_switch_handle_state,   \
			       taskstate_t,		    \
			       const sched_switch_handle&)
DECLARE_GENERIC_TRACEFN_HANDLE(sched_switch_handle_oldpid, int,	\
			       const sched_switch_handle&)
DECLARE_GENERIC_TRACEFN_POOL_HANDLE(sched_switch_handle_oldname_strdup, \
				    const char *,			\
				    const sched_switch_handle&)
DECLARE_GENERIC_TRACEFN_POOL_HANDLE(sched_switch_handle_newname_strdup, \
				    const char *,			\
				    const sched_switch_handle&)

DECLARE_GENERIC_TRACEFN(sched_wakeup_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_wakeup_cpu, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_wakeup_success, bool)
DECLARE_GENERIC_TRACEFN(sched_wakeup_prio, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_wakeup_pid, int)
DECLARE_GENERIC_TRACEFN_POOL(sched_wakeup_name_strdup, const char *)

DECLARE_GENERIC_TRACEFN(sched_process_fork_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_process_fork_childpid, int)
DECLARE_GENERIC_TRACEFN(sched_process_fork_parent_pid, int)
DECLARE_GENERIC_TRACEFN_POOL(sched_process_fork_childname_strdup, const char *)

DECLARE_GENERIC_TRACEFN(sched_process_exit_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_process_exit_pid, int)

DECLARE_GENERIC_TRACEFN(sched_waking_args_ok, bool)
DECLARE_GENERIC_TRACEFN(sched_waking_cpu, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_waking_prio, unsigned int)
DECLARE_GENERIC_TRACEFN(sched_waking_pid, int)
DECLARE_GENERIC_TRACEFN_POOL(sched_waking_name_strdup, const char *)

#endif /* GENERICPARAMS_H */
