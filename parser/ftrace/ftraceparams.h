/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
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

#include "mm/stringpool.h"
#include "parser/traceevent.h"
#include "parser/paramhelpers.h"
#include "misc/traceshark.h"
#include <cstring>
#include <cstdint>

#define ftrace_cpufreq_args_ok(EVENT) (EVENT.argc >= 2)
#define ftrace_cpufreq_cpu(EVENT) (param_after_char(EVENT, 1, '='))
#define ftrace_cpufreq_freq(EVENT) (param_after_char(EVENT, 0, '='))

#define ftrace_cpuidle_args_ok(EVENT) (EVENT.argc >= 2)
#define ftrace_cpuidle_cpu(EVENT) (param_after_char(EVENT, 1, '='))
static __always_inline int ftrace_cpuidle_state(const TraceEvent &event)
{
	int32_t state;
	uint32_t ustate;
	ustate = param_after_char(event, 0, '=');
	state = *((int*) &ustate); /* the string is a signed printed as 
				    * unsigned :) */
	return state;
}

#define ftrace_sched_migrate_args_ok(EVENT) (EVENT.argc >= 5)
#define ftrace_sched_migrate_destCPU(EVENT) (param_after_char(EVENT, \
							      EVENT.argc - 1, \
							      '='))
#define ftrace_sched_migrate_origCPU(EVENT) (param_after_char(EVENT, \
							      EVENT.argc - 2, \
							      '='))
#define ftrace_sched_migrate_prio(EVENT) (param_after_char(EVENT, \
							   EVENT.argc - 3, \
							   '='))
#define ftrace_sched_migrate_pid(EVENT) (param_after_char(EVENT, \
							  EVENT.argc - 4, \
							  '='))

#define ftrace_sched_switch_args_ok(EVENT) (EVENT.argc >= 6)
#define ftrace_sched_switch_newprio(EVENT) (param_inside_braces(EVENT, \
								EVENT.argc - 1))
#define ftrace_sched_switch_newpid(EVENT)  \
	(param_after_char(EVENT, EVENT.argc - 2, ':'))

static __always_inline taskstate_t
	ftrace_sched_switch_state(const TraceEvent &event)
{
	unsigned int i;
	for (i = 3; i < event.argc; i++)
		if (isArrowStr(event.argv[i]))
			return __sched_state_from_tstring(event.argv[i - 1]);

	return TASK_STATE_PARSER_ERROR;
}

static __always_inline unsigned int
ftrace_sched_switch_oldprio(const TraceEvent &event)
{
	unsigned int i;
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (i < event.argc)
		return param_inside_braces(event, i - 2);
	return ABSURD_UNSIGNED;
}

static __always_inline unsigned int
ftrace_sched_switch_oldpid(const TraceEvent &event)
{
	unsigned int i;
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (i < event.argc)
		return param_after_char(event, i - 3, ':');
	return ABSURD_UNSIGNED;
}

static __always_inline char
*__ftrace_sched_switch_oldname_strdup(const TraceEvent &event, StringPool *pool)
{
	unsigned int i;
	unsigned int endidx;
	unsigned int len = 0;
	char *c;
	char *d;
	char *end;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	/* Find the index of the '==>' */
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (!(i < event.argc))
		return nullptr;
	endidx = i - 3;

	/*
	 * This loop will merge any strings before the final string, in case
	 * such strings exists due to the task name containing spaces, and
	 * then the taskname would be split into several strings
	 */
	for (i = 0; i < endidx; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
		*c = ' ';
		len++;
		c++;
	}

	/*
	 * Localize the separing ':' in the final string. The final
	 * string is the only sting in case of no spaces in the task name.
	 * we are searching backwards because we are interested in the last ':',
	 * since the task name can contain ':' characters
	 */
	for (end = event.argv[endidx]->ptr + event.argv[endidx]->len - 1;
	     end > event.argv[endidx]->ptr; end--) {
		if (*end == ':')
			break;
	}

	/* Copy the final portion up to the ':' we found previously */
	for (d = event.argv[endidx]->ptr; d < end; d++) {
		len++;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		*c = *d;
		c++;
	}
	/* Terminate the string */
	*c = '\0';
	len++;

	ts.len = len;
	retstr = pool->allocString(&ts, TShark::StrHash32(&ts), 0);
	if (retstr == nullptr)
		return nullptr;

	return retstr->ptr;
}

