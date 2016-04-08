/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "parser/grammar/argnode.h"
#include "parser/grammar/eventnode.h"
#include "parser/ftrace/timenode.h"
#include "parser/grammar/cpunode.h"
#include "parser/grammar/storenode.h"

FtraceGrammar::FtraceGrammar()
	: Grammar()
{
	createFtraceGrammarTree();
}

FtraceGrammar::~FtraceGrammar()
{
}

void FtraceGrammar::createFtraceGrammarTree()
{
	ArgNode *ftraceArgNode;
	EventNode *ftraceEventNode;
	TimeNode *ftraceTimeNode;
	CpuNode *ftraceCpuNode;
	StoreNode *ftraceNamePidNode;

	ftraceArgNode = new ArgNode("ftraceArgNode");
	ftraceArgNode->nChildren = 1;
	ftraceArgNode->children[0] = ftraceArgNode;
	ftraceArgNode->isLeaf = true;

	ftraceEventNode = new EventNode("ftraceEventnode");
	ftraceEventNode->nChildren = 1;
	ftraceEventNode->children[0] = ftraceArgNode;
	ftraceEventNode->isLeaf = true;

	ftraceTimeNode = new TimeNode("ftraceTimeNode");
	ftraceTimeNode->nChildren = 1;
	ftraceTimeNode->children[0] = ftraceEventNode;
	ftraceTimeNode->isLeaf = false;

	ftraceCpuNode = new CpuNode("ftraceCpuNode");
	ftraceCpuNode->nChildren = 2;
	ftraceCpuNode->children[0] = ftraceTimeNode;
	// ftraceCpuNode->children[1] = ftraceNamePidNode;
	ftraceCpuNode->isLeaf = false;

	ftraceNamePidNode = new StoreNode("ftraceNamePidNode");
	ftraceNamePidNode->nChildren = 2;
	ftraceNamePidNode->children[0] = ftraceCpuNode;
	ftraceNamePidNode->children[1] = ftraceNamePidNode;
	ftraceNamePidNode->isLeaf = false;

	root->nChildren = 1;
	root->children[0] = ftraceNamePidNode;
	root->isLeaf = false;

	/* This is the commented out line being executed here because of the
	 * loop structure */
	ftraceCpuNode->children[1] = ftraceNamePidNode;
}
