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

#ifndef PARAMHELPERS_H
#define PARAMHELPERS_H

#include <climits>
#include "misc/errors.h"
#include "misc/string.h"
#include "parser/traceevent.h"
#include "vtl/compiler.h"

class perf_sched_switch_handle {
public:
	bool is_distro_style;
	int index;
};

typedef enum ftrace_sched_format : int {
	FTRACE_SCHED_OLD,
	FTRACE_SCHED_NEW,
} ftraceschedformat_t;

class ftrace_sched_switch_handle {
public:
	ftraceschedformat_t format;
	int index;
};

typedef class sched_switch_handle {
public:
	union {
		perf_sched_switch_handle perf;
		ftrace_sched_switch_handle ftrace;
	};
} sched_switch_handle_t;

/* Should be enough, I wouldn't expect more than about 16 */
#define TASKNAME_MAXLEN (128)

#define ABSURD_UNSIGNED ((unsigned int)INT_MAX)
#define ABSURD_INT (INT_MAX)

#define FREQ_STATE_PFIX "state="
#define FREQ_CPUID_PFIX "cpu_id="

#define IDLE_STATE_PFIX "state="
#define IDLE_CPUID_PFIX "cpu_id="

#define MIGRATE_COMM_PFIX "comm="
#define MIGRATE_PID_PFIX  "pid="
#define MIGRATE_PRIO_PFIX "prio="
#define MIGRATE_ORIG_PFIX "orig_cpu="
#define MIGRATE_DEST_PFIX "dest_cpu="

#define SWITCH_PREV_PFIX "prev_"
#define SWITCH_NEXT_PFIX "next_"

#define SWITCH_PPID_PFIX "prev_pid="
#define SWITCH_PPRI_PFIX "prev_prio="
#define SWITCH_PSTA_PFIX "prev_state="
#define SWITCH_NCOM_PFIX "next_comm="
#define SWITCH_NPID_PFIX "next_pid="
#define SWITCH_NPRI_PFIX "next_prio="

#define WAKE_SUCC_PFIX "success="
#define WAKE_PID_PFIX  "pid="
#define WAKE_PRIO_PFIX "prio="
#define WAKE_TCPU_PFIX "target_cpu="

#define EXIT_COMM_PFIX "comm="
#define EXIT_PID_PFIX  "pid="
#define EXIT_PRIO_PFIX "prio="

#define is_this_event(EVENTNAME, EVENT) (EVENT.type == EVENTNAME)

#define isArrowStr(str) (str->len == 3 && str->ptr[0] == '=' && \
			 str->ptr[1] == '=' && str->ptr[2] == '>')


/* + 1 needed for null termination */
#define taskname_prealloc(POOL) \
	((char*) POOL->preallocChars(TASKNAME_MAXLEN + 1))

/*
 * WARNING: This function does not null terminate the string!
 * This function copies everything after the delim char into the cstring
 * pointed to by c and adds the length to len
 */
static vtl_always_inline void copy_tstring_after_char_(const TString *str,
						       char delim,
						       char *&dest,
						       int &len,
						       int maxlen,
						       bool &ok)
{
	int i;
	int flen;
	char *src;
	ok = true;
	/* Find the char 'delim' in the string */
	for (i = 0; i < str->len; i++) {
		if (str->ptr[i] == delim)
			break;
	}
	if (i >= str->len)
		goto err;
	/*
	 * ...then copy everything after the delim char into the name, if there
	 * is anything to copy
	 */
	i++;
	if (i < str->len) {
		flen = str->len - i;
		if (flen > maxlen)
			goto err;
		src = str->ptr + i;
		strncpy(dest, src, flen);
		len += flen;
		dest += flen;
		return;
	}
err:
	ok = false;
	return;
}

/*
 * WARNING: This function does not null terminate the string!
 * This function copies everything after the delim char into the cstring
 * pointed to by c and adds the length to len
 */
