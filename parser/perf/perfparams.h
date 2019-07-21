// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef PERFPARAMS_H
#define PERFPARAMS_H

#include "mm/stringpool.h"
#include "parser/traceevent.h"
#include "parser/paramhelpers.h"
#include "parser/perf/helpers.h"
#include "misc/errors.h"
#include "misc/string.h"
#include "misc/traceshark.h"
#include <cstring>
#include <cstdint>

#define perf_cpufreq_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_cpufreq_cpu(EVENT)     (uint_after_pfix(EVENT, 1, FREQ_CPUID_PFIX))
#define perf_cpufreq_freq(EVENT)    (uint_after_pfix(EVENT, 0, FREQ_STATE_PFIX))

#define perf_cpuidle_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_cpuidle_cpu(EVENT)     (uint_after_pfix(EVENT, 1, IDLE_CPUID_PFIX))
static __always_inline int perf_cpuidle_state(const TraceEvent &event)
{
	int32_t state;
	uint32_t ustate;
	ustate = uint_after_pfix(event, 0, IDLE_STATE_PFIX);

	/* the string is a signed printed as unsigned :) */
	state = *((int*) &ustate);

	return state;
}

/* Normally we would require >= 5 but we don't need the first comm= arg */
#define perf_sched_migrate_args_ok(EVENT) (EVENT.argc >= 4)
#define perf_sched_migrate_destCPU(EVENT) (uint_after_pfix(EVENT,	\
							   EVENT.argc - 1, \
						           MIGRATE_DEST_PFIX))
#define perf_sched_migrate_origCPU(EVENT) (uint_after_pfix(EVENT,	\
							   EVENT.argc - 2, \
						           MIGRATE_ORIG_PFIX))
#define perf_sched_migrate_prio(EVENT) (uint_after_pfix(EVENT,		\
							EVENT.argc - 3, \
							MIGRATE_PRIO_PFIX))
#define perf_sched_migrate_pid(EVENT) (int_after_pfix(EVENT,		\
						      EVENT.argc - 4,	\
						      MIGRATE_PID_PFIX))


static __always_inline bool perf_sched_switch_parse(const TraceEvent &event,
						    sched_switch_handle& handle)
{
	int i;

	i = _perf_sched_switch_find_arrow(event, handle.perf.is_distro_style);
	if( i <= 0)
		return false;
	handle.perf.index = i;
	return true;
}

static __always_inline taskstate_t
perf_sched_switch_handle_state(const TraceEvent &event,
			       const sched_switch_handle &handle)
{
	int i, j;
	TString st;

	i = handle.perf.index - 1;

	if (handle.perf.is_distro_style)
		goto distro_style;

	/* This is the regular format */
	for (; i >= 0; i--) {
		const TString *t = event.argv[i];
		if (!prefixcmp(t->ptr, SWITCH_PSTA_PFIX)) {
			for (j = t->len - 2; j > 0; j--) {
				if (t->ptr[j] == '=') {
					st.len = t->len - 1 - j;
					st.ptr = t->ptr + j + 1;
					return  _sched_state_from_tstring(&st);
				}
			}
		}
	}
	return TASK_STATE_PARSER_ERROR;

distro_style:
	/* This is the distro format */
	const TString *stateArgStr = event.argv[i];
	if (stateArgStr->len == 1 || stateArgStr->len == 2) {
		return _sched_state_from_tstring(stateArgStr);
	}
	return TASK_STATE_PARSER_ERROR;
}

static __always_inline int
perf_sched_switch_handle_oldpid(const TraceEvent &event,
				const sched_switch_handle &handle)
{
	int idx = handle.perf.index;
	int oldpid;

	if (handle.perf.is_distro_style) {
		oldpid = int_after_char(event, idx - 3, ':');
	} else {
		oldpid = _perf_sched_switch_handle_oldpid_newformat(event,
								    handle);
	}
	return oldpid;
}