char *ftrace_sched_switch_oldname_strdup(const TraceEvent &event,
					 StringPool *pool);

/* TODO: Check what code could be shared between this and the above function */
static __always_inline char
*__ftrace_sched_switch_newname_strdup(const TraceEvent &event, StringPool *pool)
{
	unsigned int i;
	unsigned int startidx, endidx;
	unsigned int len = 0;
	char *c;
	char *d;
	char *end;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	endidx = event.argc - 2;

	/* Find the index of the '==>' */
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (!(i < event.argc))
		return nullptr;
	startidx = i + 1;

	/*
	 * This loop will merge any strings before the final string, in case
	 * such strings exists due to the task name containing spaces, and
	 * then the taskname would be split into several strings
	 */
	for (i = startidx; i < endidx; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
		*c = ' ';
		len++;
		c++;
	}

	/*
	 * Localize the separing ':' in the final string. The final
	 * string is the only sting in case of no spaces in the task name.
	 * we are searching backwards because we are interested in the last ':',
	 * since the task name can contain ':' characters
	 */
	for (end = event.argv[endidx]->ptr + event.argv[endidx]->len - 1;
	     end > event.argv[endidx]->ptr; end--) {
		if (*end == ':')
			break;
	}

	/* Copy the final portion up to the ':' we found previously */
	for (d = event.argv[endidx]->ptr; d < end; d++) {
		len++;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		*c = *d;
		c++;
	}
	/* Terminate the string */
	*c = '\0';
	len++;

	ts.len = len;
	retstr = pool->allocString(&ts, TShark::StrHash32(&ts), 0);
	if (retstr == nullptr)
		return nullptr;

	return retstr->ptr;
}

char *ftrace_sched_switch_newname_strdup(const TraceEvent &event,
					 StringPool *pool);

#define ftrace_sched_wakeup_args_ok(EVENT) (EVENT.argc >= 4)
#define ftrace_sched_wakeup_cpu(EVENT) (param_after_char(EVENT, \
							 EVENT.argc - 1, ':'))

static __always_inline bool ftrace_sched_wakeup_success(const TraceEvent &event)
{
	const TString *ss = event.argv[event.argc - 2];
	char *last = ss->ptr + ss->len - 1; /* Empty string should not be 
					     produced by parser */
	return *last == '1';
}

#define ftrace_sched_wakeup_prio(EVENT) (param_inside_braces(EVENT, \
							     EVENT.argc - 3))
#define ftrace_sched_wakeup_pid(EVENT) (param_after_char(EVENT, \
							 EVENT.argc - 4, \
							 ':'))
/* Todo, code could be shrared with the other two *_strup() functions */
static __always_inline char
*__ftrace_sched_wakeup_name_strdup(const TraceEvent &event, StringPool *pool)
{
	unsigned int i;
	char *c;
	unsigned int endidx;
	char *d;
	char *end;
	unsigned int len = 0;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	if (event.argc < 4)
		return nullptr;

	endidx = event.argc - 4;

	/* This loop will merge any strings before the final string, in case
	 * such strings exists due to the task name containing spaces, and
	 * then the taskname would be split into several strings
	 */
	for(i = 0; i < event.argc - 4; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
		*c = ' ';
		len++;
		c++;
	}

	/*
	 * Localize the separating ':' in the final string. The final
	 * string is the only sting in case of no spaces in the task name.
	 * we are searching backwards because we are interested in the last ':',
	 * since the task name can contain ':' characters
	 */
	for (end = event.argv[endidx]->ptr + event.argv[endidx]->len - 1;
	     end > event.argv[endidx]->ptr; end--) {
		if (*end == ':')
			break;
	}

	/* Copy the final portion up to the ':' we found previously */
	for (d = event.argv[endidx]->ptr; d < end; d++) {
		len++;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		*c = *d;
		c++;
	}

	/* Terminate the string */
	*c = '\0';
	len++;

	ts.len = len;
	retstr = pool->allocString(&ts, TShark::StrHash32(&ts), 0);
	if (retstr == nullptr)
		return nullptr;

	return retstr->ptr;
}