static vtl_always_inline void copy_tstring_before_char_(const TString *str,
							char delim,
							char *&dest,
							int &len,
							int maxlen,
							bool &ok)
{
	int i;
	int flen;
	char *src;
	/* Find the char 'delim' in the string */
	for (i = str->len - 1; i >= 0; i--) {
		if (str->ptr[i] == delim)
			break;
	}
	if (i < 0)
		goto err;

	/*
	 * ...then copy everything before the delim char into the name, if there
	 * is anything to copy
	 */
	if (i > 0) {
		flen = i;
		if (flen > maxlen)
			goto err;
		src = str->ptr;
		strncpy(dest, src, flen);
		len += flen;
		dest += flen;
		*dest = '\0';
		dest++;
		len++;
		return;
	}
err:
	ok = false;
	return;
}

/*
 * This function will merge event arguments from beginidx to endix into
 * a cstring. Such cases exists because tasknames that contains spaces have
 * been split into several arguments due to parsing with space as delimiter
 */
static vtl_always_inline void
merge_args_into_cstring_nullterminate(const TraceEvent &event,
				      int beginidx,
				      int endidx,
				      char *&c,
				      int &len,
				      int maxlen,
				      bool &ok)
{
	int i;
	ok = true;

	for (i = beginidx; i <= endidx; i++) {
		len += event.argv[i]->len;
		len++;
		if (len > maxlen) {
			ok = false;
			return;
		}
		*c = ' ';
		c++;
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
	}
	/*
	 * Terminate the string, it's assumed that maxlen is maximum length
	 * *excluding* terminating null character :)
	 */
	*c = '\0';
	len++;
}

/*
 * This function will merge event arguments from beginidx to endix into
 * a cstring. Such cases exists because tasknames that contains spaces have
 * been split into several arguments due to parsing with space as delimiter
 */
static vtl_always_inline void
merge_args_into_cstring(const TraceEvent &event,
			int beginidx,
			int endidx,
			char *&c,
			int &len,
			int maxlen,
			bool &ok)
{
	int i;
	ok = true;

	for (i = beginidx; i <= endidx; i++) {
		len += event.argv[i]->len;
		len++;
		if (len > maxlen) {
			ok = false;
			return;
		}
		strncpy(c, event.argv[i]->ptr, event.argv[i]->len);
		c += event.argv[i]->len;
		*c = ' ';
		c++;
	}
}

static vtl_always_inline unsigned int uint_after_char(const TraceEvent &event,
						      int n_param, char ch)
{
	char *last;
	char *first;
	char *c;
	bool found = false;
	unsigned int param = 0;
	unsigned int digit;


	last = event.argv[n_param]->ptr + event.argv[n_param]->len - 1;
	first = event.argv[n_param]->ptr;
	for (c = last; c >= first; c--) {
		if (*c == ch) {
			found = true;
			break;
		}
	}
	if (!found)
		return ABSURD_UNSIGNED; /* return absurd if error */
	c++;
	for (; c <= last; c++) {
		digit = *c - '0';
		param *= 10;
		param += digit;
	}
	return param;
}

static vtl_always_inline int int_after_char(const TraceEvent &event,
					    int n_param, char ch)
{
	char *last;
	char *first;
	char *c;
	bool found = false;
	int param = 0;
	int digit;
	bool neg = false;


	last = event.argv[n_param]->ptr + event.argv[n_param]->len - 1;
	first = event.argv[n_param]->ptr;
	for (c = last; c >= first; c--) {
		if (*c == ch) {
			found = true;
			break;
		}
	}
	if (!found)
		return ABSURD_INT; /* return absurd if error */
	c++;
	if (*c == '-') {
		neg = true;
		c++;
	}
	for (; c <= last; c++) {
		digit = *c - '0';
		param *= 10;
		param += digit;
	}
	return (neg ? -param:param);
}

static vtl_always_inline bool is_param_inside_braces(const TString *str)
{
	int s = str->len;
	bool r =  s > 2 && str->ptr[0] == '[' && str->ptr[s - 1] == ']';
	return r;
}

static vtl_always_inline
unsigned int param_inside_braces(const TraceEvent &event,
				 int n_param)
{
	int len = event.argv[n_param]->len;
	char *first = event.argv[n_param]->ptr;
	char *end = first + len - 1; /* now pointing to the final ']' */
	char *c;
	unsigned int digit, param = 0;

	first++; /* Skipt the leading '[' */

	if (len > 2) {
		for (c = first; c < end; c++) {
			digit = *c - '0';
			param *= 10;
			param += digit;
		}
		return param;
	}

	return ABSURD_UNSIGNED;
}

