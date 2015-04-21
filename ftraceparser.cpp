/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2014-2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include <QtGlobal>
#include <QString>
#include <QTextStream>
#include "cpufreq.h"
#include "cpuidle.h"
#include "ftraceparams.h"
#include "ftraceparser.h"
#include "tracefile.h"
#include "grammarroot.h"
#include "namepidnode.h"
#include "cpunode.h"
#include "threadbuffer.h"
#include "timenode.h"
#include "eventnode.h"
#include "argnode.h"
#include "traceshark.h"
#include "workthread.h"

bool FtraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	unsigned int i = 0;
	unsigned int curbuf = 0;
	bool ok = false;

	if (traceFile != NULL)
		return ok;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ok, 1024*1024);

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
		quint32 n = traceFile->ReadLine(line);
		nr += n;
		i++;
		if (i == (TBUFSIZE - 1)) {
			tbuffers[curbuf]->endProduceBuffer(i);
			curbuf++;
			if (curbuf == NR_TBUFFERS)
				curbuf = 0;
			i = 0;
			tbuffers[curbuf]->beginProduceBuffer();
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

bool FtraceParser::isOpen()
{
	return (traceFile != NULL);
}

void FtraceParser::close()
{
	if (traceFile != NULL) {
		events.resize(0);
		events.reserve(1);
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
	migrations.resize(0);
}

FtraceParser::FtraceParser()
	: cpuTaskMaps(NULL), cpuFreq(NULL), cpuIdle(NULL), black(0, 0, 0),
	  white(255, 255, 255)
{
	NamePidNode *namePidNode;
	CpuNode *cpuNode;
	TimeNode *timeNode;
	EventNode *eventNode;
	ArgNode *argNode;

	traceFile = NULL;
	ptrPool = new MemPool(2048, sizeof(TString*));
	taskNamePool = new MemPool(16, sizeof(char));

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

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<FtraceParser>
		(this, &FtraceParser::parseThread);
}

FtraceParser::~FtraceParser()
{
	FtraceParser::close();
	DeleteGrammarTree(grammarRoot);
	delete ptrPool;
	delete taskNamePool;
	delete[] tbuffers;
	delete parserThread;
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

/* This function does prescanning as well, to determine number of events,
 * number of CPUs, max/min CPU frequency etc */
void FtraceParser::parseThread()
{
	unsigned int i = 0;
	preparePreScan();
	events.resize(0);
	events.reserve(80000000); /* Reserve for eight million events first */

	while(true) {
		if (parseBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}

	finalizePreScan();
}

void FtraceParser::preparePreScan()
{
	nrEvents = 0;
	maxCPU = 0;
	startTime = 0;
	endTime = 0;
	minFreq = UINT_MAX;
	maxFreq = 0;
	minIdleState = INT_MAX;
	maxIdleState = INT_MIN;
	nrMigrateEvents = 0;
}

void FtraceParser::finalizePreScan()
{
	lastEvent = nrEvents - 1;
	if (nrEvents >= 2) {
		startTime = events[0].time;
		endTime = events[lastEvent].time;
	}

	nrCPUs = maxCPU + 1;
	cpuTaskMaps = new QMap<unsigned int, Task>[nrCPUs];
	cpuFreq = new CpuFreq[nrCPUs];
	cpuIdle = new CpuIdle[nrCPUs];
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
}

void FtraceParser::preScan()
{
}

void FtraceParser::parse()
{
}

bool FtraceParser::processMigration()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (sched_migrate(event)) {
			Migration m;
			m.pid = sched_migrate_pid(event);
			m.oldcpu = sched_migrate_origCPU(event);
			m.newcpu = sched_migrate_destCPU(event);
			m.time = event.time;
			migrations.push_back(m);
		}
	}
	return false;
}

bool FtraceParser::processSched()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (sched_switch(event)) {
			processSwitchEvent(event);
		}
		if (sched_wakeup(event)) {
			processWakeupEvent(event);
		}
	}

	/* Add the "tail" to all tasks, i.e. extend them until endTime */
	unsigned int cpu;
	for (cpu = 0; i < nrCPUs; i++) {
		DEFINE_TASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
		while (iter != cpuTaskMaps[cpu].end()) {
			Task &task = iter.value();
			double d = task.data[task.data.size() - 1];
			task.timev.push_back(endTime);
			task.data.push_back(d);
			task.t.push_back(task.lastT);
			task.lastT += 1;
			iter++;
		}
	}
	return false;
}

bool FtraceParser::processCPUfreq()
{
	unsigned int i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		/*
		 * I expect this loop to be so fast in comparison
		 * to the other functions that will be running in parallel
		 * that it's acceptable to piggy back cpuidle events here */
		if (cpuidle_event(event)) {
			processCPUidleEvent(event);
			continue;
		}
		if (cpufreq_event(event))
			processCPUfreqEvent(event);
	}
	return false;
}

TColor FtraceParser::getNewColor()
{
	TColor color;
	bool ok;
	int retries = 0;
	do {
		color = TColor::getRandomColor();
		ok = checkColorMap(color);
		retries++;
	}
	while(!ok && retries < 200);
	return color;
}

void FtraceParser::colorizeTasks()
{
	unsigned int cpu;
	for (cpu = 0; cpu < maxCPU; cpu++) {
		DEFINE_TASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
		while (iter != cpuTaskMaps[cpu].end()) {
			Task &task = iter.value();
			iter++;
			if (colorMap.contains(task.pid))
				continue;
			TColor color = getNewColor();
			colorMap.insert(task.pid, color);
		}
	}
}

void FtraceParser::setSchedOffset(unsigned int cpu, double offset)
{
	schedOffset[cpu] = offset;
}

void FtraceParser::setSchedScale(unsigned int cpu, double scale)
{
	schedScale[cpu] = scale;
}

void FtraceParser::setCpuIdleOffset(unsigned int cpu, double offset)
{
	cpuIdleOffset[cpu] = offset;
}

void FtraceParser::setCpuIdleScale(unsigned int cpu, double scale)
{
	cpuIdleScale[cpu] = scale;
}

void FtraceParser::setCpuFreqOffset(unsigned int cpu, double offset)
{
	cpuFreqOffset[cpu] = offset;
}

void FtraceParser::setCpuFreqScale(unsigned int cpu, double scale)
{
	schedOffset[cpu] = scale;
}

void FtraceParser::scaleSched(unsigned int cpu)
{
	double scale = schedScale.value(cpu);
	double offset = schedOffset.value(cpu);
	DEFINE_TASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
	while (iter != cpuTaskMaps[cpu].end()) {
		Task &task = iter.value();
		int size = task.data.size();
		task.scaledData.resize(size);
		for (int i = 0; i < size; i++)
			task.scaledData[i] = task.data.at(i) * scale + offset;
		iter++;
	}
}

void FtraceParser::scaleCpuIdle(unsigned int cpu)
{
	double scale = cpuIdleScale.value(cpu);
	double offset = cpuIdleOffset.value(cpu);
	CpuIdle *idle = cpuIdle + cpu;
	int size = idle->data.size();

	for (int i = 0; i < size; i++)
		idle->scaledData[i] = idle->data.at(i) * scale + offset;
}

void FtraceParser::scaleCpuFreq(unsigned int cpu)
{
	double scale = cpuFreqScale.value(cpu);
	double offset = cpuFreqOffset.value(cpu);
	CpuFreq *freq = cpuFreq + cpu;
	int size = freq->data.size();

	for (int i = 0; i < size; i++)
		freq->scaledData[i] = freq->data.at(i) * scale + offset;
}
