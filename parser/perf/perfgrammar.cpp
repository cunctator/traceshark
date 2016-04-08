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
#include "parser/perf/perfeventnode.h"
#include "parser/perf/perftimenode.h"
#include "parser/grammar/cpunode.h"
#include "parser/grammar/storenode.h"
#include "parser/perf/perfgrammar.h"

PerfGrammar::PerfGrammar()
	: Grammar()
{
	createPerfGrammarTree();
}

PerfGrammar::~PerfGrammar()
{
}

void PerfGrammar::createPerfGrammarTree()
{
	ArgNode *perfArgNode;
	PerfEventNode *perfEventNode;
	PerfTimeNode *perfTimeNode;
	CpuNode *perfCpuNode;
	StoreNode *perfPidNode;
	StoreNode *perfNameNode;

	perfArgNode = new ArgNode("perfArgNode");
	perfArgNode->nChildren = 1;
	perfArgNode->children[0] = perfArgNode;
	perfArgNode->isLeaf = true;

	perfEventNode = new PerfEventNode("perfEventNode");
	perfEventNode->nChildren = 1;
	perfEventNode->children[0] = perfArgNode;
	perfEventNode->isLeaf = true;

	perfTimeNode = new PerfTimeNode("perfTimeNode");
	perfTimeNode->nChildren = 1;
	perfTimeNode->children[0] = perfEventNode;
	perfTimeNode->isLeaf = false;

	perfCpuNode = new CpuNode("perfCpuNode");
	perfCpuNode->nChildren = 2;
	perfCpuNode->children[0] = perfTimeNode;
	// perfCpuNode->children[1] = perfPidNode;
	perfCpuNode->isLeaf = false;

	perfPidNode = new StoreNode("perfPidNode");
	perfPidNode->nChildren = 2;
	perfPidNode->children[0] = perfCpuNode;
	perfPidNode->children[1] = perfPidNode;
	perfPidNode->isLeaf = false;

	perfNameNode = new StoreNode("perfNameNode");
	perfNameNode->nChildren = 1;
	perfNameNode->children[0] = perfPidNode;
	perfNameNode->isLeaf = false;

	root->nChildren = 1;
	root->children[0] = perfNameNode;
	root->isLeaf = false;

	/* This is the commented out line being executed here because of the
	 * loop structure */
	perfCpuNode->children[1] = perfPidNode;
}
