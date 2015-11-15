/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
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


/* + 1 needed for null termination */
#define taskname_prealloc(POOL) \
	((char*) POOL->preallocChars(TASKNAME_MAXLEN + 1))

/* Warning, this function does not null terminate the string!
 * This function copies everything after the delim char into the cstring
 * pointed to by c and adds the length to len */
static __always_inline void __copy_tstring_after_char(TString *str,
						      char delim,
						      char *&dest,
						      unsigned int &len,
						      unsigned int maxlen,
						      bool &ok)
{
	unsigned int i;
	unsigned int flen;
	char *src;
	ok = true;
	/* Find the char 'delim' in the string */
	for (i = 0; i < str->len; i++) {
		if (str->ptr[i] == delim)
			break;
	}
	if (i >= str->len)
		goto err;
	/* ...then copy everything after the delim char into the name, if there
	 * is anything to copy */
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

/* This function will merge event arguments from beginidx to endix into
 * a cstring. Such cases exists because tasknames that contains spaces have
 * been split into several arguments due to parsing with space as delimiter */
static __always_inline void merge_args_into_cstring(TraceEvent &event,
						    unsigned int beginidx,
						    unsigned int endidx,
						    char *&c,
						    unsigned int &len,
						    unsigned int maxlen,
						    bool &ok)
{
	unsigned int i;
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
	/* Terminate the string, it's assumed that maxlen is maximum length
	 * *excluding* terminating null character :) */
	c = '\0';
	len++;
}

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
