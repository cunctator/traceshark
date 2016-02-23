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

#ifndef PERFPARAMS_H
#define PERFPARAMS_H

#include "mm/mempool.h"
#include "traceevent.h"
#include "paramhelpers.h"
#include "traceshark.h"
#include <cstring>
#include <cstdint>

#define perf_cpufreq_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_cpufreq_cpu(EVENT) (param_after_char(EVENT, 1, '='))
#define perf_cpufreq_freq(EVENT) (param_after_char(EVENT, 0, '='))

#define perf_cpuidle_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_cpuidle_cpu(EVENT) (param_after_char(EVENT, 1, '='))
static __always_inline int perf_cpuidle_state(const TraceEvent &event)
{
	int32_t state;
	uint32_t ustate;
	ustate = param_after_char(event, 0, '=');
	state = *((int*) &ustate); /* the string is a signed printed as 
				    * unsigned :) */
	return state;
}

#define perf_sched_migrate_args_ok(EVENT) (EVENT.argc >= 5)
#define perf_sched_migrate_destCPU(EVENT) (param_after_char(EVENT, EVENT.argc \
							    - 1, '='))
#define perf_sched_migrate_origCPU(EVENT) (param_after_char(EVENT, EVENT.argc \
							    - 2, '='))
#define perf_sched_migrate_prio(EVENT) (param_after_char(EVENT, EVENT.argc - 3,\
							 '='))
#define perf_sched_migrate_pid(EVENT) (param_after_char(EVENT, EVENT.argc - 4, \
							'='))

#define perf_sched_switch_args_ok(EVENT) (EVENT.argc >= 8)
#define perf_sched_switch_newprio(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 1, '='))
#define perf_sched_switch_newpid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 2, '='))

#define SWITCH_PPID_PFIX "prev_pid="
#define SWITCH_PPRI_PFIX "prev_prio="
#define SWITCH_PSTA_PFIX "prev_state="
#define SWITCH_NCOM_PFIX "next_comm="
#define SWITCH_NPID_PFIX "next_pid="
#define SWITCH_NPRI_PFIX "next_prio="

/*
 * This function finds the index of the '==>' in the switch event arguments
 * that has this format:
 *
 * prev_pid=... prev_prio=... prev_state=... ==> next_comm=...
 * 
 * This is quite paranoid but I don't see a choice. Basically, it would be
 * sufficient to check for the '==>' but then if some weird person would be
 * running a task that has a name like 'x x x ==>' then the parsing could get
 * fooled, checking that the tokens before and after the '==>' have the correct
 * prefixes protects us against such idiotic cases. I believe that the maximum
 * taskname length in the kernel (16 characters?)  will protect us from from 
 * somebody creating the truly lunatic taskname that would be able to fool 
 * this parsing, i.e. a taskname such as :
 * 
 * "prev_pid= prev_prio= prev_state= ==> next_comm="
 *
 * In a nutshell, this protects us against weirdos but not against lunatics :)
 */
static __always_inline unsigned int ___perf_sched_switch_find_arrow(TraceEvent
								    &event)
{
	unsigned int i;
	for (i = 4; i < event.argc - 3; i++) {
		char *c1 = event.argv[i - 3]->ptr;
		char *c2 = event.argv[i - 2]->ptr;
		char *c3 = event.argv[i - 1]->ptr;
		TString *c4 = event.argv[i];
		char *c5 = event.argv[i + 1]->ptr;
		if (!strncmp(c1, SWITCH_PPID_PFIX, strlen(SWITCH_PPID_PFIX)) &&
		    !strncmp(c2, SWITCH_PPRI_PFIX, strlen(SWITCH_PPRI_PFIX)) &&
		    !strncmp(c3, SWITCH_PSTA_PFIX, strlen(SWITCH_PSTA_PFIX)) &&
		    isArrowStr(c4) &&
		    !strncmp(c5, SWITCH_NCOM_PFIX, strlen(SWITCH_NCOM_PFIX)))
			break;
	}
	if (!(i < event.argc - 3))
		return 0;
	return i;
}

static __always_inline char perf_sched_switch_state(TraceEvent &event)
{
	unsigned int i;

	i = ___perf_sched_switch_find_arrow(event);
	if (i != 0) {
		TString *stateStr = event.argv[i - 1];
		return stateStr->ptr[stateStr->len - 1];
	}
	return '\0';
}