static __always_inline int
perf_sched_switch_handle_newpid(const TraceEvent &event,
				const sched_switch_handle &handle)
{
	int newpid;

	if (handle.perf.is_distro_style) {
		newpid = int_after_char(event, event.argc - 2, ':');
	} else {
		//newpid = int_after_char(event, event.argc - 2, '=');
		newpid = _perf_sched_switch_handle_newpid_newformat(event,
								    handle);
	}
	return newpid;
}

static __always_inline unsigned int
perf_sched_switch_handle_oldprio(const TraceEvent &event,
				 const sched_switch_handle &handle)
{
	int i = handle.perf.index;

	if (i <= 3)
		return ABSURD_UNSIGNED;

	if (handle.perf.is_distro_style) {
		return param_inside_braces(event, i - 2);
	} else {
		/*
		 * Since this function is not used, we just assume that the
		 * argument is in the usual place.
		 */
		return uint_after_char(event, i - 2, '=');
	}
}

static __always_inline unsigned int
perf_sched_switch_handle_newprio(const TraceEvent &event,
				 const sched_switch_handle &handle)
{
	if (handle.perf.is_distro_style) {
		return param_inside_braces(event, event.argc - 1);
	} else {
		/*
		 * Since this function is not used, we just assume that the
		 * argument is in the usual place.
		 */
		return uint_after_char(event, event.argc - 1, '=');
	}
}

static __always_inline const char *
_perf_sched_switch_handle_newname_strdup(const TraceEvent &event,
					 StringPool<> *pool,
					 const sched_switch_handle &handle)
{
	int i;
	int beginidx;
	int endidx;
	int len = 0;
	char *c;
	bool ok;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	i = handle.perf.index;


	if (!handle.perf.is_distro_style) {
		const TString *first = event.argv[i + 1];
		beginidx = i + 2;
		endidx = event.argc - 3;
		_copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN,
					  ok);
		if (!ok)
			return NullStr;

		merge_args_into_cstring_nullterminate(event, beginidx, endidx,
						      c, len, TASKNAME_MAXLEN,
						      ok);
		if (!ok)
			return NullStr;
	} else {
		const TString *last = event.argv[event.argc - 2];
		beginidx = i + 1;
		endidx = event.argc - 3;

		merge_args_into_cstring(event, beginidx, endidx,
					c, len, TASKNAME_MAXLEN,
					ok);
		if (!ok)
			return NullStr;

		_copy_tstring_before_char(last, ':', c, len, TASKNAME_MAXLEN,
					  ok);
		if (!ok)
			return NullStr;
	}

	ts.len = len;
	retstr = pool->allocString(&ts, 0);
	if (retstr == nullptr)
		return NullStr;

	return retstr->ptr;
}

const char *
perf_sched_switch_handle_newname_strdup(const TraceEvent &event,
					StringPool<> *pool,
					const sched_switch_handle &handle);

static __always_inline const char *
_perf_sched_switch_handle_oldname_strdup(const TraceEvent &event,
					 StringPool<> *pool,
					 const sched_switch_handle &handle)
{
	int i;
	int beginidx;
	int endidx;
	int len = 0;
	char *c;
	bool ok;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	i = handle.perf.index;

	if (!handle.perf.is_distro_style) {
		const TString *first = event.argv[0];
		beginidx = 1;
		endidx = i - 4;
		_copy_tstring_after_char(first, '=', c, len,
					  TASKNAME_MAXLEN, ok);
		if (!ok)
			return NullStr;

		merge_args_into_cstring_nullterminate(event, beginidx, endidx,
						      c, len, TASKNAME_MAXLEN,
						      ok);
		if (!ok)
			return NullStr;

	} else {
		const TString *last = event.argv[i - 3];
		beginidx = 0;
		endidx = i - 4;

		merge_args_into_cstring(event, beginidx, endidx,
					c, len, TASKNAME_MAXLEN,
					ok);
		if (!ok)
			return NullStr;

		_copy_tstring_before_char(last, ':',
					   c, len, TASKNAME_MAXLEN,
					   ok);

		if (!ok)
			return NullStr;
	}
	ts.len = len;
	retstr = pool->allocString(&ts, 0);
	if (retstr == nullptr)
		return NullStr;

	return retstr->ptr;
}

