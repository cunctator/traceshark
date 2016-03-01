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
#include <cstdlib>
#include <QtGlobal>
#include <QList>
#include <QString>
#include <QTextStream>
#include "cpufreq.h"
#include "cpuidle.h"
#include "ftraceparams.h"
#include "traceparser.h"
#include "tracefile.h"
#include "grammarroot.h"
#include "cpunode.h"
#include "threads/threadbuffer.h"
#include "perftimenode.h"
#include "timenode.h"
#include "eventnode.h"
#include "argnode.h"
#include "pidnode.h"
#include "perfeventnode.h"
#include "storenode.h"
#include "traceshark.h"
#include "threads/workthread.h"
#include "threads/workitem.h"
#include "threads/workqueue.h"

bool TraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	unsigned int i = 0;
	unsigned int curbuf = 0;
	bool ok = false;

	if (traceFile != NULL)
		return ok;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ok,
				  1024*1024, NR_TBUFFERS);

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
	if (cpuTaskMaps != NULL) {
		delete[] cpuTaskMaps;
		cpuTaskMaps = NULL;
	}
	if (cpuFreq != NULL) {
		delete[] cpuFreq;
		cpuFreq = NULL;
	}
	if (cpuIdle != NULL) {
		delete[] cpuIdle;
		cpuIdle = NULL;
	}
	if (CPUs != NULL) {
		delete[] CPUs;
		CPUs = NULL;
	}
	events.clear();
	migrations.clear();
	migrationArrows.clear();
	ptrPool->reset();
	taskNamePool->reset();
	clearGrammarPools(ftraceGrammarRoot);
	traceType = TRACE_TYPE_NONE;
	colorMap.clear();
}