static __always_inline unsigned int perf_sched_switch_oldprio(TraceEvent &event)
{
	unsigned int i;

	i = ___perf_sched_switch_find_arrow(event);
	if (i != 0)
		return param_after_char(event, i - 2, '=');
	return ABSURD_UNSIGNED;
}

static __always_inline unsigned int perf_sched_switch_oldpid(TraceEvent &event)
{
	unsigned int i;

	i = ___perf_sched_switch_find_arrow(event);
	if (i != 0)
		return param_after_char(event, i - 3, '=');
	return ABSURD_UNSIGNED;
}

static __always_inline char * __perf_sched_switch_oldname_strdup(TraceEvent
								 &event,
								 MemPool *pool)
{
	unsigned int i;
	unsigned int beginidx;
	unsigned int endidx;
	unsigned int len = 0;
	char *retstr;
	char *c;
	TString *first;
	bool ok;

	i = ___perf_sched_switch_find_arrow(event);
	if (i == 0)
		return NULL;
	beginidx = 1;
	endidx = i - 4;

	retstr = taskname_prealloc(pool);
	if (retstr == NULL)
		return NULL;
	c = retstr;

	/* This will copy the first part of the name, that is the portion
	 * of first that is suceeded by the '=' character */
	first = event.argv[0];
	__copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	merge_args_into_cstring(event, beginidx, endidx, c, len,
				TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

char *perf_sched_switch_oldname_strdup(TraceEvent &event, MemPool *pool);

static __always_inline char * __perf_sched_switch_newname_strdup(TraceEvent
								 &event,
								 MemPool *pool)
{
	unsigned int i;
	unsigned int beginidx;
	unsigned int endidx;
	unsigned int len = 0;
	char *retstr;
	char *c;
	TString *first;
	bool ok;

	i = ___perf_sched_switch_find_arrow(event);
	if (i == 0)
		return NULL;
	beginidx = i + 2;
	endidx = event.argc - 3;

	/* + 1 needed for null termination */
	retstr = taskname_prealloc(pool);
	if (retstr == NULL)
		return NULL;
	c = retstr;

	/* This will copy the first part of the name, that is the portion
	 * of first that is suceeded by the '=' character */
	first = event.argv[i + 1];
	__copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	merge_args_into_cstring(event, beginidx, endidx, c, len,
				TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

char *perf_sched_switch_newname_strdup(TraceEvent &event, MemPool *pool);

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
 *
 */

#define WAKE_SUCC_PFIX "success="
#define WAKE_PID_PFIX  "pid="
#define WAKE_PRIO_PFIX "prio="
#define WAKE_TCPU_PFIX "target_cpu="

#define WAKE_SUCC_PFIX_LEN (sizeof(WAKE_SUCC_PFIX) / sizeof(char) - 1)
#define WAKE_PID_PFIX_LEN  (sizeof(WAKE_PID_PFIX)  / sizeof(char) - 1)
#define WAKE_PRIO_PFIX_LEN (sizeof(WAKE_PRIO_PFIX) / sizeof(char) - 1)
#define WAKE_TCPU_PFIX_LEN (sizeof(WAKE_TCPU_PFIX) / sizeof(char) - 1)

#define perf_sched_wakeup_args_ok(EVENT) (EVENT.argc >= 4)

/* The last argument is target_cpu, regardless of old or new */
#define perf_sched_wakeup_cpu(EVENT) (param_after_char(EVENT, EVENT.argc - 1, \
						       '='))

static __always_inline bool perf_sched_wakeup_success(TraceEvent &event)
{
	const TString *ss = event.argv[event.argc - 2];

	/* Assume that wakeup is successful if no success field is found*/
	if (strncmp(ss->ptr, WAKE_SUCC_PFIX,WAKE_SUCC_PFIX_LEN) != 0)
		return true;

	/* Empty string should not be produced by parser */
	char *last = ss->ptr + ss->len - 1;
	return *last == '1';
}

static __always_inline unsigned int perf_sched_wakeup_prio(TraceEvent &event)
{
	unsigned int newidx = event.argc - 2;
	unsigned int oldidx;
	/* Check if we are on the new format */
	if (!strncmp(event.argv[newidx]->ptr, WAKE_PRIO_PFIX,
		     WAKE_PRIO_PFIX_LEN)) {
		return param_after_char(event, newidx, '=');
	}

	/* Assume that this is the old format */
	oldidx = event.argc - 3;
	return param_after_char(event, oldidx, '=');
}

static __always_inline unsigned int perf_sched_wakeup_pid(TraceEvent &event)
{
	unsigned int newidx = event.argc - 3;
	unsigned int oldidx;
	/* Check if we are on the new format */
	if (!strncmp(event.argv[newidx]->ptr, WAKE_PID_PFIX,
		     WAKE_PID_PFIX_LEN)) {
		return param_after_char(event, newidx, '=');
	}

	/* Assume that this is the old format */
	oldidx = event.argc - 4;
	return param_after_char(event, oldidx, '=');
}

static __always_inline char *__perf_sched_wakeup_name_strdup(TraceEvent &event,
							     MemPool *pool)
{
	unsigned int i;
	unsigned int beginidx;
	unsigned int endidx;
	unsigned int len = 0;
	char *retstr;
	char *c;
	TString *first;
	bool ok;

	/* Find the index of the pid=... that is followed by prio= */
	for (i = 1; i <= event.argc - 2; i++) {
		char *c1 = event.argv[i + 0]->ptr;
		char *c2 = event.argv[i + 1]->ptr;
		if (!strncmp(c1, WAKE_PID_PFIX, WAKE_PID_PFIX_LEN) &&
		    !strncmp(c2, WAKE_PRIO_PFIX, WAKE_PRIO_PFIX_LEN))
			break;
	}
	if (!(i <= event.argc - 2))
		return NULL;
	beginidx = 1;
	endidx = i - 1;

	retstr = taskname_prealloc(pool);
	if (retstr == NULL)
		return NULL;
	c = retstr;

	/* This will copy the first part of the name, that is the portion
	 * of first that is suceeded by the '=' character */
	first = event.argv[0];
	__copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	merge_args_into_cstring(event, beginidx, endidx, c, len,
				TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

char *perf_sched_wakeup_name_strdup(TraceEvent &event, MemPool *pool);

#define perf_sched_process_fork_args_ok(EVENT) (EVENT.argc >= 4)
#define perf_sched_process_fork_childpid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 1, '='))

static __always_inline unsigned int perf_sched_process_fork_parent_pid(
	TraceEvent &event) {
	unsigned int i;
	unsigned int endidx;

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
__perf_sched_process_fork_childname_strdup(TraceEvent &event,
					   MemPool *pool)
{
	unsigned int i;
	unsigned int beginidx;
	const unsigned int endidx = event.argc - 2;
	char *c;
	char *retstr;
	unsigned int len = 0;
	bool ok;
	TString *first;

	for (i = 2; i <= endidx; i++) {
		if (!strncmp(event.argv[i - 1]->ptr, "pid=", 4) &&
		    !strncmp(event.argv[i]->ptr,     "child_comm=", 11))
			break;
	}
	if (i > endidx)
		return NULL;
	beginidx = i + 1;

	retstr = taskname_prealloc(pool);
	if (retstr == NULL)
		return NULL;
	c = retstr;
	
	/* This will copy the first part of the name, that is the portion
	 * of first that is suceeded by the '=' character */
	first = event.argv[i];
	__copy_tstring_after_char(first, '=', c, len, TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	merge_args_into_cstring(event, beginidx, endidx, c, len,
				TASKNAME_MAXLEN, ok);
	if (!ok)
		return NULL;

	/* commmit the allocation */
	if (pool->commitChars(len))
		return retstr;
	return NULL;
}

#define perf_sched_process_exit_args_ok(EVENT) (EVENT.argc >= 3)
#define perf_sched_process_exit_pid(EVENT) \
	(param_after_char(EVENT, EVENT.argc - 2, '='));

#define perf_irq_handler_entry_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_irq_handler_entry_irq(EVENT) \
	(param_after_char(EVENT, 0, '='))
#define perf_irq_handler_entry_name(EVENT, LEN_UINTPTR)			\
	(substr_after_char(EVENT.argv[1]->ptr, EVENT.argv[1].len, LEN_UINTPTR))

#define perf_irq_handler_exit_args_ok(EVENT) (EVENT.argc >= 2)
#define perf_irq_handler_exit_irq(EVENT) \
	(param_after_char(EVENT, 0, '='))
#define perf_irq_handler_exit_handled(EVENT) \
	(strncmp(EVENT.argv[1]->ptr, "ret=handled", EVENT.argv[1]->len) == 0)
#define perf_irq_handler_exit_ret(EVENT, LEN_UINTPTR) \
	(substr_after_char(EVENT.argv[1]->ptr, EVENT.argv[1].len, LEN_UINTPTR))

#endif /* PERFPARAMS_H*/
