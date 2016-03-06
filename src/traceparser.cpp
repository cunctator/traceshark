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

#include <climits>
#include <limits>

#include <QTextStream>

#include "tstring.h"
#include "genericparams.h"
#include "grammarroot.h"
#include "mm/mempool.h"
#include "cpunode.h"
#include "perftimenode.h"
#include "timenode.h"
#include "eventnode.h"
#include "argnode.h"
#include "pidnode.h"
#include "perfeventnode.h"
#include "storenode.h"
#include "tracefile.h"
#include "traceparser.h"
#include "traceshark.h"
#include "threads/threadbuffer.h"

TraceParser::TraceParser(TList<TraceEvent> *analyzerEvents)
	: traceType(TRACE_TYPE_NONE)
{
	traceFile = NULL;
	ptrPool = new MemPool(16384, sizeof(TString*));
	postEventPool = new MemPool(16384, sizeof(TString));

	createFtraceGrammarTree();
	createPerfGrammarTree();

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<TraceParser>
		(this, &TraceParser::parseThread);
	events = analyzerEvents;
}

TraceParser::~TraceParser()
{
	DeleteGrammarTree(ftraceGrammarRoot);
	DeleteGrammarTree(perfGrammarRoot);
	delete ptrPool;
	delete postEventPool;
	delete[] tbuffers;
	delete parserThread;
}

bool TraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	unsigned int i = 0;
	unsigned int curbuf = 0;
	bool ok = false;

	if (traceFile != NULL)
		return ok;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ok,
				  1024 * 1024, NR_TBUFFERS);

	if (!ok) {
		delete traceFile;
		traceFile = NULL;
		return ok;
	}

	for (i = 0; i < NR_TBUFFERS; i++)
		tbuffers[i] = new ThreadBuffer<TraceLine>(TBUFSIZE,
							  NR_TBUFFERS);
	parserThread->start();

	i = 0;
	tbuffers[curbuf]->beginProduceBuffer();
	while(!traceFile->atEnd()) {
		TraceLine *line = &tbuffers[curbuf]->buffer[i];
		quint32 n = traceFile->ReadLine(line, curbuf);
		nr += n;
		i++;
		if (i == (TBUFSIZE - 1)) {
			tbuffers[curbuf]->endProduceBuffer(i);
			curbuf++;
			if (curbuf == NR_TBUFFERS)
				curbuf = 0;
			i = 0;
			tbuffers[curbuf]->beginProduceBuffer();
			traceFile->clearPool(curbuf);
		}
	}
	tbuffers[curbuf]->endProduceBuffer(i);
	/* We must send an empty buffer at the end to signal that we are EOF */
	if (i != 0) {
		curbuf++;
		if (curbuf == NR_TBUFFERS)
			curbuf = 0;
		tbuffers[curbuf]->beginProduceBuffer();
		tbuffers[curbuf]->endProduceBuffer(0);
	}

	parserThread->wait();

	for (i = 0; i < NR_TBUFFERS; i++)
		delete tbuffers[i];

	QTextStream(stdout) << nr << "\n";
	return true;
}

bool TraceParser::isOpen()
{
	return (traceFile != NULL);
}

void TraceParser::close()
{
	if (traceFile != NULL) {
		delete traceFile;
		traceFile = NULL;
	}
	ptrPool->reset();
	clearGrammarPools(ftraceGrammarRoot);
	clearGrammarPools(perfGrammarRoot);
	traceType = TRACE_TYPE_NONE;
}

void TraceParser::createFtraceGrammarTree()
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

	ftraceGrammarRoot = new GrammarRoot("ftraceRootNode");
	ftraceGrammarRoot->nChildren = 1;
	ftraceGrammarRoot->children[0] = ftraceNamePidNode;
	ftraceGrammarRoot->isLeaf = false;

	/* This is the commented out line being executed here because of the
	 * loop structure */
	ftraceCpuNode->children[1] = ftraceNamePidNode;
}

void TraceParser::createPerfGrammarTree()
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

	perfGrammarRoot = new GrammarRoot("perfRootNode");
	perfGrammarRoot->nChildren = 1;
	perfGrammarRoot->children[0] = perfNameNode;
	perfGrammarRoot->isLeaf = false;

	/* This is the commented out line being executed here because of the
	 * loop structure */
	perfCpuNode->children[1] = perfPidNode;
}

void TraceParser::DeleteGrammarTree(GrammarNode* node) {
	unsigned int i;
	node->reaped = true;
	for (i = 0; i < node->nChildren; i++) {
		/* Delete subtree if it has not been visited */
		if (!node->children[i]->reaped)
			DeleteGrammarTree(node->children[i]);
	}
	delete node;
}

/* This function does prescanning as well, to determine number of events,
 * number of CPUs, max/min CPU frequency etc */
