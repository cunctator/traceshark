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

#ifndef FTRACEGRAMMAR_H
#define FTRACEGRAMMAR_H

#include "misc/traceshark.h"
#include "mm/stringpool.h"
#include "mm/stringtree.h"
#include "parser/paramhelpers.h"
#include "parser/traceevent.h"
#include "vtl/time.h"

#define NEXTTOKEN(IS_LEAF)			\
	{					\
		n--;				\
		if (n == 0)			\
			return (IS_LEAF);	\
		str++;				\
	}					\

class FtraceGrammar
{
public:
	FtraceGrammar();
	~FtraceGrammar();
	void clear();
	__always_inline bool parseLine(const TraceLine &line,
				       TraceEvent &event);
	StringTree<> *eventTree;
private:
	void setupEventTree();
	__always_inline bool NamePidMatch(const TString *str,
					  TraceEvent &event);
	__always_inline bool CPUMatch(const TString *str,
				      TraceEvent &event);
	__always_inline bool extractNameAndPid(int &pid, TString &compound);
	__always_inline bool TimeMatch(const TString *str, TraceEvent &event);
	__always_inline bool EventMatch(const TString *str, TraceEvent &event);
	__always_inline bool ArgMatch(const TString *str, TraceEvent &event);
	StringPool<> *argPool;
	StringPool<> *namePool;
	int unknownTypeCounter;
	typedef enum : int {
		STATE_NAMEPID = 0,
		STATE_CPU,
		STATE_TIME,
		STATE_EVENT,
		STATE_ARG
	} grammarstate_t;
	int tmp_argc;
	const TString *tmp_argv[EVENT_MAX_NR_ARGS];
};

__always_inline bool FtraceGrammar::NamePidMatch(const TString *str,
						 TraceEvent &/*event*/)
{
	/*
	 * We temporarily store the process name string(s) into the
	 * argv/argc fields of the event, because we don't know how many
	 * strings the process name will be split into. It may have been
	 * split into several strings due to the process name containing
	 * spaces. We will then consume this stored information in the
	 * TimeMatch function.
	 */
	if (tmp_argc >= EVENT_MAX_NR_ARGS)
		return false;
	tmp_argv[tmp_argc] = str;
	tmp_argc++;
	return true;
}

__always_inline bool FtraceGrammar::CPUMatch(const TString *str,
					     TraceEvent &event)
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
	tmp_argv[tmp_argc] = str;
	tmp_argc++;
	return true;
error:
	event.cpu = 0;
	return false;
}

__always_inline bool FtraceGrammar::extractNameAndPid(int &pid,
						      TString &compound)
{
	char *nullChr = compound.ptr + compound.len;
	char *lastChr = nullChr - 1;
	char *c;
	char *beginPid;
	int digit;

	if (compound.len < 2)
		return false;

	for (c = lastChr - 1; c >= compound.ptr; c--) {
		if (*c == '-')
			goto found1;
	}
	return false;
found1:
	/* Cut the compound string so that only the name remains */
	compound.len = compound.len - (nullChr - c);
	*c = '\0';
	beginPid = c + 1;

	/*
	 * Use the remainder, that was cutoff from the name, as separated by the
	 * '-' sign get the pid
	 */
	pid = 0;
	for (c = beginPid; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return false;
	}

	return true;
}

__always_inline bool FtraceGrammar::TimeMatch(const TString *str,
					      TraceEvent &event)
{
	bool rval;
	TString namestr;
	TString finistr;
	const TString *newname;
	char nbuf[256];
	char fbuf[256];
	const int maxlen = arraylen(nbuf) - 1;
	int i;
	int fini;

	namestr.ptr = nbuf;
	namestr.len = 0;
	finistr.ptr = fbuf;
	finistr.len = 0;

	/*
	 * atof() and sscanf() are not up to the task because they are
	 * too slow and get confused by locality issues.
	 */
	event.time = vtl::Time::fromString(str->ptr, rval);;

	/*
	 * This is the time field, if it is successful we need to assemble
	 * the name and pid strings that has been temporarily stored in
	 * tmp_argv/tmp_argc.
	 */
	if (rval) {
		if (tmp_argc < 2)
			return false;

		fini = tmp_argc - 2;
		/*
		 * Extract the pid and the final portion of the name from
		 * tmp_argv[fini]
		 */
		finistr.set(tmp_argv[fini], maxlen);
		if (!extractNameAndPid(event.pid, finistr))
			return false;

		if (tmp_argc > 2) {
			namestr.set(tmp_argv[0], maxlen);
			/*
			 * This will assemble broken up parts of the process
			 * name; those that have been broken up by spaces in
			 * the name.
			 */
			for (i = 1; i < tmp_argc - 2; i++) {
				if (!namestr.merge(tmp_argv[i], maxlen))
					return false;
			}
			if (!namestr.merge(&finistr, maxlen))
				return false;
			newname = namePool->allocString(&namestr, 0);
		} else {
			/* This is the common case, no spaces in the name. */
			newname = namePool->allocString(&finistr, 0);
		}

		if (newname == nullptr)
			return false;
		event.taskName = newname;
	}
	return rval;
}

__always_inline bool FtraceGrammar::EventMatch(const TString *str,
					       TraceEvent &event)
{
	char buf[512];
	TString estr;
	estr.len = 0;
	estr.ptr = buf;
	const int maxlen = arraylen(buf) - 1;
	estr.set(str, maxlen);
	char *lastChr = estr.ptr + estr.len - 1;
	event_t type;

	if (estr.len < 1)
		return false;

	if (*lastChr == ':') {
		*lastChr = '\0';
		estr.len--;
	} else
		return false;

	type = eventTree->searchAllocString(&estr,
					    (event_t) unknownTypeCounter);
	if (type == EVENT_ERROR)
		return false;
	else if (type == unknownTypeCounter) {
		/*
		 * This event is a new event, so for the next one we need to
		 * bump the counter in order to use a unique eventType value 
		 * for every event name
		 */
		unknownTypeCounter++;
	}
	event.type = type;
	return true;
}

__always_inline bool FtraceGrammar::ArgMatch(const TString *str,
					     TraceEvent &event)
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


__always_inline bool FtraceGrammar::parseLine(const TraceLine &line,
					      TraceEvent &event)
{
	const TString *str = line.strings;
	int n = line.nStrings;
	grammarstate_t state = STATE_NAMEPID;
	tmp_argc = 0;

	if (n == 0)
		return false;

	do {
		switch(state) {
		case STATE_NAMEPID:
			if (!NamePidMatch(str, event))
				return false;
			NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_CPU:
			if (!CPUMatch(str, event)) {
				state = STATE_NAMEPID;
				break;
			}
			NEXTTOKEN(false);
			ts_fallthrough;
		case STATE_TIME:
			if (!TimeMatch(str, event)) {
				state = STATE_NAMEPID;
				break;
			}
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

#endif /* FTRACEGRAMMAR_H */