TraceParser::TraceParser()
	: cpuTaskMaps(NULL), cpuFreq(NULL), cpuIdle(NULL),
	  traceType(TRACE_TYPE_NONE), black(0, 0, 0), white(255, 255, 255),
	  CPUs(NULL)
{
	traceFile = NULL;
	ptrPool = new MemPool(16384, sizeof(TString*));
	taskNamePool = new MemPool(16384, sizeof(char));
	postEventPool = new MemPool(16384, sizeof(TString));

	createFtraceGrammarTree();
	createPerfGrammarTree();

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<TraceParser>
		(this, &TraceParser::parseThread);
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

TraceParser::~TraceParser()
{
	TraceParser::close();
	DeleteGrammarTree(ftraceGrammarRoot);
	DeleteGrammarTree(perfGrammarRoot);
	delete ptrPool;
	delete taskNamePool;
	delete postEventPool;
	delete[] tbuffers;
	delete parserThread;
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
	events.clear();

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
	lastEvent = nrEvents - 1;
	if (nrEvents >= 2) {
		startTime = events[0].time;
		endTime = events[lastEvent].time;
	}

	nrCPUs = maxCPU + 1;
	cpuTaskMaps = new QMap<unsigned int, CPUTask>[nrCPUs];
	cpuFreq = new CpuFreq[nrCPUs];
	cpuIdle = new CpuIdle[nrCPUs];
	CPUs = new CPU[nrCPUs];
	schedOffset.resize(0);
	schedOffset.resize(nrCPUs);
	schedScale.resize(0);
	schedScale.resize(nrCPUs);
	cpuIdleOffset.resize(0);
	cpuIdleOffset.resize(nrCPUs);
	cpuIdleScale.resize(0);
	cpuIdleScale.resize(nrCPUs);
	cpuFreqOffset.resize(0);
	cpuFreqOffset.resize(nrCPUs);
	cpuFreqScale.resize(0);
	cpuFreqScale.resize(nrCPUs);
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
	TraceEvent &lastEvent = events.last();
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

void TraceParser::preScan()
{
}

void TraceParser::parse()
{
}

/* This parses a buffer regardless if it's perf or ftrace */
bool TraceParser::parseBuffer(unsigned int index)
{
	unsigned int i, s;
	double prevtime = std::numeric_limits<double>::lowest();

	ThreadBuffer<TraceLine> *tbuf = tbuffers[index];
	if (tbuf->beginConsumeBuffer()) {
		tbuf->endConsumeBuffer(); /* Uncessary but beatiful */
		return true;
	}

	s = tbuf->nRead;

	for(i = 0; i < s; i++) {
		TraceLine *line = &tbuf->buffer[i];
		TraceEvent &event = events.preAlloc();
		event.argv = (TString**) ptrPool->preallocN(256);
		if (parseLine(line, &event, ftraceGrammarRoot)) {
			/* Check if the timestamp of this event is affected by
			 * the infamous ftrace timestamp rollover bug and
			 * try to correct it */
			if (event.time < prevtime) {
				if (!parseLineBugFixup(&event, prevtime))
					continue;
			}
			prevtime = event.time;
			ptrPool->commitN(event.argc);
			event.postEventInfo = NULL;
			events.commit();
			nrFtraceEvents++;
			preScanFtraceEvent(event);
			/* probably not necessary because ftrace traces doesn't
			 * have backtraces and stuff but do it anyway */
			prevLineIsEvent = true;
		} else if (parseLine(line, &event, perfGrammarRoot)) {
			/* Check if the timestamp of this event is affected by
			 * the infamous ftrace timestamp rollover bug and
			 * try to correct it */
			if (event.time < prevtime) {
				if (!parseLineBugFixup(&event, prevtime))
					continue;
			}
			prevtime = event.time;
			ptrPool->commitN(event.argc);
			if (prevLineIsEvent) {
				prevEvent->postEventInfo = NULL;
			} else {
				TString *str = (TString*) postEventPool->
					allocObj();
				str->ptr = infoBegin;
				str->len = line->begin - infoBegin;
				prevEvent->postEventInfo = str;
				prevLineIsEvent = true;
			}
			events.commit();
			prevEvent = &event;
			nrPerfEvents++;
			preScanPerfEvent(event);
		} else {
			if (prevLineIsEvent) {
				infoBegin = line->begin;
				prevLineIsEvent = false;
			}
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}

bool TraceParser::processMigration()
{
	if (traceType == TRACE_TYPE_FTRACE) {
		processMigrationFtrace();
	} else if (traceType == TRACE_TYPE_PERF) {
		processMigrationPerf();
	}
	return false;
}

void TraceParser::processMigrationFtrace()
{
	unsigned long i;
	Migration m;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		switch (event.type) {
		case SCHED_MIGRATE_TASK:
			if (!sched_migrate_args_ok(event))
				break;
			m.pid = sched_migrate_pid(event);
			m.oldcpu = sched_migrate_origCPU(event);
			m.newcpu = sched_migrate_destCPU(event);
			m.time = event.time;
			migrations.append(m);
			break;
		case SCHED_PROCESS_FORK:
			if (!sched_process_fork_args_ok(event))
				break;
			m.pid = sched_process_fork_childpid(event);
			m.oldcpu = -1;
			m.newcpu = event.cpu;
			m.time = event.time;
			migrations.append(m);
			break;
		case SCHED_PROCESS_EXIT:
			if (!sched_process_exit_args_ok(event))
				break;
			m.pid = sched_process_exit_pid(event);
			m.oldcpu = event.cpu;
			m.newcpu = -1;
			m.time = event.time;
			migrations.append(m);
			break;
		default:
			break;
		}
	}
}

void TraceParser::processMigrationPerf()
{
	unsigned long i;
	Migration m;

	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		switch (event.type) {
		case SCHED_MIGRATE_TASK:
			if (!perf_sched_migrate_args_ok(event))
				break;
			m.pid = perf_sched_migrate_pid(event);
			m.oldcpu = perf_sched_migrate_origCPU(event);
			m.newcpu = perf_sched_migrate_destCPU(event);
			m.time = event.time;
			migrations.append(m);
			break;
		case SCHED_PROCESS_FORK:
			if (perf_sched_process_fork_args_ok(event))
				break;
			m.pid = perf_sched_process_fork_childpid(event);
			m.oldcpu = -1;
			m.newcpu = event.cpu;
			m.time = event.time;
			migrations.append(m);
			break;
		case SCHED_PROCESS_EXIT:
			if (perf_sched_process_exit_args_ok(event))
				break;
			m.pid = perf_sched_process_exit_pid(event);
			m.oldcpu = event.cpu;
			m.newcpu = -1;
			m.time = event.time;
			migrations.append(m);
			break;
		default:
			break;
		}
	}
}

bool TraceParser::processSched()
{
	switch (traceType) {
	case TRACE_TYPE_FTRACE:
		processSchedFtrace();
		break;
	case TRACE_TYPE_PERF:
		processSchedPerf();
		break;
	default:
		break;
	}
	return false;
}

void TraceParser::processSchedFtrace()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		switch (event.type) {
		case  SCHED_SWITCH:
			if (sched_switch_args_ok(event))
				processFtraceSwitchEvent(event);
			break;
		case SCHED_WAKEUP:
		case SCHED_WAKEUP_NEW:
			if (sched_wakeup_args_ok(event))
				processFtraceWakeupEvent(event);
			break;
		default:
			break;
		}
	}
	processSchedAddTail();
}

