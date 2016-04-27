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

#ifndef FTRACEGRAMMAR_H
#define FTRACEGRAMMAR_H

#include "misc/traceshark.h"
#include "mm/stringpool.h"
#include "mm/stringtree.h"

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
	__always_inline bool parseLine(TraceLine &line, TraceEvent &event);
private:
	void setupEventTree();
	__always_inline bool NamePidMatch(TString *str, TraceEvent &event);
	__always_inline bool CPUMatch(TString *str, TraceEvent &event);
	__always_inline bool extractNameAndPid(TString &dstr,
					       unsigned int &pid,
					       TString &compound,
					       unsigned int maxlen);
	__always_inline bool TimeMatch(TString *str, TraceEvent &event);
	__always_inline bool EventMatch(TString *str, TraceEvent &event);
	__always_inline bool ArgMatch(TString *str, TraceEvent &event);
	StringPool *argPool;
	StringPool *namePool;
	StringTree *eventTree;
	typedef enum {
		STATE_NAMEPID = 0,
		STATE_CPU,
		STATE_TIME,
		STATE_EVENT,
		STATE_ARG
	} grammarstate_t;
};

__always_inline bool FtraceGrammar::NamePidMatch(TString *str,
						 TraceEvent &event)
{
	/* We temporarily store the process name string(s) into the
	 * argv/argc fields of the event, because we don't know how many
	 * strings the process name will be split into. It may have been
	 * split into several strings due to the process name containing
	 * spaces. We will then consume this stored information in the
	 * TimeMatch function */
	if (event.argc >= 256)
		return false;
	event.argv[event.argc] = str;
	event.argc++;
	return true;
}

__always_inline bool FtraceGrammar::CPUMatch(TString *str,
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

__always_inline bool FtraceGrammar::extractNameAndPid(TString &dstr,
						      unsigned int &pid,
						      TString &compound,
						      unsigned int maxlen)
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

	/* Use the remainder, that was cutoff from the name, as separated by the
	 * '-' sign get the pid */
	pid = 0;
	for (c = beginPid; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return false;
	}

	dstr.merge(&compound, maxlen);
	return true;
}

__always_inline bool FtraceGrammar::TimeMatch(TString *str,
					      TraceEvent &event)
{
	bool rval;
	TString namestr;
	TString *newname;
	char cstr[256];
	const unsigned int maxlen = sizeof(cstr) / sizeof(char) - 1;
	unsigned int i;

	namestr.ptr = cstr;
	namestr.len = 0;

	/* atof() and sscanf() are not up to the task because they are
	 * too slow and get confused by locality issues */
	event.time = TShark::timeStrToDouble(str->ptr, rval);

	/* This is the time field, if it is successful we need to assemble
	 * the name and pid strings that has been temporarily stored in
	 * argv/argc */
	if (rval) {
		if (event.argc < 2)
			return false;


		i = 0;
		if (event.argc > 2) {
			namestr.set(event.argv[0], maxlen);
			/* This will assemble broken up parts of the process
			 * name; those that have been broken up by spaces in
			 * the name */
			for (i = 1; i < event.argc - 2; i++) {
				if (!namestr.merge(event.argv[i], maxlen))
					return false;
			}
		}

		/* Extract the final portion of the name from event.argv[i]
		 * and the pid. */
		if (!extractNameAndPid(namestr, event.pid,
				       *event.argv[i],
				       maxlen))
			return false;

		newname = namePool->allocString(&namestr,
						TShark::StrHash32(&namestr), 0);
		if (newname == nullptr)
			return false;
		event.taskName = newname;
		/* Need to reset the arguments because the real arguments
		 * will be stored after the succesful completion of the next
		 * node */
		event.argc = 0;
	}
	return rval;
}

__always_inline bool FtraceGrammar::EventMatch(TString *str,
					       TraceEvent &event)
{
	char *lastChr = str->ptr + str->len - 1;
	TString *newstr;
	event_t type;

	if (str->len < 1)
		return false;

	if (*lastChr == ':') {
		*lastChr = '\0';
		str->len--;
	} else
		return false;

	newstr = eventTree->searchAllocString(str, TShark::StrHash32(str),
					      &type, EVENT_UNKNOWN);
	if (newstr == nullptr)
		return false;
	event.eventName = newstr;
	event.type = type;
	return true;
}

__always_inline bool FtraceGrammar::ArgMatch(TString *str,
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


__always_inline bool FtraceGrammar::parseLine(TraceLine &line,
					      TraceEvent &event)
{
	TString *str = line.strings;
	unsigned int n = line.nStrings;
	grammarstate_t state = STATE_NAMEPID;

	if (n == 0)
		return false;

	do {
		switch(state) {
		case STATE_NAMEPID:
			if (NamePidMatch(str, event)) {
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
			state = STATE_NAMEPID;
			break;
		case STATE_TIME:
			if (TimeMatch(str, event)) {
				NEXTTOKEN(false);
				state = STATE_EVENT;
				break;
			}
			state = STATE_NAMEPID;
			break;
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

#endif /* FTRACEGRAMMAR_H */