static vtl_always_inline const char *substr_after_char(const char *str,
						       int len,
						       char c,
						       int *sublen)
{
	int i;

	for (i = 0; i < len; i++) {
		if (*str == c) {
			i++;
			if (i == len)
				return NullStr;
			str++;
			*sublen = len - i;
			return str;
		}
		str++;
	}
	return NullStr;
}

static vtl_always_inline taskstate_t
sched_state_from_tstring_(const TString *str)
{
	int i;
	char c;
	taskstate_t state = 0;
	taskstate_t flag;

	if (str->len == 1 && str->ptr[0] == TASK_SCHAR_RUNNABLE)
		return TASK_STATE_RUNNABLE;
	else if (str->len == 2 &&
		 str->ptr[0] == TASK_SCHAR_RUNNABLE &&
		 str->ptr[1] == TASK_CHAR_PREEMPT)
		return TASK_STATE_RUNNABLE | TASK_FLAG_PREEMPT;

	for (i = 0; i < str->len; i++) {
		c = str->ptr[i];
		switch (c) {
		case TASK_CHAR_INTERRUPTIBLE:
			flag = TASK_FLAG_INTERRUPTIBLE;
			break;
		case TASK_CHAR_UNINTERRUPTIBLE:
			flag = TASK_FLAG_UNINTERRUPTIBLE;
			break;
		case TASK_CHAR_STOPPED:
			flag = TASK_FLAG_STOPPED;
			break;
		case TASK_CHAR_TRACED:
			flag = TASK_FLAG_TRACED;
			break;
		case TASK_CHAR_EXIT_DEAD:
			flag = TASK_FLAG_EXIT_DEAD;
			break;
		case TASK_CHAR_EXIT_ZOMBIE:
			flag = TASK_FLAG_EXIT_ZOMBIE;
			break;
		case TASK_CHAR_DEAD:
			flag = TASK_FLAG_DEAD;
			break;
		case TASK_CHAR_WAKEKILL:
			flag = TASK_FLAG_WAKEKILL;
			break;
		case TASK_CHAR_WAKING:
			flag = TASK_FLAG_WAKING;
			break;
		case TASK_CHAR_PARKED:
			flag = TASK_FLAG_PARKED;
			break;
		case TASK_CHAR_NOLOAD:
			flag = TASK_FLAG_NOLOAD;
			break;
		case TASK_CHAR_PREEMPT:
			flag = TASK_FLAG_PREEMPT;
			break;
		case TASK_CHAR_SEPARATOR:
			flag = 0;
			break;
		 /* Let's accept spaces in the task state string also */
		case ' ':
			flag = 0;
			break;
		default:
			return TASK_STATE_PARSER_ERROR;
			break;
		};
		state |= flag;
	}
	return state;
}

static vtl_always_inline
unsigned int uint_after_pfix(const TraceEvent &event,
			     int idx_guess,
			     const char* pfix)
{
	int i;

	if (unlikely(prefixcmp(event.argv[idx_guess]->ptr, pfix) != 0)) {
		/*
		 * If the expected argument doesn't contain the wanted prefix
		 * we will search for it. If we don't find it, we just use
		 * idx_guess anyway.
		 */
		for (i = 0; i < event.argc; i++) {
			if (prefixcmp(event.argv[i]->ptr, pfix) == 0)
				return uint_after_char(event, i, '=');
		}
	}
	return uint_after_char(event, idx_guess, '=');
}

static vtl_always_inline int int_after_pfix(const TraceEvent &event,
					    int idx_guess,
					    const char* pfix)
{
	int i;

	if (unlikely(prefixcmp(event.argv[idx_guess]->ptr, pfix) != 0)) {
		/*
		 * If the expected argument doesn't contain the wanted prefix
		 * we will search for it. If we don't find it, we just use
		 * idx_guess anyway.
		 */
		for (i = 0; i < event.argc; i++) {
			if (prefixcmp(event.argv[i]->ptr, pfix) == 0)
				return int_after_char(event, i, '=');
		}
	}
	return int_after_char(event, idx_guess, '=');
}

#endif /* PARAMHELPERS_H */
