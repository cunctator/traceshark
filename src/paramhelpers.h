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

#ifndef PARAMHELPERS_H
#define PARAMHELPERS_H

#include "traceevent.h"

#define TASKNAME_MAXLEN (128) /* Should be enough, I wouldn't expect more
				 than about 16 */

#define ABSURD_UNSIGNED (2147483647)

#define is_this_event(EVENTNAME, EVENT) (EVENT.type == EVENTNAME)

#define isArrowStr(str) (str->len == 3 && str->ptr[0] == '=' && \
			 str->ptr[1] == '=' && str->ptr[2] == '>')


static __always_inline unsigned int param_after_char(const TraceEvent &event,
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

static __always_inline unsigned int param_inside_braces(TraceEvent &event,
					       unsigned int n_param)
{
	unsigned int len = event.argv[n_param]->len;
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

static __always_inline const char *substr_after_char(const char *str,
					       unsigned int len,
					       char c,
					       unsigned int *sublen)
{
	unsigned int i;

	for (i = 0; i < len; i++) {
		if (*str == c) {
			i++;
			if (i == len)
				return NULL;
			str++;
			*sublen = len - i;
			return str;
		}
		str++;
	}
	return NULL;
}

#endif /* PARAMHELPERS_H */
