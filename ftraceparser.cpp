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
#include "timenode.h"
#include "eventnode.h"
#include "argnode.h"
#include "traceshark.h"

using namespace TraceShark;

#define FAKE_DELTA ((double) 0.0000005)

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

bool FtraceParser::isOpen()
{
	return (traceFile != NULL);
}

void FtraceParser::close()
{
	if (traceFile != NULL) {
		lines.resize(0);
		lines.reserve(1);
		events.resize(0);
		events.reserve(1);
		delete traceFile;
		traceFile = NULL;
	}
	if (cpuTaskMaps != NULL)
		delete[] cpuTaskMaps;
	if (cpuFreq != NULL)
		delete[] cpuFreq;
	if (cpuIdle != NULL)
		delete[] cpuIdle;
	migrations.resize(0);
}

FtraceParser::FtraceParser()
	: cpuTaskMaps(NULL), cpuFreq(NULL), cpuIdle(NULL)
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
}

FtraceParser::~FtraceParser()
{
	FtraceParser::close();
	DeleteGrammarTree(grammarRoot);
	delete ptrPool;
	delete taskNamePool;
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
		event.argv = (TString**) ptrPool->preallocN(256);
		if (parseLine(&line, &event)) {
			ptrPool->commitN(event.argc);
			events.push_back(event);
		}
	}

	return true;
}

void FtraceParser::preScan()
{
	unsigned long i;

	nrEvents = events.size();
	lastEvent = nrEvents - 1;
	maxCPU = 0;
	startTime = 0;
	endTime = 0;
	minFreq = 2147483647;
	maxFreq = 0;
	minIdleState = 31000;
	maxIdleState = -31000;
	nrMigrateEvents = 0;

	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (event.cpu > maxCPU)
			maxCPU = event.cpu;
		if (cpuidle_event(event)) {
			int state = cpuidle_state(event);
			unsigned int cpu = cpuidle_cpu(event);
			if (cpu > maxCPU)
				maxCPU = cpu;
			if (state < minIdleState)
				minIdleState = state;
			if (state > maxIdleState)
				maxIdleState = state;
		} else if (cpufreq_event(event)) {
			unsigned int cpu = cpufreq_cpu(event);
			unsigned int freq = cpufreq_freq(event);
			if (freq > maxFreq)
				maxFreq = freq;
			if (freq < minFreq)
				minFreq = freq;
			if (cpu > maxCPU)
				maxCPU = cpu;
		} else if (sched_migrate(event)) {
			unsigned int dest = sched_migrate_destCPU(event);
			unsigned int orig = sched_migrate_origCPU(event);
			if (dest > maxCPU)
				maxCPU = dest;
			if (orig > maxCPU)
				maxCPU = dest;
			nrMigrateEvents++;
		}
	}

	if (nrEvents >= 2) {
		startTime = events[0].time;
		endTime = events[lastEvent].time;
	}

	nrCPUs = maxCPU + 1;
	cpuTaskMaps = new QMap<unsigned int, Task>[nrCPUs];
	cpuFreq = new CpuFreq[nrCPUs];
	cpuIdle = new CpuIdle[nrCPUs];
}

void FtraceParser::processMigration()
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
}

static __always_inline void processSwitchEvent(TraceEvent &event,
					       QMap<unsigned int, Task>
					       *taskMaps, double &startTime,
					       MemPool *pool)
{
	unsigned int cpu = event.cpu;
	double oldtime = event.time - FAKE_DELTA;
	double newtime = event.time + FAKE_DELTA;
	unsigned int oldpid = sched_switch_oldpid(event);
	unsigned int newpid = sched_switch_newpid(event);
	Task *task;

	if (oldpid == 0) /* We don't care about the idle task */
		goto skip;

	/* Handle the outgoing task */
	task = &taskMaps[cpu][oldpid]; /* Modifiable reference */
	if (task->lastT == 0) { /* 0 means task is newly constructed above */
		double lastT = (unsigned long long) task->lastT;
		task->pid = oldpid;

		/* Apparenly this task was on CPU when we started tracing */
		task->timev.push_back(startTime);
		task->data.push_back(1);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(oldtime);
		task->data.push_back(1);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(oldtime);
		task->data.push_back(0);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
		task->name = sched_switch_oldname_strdup(event, pool);
	} else {
		double lastT = task->lastT;

		task->timev.push_back(oldtime);
		task->data.push_back(1);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(oldtime);
		task->data.push_back(0);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
	}

skip:
	if (newpid == 0) /* We don't care about the idle task */
		return;

	/* Handle the incoming task */
	task = &taskMaps[cpu][newpid]; /* Modifiable reference */
	if (task->lastT == 0) { /* 0 means task is newly constructed above */
		unsigned long long lastT = task->lastT;
		task->pid = newpid;

		task->timev.push_back(startTime);
		task->data.push_back(0);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(0);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(0);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
		task->name = sched_switch_newname_strdup(event, pool);
	} else {
		double lastT = task->lastT;

		task->timev.push_back(newtime);
		task->data.push_back(0);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(1);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
	}
}

void FtraceParser::processSched()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (sched_switch(event)) {
			processSwitchEvent(event, cpuTaskMaps, startTime,
					   taskNamePool);
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
}

static __always_inline void processCPUfreqEvent(TraceEvent &event,
						CpuFreq *cpuFreq)
{
	unsigned int cpu = cpufreq_cpu(event);
	double time = event.time;
	unsigned int freq = cpufreq_freq(event);

	cpuFreq[cpu].timev.push_back(time);
	cpuFreq[cpu].data.push_back((double) freq);
}

static __always_inline void processCPUidleEvent(TraceEvent &event,
						CpuIdle *cpuIdle)
{
	unsigned int cpu = cpuidle_cpu(event);
	double time = event.time;
	unsigned int state = cpuidle_state(event);

	cpuIdle[cpu].timev.push_back(time);
	cpuIdle[cpu].data.push_back((double) state);
}

void FtraceParser::processCPUfreq()
{
	unsigned int i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		/*
		 * I expect this loop to be so fast in comparison
		 * to the other functions that will be running in parallel
		 * that it's acceptable to piggy back cpuidle events here */
		if (cpuidle_event(event)) {
			processCPUidleEvent(event, cpuIdle);
			continue;
		}
		if (cpufreq_event(event))
			processCPUfreqEvent(event, cpuFreq);
	}
}
