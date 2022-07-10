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

#ifndef FTRACEPARAMS_H
#define FTRACEPARAMS_H

#include <cstring>
#include <cstdint>

#include "vtl/compiler.h"
#include "mm/stringpool.h"
#include "parser/traceevent.h"
#include "parser/paramhelpers.h"
#include "misc/errors.h"
#include "misc/string.h"
#include "misc/traceshark.h"

#define ftrace_cpufreq_args_ok(EVENT) (EVENT.argc >= 2)
#define ftrace_cpufreq_cpu(EVENT) (uint_after_char(EVENT, 1, '='))
#define ftrace_cpufreq_freq(EVENT) (uint_after_char(EVENT, 0, '='))

#define ftrace_cpuidle_args_ok(EVENT) (EVENT.argc >= 2)
#define ftrace_cpuidle_cpu(EVENT) (uint_after_char(EVENT, 1, '='))
static vtl_always_inline int ftrace_cpuidle_state(const TraceEvent &event)
{
	int32_t state;
	uint32_t ustate;
	ustate = uint_after_char(event, 0, '=');
	state = *((int*) &ustate); /* the string is a signed printed as 
				    * unsigned :) */
	return state;
}

#define ftrace_sched_migrate_args_ok(EVENT) (EVENT.argc >= 5)
#define ftrace_sched_migrate_destCPU(EVENT) (uint_after_char(EVENT, \
							     EVENT.argc - 1, \
							     '='))
#define ftrace_sched_migrate_origCPU(EVENT) (uint_after_char(EVENT, \
							     EVENT.argc - 2, \
							     '='))
#define ftrace_sched_migrate_prio(EVENT) (uint_after_char(EVENT,	\
							  EVENT.argc - 3, \
							  '='))
#define ftrace_sched_migrate_pid(EVENT) (int_after_char(EVENT, \
							EVENT.argc - 4, \
							'='))
static vtl_always_inline bool
ftrace_sched_switch_parse(const TraceEvent &event, sched_switch_handle& handle)
{
	int i;
	ftraceschedformat_t format = FTRACE_SCHED_OLD;

	if (event.argc < 6)
		return false;

	/* Find the index of the '==>' */
	for (i = 3; i < event.argc - 2; i++) {
		if (!isArrowStr(event.argv[i]))
			continue;
		const char *c1 = event.argv[i - 3]->ptr;
		const char *c2 = event.argv[i - 2]->ptr;
		const char *c3 = event.argv[i - 1]->ptr;
		const char *c4 = event.argv[i + 1]->ptr;
		/* Check if it is the new format */
		if (!prefixcmp(c1, SWITCH_PPID_PFIX) &&
		    !prefixcmp(c2, SWITCH_PPRI_PFIX) &&
		    !prefixcmp(c3, SWITCH_PSTA_PFIX) &&
		    !prefixcmp(c4, SWITCH_NCOM_PFIX)) {
			format = FTRACE_SCHED_NEW;
			break;
		} else {
			/*
			 * Check if if is the old style format. We do this by
			 * checking that the priority fields have their
			 * [] braces
			 */
			const TString *t1 = event.argv[i - 2];
			const TString *t2 = event.argv[event.argc - 1];
			if (is_param_inside_braces(t1) &&
			    is_param_inside_braces(t2)) {
				break;
			}
		}
	}
	if (!(i < event.argc))
		return false;

	handle.ftrace.index = i;
	handle.ftrace.format = format;
	return true;
}

