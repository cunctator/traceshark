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

#include <cstdlib>
#include <cstring>

#include "mm/stringpool.h"
#include "timenode.h"
#include "traceevent.h"
#include "traceshark.h"

TimeNode::TimeNode(const char *name)
	: GrammarNode(name)
{
	namePool = new StringPool(1024, 65536);
}

TimeNode::~TimeNode()
{
	delete namePool;
}

bool TimeNode::match(TString *str, TraceEvent *event)
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
	event->time = TShark::timeStrToDouble(str->ptr, rval);

	/* This is the time field, if it is successful we need to assemble
	 * the name and pid strings that has been temporarily stored in
	 * argv/argc */
	if (rval) {
		if (event->argc < 2)
			return false;


		i = 0;
		if (event->argc > 2) {
			namestr.set(event->argv[0], maxlen);
			/* This will assemble broken up parts of the process
			 * name; those that have been broken up by spaces in
			 * the name */
			for (i = 1; i < event->argc - 2; i++) {
				if (!namestr.merge(event->argv[i], maxlen))
					return false;
			}
		}

		/* Extract the final portion of the name from event->argv[i]
		 * and the pid. */
		if (!extractNameAndPid(namestr, event->pid,
				       *event->argv[i],
				       maxlen))
			return false;

		newname = namePool->allocString(&namestr,
						TShark::StrHash32(&namestr), 0);
		if (newname == nullptr)
			return false;
		event->taskName = newname;
		/* Need to reset the arguments because the real arguments
		 * will be stored after the succesful completion of the next
		 * node */
		event->argc = 0;
	}
	return rval;
}

