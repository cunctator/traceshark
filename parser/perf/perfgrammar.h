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

#ifndef PERFGRAMMAR_H
#define PERFGRAMMAR_H

#include "misc/traceshark.h"
#include "mm/stringpool.h"
#include "mm/stringtree.h"
#include "parser/traceevent.h"

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
	__always_inline bool parseLine(TraceLine &line, TraceEvent &event);
	StringTree *eventTree;
private:
	void setupEventTree();
	__always_inline bool StoreMatch(TString *str, TraceEvent &event);
	__always_inline bool NameMatch(TString *str, TraceEvent &event);
	__always_inline bool IntArgMatch(TString *str, TraceEvent &event);
	__always_inline int pidFromString(const TString &str);
	__always_inline bool PidMatch(TString *str, TraceEvent &event);
	__always_inline bool CPUMatch(TString *str, TraceEvent &event);
	__always_inline bool TimeMatch(TString *str, TraceEvent &event);
	__always_inline bool EventMatch(TString *str, TraceEvent &event);
	__always_inline bool ArgMatch(TString *str, TraceEvent &event);
	StringPool *argPool;
	StringPool *namePool;
	/* This is a counter that will count up every time a new event name
	 * is encountered, so that we get unique event types for every 
	 * unknown event name */
	int unknownTypeCounter;
	typedef enum {
		STATE_NAME = 0,
		STATE_PID,
		STATE_CPU,
		STATE_TIME,
		STATE_INTARG,
		STATE_EVENT,
		STATE_ARG
	} grammarstate_t;
};

__always_inline bool PerfGrammar::StoreMatch(TString *str,
					       TraceEvent &event)
{
	/* We temporarily store the process name string(s) into the
	 * argv/argc fields of the event, because we don't know how many
	 * strings the process name will be split into. It may have been
	 * split into several strings due to the process name containing
	 * spaces. We will then consume this stored information in the
	 * TimeNode class */
	if (event.argc >= 256)
		return false;
	event.argv[event.argc] = str;
	event.argc++;
	return true;
}

__always_inline bool PerfGrammar::NameMatch(TString *str,
					      TraceEvent &event)
{
	return StoreMatch(str, event);
}

__always_inline bool PerfGrammar::IntArgMatch(TString *str,
					      TraceEvent &event)
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

__always_inline bool PerfGrammar::PidMatch(TString *str,
					      TraceEvent &event)
{
	return StoreMatch(str, event);
}

__always_inline bool PerfGrammar::CPUMatch(TString *str,
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
	event.argv[event.argc] = str;
	event.argc++;
	return true;
error:
	event.cpu = 0;
	return false;
}

__always_inline int PerfGrammar::pidFromString(const TString &str)
{
	char *lastChr = str.ptr + str.len - 1;
	int pid;
	int digit;
	char *c;

	if (str.len < 1 || str.len > 10)
		return false;

	pid = 0;
	for (c = str.ptr; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return -1;
	}
	return pid;
}

__always_inline bool PerfGrammar::TimeMatch(TString *str,
					      TraceEvent &event)
{
	bool rval;
	TString namestr;
	TString *newname;
	char cstr[256];
	const unsigned int maxlen = sizeof(cstr) / sizeof(char) - 1;
	unsigned int i;
	int pid;
	uint32_t hash;

	namestr.ptr = cstr;
	namestr.len = 0;

	/* atof() and sscanf() are buggy */
	event.time = TShark::timeStrToDouble(str->ptr, rval);

	/* This is the time field, if it is successful we need to assemble
	 * the name and pid strings that has been temporarily stored in
	 * argv/argc */
	if (rval) {
		if (event.argc < 3)
			return false;

		pid = pidFromString(*event.argv[event.argc - 2]);
		if (pid < 0)
			return false;
		event.pid = pid;

		if (event.argc > 3) {
			namestr.set(event.argv[0], maxlen);
			for (i = 1; i < event.argc - 2; i++) {
				if (!namestr.merge(event.argv[i], maxlen))
					return false;
			}

			hash = TShark::StrHash32(&namestr);
			newname = namePool->allocString(&namestr, hash, 0);
		} else {
			hash = TShark::StrHash32(event.argv[0]);
			newname = namePool->allocString(event.argv[0], hash,
							0);
		}
		if (newname == nullptr)
			return false;
		event.taskName = newname;
		event.argc = 0;
	}
	return rval;
}

__always_inline bool PerfGrammar::EventMatch(TString *str,
					       TraceEvent &event)
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
					    TShark::StrHash32(&tmpstr),
					    (event_t) unknownTypeCounter);
	if (type == EVENT_ERROR)
		return false;
	else if (type == unknownTypeCounter) {
		/* This event is a new event, so for the next one we need to
		 * bump the counter in order to use a unique eventType value 
		 * for every event name */
		unknownTypeCounter++;
	}
	event.type = type;
	return true;
}

__always_inline bool PerfGrammar::ArgMatch(TString *str,
					     TraceEvent &event)
{
	TString *newstr;
	if (event.argc < 255) {
		newstr = argPool->allocString(str, TShark::StrHash32(str), 16);
		if (newstr == nullptr)
			return false;
		event.argv[event.argc] = newstr;
		event.argc++;
		return true;
	}
	return false;
}


__always_inline bool PerfGrammar::parseLine(TraceLine &line, TraceEvent &event)
{
	TString *str = line.strings;
	unsigned int n = line.nStrings;
	grammarstate_t state = STATE_NAME;

	if (n == 0)
		return false;

	do {
		switch(state) {
		case STATE_NAME:
			if (NameMatch(str, event)) {
				NEXTTOKEN(false);
				state = STATE_PID;
				break;
			}
			return false;
		case STATE_PID:
			if (PidMatch(str, event)) {
				NEXTTOKEN(false);
				state = STATE_CPU;
				break;
			}
			return false;
		case STATE_CPU:
			if (CPUMatch(str, event)) {
				NEXTTOKEN(false);
				state = STATE_TIME;
				break;
			}
			state = STATE_PID;
			break;
		case STATE_TIME:
			if (TimeMatch(str, event)) {
				NEXTTOKEN(false);
				state = STATE_INTARG;
				break;
			}
			state = STATE_PID;
			break;
		case STATE_INTARG:
			if (IntArgMatch(str, event))
				NEXTTOKEN(false);
			/* This is an intentional fall through, there is no
			 * missing break statement here */
		case STATE_EVENT:
			if (EventMatch(str, event)) {
				NEXTTOKEN(true);
				state = STATE_ARG;
				break;
			}
			return false;
		case STATE_ARG:
			if (ArgMatch(str, event)) {
				NEXTTOKEN(true);
				state = STATE_ARG;
				break;
			}
			return false;
		}
	} while(true);
}

#undef NEXTTOKEN

#endif /* PERFGRAMMAR_H */