void TraceParser::processSchedPerf()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		switch(event.type) {
		case SCHED_SWITCH:
			if (perf_sched_switch_args_ok(event))
				processPerfSwitchEvent(event);
			break;
			/* sched_wakeup_new and sched_wakeup both have the same
			 * argument structure */
		case SCHED_WAKEUP:
		case SCHED_WAKEUP_NEW:
			if (perf_sched_wakeup_args_ok(event))
				processPerfWakeupEvent(event);
			break;
		default:
			break;
		}
	}
	processSchedAddTail();
}

void TraceParser::processSchedAddTail()
{
	/* Add the "tail" to all tasks, i.e. extend them until endTime */
	unsigned int cpu;
	for (cpu = 0; cpu < nrCPUs; cpu++) {
		DEFINE_CPUTASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
		while (iter != cpuTaskMaps[cpu].end()) {
			CPUTask &task = iter.value();
			double d;
			iter++;
			/* Check if tail is necessary */
			if (task.timev[task.timev.size() - 1] >= endTime)
				continue;
			d = task.data[task.data.size() - 1];
			task.timev.append(endTime);
			task.data.append(d);
		}
	}
}

void TraceParser::handleWrongTaskOnCPU(TraceEvent &event, unsigned int cpu,
				       CPU *eventCPU, unsigned int oldpid,
				       double oldtime)
{
	unsigned int epid = eventCPU->pidOnCPU;
	double prevtime, faketime;
	CPUTask *cpuTask;
	Task *task;

	if (epid != 0) {
		cpuTask = &cpuTaskMaps[cpu][epid];
		Q_ASSERT(!cpuTask->isNew);
		Q_ASSERT(!cpuTask->timev.isEmpty());
		prevtime = cpuTask->timev.last();
		faketime = prevtime + FAKE_DELTA;
		cpuTask->timev.append(faketime);
		cpuTask->data.append(FLOOR_HEIGHT);
		task = getTask(epid);
		task->lastWakeUP = faketime;
	}

	if (oldpid != 0) {
		cpuTask = &cpuTaskMaps[cpu][oldpid];
		if (cpuTask->isNew) {
			cpuTask->pid = oldpid;
			if (traceType == TRACE_TYPE_FTRACE) {
				cpuTask->name = sched_switch_oldname_strdup(
					event,
					taskNamePool);
			} else {
				cpuTask->name =
					perf_sched_switch_oldname_strdup(event,
									 taskNamePool);
			}
		}
		cpuTask->isNew = false;
		faketime = oldtime - FAKE_DELTA;
		cpuTask->timev.append(faketime);
		cpuTask->data.append(SCHED_HEIGHT);
	}
}

bool TraceParser::processCPUfreq()
{
	unsigned int cpu;

	if (traceType == TRACE_TYPE_NONE)
		return true;

	if (traceType != TRACE_TYPE_FTRACE && traceType != TRACE_TYPE_PERF)
		return false;

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		if (startFreq[cpu] > 0) {
			cpuFreq[cpu].timev.append(startTime);
			cpuFreq[cpu].data.append(startFreq[cpu]);
		}
	}

	if (traceType == TRACE_TYPE_FTRACE)
		_processCPUfreqFtrace();
	else
		_processCPUfreqPerf();

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		if (!cpuFreq[cpu].data.isEmpty()) {
			double freq = cpuFreq[cpu].data.last();
			cpuFreq[cpu].data.append(freq);
			cpuFreq[cpu].timev.append(endTime);
		}
	}
	return false;
}

void TraceParser::_processCPUfreqFtrace()
{
	unsigned int i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		/*
		 * I expect this loop to be so fast in comparison
		 * to the other functions that will be running in parallel
		 * that it's acceptable to piggy back cpuidle events here */
		switch (event.type) {
		case CPU_IDLE:
			if (cpufreq_args_ok(event))
				processFtraceCPUidleEvent(event);
			break;
		case CPU_FREQUENCY:
			if (perf_cpuidle_args_ok(event))
				processFtraceCPUfreqEvent(event);
			break;
		default:
			break;
		}
	}
}

