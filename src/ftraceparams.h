/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef FTRACEPARAMS_H
#define FTRACEPARAMS_H

#include "mm/mempool.h"
#include "traceevent.h"
#include "paramhelpers.h"
#include "traceshark.h"
#include <cstring>
#include <cstdint>

#define cpufreq_args_ok(EVENT) (EVENT.argc >= 2)
#define cpufreq_cpu(EVENT) (param_after_char(EVENT, 1, '='))
#define cpufreq_freq(EVENT) (param_after_char(EVENT, 0, '='))

#define cpuidle_args_ok(EVENT) (EVENT.argc >= 2)
#define cpuidle_cpu(EVENT) (param_after_char(EVENT, 1, '='))
static __always_inline int cpuidle_state(const TraceEvent &event)
{
	int32_t state;
	uint32_t ustate;
	ustate = param_after_char(event, 0, '=');
	state = *((int*) &ustate); /* the string is a signed printed as 
				    * unsigned :) */
	return state;
}

#define sched_migrate_args_ok(EVENT) (EVENT.argc >= 5)
#define sched_migrate_destCPU(EVENT) (param_after_char(EVENT, EVENT.argc - 1, \
						       '='))
#define sched_migrate_origCPU(EVENT) (param_after_char(EVENT, EVENT.argc - 2, \
						       '='))
#define sched_migrate_prio(EVENT) (param_after_char(EVENT, EVENT.argc - 3, \
						    '='))
#define sched_migrate_pid(EVENT) (param_after_char(EVENT, EVENT.argc - 4, \
						   '='))

#define sched_switch_args_ok(EVENT) (EVENT.argc >= 6)
#define sched_switch_newprio(EVENT) (param_inside_braces(EVENT, EVENT.argc - 1))
#define sched_switch_newpid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 2, ':'))

static __always_inline taskstate_t sched_switch_state(TraceEvent &event)
{
	unsigned int i;
	taskstate_t state = TASK_STATE_UNKNOWN;
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (i < event.argc) {
		if (event.argv[i - 1]->ptr[0] == 'R') {
			state = TASK_STATE_RUNNABLE;
		} else {
			state = TASK_STATE_NOT_RUNNABLE;
		}
	}
	return state;
}

static __always_inline unsigned int sched_switch_oldprio(TraceEvent &event)
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

static __always_inline unsigned int sched_switch_oldpid(TraceEvent &event)
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

static __always_inline char * __sched_switch_oldname_strdup(TraceEvent &event,
	MemPool *pool)
{
	unsigned int i;
	unsigned int endidx;
	unsigned int len = 0;
	char *retstr;
	char *c;
	char *d;
	char *end;

	/* Find the index of the '==>' */
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (!(i < event.argc))
		return NULL;
	endidx = i - 3;

	/* + 1 needed for null termination */
	retstr = (char*) pool->preallocChars(TASKNAME_MAXLEN + 1);
	c = retstr;

	/* This loop will merge any strings before the final string, in case
	 * such strings exists due to the task name containing spaces, and
	 * then the taskname would be split into several strings
	 */
	for (i = 0; i < endidx; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return NULL;
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
			return NULL;
		*c = *d;
		c++;
	}
	/* Terminate the string */
	*c = '\0';
	len++;
	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

char *sched_switch_oldname_strdup(TraceEvent &event, MemPool *pool);

/* TODO: Check what code could be shared between this and the above function */
static __always_inline char * __sched_switch_newname_strdup(TraceEvent &event,
	MemPool *pool)
{
	unsigned int i;
	unsigned int startidx, endidx;
	unsigned int len = 0;
	char *retstr;
	char *c;
	char *d;
	char *end;

	endidx = event.argc - 2;

	/* Find the index of the '==>' */
	for (i = 3; i < event.argc; i++) {
		if (isArrowStr(event.argv[i]))
			break;
	}
	if (!(i < event.argc))
		return NULL;
	startidx = i + 1;

	/* + 1 needed for null termination */
	retstr = (char*) pool->preallocChars(TASKNAME_MAXLEN + 1);
	c = retstr;

	/* This loop will merge any strings before the final string, in case
	 * such strings exists due to the task name containing spaces, and
	 * then the taskname would be split into several strings
	 */
	for (i = startidx; i < endidx; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return NULL;
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
			return NULL;
		*c = *d;
		c++;
	}
	/* Terminate the string */
	*c = '\0';
	len++;
	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

char *sched_switch_newname_strdup(TraceEvent &event, MemPool *pool);

#define sched_wakeup_args_ok(EVENT) (EVENT.argc >= 4)
#define sched_wakeup_cpu(EVENT) (param_after_char(EVENT, EVENT.argc - 1, \
						  ':'))