static vtl_always_inline const char *
ftrace_sched_switch_handle_newname_strdup_(const TraceEvent &event,
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
	/*
	 * This will copy the first part of the name, that is the portion
	 * of first that is suceeded by the '=' character.
	 */

	if (handle.ftrace.format == FTRACE_SCHED_NEW) {
		const TString * first = event.argv[i + 1];
		beginidx = i + 2;
		endidx = event.argc - 3;
		copy_tstring_after_char_(first, '=', c, len, TASKNAME_MAXLEN,
					 ok);
		if (!ok)
			return NullStr;

		merge_args_into_cstring_nullterminate(event, beginidx, endidx,
						      c, len, TASKNAME_MAXLEN,
						      ok);
		if (!ok)
			return NullStr;
	} else {
		/* handle.ftrace.format == FTRACE_SCHED_OLD */
		const TString * last = event.argv[event.argc - 2];
		beginidx = i + 1;
		endidx = event.argc - 3;

		merge_args_into_cstring(event, beginidx, endidx,
					c, len, TASKNAME_MAXLEN,
					ok);
		if (!ok)
			return NullStr;

		copy_tstring_before_char_(last, ':', c, len, TASKNAME_MAXLEN,
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
ftrace_sched_switch_handle_newname_strdup(const TraceEvent &event,
					  StringPool<> *pool,
					  const sched_switch_handle &handle);


static vtl_always_inline const char *
ftrace_sched_switch_handle_oldname_strdup_(const TraceEvent &event,
					   StringPool<> *pool,
					   const sched_switch_handle &handle)
{
	int i;
	int beginidx;
	int endidx;
	int len = 0;
	char *c;
	const TString *first;
	const TString *last;
	bool ok;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	i = handle.perf.index;
	/*
	 * This will copy the first part of the name, that is the portion
	 * of first that is suceeded by the '=' character
	 */

	if (handle.ftrace.format == FTRACE_SCHED_NEW) {
		first = event.argv[0];
		beginidx = 1;
		endidx = i - 4;
		copy_tstring_after_char_(first, '=', c, len,
					 TASKNAME_MAXLEN, ok);
		if (!ok)
			return NullStr;

		merge_args_into_cstring_nullterminate(event, beginidx, endidx,
						      c, len, TASKNAME_MAXLEN,
						      ok);
		if (!ok)
			return NullStr;

	} else {
		/* handle.ftrace.format == FTRACE_SCHED_NEW */
		last = event.argv[i - 3];
		beginidx = 0;
		endidx = i - 4;

		merge_args_into_cstring(event, beginidx, endidx,
					c, len, TASKNAME_MAXLEN,
					ok);
		if (!ok)
			return NullStr;

		copy_tstring_before_char_(last, ':',
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
ftrace_sched_switch_handle_oldname_strdup(const TraceEvent &event,
					  StringPool<> *pool,
					  const sched_switch_handle &handle);

static vtl_always_inline int
ftrace_sched_switch_handle_newpid(const TraceEvent &event,
				  const sched_switch_handle &handle)
{
	if (handle.ftrace.format == FTRACE_SCHED_OLD)
		return int_after_char(event, event.argc - 2, ':');
	else
		return int_after_char(event, event.argc - 2, '=');
}

static vtl_always_inline int
ftrace_sched_switch_handle_oldpid(const TraceEvent &event,
				  const sched_switch_handle &handle)
{
	int i = handle.ftrace.index;
	if (handle.ftrace.format == FTRACE_SCHED_OLD)
		return int_after_char(event, i - 3, ':');
	else
		return int_after_char(event, i - 3, '=');
}

static vtl_always_inline taskstate_t
ftrace_sched_switch_handle_state(const TraceEvent &event,
				 const sched_switch_handle &handle)
{
	int j;
	TString stateStr;

	int i = handle.ftrace.index;
	const TString *stateArgStr = event.argv[i - 1];

	if (handle.ftrace.format == FTRACE_SCHED_NEW) {
		for (j = stateArgStr->len - 2; j > 0; j--) {
			if (stateArgStr->ptr[j] == '=') {
				stateStr.len = stateArgStr->len - 1 - j;
				stateStr.ptr = stateArgStr->ptr + j + 1;
				return  sched_state_from_tstring_(&stateStr);
			}
		}
	} else if (stateArgStr->len == 1 || stateArgStr->len == 2)
		return sched_state_from_tstring_(stateArgStr);

	return TASK_STATE_PARSER_ERROR;
}

static vtl_always_inline taskstate_t
ftrace_sched_switch_handle_oldprio(const TraceEvent &event,
				   const sched_switch_handle &handle)
{
	int i = handle.ftrace.index;
	if (handle.ftrace.format == FTRACE_SCHED_NEW)
		return uint_after_char(event, i - 2, '=');
	else
		return param_inside_braces(event, i - 2);
}

static vtl_always_inline taskstate_t
ftrace_sched_switch_handle_newprio(const TraceEvent &event,
				   const sched_switch_handle &handle)
{
	if (handle.ftrace.format == FTRACE_SCHED_NEW)
		return uint_after_char(event, event.argc - 1, '=');
	else
		return param_inside_braces(event, event.argc - 1);
}

#define WAKEUP_SUCC_PFIX "success="

static vtl_always_inline bool ftrace_sched_wakeup_args_ok(
	const TraceEvent &event)
{
	const TString *ss = event.argv[event.argc - 2];
	const char *c = ss->ptr;

	if (event.argc < 3)
		return false;

	if (!prefixcmp(c, WAKEUP_SUCC_PFIX))
		return event.argc >= 4;

	return event.argc >= 3;
}

static vtl_always_inline
unsigned int ftrace_sched_wakeup_cpu(const TraceEvent &event)
{
	unsigned int cpu = uint_after_char(event, event.argc - 1, ':');

	if (cpu == ABSURD_UNSIGNED)
		cpu = uint_after_char(event, event.argc - 1, '=');
	return cpu;
}

static vtl_always_inline bool ftrace_sched_wakeup_success(const TraceEvent &event)
{
	const TString *ss = event.argv[event.argc - 2];
	const char *c = ss->ptr;
	if (!prefixcmp(c, WAKEUP_SUCC_PFIX)) {
		char *last = ss->ptr + ss->len - 1;
		return *last == '1';
	}
	return true;
}

/*
 * This function is currently not used. However, let it be known that it would
 * in its current form not work for all known trace formats. In particular, it
 * would break down for wakeup events with the warning in the trace arguments:
 * xterm:6879 [120]<CANT FIND FIELD success> CPU:006
 */
static vtl_always_inline
unsigned int ftrace_sched_wakeup_prio(const TraceEvent &event)
{
	int idx = event.argc - 3;
	if (is_param_inside_braces(event.argv[idx])) {
		return param_inside_braces(event, idx);
	} else {
		/* Perhaps we should check here that we have the prio= prefix */
		idx = event.argc - 2;
		return uint_after_char(event, idx, '=');
	}
}

#define WAKEUP_PID_PFIX "pid="

static vtl_always_inline int ftrace_sched_wakeup_pid(const TraceEvent &event)
{
	int idx = event.argc - 3;
	const char *c = event.argv[idx]->ptr;

	if (!prefixcmp(c, WAKEUP_PID_PFIX))
		return int_after_char(event, idx, '=');
	/* We assume the old format here */
	idx = event.argc - 3;
	if (!strcmp(event.argv[idx]->ptr, "FIELD")) {
		/*
		 * Here we assume that the arguments of this wakeup event
		 * contains a warning, of a missing field, like this:
		 * xterm:6879 [120]<CANT FIND FIELD success> CPU:006
		 */
		if (event.argc >= 6) {
			idx = event.argc - 6;
			goto out;
		}
	}
	idx = event.argc - 2;
	if (!prefixcmp(event.argv[idx]->ptr, WAKEUP_SUCC_PFIX)) {
		/* This means that we have the sucess=1 parameter, as in:
		 *  xterm:1656 [120] success=1 CPU:000
		 */
		idx = event.argc - 4;
	} else {
		/* No success=1 param, so it should look like this:
		 * xterm:1656 [120] CPU:000
		 */
		idx = event.argc - 3;
	}
out:
	return int_after_char(event, idx, ':');
}

static vtl_always_inline const char
*ftrace_sched_wakeup_name_strdup_(const TraceEvent &event, StringPool<> *pool)
{
	int beginidx;
	int endidx;
	int pfix_idx;
	int len = 0;
	char *c;
	const TString *first;
	const TString *last;
	bool ok;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	/* We use the "pid=" prefix as a marker for the new format */
	const char *d = event.argv[event.argc - 3]->ptr;
	bool is_newformat = !prefixcmp(d, WAKEUP_PID_PFIX);

	if (is_newformat) {
		/* newformat */
		first = event.argv[0];
		beginidx = 1;
		endidx = event.argc - 4;
		copy_tstring_after_char_(first, '=', c, len,
					 TASKNAME_MAXLEN, ok);
		if (!ok)
			return NullStr;

		merge_args_into_cstring_nullterminate(event, beginidx, endidx,
						      c, len, TASKNAME_MAXLEN,
						      ok);
		if (!ok)
			return NullStr;

	} else {
		/* oldformat */
		beginidx = 0;
		pfix_idx = event.argc - 2;
		last = event.argv[event.argc - 3];

		if (event.argc >= 6 && !strcmp(last->ptr, "FIELD")) {
			/*
			 * Here again we assume that the arguments of this
			 * wakeup event contains a warning, of a missing field,
			 * this:
			 * xterm:6879 [120]<CANT FIND FIELD success> CPU:006
			 */
			last = event.argv[event.argc - 6];
			endidx = event.argc - 7;
		} else if (prefixcmp(event.argv[pfix_idx]->ptr,
				     WAKEUP_SUCC_PFIX)) {
			endidx = event.argc - 4;
		} else {
			last = event.argv[event.argc - 4];
			endidx = event.argc - 5;
		}

		merge_args_into_cstring(event, beginidx, endidx,
					c, len, TASKNAME_MAXLEN,
					ok);
		if (!ok)
			return NullStr;

		copy_tstring_before_char_(last, ':',
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

const char *ftrace_sched_wakeup_name_strdup(const TraceEvent &event,
					    StringPool<> *pool);

#define ftrace_sched_process_fork_args_ok(EVENT) (EVENT.argc >= 4)
#define ftrace_sched_process_fork_childpid(EVENT) \
	(int_after_char(EVENT, EVENT.argc - 1, '='))

static vtl_always_inline int
ftrace_sched_process_fork_parent_pid(const TraceEvent &event) {
	int i;
	int endidx;

	if (event.argc < 4)
		return ABSURD_INT;

	endidx = event.argc - 2;

	for (i = endidx; i > 0; i--) {
		if (prefixcmp(event.argv[i]->ptr, "child_comm=") == 0 &&
		    prefixcmp(event.argv[i - 1]->ptr, "pid=") == 0)
			break;
	}
	if (i < 2)
		return ABSURD_INT;

	return int_after_char(event, i - 1, '=');
}

static vtl_always_inline const char *
ftrace_sched_process_fork_childname_strdup_(const TraceEvent &event,
					    StringPool<> *pool)
{
	int i;
	const int endidx = event.argc - 2;
	char *c;
	int len;
	int sublen;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	if (event.argc < 4)
		return NullStr;

	for (i = 2; i <= endidx; i++) {
		if (!prefixcmp(event.argv[i]->ptr, "child_comm="))
			goto found;
	}
	return NullStr;

found:
	len = 0;

	const char *d = substr_after_char(event.argv[i]->ptr,
					  event.argv[i]->len, '=', &sublen);
	if (d == NullStr)
		return NullStr;
	len += sublen;
	if (len > TASKNAME_MAXLEN)
		return NullStr;
	strncpy(c, d, sublen);
	c += sublen;
	i++;

	for (;i <= endidx; i++) {
		len++;
		if (len > TASKNAME_MAXLEN)
			return NullStr;
		*c = ' ';
		c++;
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return NullStr;
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
	}

	/* Terminate the string */
	*c  = '\0';
	len++;

	ts.len = len;
	retstr = pool->allocString(&ts, 0);
	if (retstr == nullptr)
		return NullStr;

	return retstr->ptr;
}

const char *ftrace_sched_process_fork_childname_strdup(const TraceEvent &event,
						       StringPool<> *pool);

#define ftrace_sched_process_exit_args_ok(EVENT) (EVENT.argc >= 3)
#define ftrace_sched_process_exit_pid(EVENT) \
	(int_after_char(EVENT, EVENT.argc - 2, '='))

#define FTRACE_WAKING_COMM_PFIX "comm="
#define FTRACE_WAKING_PID_PFIX  "pid="
#define FTRACE_WAKING_PRIO_PFIX "prio="
#define FTRACE_WAKING_CPU_PFIX  "target_cpu="

#define ftrace_sched_waking_args_ok(EVENT)				\
	((EVENT.argc >= 4) &&						\
	 !prefixcmp(EVENT.argv[0]->ptr, FTRACE_WAKING_COMM_PFIX) &&	\
	 !prefixcmp(EVENT.argv[EVENT.argc - 3]->ptr, FTRACE_WAKING_PID_PFIX) \
	 &&								\
	 !prefixcmp(EVENT.argv[EVENT.argc - 2]->ptr, FTRACE_WAKING_PRIO_PFIX) \
	 &&								\
	 !prefixcmp(EVENT.argv[EVENT.argc - 1]->ptr, FTRACE_WAKING_CPU_PFIX))

static vtl_always_inline const char *
ftrace_sched_waking_name_strdup_(const TraceEvent &event, StringPool<> *pool)
{
	int beginidx;
	int endidx;
	int len = 0;
	char *c;
	const TString *first;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	const TString *retstr;
	bool ok;

	c = &sbuf[0];
	ts.ptr = c;

	beginidx = 1;
	endidx = event.argc - 4;

	/*
	 * This will copy the first part of the name, that is the
	 * portion of first that is suceeded by the '=' character
	 */
	first = event.argv[0];
	copy_tstring_after_char_(first, '=', c, len, TASKNAME_MAXLEN,
				 ok);
	if (!ok)
		return NullStr;

	merge_args_into_cstring_nullterminate(event, beginidx, endidx,
					      c, len, TASKNAME_MAXLEN,
					      ok);
	if (!ok)
		return NullStr;

	ts.len = len;
	retstr = pool->allocString(&ts, 0);
	if (retstr == nullptr)
		return NullStr;
	return retstr->ptr;
}

const char *
ftrace_sched_waking_name_strdup(const TraceEvent &event, StringPool<> *pool);

#define ftrace_sched_waking_pid(EVENT) \
	(int_after_char(EVENT, EVENT.argc - 3, '='))

#define ftrace_sched_waking_prio(EVENT) \
	(uint_after_char(EVENT, EVENT.argc - 2, '='))
#define ftrace_sched_waking_cpu(EVENT)			\
	(uint_after_char(EVENT, EVENT.argc - 1, '='))

#endif