const char *
perf_sched_switch_handle_oldname_strdup(const TraceEvent &event,
					StringPool<> *pool,
					const sched_switch_handle &handle);

/*
 * These functions for sched_wakeup assumes that the format is either the "old"
 * or "new", that
 * is:
 * Xorg   829 [003]  2726.130986: sched:sched_wakeup: comm=spotify pid=9288 \
 * prio=120 success=1 target_cpu=000
 *
 * ..or:
 * Xorg   829 [003]  2726.130986: sched:sched_wakeup: comm=spotify pid=9288 \
 * prio=120 target_cpu=000
 */

#define perf_sched_wakeup_args_ok(EVENT) (EVENT.argc >= 4)

/* The last argument is target_cpu, regardless of old or new */
#define perf_sched_wakeup_cpu(EVENT) (uint_after_pfix(EVENT, EVENT.argc - 1, \
						      WAKE_TCPU_PFIX))

static __always_inline bool perf_sched_wakeup_success(const TraceEvent &event)
{
	const TString *ss = event.argv[event.argc - 2];
	int i;

	if (prefixcmp(ss->ptr, WAKE_SUCC_PFIX) == 0)
		return int_after_char(event, event.argc - 2, '=');

	for (i = 0; i < event.argc; i++) {
		if (prefixcmp(event.argv[i]->ptr, WAKE_SUCC_PFIX) == 0)
			return int_after_char(event, i, '=');
	}
	/* Assume that wakeup is successful if no success field is found */
	return true;
}

static __always_inline unsigned int
perf_sched_wakeup_prio(const TraceEvent &event)
{
	unsigned int newidx = event.argc - 2;
	/* Check if we are on the new format */
	if (!prefixcmp(event.argv[newidx]->ptr, WAKE_PRIO_PFIX)) {
		return uint_after_char(event, newidx, '=');
	}

	/* Assume that this is the old format */
	return uint_after_pfix(event, event.argc - 3, WAKE_PRIO_PFIX);
}

static __always_inline int perf_sched_wakeup_pid(const TraceEvent &event)
{
	int newidx = event.argc - 3;
	int oldidx;

	/* Check if we are on the new format */
	if (!prefixcmp(event.argv[newidx]->ptr, WAKE_PID_PFIX)) {
		return int_after_char(event, newidx, '=');
	}

	const TString *priostr = event.argv[event.argc - 3];
	if (is_param_inside_braces(priostr)) {
		/* This must be distro format */
	        int name_pid_idx =event.argc - 4;
		return int_after_char(event, name_pid_idx, ':');
	}

	/* Assume that this is the old format */
	oldidx = event.argc - 4;
	return int_after_char(event, oldidx, '=');
}

static __always_inline const char *
_perf_sched_wakeup_name_strdup(const TraceEvent &event, StringPool<> *pool)
{
	int i;
	int beginidx;
	int endidx;
	int len = 0;
	char *c;
	const TString *first;
	bool ok;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	/* Find the index of the pid=... that is followed by prio= */
	for (i = 1; i <= event.argc - 2; i++) {
		char *c1 = event.argv[i + 0]->ptr;
		char *c2 = event.argv[i + 1]->ptr;
		if (!prefixcmp(c1, WAKE_PID_PFIX) &&
		    !prefixcmp(c2, WAKE_PRIO_PFIX))
			break;
	}
	if (!(i <= event.argc - 2)) {
		beginidx = 0;
		endidx = event.argc - 5;
		merge_args_into_cstring(event, beginidx, endidx, c, len,
					TASKNAME_MAXLEN, ok);
		if (!ok)
			return NullStr;

		first = event.argv[0];
		_copy_tstring_before_char(first, ':', c, len, TASKNAME_MAXLEN,
					   ok);
		if (!ok)
			return NullStr;
	} else {

		beginidx = 1;
		endidx = i - 1;

		first = event.argv[0];
		_copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN,
					  ok);
		if (!ok)
			return NullStr;

		merge_args_into_cstring_nullterminate(event, beginidx, endidx,
						      c, len, TASKNAME_MAXLEN,
						      ok);
		if (!ok)
			return NullStr;
	}

	ts.len = len;
	retstr = pool->allocString(&ts, 0);
	if (retstr == nullptr)
		return NullStr;

	return retstr->ptr;
}