static __always_inline bool sched_wakeup_success(TraceEvent &event)
{
	const TString *ss = event.argv[event.argc - 2];
	char *last = ss->ptr + ss->len - 1; /* Empty string should not be 
					     produced by parser */
	return *last == '1';
}

#define sched_wakeup_prio(EVENT) (param_inside_braces(EVENT, EVENT.argc - 3))
#define sched_wakeup_pid(EVENT) (param_after_char(EVENT, EVENT.argc - 4, \
						  ':'))
/* Todo, code could be shrared with the other two *_strup() functions */
static __always_inline char *__sched_wakeup_name_strdup(TraceEvent &event,
	MemPool *pool)
{
	unsigned int i;
	char *c, *retstr;
	unsigned int endidx;
	char *d;
	char *end;
	unsigned int len = 0;

	if (event.argc < 4)
		return NULL;

	endidx = event.argc - 4;

	retstr = (char*) pool->preallocChars(TASKNAME_MAXLEN + 1);
	c = retstr;

	/* This loop will merge any strings before the final string, in case
	 * such strings exists due to the task name containing spaces, and
	 * then the taskname would be split into several strings
	 */
	for(i = 0; i < event.argc - 4; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return NULL;
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
			return NULL;
		*c = *d;
		c++;
	}

	/* Terminate the string */
	*c = '\0';
	len++;
	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

char *sched_wakeup_name_strdup(TraceEvent &event, MemPool *pool);

#define sched_process_fork_args_ok(EVENT) (EVENT.argc >= 4)
#define sched_process_fork_childpid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 1, '='))

static __always_inline unsigned int sched_process_fork_parent_pid(
	TraceEvent &event) {
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
__sched_process_fork_childname_strdup(TraceEvent &event,
				      MemPool *pool)
{
	unsigned int i;
	const unsigned int endidx = event.argc - 2;
	char *c;
	char *retstr;
	unsigned int len;
	unsigned int sublen;

	if (event.argc < 4)
		return NULL;

	for (i = 2; i <= endidx; i++) {
		if (!strncmp(event.argv[i]->ptr, "child_comm=", 11))
			goto found;
	}
	return NULL;

found:
	retstr = (char*) pool->preallocChars(TASKNAME_MAXLEN + 1);
	c = retstr;
	len = 0;

	const char *d = substr_after_char(event.argv[i]->ptr,
					  event.argv[i]->len, '=', &sublen);
	if (d == NULL || sublen > TASKNAME_MAXLEN)
		return NULL;
	strncpy(c, d, sublen);
	i++;

	for (;i <= endidx; i++) {
		len += event.argv[i]->len;
		if (len > TASKNAME_MAXLEN)
			return NULL;
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
	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

#define sched_process_exit_args_ok(EVENT) (EVENT.argc >= 3)
#define sched_process_exit_pid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 2, '='));

#define irq_handler_entry_args_ok(EVENT) (EVENT.argc >= 2)
#define irq_handler_entry_irq(EVENT) \
	(param_after_char(EVENT, 0, '='))
#define irq_handler_entry_name(EVENT, LEN_UINTPTR) \
	(substr_after_char(EVENT.argv[1]->ptr, EVENT.argv[1].len, LEN_UINTPTR))

#define irq_handler_exit_args_ok(EVENT)	(EVENT.argc >= 2)
#define irq_handler_exit_irq(EVENT) \
	(param_after_char(EVENT, 0, '='))
#define irq_handler_exit_handled(EVENT) \
	(strncmp(EVENT.argv[1]->ptr, "ret=handled", EVENT.argv[1]->len) == 0)
#define irq_handler_exit_ret(EVENT, LEN_UINTPTR) \
	(substr_after_char(EVENT.argv[1]->ptr, EVENT.argv[1].len, LEN_UINTPTR))

#endif
