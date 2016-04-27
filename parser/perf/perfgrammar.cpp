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

#include "parser/perf/perfgrammar.h"
#include "parser/traceevent.h"

PerfGrammar::PerfGrammar()
{
	argPool = new StringPool(2048, 1024 * 1024);
	namePool =  new StringPool(1024, 65536);
	eventTree = new StringTree(8, 256);
	setupEventTree();
}

PerfGrammar::~PerfGrammar()
{
	delete argPool;
	delete namePool;
	delete eventTree;
}

void PerfGrammar::clear()
{
	argPool->clear();
	namePool->clear();
	eventTree->clear();
	setupEventTree();
}

void PerfGrammar::setupEventTree()
{
	int t;
	event_t dummy;
	TString str;

	for (t = 0; t < NR_EVENTS; t++) {
		str.ptr = eventstrings[t];
		str.len = strlen(eventstrings[t]);
		eventTree->searchAllocString(&str, TShark::StrHash32(&str),
					     &dummy, (event_t) t);
	}
}
