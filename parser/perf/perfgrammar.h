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

#ifndef PERFGRAMMAR_H
#define PERFGRAMMAR_H

#include "misc/traceshark.h"
#include "mm/stringpool.h"
#include "mm/stringtree.h"
#include "parser/traceevent.h"
#include "vtl/compiler.h"
#include "vtl/time.h"

#define NEXTTOKEN(IS_LEAF)			\
	{					\
		n--;				\
		if (n == 0)			\
			return (IS_LEAF);	\
		str++;				\
	}					\

class PerfGrammar
{
public:
	PerfGrammar();
	~PerfGrammar();
	void clear();
	vtl_always_inline bool parseLine(TraceLine &line, TraceEvent &event);
	StringTree<> *eventTree;
private:
	void setupEventTree();
	vtl_always_inline bool StoreMatch(TString *str, TraceEvent &event);
	vtl_always_inline bool NameMatch(TString *str, TraceEvent &event);
	vtl_always_inline bool IntArgMatch(TString *str, TraceEvent &event);
	vtl_always_inline int pidFromString(const TString &str, bool &ok);
	vtl_always_inline bool PidMatch(TString *str, TraceEvent &event);
	vtl_always_inline bool CPUMatch(TString *str, TraceEvent &event);
	vtl_always_inline bool TimeMatch(TString *str, TraceEvent &event);
	vtl_always_inline bool EventMatch(TString *str, TraceEvent &event);
	vtl_always_inline bool ArgMatch(TString *str, TraceEvent &event);
	StringPool<> *argPool;
	StringPool<> *namePool;

	/*
	 * This is a counter that will count up every time a new event name
	 * is encountered, so that we get unique event types for every 
	 * unknown event name.
	 */
	int unknownTypeCounter;

	typedef enum : int {
		STATE_NAME = 0,
		STATE_PID,
		STATE_CPU,
		STATE_TIME,
		STATE_INTARG,
		STATE_EVENT,
		STATE_ARG
	} grammarstate_t;
};

vtl_always_inline bool PerfGrammar::StoreMatch(TString *str, TraceEvent &event)
{
	/*
	 * We temporarily store the process name string(s) into the
	 * argv/argc fields of the event, because we don't know how many
	 * strings the process name will be split into. It may have been
	 * split into several strings due to the process name containing
	 * spaces. We will then consume this stored information in the
	 * TimeNode class.
	 */
	if (event.argc >= EVENT_MAX_NR_ARGS)
		return false;
	event.argv[event.argc] = str;
	event.argc++;
	return true;
}

vtl_always_inline bool PerfGrammar::NameMatch(TString *str, TraceEvent &event)
{
	return StoreMatch(str, event);
}

vtl_always_inline bool PerfGrammar::IntArgMatch(TString *str, TraceEvent &event)
{
	char *lastChr = str->ptr + str->len - 1;
	char *c = str->ptr;
	int arg = 0;
	bool negative = false;
	int digit;

	if (*c == '-') {
		negative = true;
		c++;
	}

	for (; c <= lastChr; c++) {
		arg *= 10;
		digit = *c - '0';
		if (digit <=9 && digit >= 0) {
			arg += digit;
		} else {
			event.intArg = 0;
			return false;
		}
	}
	if (negative)
		arg = - arg;
	event.intArg = arg;
	return true;
}

vtl_always_inline bool PerfGrammar::PidMatch(TString *str, TraceEvent &event)
{
	return StoreMatch(str, event);
}

vtl_always_inline bool PerfGrammar::CPUMatch(TString *str, TraceEvent &event)
{
	char *c;
	unsigned int cpu = 0;
	int digit;

	if (str->ptr[0] != '[')
		return false;

	cpu = 0;
	for (c = str->ptr + 1; *c != '\0' && *c != ']'; c++) {
		digit = *c - '0';
		if (digit > 9 || digit < 0)
			goto error;
		cpu *= 10;
		cpu += digit;
	}
	event.cpu = cpu;
	event.argv[event.argc] = str;
	event.argc++;
	return true;
error:
	event.cpu = 0;
	return false;
}