char *ftrace_sched_wakeup_name_strdup(const TraceEvent &event,
				      StringPool *pool);

#define ftrace_sched_process_fork_args_ok(EVENT) (EVENT.argc >= 4)
#define ftrace_sched_process_fork_childpid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 1, '='))

static __always_inline unsigned int
ftrace_sched_process_fork_parent_pid(const TraceEvent &event) {
	unsigned int i;
	unsigned int endidx;

	if (event.argc < 4)
		return ABSURD_UNSIGNED;

	endidx = event.argc - 2;

	for (i = endidx; i > 0; i--) {
		if (strncmp(event.argv[i]->ptr, "child_comm=", 11) == 0 &&
		    strncmp(event.argv[i - 1]->ptr, "pid=", 4) == 0)
			break;
	}
	if (i < 2)
		return ABSURD_UNSIGNED;

	return param_after_char(event, i - 1, '=');
}

static __always_inline char *
__ftrace_sched_process_fork_childname_strdup(const TraceEvent &event,
					     StringPool *pool)
{
	unsigned int i;
	const unsigned int endidx = event.argc - 2;
	char *c;
	unsigned int len;
	unsigned int sublen;
	char sbuf[TASKNAME_MAXLEN + 1];
	TString ts;
	TString *retstr;

	c = &sbuf[0];
	ts.ptr = c;

	if (event.argc < 4)
		return nullptr;

	for (i = 2; i <= endidx; i++) {
		if (!strncmp(event.argv[i]->ptr, "child_comm=", 11))
			goto found;
	}
	return nullptr;

found:
	len = 0;

	const char *d = substr_after_char(event.argv[i]->ptr,
					  event.argv[i]->len, '=', &sublen);
	if (d == nullptr || sublen > TASKNAME_MAXLEN)
		return nullptr;
	strncpy(c, d, sublen);
	i++;

	for (;i <= endidx; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return nullptr;
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
		if (i == endidx)
			goto finalize;
		*c = ' ';
		len++;
		c++;
	}
finalize:
	/* Terminate the string */
	*c  = '\0';
	len++;

	ts.len = len;
	retstr = pool->allocString(&ts, TShark::StrHash32(&ts), 0);
	if (retstr == nullptr)
		return nullptr;

	return retstr->ptr;
}

char *ftrace_sched_process_fork_childname_strdup(const TraceEvent &event,
						 StringPool *pool);

#define ftrace_sched_process_exit_args_ok(EVENT) (EVENT.argc >= 3)
#define ftrace_sched_process_exit_pid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 2, '='))

#define ftrace_irq_handler_entry_args_ok(EVENT) (EVENT.argc >= 2)
#define ftrace_irq_handler_entry_irq(EVENT) \
	(param_after_char(EVENT, 0, '='))
#define ftrace_irq_handler_entry_name(EVENT, LEN_UINTPTR) \
	(substr_after_char(EVENT.argv[1]->ptr, EVENT.argv[1].len, LEN_UINTPTR))

#define ftrace_irq_handler_exit_args_ok(EVENT)	(EVENT.argc >= 2)
#define ftrace_irq_handler_exit_irq(EVENT) \
	(param_after_char(EVENT, 0, '='))
#define ftrace_irq_handler_exit_handled(EVENT) \
	(strncmp(EVENT.argv[1]->ptr, "ret=handled", EVENT.argv[1]->len) == 0)
#define ftrace_irq_handler_exit_ret(EVENT, LEN_UINTPTR) \
	(substr_after_char(EVENT.argv[1]->ptr, EVENT.argv[1].len, LEN_UINTPTR))

#endif
