/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2014-2015  Viktor Rosendahl
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

#include <QtGlobal>
#include <QString>
#include <QTextStream>
#include "ftraceparser.h"
#include "tracefile.h"
#include "grammarroot.h"
#include "namepidnode.h"
#include "cpunode.h"
#include "timenode.h"
#include "eventnode.h"
#include "argnode.h"

bool FtraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	bool ok = false;

	if (traceFile != NULL)
		return ok;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ok, 1024*1024);

	if (!ok) {
		delete traceFile;
		traceFile = NULL;
		return ok;
	}

	lines.reserve(80000000);

	while(!traceFile->atEnd()) {
		TraceLine line;
		quint32 n = traceFile->ReadLine(&line);
		lines.append(line);
		nr += n;
	}
	QTextStream(stdout) << nr << "\n";
	return true;
}

FtraceParser::FtraceParser()
{
	NamePidNode *namePidNode;
	CpuNode *cpuNode;
	TimeNode *timeNode;
	EventNode *eventNode;
	ArgNode *argNode;

	traceFile = NULL;
	ptrPool = new MemPool(1024*1024*sizeof(char *), sizeof(char*));

	argNode = new ArgNode("argnode");
	argNode->nChildren = 1;
	argNode->children[0] = argNode;
	argNode->isLeaf = true;

	eventNode = new EventNode("eventnode");
	eventNode->nChildren = 1;
	eventNode->children[0] = argNode;
	eventNode->isLeaf = true;

	timeNode = new TimeNode("timenode");
	timeNode->nChildren = 1;
	timeNode->children[0] = eventNode;
	timeNode->isLeaf = false;

	cpuNode = new CpuNode("cpunode");
	cpuNode->nChildren = 1;
	cpuNode->children[0] = timeNode;
	cpuNode->isLeaf = false;

	namePidNode = new NamePidNode("namepidnode");
	namePidNode->nChildren = 1;
	namePidNode->children[0] = cpuNode;
	namePidNode->isLeaf = false;

	grammarRoot = new GrammarRoot("rootnode");
	grammarRoot->nChildren = 1;
	grammarRoot->children[0] = namePidNode;
	grammarRoot->isLeaf = false;
}

FtraceParser::~FtraceParser()
{
	DeleteGrammarTree(grammarRoot);
	if (traceFile != NULL)
		delete traceFile;
	delete ptrPool;
}

void FtraceParser::DeleteGrammarTree(GrammarNode* node) {
	unsigned int i;
	for (i = 0; i < node->nChildren; i++) {
		/* Delete subtree unless it's a node being it's own child */
		if (node->children[i] != node)
			DeleteGrammarTree(node->children[i]);
	}
	delete node;
}

bool FtraceParser::parse(void)
{
	quint32 s = lines.size();
	quint32 i;

	events.resize(0);
	events.reserve(s);

	for(i = 0; i < s; i++) {
		TraceLine &line = lines[i];
		TraceEvent event;
		event.argc = 0;
		event.argv = (char**) ptrPool->PreAllocN(256);
		if (parseLine(&line, &event)) {
			ptrPool->CommitN(event.argc);
			events.push_back(event);
		}
	}

	return true;
}
