/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef FTRACEPARSER_H
#define FTRACEPARSER_H

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QMap>
#include <QtGlobal>

#include "cpu.h"
#include "cpufreq.h"
#include "cpuidle.h"
#include "ftraceparams.h"
#include "mm/mempool.h"
#include "task.h"
#include "tcolor.h"
#include "traceevent.h"
#include "traceline.h"
#include "grammarnode.h"
#include "migration.h"
#include "threads/threadbuffer.h"
#include "traceshark.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"

using namespace TraceShark;

#define FAKE_DELTA ((double) 0.00000005)

/* Macros for the heights of the scheduling graph */
#define FULL_HEIGHT  ((double) 1)
#define WAKEUP_HEIGHT ((double) 1)
#define SCHED_HEIGHT ((double) 0.8)
#define FLOOR_HEIGHT ((double) 0)
#define NR_TBUFFERS (3)
#define TBUFSIZE (10000)

#define FULLDELAY (0.02)

class TraceFile;

class FtraceParser
{
public:
	FtraceParser();
	~FtraceParser();
	void DeleteGrammarTree(GrammarNode* node);
	bool open(const QString &fileName);
	bool isOpen();
	void close();
	void parse();
	void parseThread();
	void preScan();
	bool processMigration();
	bool processSched();
	bool processCPUfreq();
	QList<TraceEvent> events;
	__always_inline unsigned int getMaxCPU();
	__always_inline unsigned int getNrCPUs();
	__always_inline double getStartTime();
	__always_inline double getEndTime();
	__always_inline unsigned long getNrEvents();
	__always_inline int getMinIdleState();
	__always_inline int getMaxIdleState();
	__always_inline int getNrMigrateEvents();
	__always_inline QColor getTaskColor(unsigned int pid);
	void setSchedOffset(unsigned int cpu, double offset);
	void setSchedScale(unsigned int cpu, double scale);
	void setCpuIdleOffset(unsigned int cpu, double offset);
	void setCpuIdleScale(unsigned int cpu, double scale);
	void setCpuFreqOffset(unsigned int cpu, double offset);
	void setCpuFreqScale(unsigned int cpu, double scale);
	void doScale();
	void colorizeTasks();
	QMap<unsigned int, Task> *cpuTaskMaps;
	CpuFreq *cpuFreq;
	CpuIdle *cpuIdle;
	QVector<Migration> migrations;
private:
	void preparePreScan();
	void finalizePreScan();
	__always_inline bool parseBuffer(unsigned int index);
	__always_inline void preScanEvent(TraceEvent &event);
	__always_inline bool parseLine(TraceLine* line, TraceEvent* event);
	__always_inline void handleWrongTaskOnCPU(unsigned int cpu,
						  CPU *eventCPU,
						  unsigned int oldpid,
						  double oldtime);
	__always_inline void processSwitchEvent(TraceEvent &event);
	__always_inline void processWakeupEvent(TraceEvent &event);
	__always_inline void processCPUfreqEvent(TraceEvent &event);
	__always_inline void processCPUidleEvent(TraceEvent &event);
	ThreadBuffer<TraceLine> **tbuffers;
	WorkThread<FtraceParser> *parserThread;
	WorkQueue scalingQueue;
	GrammarNode *grammarRoot;
	TraceFile *traceFile;
	MemPool *ptrPool;
	MemPool *taskNamePool;
	unsigned int maxCPU;
	unsigned int nrCPUs;
	double startTime;
	double endTime;
	unsigned long nrEvents;
	unsigned long lastEvent;
	unsigned int maxFreq;
	unsigned int minFreq;
	int minIdleState;
	int maxIdleState;
	unsigned int nrMigrateEvents;
	QMap <unsigned int, TColor> colorMap;
	TColor black;
	TColor white;
	__always_inline bool checkColorMap(const TColor &color);
	TColor getNewColor();
	QVector<double> schedOffset;
	QVector<double> schedScale;
	QVector<double> cpuIdleOffset;
	QVector<double> cpuIdleScale;
	QVector<double> cpuFreqOffset;
	QVector<double> cpuFreqScale;
	CPU *CPUs;
	void addCpuFreqWork(unsigned int cpu,
		       QList<AbstractWorkItem*> &list);
	void addCpuIdleWork(unsigned int cpu,
		       QList<AbstractWorkItem*> &list);
	void addCpuSchedWork(unsigned int cpu,
		       QList<AbstractWorkItem*> &list);
	QVector<double> startFreq;
};

