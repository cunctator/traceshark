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
#include "parser/perftimenode.h"
#include "parser/traceevent.h"
#include "misc/traceshark.h"

PerfTimeNode::PerfTimeNode(const char *name)
	: GrammarNode(name)
{
	namePool = new StringPool(1024, 65536);
}

PerfTimeNode::~PerfTimeNode()
{
	delete namePool;
}

bool PerfTimeNode::match(TString *str, TraceEvent *event)
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
	event->time = TShark::timeStrToDouble(str->ptr, rval);

	/* This is the time field, if it is successful we need to assemble
	 * the name and pid strings that has been temporarily stored in
	 * argv/argc */
	if (rval) {
		if (event->argc < 3)
			return false;

		pid = pidFromString(*event->argv[event->argc - 2]);
		if (pid < 0)
			return false;
		event->pid = pid;

		if (event->argc > 3) {
			namestr.set(event->argv[0], maxlen);
			for (i = 1; i < event->argc - 2; i++) {
				if (!namestr.merge(event->argv[i], maxlen))
					return false;
			}

			hash = TShark::StrHash32(&namestr);
			newname = namePool->allocString(&namestr, hash, 0);
		} else {
			hash = TShark::StrHash32(event->argv[0]);
			newname = namePool->allocString(event->argv[0], hash,
							0);
		}
		if (newname == nullptr)
			return false;
		event->taskName = newname;
		event->argc = 0;
	}
	return rval;
}
