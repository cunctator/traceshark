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

#include "cpufreq.h"
#include "cpuidle.h"
#include "ftraceparams.h"
#include "mempool.h"
#include "task.h"
#include "tcolor.h"
#include "traceevent.h"
#include "traceline.h"
#include "grammarnode.h"
#include "migration.h"
#include "threadbuffer.h"
#include "traceshark.h"

using namespace TraceShark;

#define FAKE_DELTA ((double) 0.0000005)

/* Macros for the heights of the scheduling graph */
#define FULL_HEIGHT  ((double) 1)
#define SCHED_HEIGHT ((double) 0.6)
#define MAXDELAY_RISER  (FULL_HEIGHT - SCHED_HEIGHT)
#define FLOOR_HEIGHT ((double) 0.1)
#define SUBFLOOR_HEIGHT ((double) 0)
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
	void processMigration();
	void processSched();
	void processCPUfreq();
	QVector<TraceEvent> events;
	__always_inline unsigned int getMaxCPU();
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
	void scaleSched(unsigned int cpu);
	void scaleCpuIdle(unsigned int cpu);
	void scaleCpuFreq(unsigned int cpu);
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
	__always_inline void processSwitchEvent(TraceEvent &event);
	__always_inline void processWakeupEvent(TraceEvent &event);
	__always_inline void processCPUfreqEvent(TraceEvent &event);
	__always_inline void processCPUidleEvent(TraceEvent &event);
	ThreadBuffer<TraceLine> **tbuffers;
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

__always_inline void FtraceParser::processSwitchEvent(TraceEvent &event)
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
	task = &cpuTaskMaps[cpu][oldpid]; /* Modifiable reference */
	if (task->lastT == 0) { /* 0 means task is newly constructed above */
		unsigned long long lastT = 0ULL;
		task->pid = oldpid;
		char state = sched_switch_state(event);

		/* Apparenly this task was on CPU when we started tracing */
		task->timev.push_back(startTime);
		task->data.push_back(SCHED_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(oldtime);
		task->data.push_back(SCHED_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		/* If task is still running we indicate it with a small 
		 * "subfloor" spike */
		if (state == 'R') {
			task->timev.push_back(oldtime);
			task->data.push_back(SUBFLOOR_HEIGHT);
			task->t.push_back(lastfunc(lastT));
			task->lastWakeUP = oldtime;
			lastT++;
		}

		task->timev.push_back(oldtime);
		task->data.push_back(FLOOR_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
		task->name = sched_switch_oldname_strdup(event, taskNamePool);
	} else {
		unsigned long long lastT = task->lastT;

		task->timev.push_back(oldtime);
		task->data.push_back(SCHED_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(oldtime);
		task->data.push_back(FLOOR_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
	}

skip:
	if (newpid == 0) /* We don't care about the idle task */
		return;

	/* Handle the incoming task */
	task = &cpuTaskMaps[cpu][newpid]; /* Modifiable reference */
	if (task->lastT == 0) { /* 0 means task is newly constructed above */
		unsigned long long lastT = 0ULL;
		/* A tasks woken up after startTime would have been created by
		 * the wakeup event */
		double delay = newtime - startTime;
		delay = TSMAX(delay, FULLDELAY);
		double riser = MAXDELAY_RISER * delay / FULLDELAY;
		riser += SCHED_HEIGHT;

		task->pid = newpid;

		task->timev.push_back(startTime);
		task->data.push_back(FLOOR_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(FLOOR_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(riser);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(SCHED_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
		task->name = sched_switch_newname_strdup(event, taskNamePool);
	} else {
		unsigned long long lastT = task->lastT;
		double delay = newtime - task->lastWakeUP;
		delay = TSMAX(delay, FULLDELAY);
		double riser = MAXDELAY_RISER * delay / FULLDELAY;
		riser += SCHED_HEIGHT;

		task->timev.push_back(newtime);
		task->data.push_back(FLOOR_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(riser);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->timev.push_back(newtime);
		task->data.push_back(SCHED_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
	}
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

	if (task->lastT == 0) { /* 0 means task is newly constructed above */
		unsigned long long lastT = 0;
		task->pid = pid;

		task->timev.push_back(startTime);
		task->data.push_back(FLOOR_HEIGHT);
		task->t.push_back(lastfunc(lastT));
		lastT++;

		task->lastT = lastT;
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