/* This parses a buffer */
__always_inline bool FtraceParser::parseBuffer(unsigned int index)
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
		TraceEvent event;
		event.argc = 0;
		event.argv = (TString**) ptrPool->preallocN(256);
		if (parseLine(line, &event)) {
			ptrPool->commitN(event.argc);
			events.push_back(event);
			nrEvents++;
			preScanEvent(event);
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}

__always_inline void FtraceParser::preScanEvent(TraceEvent &event)
{
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
		if (cpu <= HIGHEST_CPU_EVER && startFreq[cpu] < 0) {
			startFreq[cpu] = freq;
		}
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

__always_inline bool FtraceParser::parseLine(TraceLine* line, TraceEvent* event)
{
	unsigned int i,j;
	GrammarNode *node = grammarRoot;
	bool retval = grammarRoot->isLeaf;

	for (i = 0; i < line->nStrings; i++)
	{
		TString *str = &line->strings[i];
		if (node->nChildren == 0)
			break;
		for (j = 0; j < node->nChildren; j++) {
			if (node->children[j]->match(str, event)) {
				node = node->children[j];
				retval = node->children[j]->isLeaf;
				goto cont;
			}
		}
		return false;
	cont:
		continue;
	}
	return retval;
}

__always_inline unsigned int FtraceParser::getMaxCPU()
{
	return maxCPU;
}

__always_inline unsigned int FtraceParser::getNrCPUs()
{
	return nrCPUs;;
}

__always_inline double  FtraceParser::getStartTime()
{
	return startTime;
}

__always_inline double FtraceParser::getEndTime()
{
	return endTime;
}

__always_inline unsigned long int FtraceParser::getNrEvents()
{
	return nrEvents;
}

__always_inline int FtraceParser::getMinIdleState()
{
	return minIdleState;
}

__always_inline int FtraceParser::getMaxIdleState()
{
	return minIdleState;
}

__always_inline int FtraceParser::getNrMigrateEvents()
{
	return nrMigrateEvents;
}

__always_inline QColor FtraceParser::getTaskColor(unsigned int pid)
{
	TColor taskColor = colorMap.value(pid, black);
	return taskColor.toQColor();
}

__always_inline void FtraceParser::handleWrongTaskOnCPU(unsigned int cpu,
							CPU *eventCPU,
							unsigned int oldpid,
							double oldtime)
{
	unsigned int epid = eventCPU->pidOnCPU;
	double prevtime, faketime;
	Task *task;

	if (epid != 0) {
		task = &cpuTaskMaps[cpu][epid];
		Q_ASSERT(!task->isNew);
		Q_ASSERT(!task->timev.isEmpty());
		prevtime = task->timev.last();
		faketime = prevtime + FAKE_DELTA;
		task->timev.push_back(faketime);
		task->data.push_back(FLOOR_HEIGHT);
		task->lastWakeUP = faketime;
	}

	if (oldpid != 0) {
		task = &cpuTaskMaps[cpu][oldpid];
		if (task->isNew)
			task->isNew = false;
		faketime = oldtime - FAKE_DELTA;
		task->timev.push_back(faketime);
		task->data.push_back(SCHED_HEIGHT);
	}
}

__always_inline void FtraceParser::processSwitchEvent(TraceEvent &event)
{
	unsigned int cpu = event.cpu;
	double oldtime = event.time - FAKE_DELTA;
	double newtime = event.time + FAKE_DELTA;
	unsigned int oldpid = sched_switch_oldpid(event);
	unsigned int newpid = sched_switch_newpid(event);
	Task *task;
	CPU *eventCPU = &CPUs[cpu];

	if (cpu > maxCPU)
		return;

	if (eventCPU->pidOnCPU != oldpid) {
		if (eventCPU->hasBeenScheduled)
			handleWrongTaskOnCPU(cpu, eventCPU, oldpid, oldtime);
		/* else { do nothing, non scheduled CPU is handled below */
	}

	if (oldpid == 0) /* We don't care about the idle task */
		goto skip;

	/* Handle the outgoing task */
	task = &cpuTaskMaps[cpu][oldpid]; /* Modifiable reference */
	if (task->isNew) { /* true means task is newly constructed above */
		task->pid = oldpid;
		task->isNew = false;
		char state = sched_switch_state(event);

		/* Apparenly this task was on CPU when we started tracing */
		task->timev.push_back(startTime);
		task->data.push_back(SCHED_HEIGHT);

		if (state == 'R') {
			task->runningTimev.push_back(oldtime);
			task->runningData.push_back(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}

		task->timev.push_back(oldtime);
		task->data.push_back(FLOOR_HEIGHT);

		task->name = sched_switch_oldname_strdup(event, taskNamePool);
	} else {
		char state = sched_switch_state(event);

		task->timev.push_back(oldtime);
		task->data.push_back(FLOOR_HEIGHT);

		if (state == 'R') {
			task->runningTimev.push_back(oldtime);
			task->runningData.push_back(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}
	}

skip:
	if (newpid == 0) /* We don't care about the idle task */
		goto out;

	/* Handle the incoming task */
	task = &cpuTaskMaps[cpu][newpid]; /* Modifiable reference */
	if (task->isNew) { /* true means task is newly constructed above */
		/* A tasks woken up after startTime would have been created by
		 * the wakeup event */
		double delay = newtime - startTime;

		task->pid = newpid;
		task->isNew = false;

		task->timev.push_back(startTime);
		task->data.push_back(FLOOR_HEIGHT);

		task->wakeTimev.push_back(newtime);
		task->wakeDelay.push_back(delay);

		task->timev.push_back(newtime);
		task->data.push_back(SCHED_HEIGHT);

		task->name = sched_switch_newname_strdup(event, taskNamePool);
	} else {
		double delay = newtime - task->lastWakeUP;

		task->wakeTimev.push_back(newtime);
		task->wakeDelay.push_back(delay);

		task->timev.push_back(newtime);
		task->data.push_back(SCHED_HEIGHT);
	}
out:
	eventCPU->hasBeenScheduled = true;
	eventCPU->pidOnCPU = newpid;
	eventCPU->lastSched = newtime;
	return;
}

__always_inline void FtraceParser::processWakeupEvent(TraceEvent &event)
{
	unsigned int cpu, pid;
	Task *task;
	double time;

	if (!sched_wakeup_success(event)) /* Only interested in success */
		return;

	time = event.time;
	cpu = sched_wakeup_cpu(event);
	pid = sched_wakeup_pid(event);

	/* Handle the woken up task */
	task = &cpuTaskMaps[cpu][pid]; /* Modifiable reference */

	if (task->isNew) { /* true means task is newly constructed above */
		task->pid = pid;

		task->timev.push_back(startTime);
		task->data.push_back(FLOOR_HEIGHT);

		task->isNew = false;
		task->name = sched_wakeup_name_strdup(event, taskNamePool);
	}
	task->lastWakeUP = time;
}

__always_inline void FtraceParser::processCPUfreqEvent(TraceEvent &event)
{
	unsigned int cpu = cpufreq_cpu(event);
	double time = event.time;
	unsigned int freq = cpufreq_freq(event);

	cpuFreq[cpu].timev.push_back(time);
	cpuFreq[cpu].data.push_back((double) freq);
}

__always_inline void FtraceParser::processCPUidleEvent(TraceEvent &event)
{
	unsigned int cpu = cpuidle_cpu(event);
	double time = event.time;
	unsigned int state = cpuidle_state(event);

	cpuIdle[cpu].timev.push_back(time);
	cpuIdle[cpu].data.push_back((double) state);
}

__always_inline bool FtraceParser::checkColorMap(const TColor &color)
{
	if (black.SqDistance(color) < 500)
		return false;
	if (white.SqDistance(color) < 500)
		return false;

	DEFINE_COLORMAP_ITERATOR(iter) = colorMap.begin();

	while (iter != colorMap.end()) {
		TColor c = iter.value();
		if (c.SqDistance(color) < 500)
			return false;
		iter++;
	}
	return true;
}

#endif
