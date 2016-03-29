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

#include "eventnode.h"
#include "mm/stringtree.h"
#include "traceevent.h"
#include "tstring.h"

EventNode::EventNode(const char *name)
	: GrammarNode(name)
{
	eventTree = new StringTree(8, 256);
	setupTree();
}

EventNode::~EventNode()
{
	delete eventTree;
}

bool EventNode::match(TString *str, TraceEvent *event)
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
	event->eventName = newstr;
	event->type = type;
	return true;
}

void EventNode::clearStringPool()
{
	eventTree->clear();
	setupTree();
}

void EventNode::setupTree()
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
