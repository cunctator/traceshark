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

#include "parser/perfeventnode.h"
#include "mm/stringtree.h"
#include "parser/ftraceparams.h"
#include "parser/traceevent.h"
#include "misc/tstring.h"

PerfEventNode::PerfEventNode(const char *name)
	: GrammarNode(name)
{
	eventTree = new StringTree(8, 256);
	setupTree();
}

PerfEventNode::~PerfEventNode()
{
	delete eventTree;
}

bool PerfEventNode::match(TString *str, TraceEvent *event)
{
	char *lastChr = str->ptr + str->len - 1;
	char *c = str->ptr;
	TString *newstr;
	TString tmpstr;
	event_t type;

	if (str->len < 1)
		return false;

	if (*lastChr == ':') {
		*lastChr = '\0';
		str->len--;
	} else
		return false;

	do {
		if (c >= lastChr)
			return false;
		if (*c == ':')
			break;
		c++;
	} while(true);

	tmpstr.ptr = c + 1;
	if (tmpstr.ptr >= lastChr)
		return false;
	tmpstr.len = lastChr - tmpstr.ptr;

	newstr = eventTree->searchAllocString(&tmpstr,
					      TShark::StrHash32(&tmpstr),
					      &type, EVENT_UNKNOWN);
	if (newstr == nullptr)
		return false;
	event->eventName = newstr;
	event->type = type;
	return true;
}

void PerfEventNode::clearStringPool()
{
	eventTree->clear();
	setupTree();
}

void PerfEventNode::setupTree()
{
	int t;
	event_t dummy;
	TString str;
	QTextStream qout(stdout);

	for (t = 0; t < NR_EVENTS; t++) {
		str.ptr = eventstrings[t];
		str.len = strlen(eventstrings[t]);
		eventTree->searchAllocString(&str, TShark::StrHash32(&str),
					     &dummy, (event_t) t);
	}
}