vtl_always_inline int PerfGrammar::pidFromString(const TString &str, bool &ok)
{
	char *lastChr = str.ptr + str.len - 1;
	int pid;
	int digit;
	char *c;
	bool neg = false;
	ok = true;

	if (str.len < 1 || str.len > 10)
		return false;

	pid = 0;
	c = str.ptr;
	if (*c == '-') {
		neg = true;
		c++;
	}
	for (; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else {
			ok = false;
			return 0;
		}
	}
	return (neg ? -pid:pid);
}

vtl_always_inline bool PerfGrammar::TimeMatch(TString *str, TraceEvent &event)
{
	bool rval;
	TString namestr;
	const TString *newname;
	char cstr[256];
	const unsigned int maxlen = arraylen(cstr) - 1;
	int i;
	int pid;
	bool ok;

	namestr.ptr = cstr;
	namestr.len = 0;

	/* atof() and sscanf() are buggy. */
	event.time = vtl::Time::fromString(str->ptr, rval);

	/*
	 * This is the time field, if it is successful we need to assemble
	 * the name and pid strings that has been temporarily stored in
	 * argv/argc.
	 */
	if (rval) {
		if (event.argc < 3)
			return false;

		pid = pidFromString(*event.argv[event.argc - 2], ok);
		if (!ok)
			return false;
		event.pid = pid;

		if (event.argc > 3) {
			namestr.set(event.argv[0], maxlen);
			for (i = 1; i < event.argc - 2; i++) {
				if (!namestr.merge(event.argv[i], maxlen))
					return false;
			}

			newname = namePool->allocString(&namestr, 0);
		} else {
			newname = namePool->allocString(event.argv[0], 0);
		}
		if (newname == nullptr)
			return false;
		event.taskName = newname;
		event.argc = 0;
	}
	return rval;
}

vtl_always_inline bool PerfGrammar::EventMatch(TString *str, TraceEvent &event)
{
	char *lastChr = str->ptr + str->len - 1;
	char *c;
	TString tmpstr;
	event_t type;

	if (str->len < 1)
		return false;

	if (*lastChr == ':') {
		*lastChr = '\0';
		str->len--;
	} else
		return false;

	for (c = str->ptr; c < lastChr; c++)
		if (*c == ':')
			break;

	if (c < lastChr) {
		tmpstr.ptr = c + 1;
		tmpstr.len = lastChr - tmpstr.ptr;
	} else {
		tmpstr.ptr = str->ptr;
		tmpstr.len = str->len;
	}

	type = eventTree->searchAllocString(&tmpstr,
					    (event_t) unknownTypeCounter);
	if (type == EVENT_ERROR)
		return false;
	else if (type == unknownTypeCounter) {
		/*
		 * This event is a new event, so for the next one we need to
		 * bump the counter in order to use a unique eventType value 
		 * for every event name.
		 */
		unknownTypeCounter++;
	}
	event.type = type;
	return true;
}

vtl_always_inline bool PerfGrammar::ArgMatch(TString *str, TraceEvent &event)
{
	const TString *newstr;
	if (event.argc < EVENT_MAX_NR_ARGS) {
		newstr = argPool->allocString(str, 16);
		if (newstr == nullptr)
			return false;
		event.argv[event.argc] = newstr;
		event.argc++;
		return true;
	}
	return false;
}


vtl_always_inline
bool PerfGrammar::parseLine(TraceLine &line, TraceEvent &event)
{
	TString *str = line.strings;
	unsigned int n = line.nStrings;
	grammarstate_t state = STATE_NAME;

	if (n == 0)
		return false;

	do {
		switch(state) {
		case STATE_NAME:
			if (!NameMatch(str, event))
				return false;
			NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_PID:
			if (!PidMatch(str, event))
				return false;
			NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_CPU:
			if (!CPUMatch(str, event)) {
				state = STATE_PID;
				break;
			}
			NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_TIME:
			if (!TimeMatch(str, event)) {
				state = STATE_PID;
				break;
			}
			NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_INTARG:
			/*
			 * Intarg is optional, if it's here we fetch the
			 * next token and continue, if it's not then we just
			 * continue.
			 */
			if (IntArgMatch(str, event))
				NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_EVENT:
			if (!EventMatch(str, event))
				return false;
			NEXTTOKEN(true);
			ts_fallthrough;
		case STATE_ARG:
			while (ArgMatch(str, event))
				NEXTTOKEN(true);
			return false;
		}
	} while(true);
}

#undef NEXTTOKEN

#endif /* PERFGRAMMAR_H */