void TraceParser::_processCPUfreqPerf()
{
	unsigned int i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		/*
		 * I expect this loop to be so fast in comparison
		 * to the other functions that will be running in parallel
		 * that it's acceptable to piggy back cpuidle events here */
		switch (event.type) {
		case CPU_IDLE:
			if (perf_cpufreq_args_ok(event))
				processPerfCPUidleEvent(event);
			break;
		case CPU_FREQUENCY:
			if (perf_cpuidle_args_ok(event))
				processPerfCPUfreqEvent(event);
			break;
		default:
			break;
		}
	}
}

void TraceParser::colorizeTasks()
{
	unsigned int cpu;
	double nf;
	unsigned int n;
	unsigned int ncolor;
	double s;
	int step;
	int red;
	int green;
	int blue;
	unsigned int seed = 290876;
	unsigned int i, j;
	QList<TColor> colorList;
	const TColor black(0, 0, 0);
	const TColor white(255, 255, 255);
	TColor tmp;

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		DEFINE_CPUTASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
		while (iter != cpuTaskMaps[cpu].end()) {
			CPUTask &task = iter.value();
			iter++;
			if (colorMap.contains(task.pid))
				continue;
			TColor color;
			colorMap.insert(task.pid, color);
		}
	}

	n = colorMap.size();
	nf = (double) n;
	s = cbrt( (1 / nf) * (255 * 255 * 255 ));
	s = TSMIN(s, 128.0);
	s = TSMAX(s, 1.0);
retry:
	step = (unsigned int) s;
	for (red = 0; red < 256; red += step) {
		for (green = 0; green < 256; green += step)  {
			for (blue = 0; blue < 256; blue += step) {
				TColor color(red, green, blue);
				if (color.SqDistance(black) < 1000)
					continue;
				if (color.SqDistance(white) < 10000)
					continue;
				colorList.append(color);
			}
		}
	}

	ncolor = colorList.size();
	if (ncolor < n) {
		s = s * 0.95;
		if (s >= 1) {
			colorList.clear();
			goto retry;
		}
	}

	/* Randomize the order by swapping every element with a random
	 * element */
	for (i = 0; i < ncolor; i++) {
		j = rand_r(&seed) % ncolor;
		tmp = colorList[j];
		colorList[j] = colorList[i];
		colorList[i] = tmp;
	}

	i = 0;

	DEFINE_COLORMAP_ITERATOR(iter) = colorMap.begin();
	while (iter != colorMap.end()) {
		TColor &color = iter.value();
		iter++;
		color = colorList.at(i % ncolor);
		i++;
	}
}


int TraceParser::binarySearch(double time, int start, int end)
{
	int pivot = (end + start) / 2;
	if (pivot == start)
		return pivot;
	if (time < events.at(pivot).time)
		return binarySearch(time, start, pivot);
	else
		return binarySearch(time, pivot, end);
}

int TraceParser::findIndexBefore(double time)
{
	if (events.size() < 1)
		return -1;

	int end = events.size() - 1;

	/* Basic sanity checks */
	if (time > events.at(end).time)
		return end;
	if (time < events.at(0).time)
		return 0;

	int c = binarySearch(time, 0, end);

	if (events.at(c).time >= time)
		c--;
	return c;
}

TraceEvent *TraceParser::findPreviousSchedEvent(double time,
						unsigned int pid)
{
	int index = findIndexBefore(time);
	int i;

	if (index < 0)
		return nullptr;

	for (i = index; i >= 0; i--) {
		TraceEvent &event = events[i];
		if (event.type == SCHED_SWITCH  &&
		    generic_sched_switch_newpid(event) == pid) {
			return &event;
		}
	}
	return nullptr;
}

TraceEvent *TraceParser::findPreviousWakeupEvent(double time,
						 unsigned int pid)
{
	int index = findIndexBefore(time);
	int i;

	if (index < 0)
		return nullptr;

	for (i = index; i >= 0; i--) {
		TraceEvent &event = events[i];
		if ((event.type == SCHED_WAKEUP ||
		     event.type == SCHED_WAKEUP_NEW) &&
		    generic_sched_wakeup_pid(event) == pid) {
			return &event;
		}
	}
	return nullptr;
}



void TraceParser::setSchedOffset(unsigned int cpu, double offset)
{
	schedOffset[cpu] = offset;
}

