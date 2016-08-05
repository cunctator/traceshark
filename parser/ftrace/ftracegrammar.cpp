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

#include "parser/ftrace/ftracegrammar.h"
#include "parser/traceevent.h"

FtraceGrammar::FtraceGrammar() :
	unknownTypeCounter(EVENT_UNKNOWN)
{
	argPool = new StringPool(2048, 1024 * 1024);
	namePool =  new StringPool(1024, 65536);
	eventTree = new StringTree(8, 256, 4096);
	setupEventTree();
}

FtraceGrammar::~FtraceGrammar()
{
	delete argPool;
	delete namePool;
	delete eventTree;
}

void FtraceGrammar::clear()
{
	argPool->clear();
	namePool->clear();
	eventTree->clear();
	setupEventTree();
	unknownTypeCounter = EVENT_UNKNOWN;
}

void FtraceGrammar::setupEventTree()
{
	int t;
	TString str;

	for (t = 0; t < NR_EVENTS; t++) {
		str.ptr = eventstrings[t];
		str.len = strlen(eventstrings[t]);
		eventTree->searchAllocString(&str, TShark::StrHash32(&str),
					     (event_t) t);
	}
}