void TraceParser::parseThread()
{
	unsigned int i = 0;
	preparePreScan();
	events->clear();
	prevTime = std::numeric_limits<double>::lowest();

	while(true) {
		if (parseBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
		determineTraceType();
		if (traceType == TRACE_TYPE_FTRACE) {
			goto ftrace;
		}
		if (traceType == TRACE_TYPE_PERF) {
			goto perf;
		}
	}
	/* Must have been a short trace or a lot of unknown garbage in the
	 * trace if we end up here */
	fixLastEvent();
	finalizePreScan();
	return;

	/* The purpose of jumping to these loops is to  be able to use the
	 * (hopefully faster) specialized parse functions */
ftrace:
	while(true) {
		if (parseFtraceBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
	fixLastEvent();
	finalizePreScan();
	return;

perf:
	while(true) {
		if (parsePerfBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
	fixLastEvent();
	finalizePreScan();
}

void TraceParser::preparePreScan()
{
	nrEvents = 0;
	infoBegin = traceFile->mappedFile;
	prevLineIsEvent = true;
	fakePostEventInfo.ptr = traceFile->mappedFile;
	fakeEvent.postEventInfo = &fakePostEventInfo;
	prevEvent = &fakeEvent;
	nrFtraceEvents = 0;
	nrPerfEvents = 0;
	maxCPU = 0;
	startTime = 0;
	endTime = 0;
	minFreq = UINT_MAX;
	maxFreq = 0;
	minIdleState = INT_MAX;
	maxIdleState = INT_MIN;
	nrMigrateEvents = 0;
	startFreq.fill(-1, HIGHEST_CPU_EVER + 1);
}

void TraceParser::finalizePreScan()
{
	nrEvents = events->size();
	lastEvent = nrEvents - 1;
	if (nrEvents >= 2) {
		startTime = events->at(0).time;
		endTime = events->last().time;
	}

	nrCPUs = maxCPU + 1;
	startFreq.resize(nrCPUs);
}

/*
 * This function is to be called after the parsing of all the events, it's
 * for fixing the postEventInfo pointer of the last event, because that info is
 * normally set when processing the next event in the
 * parse[Ftrace|Perf]Buffer() functions and for the last event there will of
 * course not be any next event.
 */
void TraceParser::fixLastEvent()
{
	/* Only perf traces will have backtraces after events, I think */
	if (traceType != TRACE_TYPE_PERF)
		return;
	TraceEvent &lastEvent = events->last();
	if (prevLineIsEvent) {
		lastEvent.postEventInfo = NULL;
	} else {
		TString *str = (TString*) postEventPool->
			allocObj();
		str->ptr = infoBegin;
		str->len = traceFile->mappedFile + traceFile->fileSize
			- infoBegin;
		lastEvent.postEventInfo = str;
	}
}

bool TraceParser::parseLineBugFixup(TraceEvent* event)
{
	double corrtime = event->time + 0.9;
	double delta = corrtime - prevTime;
	bool retval = false;

	if (delta >= (double)0 && delta < 0.00001) {
		event->time = corrtime;
		retval = true;
	}
	return retval;
}

void TraceParser::_clearGrammarPools(GrammarNode *tree)
{
	unsigned int i;
	tree->reaped = true;
	tree->clearStringPool();
	for (i = 0; i < tree->nChildren; i++) {
		/* Clear subtree if it hasn't been visited */
		if (!tree->children[i]->reaped)
			_clearGrammarPools(tree->children[i]);
	}
}

void TraceParser::resetGrammarReapedFlag(GrammarNode *tree)
{
	unsigned int i;
	tree->reaped = false;
	for (i = 0; i < tree->nChildren; i++) {
		/* Reset subtree if it hasn't been visited, N.B. since we are
		 * resetting, reaped flag will be false for visited nodes */
		if (tree->children[i]->reaped)
			resetGrammarReapedFlag(tree->children[i]);
	}
}

void TraceParser::clearGrammarPools(GrammarNode *tree)
{
	_clearGrammarPools(tree);
	resetGrammarReapedFlag(tree);
}

void TraceParser::determineTraceType()
{
	if (nrFtraceEvents > 0 && nrPerfEvents == 0) {
		traceType = TRACE_TYPE_FTRACE;
		return;
	} else if (nrFtraceEvents  == 0 && nrPerfEvents > 0) {
		traceType = TRACE_TYPE_PERF;
		return;
	}
	traceType = TRACE_TYPE_NONE;
}

/* This parses a buffer regardless if it's perf or ftrace */
bool TraceParser::parseBuffer(unsigned int index)
{
	unsigned int i, s;

	ThreadBuffer<TraceLine> *tbuf = tbuffers[index];
	if (tbuf->beginConsumeBuffer()) {
		tbuf->endConsumeBuffer(); /* Uncessary but beatiful */
		return true;
	}

	s = tbuf->nRead;

	for(i = 0; i < s; i++) {
		TraceLine *line = &tbuf->buffer[i];
		TraceEvent &event = events->preAlloc();
		event.argc = 0;
		event.argv = (TString**) ptrPool->preallocN(256);
		if (parseLineFtrace(line, event))
			continue;
		else {
			parseLinePerf(line, event);
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}