void TraceParser::setSchedScale(unsigned int cpu, double scale)
{
	schedScale[cpu] = scale;
}

void TraceParser::setCpuIdleOffset(unsigned int cpu, double offset)
{
	cpuIdleOffset[cpu] = offset;
}

void TraceParser::setCpuIdleScale(unsigned int cpu, double scale)
{
	cpuIdleScale[cpu] = scale / maxIdleState;
}

void TraceParser::setCpuFreqOffset(unsigned int cpu, double offset)
{
	cpuFreqOffset[cpu] = offset;
}

void TraceParser::setCpuFreqScale(unsigned int cpu, double scale)
{
	cpuFreqScale[cpu] = scale / maxFreq;
}

void TraceParser::setMigrationOffset(double offset)
{
	migrationOffset = offset;
}

void TraceParser::setMigrationScale(double scale)
{
	migrationScale = scale;
}

void TraceParser::setQCustomPlot(QCustomPlot *plot)
{
	customPlot = plot;
}

void TraceParser::addCpuFreqWork(unsigned int cpu,
				 QList<AbstractWorkItem*> &list)
{
	double scale = cpuFreqScale.value(cpu);
	double offset = cpuFreqOffset.value(cpu);
	CpuFreq *freq = cpuFreq + cpu;
	freq->scale = scale;
	freq->offset = offset;
	WorkItem<CpuFreq> *freqItem = new WorkItem<CpuFreq>
		(freq, &CpuFreq::doScale);
	list.append(freqItem);
}

void TraceParser::addCpuIdleWork(unsigned int cpu,
				 QList<AbstractWorkItem*> &list)
{
	double scale = cpuIdleScale.value(cpu);
	double offset = cpuIdleOffset.value(cpu);
	CpuIdle *idle = cpuIdle + cpu;
	idle->scale = scale;
	idle->offset = offset;
	WorkItem<CpuIdle> *idleItem = new WorkItem<CpuIdle>
		(idle, &CpuIdle::doScale);
	list.append(idleItem);
}

void TraceParser::addCpuSchedWork(unsigned int cpu,
				  QList<AbstractWorkItem*> &list)
{
	double scale = schedScale.value(cpu);
	double offset = schedOffset.value(cpu);
	DEFINE_CPUTASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
	while (iter != cpuTaskMaps[cpu].end()) {
		CPUTask &task = iter.value();
		task.scale = scale;
		task.offset = offset;
		WorkItem<CPUTask> *taskItem = new WorkItem<CPUTask>
			(&task, &CPUTask::doScale);
		list.append(taskItem);
		taskItem = new WorkItem<CPUTask>(&task,
						 &CPUTask::doScaleWakeup);
		list.append(taskItem);
		taskItem = new WorkItem<CPUTask>(&task,
						 &CPUTask::doScaleRunning);
		list.append(taskItem);
		iter++;
	}
}

/*
 * This function must be called from the application mainthread because it
 * creates objects that are children customPlot, which is created by the
 * mainthread
 */
void TraceParser::scaleMigration()
{
	MigrationArrow *a;
	QList<Migration>::iterator iter;
	double unit = migrationScale / nrCPUs;
	for (iter = migrations.begin(); iter != migrations.end(); iter++) {
		Migration &m = *iter;
		double s = migrationOffset + (m.oldcpu + 1) * unit;
		double e = migrationOffset + (m.newcpu + 1) * unit;
		QColor color = getTaskColor(m.pid);
		a = new MigrationArrow(s, e, m.time, color, customPlot);
		customPlot->addItem(a);
		migrationArrows.append(a);
	}
}

void TraceParser::doScale()
{
	QList<AbstractWorkItem*> workList;
	unsigned int cpu;
	int i, s;

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		/* CpuFreq items */
		addCpuFreqWork(cpu, workList);
		/* CpuIdle items */
		addCpuIdleWork(cpu, workList);
		/* Task items */
		addCpuSchedWork(cpu, workList);
	}
	s = workList.size();
	for (i = 0; i < s; i++)
		scalingQueue.addWorkItem(workList[i]);
	scalingQueue.start();
	scaleMigration(); /* Migration scaling is done from the mainthread */
	scalingQueue.wait();
	for (i = 0; i < s; i++)
		delete workList[i];
}

bool TraceParser::parseLineBugFixup(TraceEvent* event, double prevtime)
{
	double corrtime = event->time + 0.9;
	double delta = corrtime - prevtime;
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