const char *perf_sched_wakeup_name_strdup(const TraceEvent &event,
					  StringPool<> *pool);

#define perf_sched_process_fork_args_ok(EVENT) (EVENT.argc >= 4)

static __always_inline
int perf_sched_process_fork_childpid(const TraceEvent &event) {
	int i;
	int endidx = event.argc - 1;
	int guessidx = endidx;

	if (likely(prefixcmp(event.argv[guessidx]->ptr, "child_pid=") == 0))
		return int_after_char(event, guessidx, '=');

	for (i = endidx - 1; i > 0; i--) {
		if (prefixcmp(event.argv[i]->ptr, "child_pid=") == 0)
			break;
	}
	if (i < 0)
		return ABSURD_INT;

	return int_after_char(event, i, '=');
}

static __always_inline
int perf_sched_process_fork_parent_pid(const TraceEvent &event) {
	int i;
	int endidx = event.argc - 1;
	int guessidx = 1;

	if (prefixcmp(event.argv[guessidx]->ptr, "pid=") == 0 &&
	    prefixcmp(event.argv[guessidx + 1]->ptr, "child_comm=") == 0)
		return int_after_char(event, guessidx, '=');

	for (i = guessidx; i < endidx - 1; i++) {
		if (prefixcmp(event.argv[i]->ptr, "pid=") == 0 &&
		    prefixcmp(event.argv[i + 1]->ptr, "child_comm=") == 0)
			break;
	}
	if (i >= endidx)
		return ABSURD_INT;

	return int_after_char(event, i, '=');
}

static __always_inline const char *
_perf_sched_process_fork_childname_strdup(const TraceEvent &event,
					  StringPool<> *pool)
{
	int i;
	int beginidx;
	const int endidx = event.argc - 2;
	char *c;
	int len = 0;
	bool ok;
	const TString *first;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	for (i = 2; i <= endidx; i++) {
		if (!prefixcmp(event.argv[i - 1]->ptr, "pid=") &&
		    !prefixcmp(event.argv[i]->ptr,     "child_comm="))
			break;
	}
	if (i > endidx)
		return NullStr;
	beginidx = i + 1;

	first = event.argv[i];
	_copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN, ok);
	if (!ok)
		return NullStr;

	merge_args_into_cstring_nullterminate(event, beginidx, endidx, c, len,
					      TASKNAME_MAXLEN, ok);
	if (!ok)
		return NullStr;

	ts.len = len;
	retstr = pool->allocString(&ts, 0);
	if (retstr == nullptr)
		return NullStr;

	return retstr->ptr;
}

const char *perf_sched_process_fork_childname_strdup(const TraceEvent &event,
						     StringPool<> *pool);

/* Normally should be >= 3 but we don't care if the prio argument is missing */
#define perf_sched_process_exit_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_sched_process_exit_pid(EVENT) \
	(int_after_pfix(EVENT, EVENT.argc - 2, EXIT_PID_PFIX))

/*
 * As a first approximation we assume that waking events and wakeup can be
 * parsed by the same code, through all kernel version where traceshark is
 * supposed to work.
 */
#define perf_sched_waking_args_ok(EVENT) (perf_sched_wakeup_args_ok(EVENT))
#define perf_sched_waking_cpu(EVENT) (perf_sched_wakeup_cpu(EVENT))
#define perf_sched_waking_prio(EVENT) (perf_sched_wakeup_prio(EVENT))
#define perf_sched_waking_pid(EVENT) (perf_sched_wakeup_pid(EVENT))
#define perf_sched_waking_name_strdup(EVENT, POOL) \
	perf_sched_wakeup_name_strdup(EVENT, POOL)

#endif /* PERFPARAMS_H*/
