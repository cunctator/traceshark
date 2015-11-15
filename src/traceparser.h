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

#ifndef TRACEPARSER_H
#define TRACEPARSER_H

#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QMap>
#include <QtGlobal>
#include <limits>

#include "cpu.h"
#include "cpufreq.h"
#include "cpuidle.h"
#include "ftraceparams.h"
#include "mm/mempool.h"
#include "cputask.h"
#include "tcolor.h"
#include "traceevent.h"
#include "traceline.h"
#include "grammarnode.h"
#include "migration.h"
#include "migrationarrow.h"
#include "perfparams.h"
#include "task.h"
#include "threads/threadbuffer.h"
#include "traceshark.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"

#define FAKE_DELTA ((double) 0.00000005)

/* Macros for the heights of the scheduling graph */
#define FULL_HEIGHT  ((double) 1)
#define WAKEUP_HEIGHT ((double) 0.6)
#define WAKEUP_SIZE ((double) 0.4)
#define VERT_WAKEUP_HEIGHT ((double) 0.6)
#define SCHED_HEIGHT ((double) 0.5)
#define FLOOR_HEIGHT ((double) 0)
#define NR_TBUFFERS (3)
#define TBUFSIZE (10000)

#define FULLDELAY (0.02)

class TraceFile;
class QCustomPlot;

typedef enum {
	TRACE_TYPE_FTRACE = 0,
	TRACE_TYPE_PERF,
	TRACE_TYPE_MAX
} tracetype_t;

#define TRACE_TYPE_NONE (TRACE_TYPE_MAX)

class TraceParser
{
public:
	TraceParser();
	~TraceParser();
	void createFtraceGrammarTree();
	void createPerfGrammarTree();
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
	void setMigrationOffset(double offset);
	void setMigrationScale(double scale);
	void doScale();
	void colorizeTasks();
	void setQCustomPlot(QCustomPlot *plot);
	__always_inline Task *findTask(unsigned int pid);
	QMap<unsigned int, CPUTask> *cpuTaskMaps;
	QMap<unsigned int, Task> taskMap;
	CpuFreq *cpuFreq;
	CpuIdle *cpuIdle;
	QList<Migration> migrations;
	QList<MigrationArrow*> migrationArrows;
private:
	void preparePreScan();
	void finalizePreScan();
	bool parseBuffer(unsigned int index);
	__always_inline bool parseFtraceBuffer(unsigned int index);
	__always_inline bool parsePerfBuffer(unsigned int index);
	__always_inline void preScanFtraceEvent(TraceEvent &event);
	__always_inline void preScanPerfEvent(TraceEvent &event);
	__always_inline bool parseLine(TraceLine* line, TraceEvent* event,
				       GrammarNode *root);
	bool parseLineBugFixup(TraceEvent* event, double prevtime);
	__always_inline Task *getTask(unsigned int pid);
	__always_inline double estimateWakeUpNew(CPU *eventCPU, double newTime,
						 double startTime);
	__always_inline double estimateWakeUp(Task *task, CPU *eventCPU,
					      double newTime, double startTime);
	__always_inline void handleWrongTaskOnCPU(TraceEvent &event,
						  unsigned int cpu,
						  CPU *eventCPU,
						  unsigned int oldpid,
						  double oldtime);
	void processMigrationFtrace();
	void processMigrationPerf();
	__always_inline void processFtraceSwitchEvent(TraceEvent &event);
	__always_inline void processFtraceWakeupEvent(TraceEvent &event);
	__always_inline void processFtraceCPUfreqEvent(TraceEvent &event);
	__always_inline void processFtraceCPUidleEvent(TraceEvent &event);
	__always_inline void processPerfSwitchEvent(TraceEvent &event);
	__always_inline void processPerfWakeupEvent(TraceEvent &event);
	__always_inline void processPerfCPUfreqEvent(TraceEvent &event);
	__always_inline void processPerfCPUidleEvent(TraceEvent &event);
	void addCpuFreqWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuIdleWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuSchedWork(unsigned int cpu,
			     QList<AbstractWorkItem*> &list);
	void scaleMigration();
	void clearGrammarPools(GrammarNode *tree);
	void determineTraceType();
	void processSchedFtrace();
	void processSchedPerf();
	void processSchedAddTail();
	void _processCPUfreqFtrace();
	void _processCPUfreqPerf();
	ThreadBuffer<TraceLine> **tbuffers;
	WorkThread<TraceParser> *parserThread;
	WorkQueue scalingQueue;
	GrammarNode *ftraceGrammarRoot;
	GrammarNode *perfGrammarRoot;
	TraceFile *traceFile;
	MemPool *ptrPool;
	MemPool *taskNamePool;
	unsigned int maxCPU;
	unsigned int nrCPUs;
	double startTime;
	double endTime;
	unsigned long nrEvents;
	unsigned long nrFtraceEvents;
	unsigned long nrPerfEvents;
	tracetype_t traceType;
	unsigned long lastEvent;
	unsigned int maxFreq;
	unsigned int minFreq;
	int minIdleState;
	int maxIdleState;
	unsigned int nrMigrateEvents;
	QMap <unsigned int, TColor> colorMap;
	TColor black;
	TColor white;
	QVector<double> schedOffset;
	QVector<double> schedScale;
	QVector<double> cpuIdleOffset;
	QVector<double> cpuIdleScale;
	QVector<double> cpuFreqOffset;
	QVector<double> cpuFreqScale;
	double migrationOffset;
	double migrationScale;
	CPU *CPUs;
	QVector<double> startFreq;
	QCustomPlot *customPlot;
};

/* This parses a buffer */
__always_inline bool TraceParser::parseFtraceBuffer(unsigned int index)
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
		TraceEvent event;
		event.argc = 0;
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
			events.push_back(event);
			nrEvents++;
			preScanFtraceEvent(event);
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}

/* This parses a buffer */
__always_inline bool TraceParser::parsePerfBuffer(unsigned int index)
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
		TraceEvent event;
		event.argc = 0;
		event.argv = (TString**) ptrPool->preallocN(256);
		if (parseLine(line, &event, perfGrammarRoot)) {
			/* Check if the timestamp of this event is affected by
			 * the infamous ftrace timestamp rollover bug and
			 * try to correct it */
			if (event.time < prevtime) {
				if (!parseLineBugFixup(&event, prevtime))
					continue;
			}
			prevtime = event.time;
			ptrPool->commitN(event.argc);
			events.push_back(event);
			nrEvents++;
			preScanPerfEvent(event);
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}

__always_inline void TraceParser::preScanFtraceEvent(TraceEvent &event)
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

__always_inline void TraceParser::preScanPerfEvent(TraceEvent &event)
{
	if (event.cpu > maxCPU)
		maxCPU = event.cpu;
	if (perf_cpuidle_event(event)) {
		int state = perf_cpuidle_state(event);
		unsigned int cpu = perf_cpuidle_cpu(event);
		if (cpu > maxCPU)
			maxCPU = cpu;
		if (state < minIdleState)
			minIdleState = state;
		if (state > maxIdleState)
			maxIdleState = state;
	} else if (perf_cpufreq_event(event)) {
		unsigned int cpu = perf_cpufreq_cpu(event);
		unsigned int freq = perf_cpufreq_freq(event);
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
		unsigned int dest = perf_sched_migrate_destCPU(event);
		unsigned int orig = perf_sched_migrate_origCPU(event);
		if (dest > maxCPU)
			maxCPU = dest;
		if (orig > maxCPU)
			maxCPU = dest;
		nrMigrateEvents++;
	}
}

__always_inline bool TraceParser::parseLine(TraceLine* line, TraceEvent* event,
					    GrammarNode *root)
{
	unsigned int i,j;
	GrammarNode *node = root;
	bool retval = root->isLeaf;

	for (i = 0; i < line->nStrings; i++)
	{
		TString *str = &line->strings[i];
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

__always_inline double TraceParser::estimateWakeUpNew(CPU *eventCPU,
						       double newTime,
						       double startTime)
{
	double delay;

	if (!eventCPU->hasBeenScheduled)
		goto regular;

	if (eventCPU->lastEnterIdle < eventCPU->lastExitIdle)
		return FAKE_DELTA;
regular:
	delay = newTime - startTime;
	return delay;
}

__always_inline double TraceParser::estimateWakeUp(Task *task,
						    CPU *eventCPU,
						    double newTime,
						    double /* startTime */)
{
	double delay;

	/* Is this reasonable ? */
	if (task->lastWakeUP < eventCPU->lastEnterIdle)
		return FAKE_DELTA;

	delay = newTime - task->lastWakeUP;
	return delay;
}

__always_inline unsigned int TraceParser::getMaxCPU()
{
	return maxCPU;
}

__always_inline unsigned int TraceParser::getNrCPUs()
{
	return nrCPUs;;
}

__always_inline double  TraceParser::getStartTime()
{
	return startTime;
}

__always_inline double TraceParser::getEndTime()
{
	return endTime;
}

__always_inline unsigned long int TraceParser::getNrEvents()
{
	return nrEvents;
}

__always_inline int TraceParser::getMinIdleState()
{
	return minIdleState;
}

__always_inline int TraceParser::getMaxIdleState()
{
	return minIdleState;
}

__always_inline int TraceParser::getNrMigrateEvents()
{
	return nrMigrateEvents;
}

__always_inline QColor TraceParser::getTaskColor(unsigned int pid)
{
	TColor taskColor = colorMap.value(pid, black);
	return taskColor.toQColor();
}


__always_inline Task *TraceParser::getTask(unsigned int pid)
{
	Task *task = &taskMap[pid]; /* Modifiable reference */ ;
	if (task->isNew) { /* true means task is newly constructed above */
		task->pid = pid;
		task->isNew = false;
	}
	return task;
}

__always_inline Task *TraceParser::findTask(unsigned int pid)
{
	DEFINE_TASKMAP_ITERATOR(iter) = taskMap.find(pid);
	if (iter == taskMap.end())
		return NULL;
	else
		return &iter.value();
}

__always_inline void TraceParser::handleWrongTaskOnCPU(TraceEvent &event,
						       unsigned int cpu,
						       CPU *eventCPU,
						       unsigned int oldpid,
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
		cpuTask->timev.push_back(faketime);
		cpuTask->data.push_back(FLOOR_HEIGHT);
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
		cpuTask->timev.push_back(faketime);
		cpuTask->data.push_back(SCHED_HEIGHT);
	}
}

__always_inline void TraceParser::processFtraceSwitchEvent(TraceEvent &event)
{
	unsigned int cpu = event.cpu;
	double oldtime = event.time - FAKE_DELTA;
	double newtime = event.time + FAKE_DELTA;
	unsigned int oldpid = sched_switch_oldpid(event);
	unsigned int newpid = sched_switch_newpid(event);
	CPUTask *cpuTask;
	Task *task;
	double delay;
	CPU *eventCPU = &CPUs[cpu];

	if (cpu > maxCPU)
		return;

	if (eventCPU->pidOnCPU != oldpid) {
		if (eventCPU->hasBeenScheduled)
			handleWrongTaskOnCPU(event, cpu, eventCPU, oldpid,
					     oldtime);
		/* else { do nothing, non scheduled CPU is handled below } */
	}

	if (oldpid == 0) {
		eventCPU->lastExitIdle = oldtime;
		goto skip;  /* We don't care about the idle task */
	}

	/* Handle the outgoing task */
	cpuTask = &cpuTaskMaps[cpu][oldpid]; /* Modifiable reference */
	task = getTask(oldpid);
	if (cpuTask->isNew) { /* true means task is newly constructed above */
		cpuTask->pid = oldpid;
		cpuTask->isNew = false;
		char state = sched_switch_state(event);

		/* Apparenly this task was on CPU when we started tracing */
		cpuTask->timev.push_back(startTime);
		cpuTask->data.push_back(SCHED_HEIGHT);

		if (state == 'R') {
			cpuTask->runningTimev.push_back(oldtime);
			cpuTask->runningData.push_back(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}

		cpuTask->timev.push_back(oldtime);
		cpuTask->data.push_back(FLOOR_HEIGHT);
		cpuTask->name = sched_switch_oldname_strdup(event,
							    taskNamePool);
	} else {
		char state = sched_switch_state(event);

		cpuTask->timev.push_back(oldtime);
		cpuTask->data.push_back(FLOOR_HEIGHT);

		if (state == 'R') {
			cpuTask->runningTimev.push_back(oldtime);
			cpuTask->runningData.push_back(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}
	}

skip:
	if (newpid == 0) {
		eventCPU->lastEnterIdle = newtime;
		goto out; /* We don't care about the idle task */
	}

	/* Handle the incoming task */
	task = &taskMap[newpid]; /* Modifiable reference */
	if (task->isNew) {
		task->pid = newpid;
		task->isNew = false;
		delay = estimateWakeUpNew(eventCPU, newtime, startTime);
	} else
		delay = estimateWakeUp(task, eventCPU, newtime, startTime);

	cpuTask = &cpuTaskMaps[cpu][newpid]; /* Modifiable reference */
	if (cpuTask->isNew) { /* true means task is newly constructed above */
		cpuTask->pid = newpid;
		cpuTask->isNew = false;

		cpuTask->timev.push_back(startTime);
		cpuTask->data.push_back(FLOOR_HEIGHT);

		cpuTask->wakeTimev.push_back(newtime);
		cpuTask->wakeDelay.push_back(delay);

		cpuTask->timev.push_back(newtime);
		cpuTask->data.push_back(SCHED_HEIGHT);

		cpuTask->name = sched_switch_newname_strdup(event, taskNamePool);
	} else {
		cpuTask->wakeTimev.push_back(newtime);
		cpuTask->wakeDelay.push_back(delay);

		cpuTask->timev.push_back(newtime);
		cpuTask->data.push_back(SCHED_HEIGHT);
	}
out:
	eventCPU->hasBeenScheduled = true;
	eventCPU->pidOnCPU = newpid;
	eventCPU->lastSched = newtime;
	return;
}

__always_inline void TraceParser::processPerfSwitchEvent(TraceEvent &event)
{
	unsigned int cpu = event.cpu;
	double oldtime = event.time - FAKE_DELTA;
	double newtime = event.time + FAKE_DELTA;
	unsigned int oldpid = perf_sched_switch_oldpid(event);
	unsigned int newpid = perf_sched_switch_newpid(event);
	CPUTask *cpuTask;
	Task *task;
	double delay;
	CPU *eventCPU = &CPUs[cpu];

	if (cpu > maxCPU)
		return;

	if (eventCPU->pidOnCPU != oldpid) {
		if (eventCPU->hasBeenScheduled)
			handleWrongTaskOnCPU(event, cpu, eventCPU, oldpid,
					     oldtime);
		/* else { do nothing, non scheduled CPU is handled below } */
	}

	if (oldpid == 0) {
		eventCPU->lastExitIdle = oldtime;
		goto skip;  /* We don't care about the idle task */
	}

	/* Handle the outgoing task */
	cpuTask = &cpuTaskMaps[cpu][oldpid]; /* Modifiable reference */
	task = getTask(oldpid);
	if (cpuTask->isNew) { /* true means task is newly constructed above */
		cpuTask->pid = oldpid;
		cpuTask->isNew = false;
		char state = perf_sched_switch_state(event);

		/* Apparenly this task was on CPU when we started tracing */
		cpuTask->timev.push_back(startTime);
		cpuTask->data.push_back(SCHED_HEIGHT);

		if (state == 'R') {
			cpuTask->runningTimev.push_back(oldtime);
			cpuTask->runningData.push_back(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}

		cpuTask->timev.push_back(oldtime);
		cpuTask->data.push_back(FLOOR_HEIGHT);
		cpuTask->name = perf_sched_switch_oldname_strdup(event,
								 taskNamePool);
	} else {
		char state = perf_sched_switch_state(event);

		cpuTask->timev.push_back(oldtime);
		cpuTask->data.push_back(FLOOR_HEIGHT);

		if (state == 'R') {
			cpuTask->runningTimev.push_back(oldtime);
			cpuTask->runningData.push_back(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}
	}

skip:
	if (newpid == 0) {
		eventCPU->lastEnterIdle = newtime;
		goto out; /* We don't care about the idle task */
	}

	/* Handle the incoming task */
	task = &taskMap[newpid]; /* Modifiable reference */
	if (task->isNew) {
		task->pid = newpid;
		task->isNew = false;
		delay = estimateWakeUpNew(eventCPU, newtime, startTime);
	} else
		delay = estimateWakeUp(task, eventCPU, newtime, startTime);

	cpuTask = &cpuTaskMaps[cpu][newpid]; /* Modifiable reference */
	if (cpuTask->isNew) { /* true means task is newly constructed above */
		cpuTask->pid = newpid;
		cpuTask->isNew = false;

		cpuTask->timev.push_back(startTime);
		cpuTask->data.push_back(FLOOR_HEIGHT);

		cpuTask->wakeTimev.push_back(newtime);
		cpuTask->wakeDelay.push_back(delay);

		cpuTask->timev.push_back(newtime);
		cpuTask->data.push_back(SCHED_HEIGHT);

		cpuTask->name = perf_sched_switch_newname_strdup(event,
								 taskNamePool);
	} else {
		cpuTask->wakeTimev.push_back(newtime);
		cpuTask->wakeDelay.push_back(delay);

		cpuTask->timev.push_back(newtime);
		cpuTask->data.push_back(SCHED_HEIGHT);
	}
out:
	eventCPU->hasBeenScheduled = true;
	eventCPU->pidOnCPU = newpid;
	eventCPU->lastSched = newtime;
	return;
}

__always_inline void TraceParser::processFtraceWakeupEvent(TraceEvent &event)
{
	unsigned int pid;
	Task *task;
	double time;

	if (!sched_wakeup_success(event)) /* Only interested in success */
		return;

	time = event.time;
	pid = sched_wakeup_pid(event);

	/* Handle the woken up task */
	task = getTask(pid);
	task->lastWakeUP = time;
}

__always_inline void TraceParser::processPerfWakeupEvent(TraceEvent &event)
{
	unsigned int pid;
	Task *task;
	double time;

	if (!perf_sched_wakeup_success(event)) /* Only interested in success */
		return;

	time = event.time;
	pid = perf_sched_wakeup_pid(event);

	/* Handle the woken up task */
	task = getTask(pid);
	task->lastWakeUP = time;
}

__always_inline void TraceParser::processFtraceCPUfreqEvent(TraceEvent &event)
{
	unsigned int cpu = cpufreq_cpu(event);
	double time = event.time;
	unsigned int freq = cpufreq_freq(event);

	cpuFreq[cpu].timev.push_back(time);
	cpuFreq[cpu].data.push_back((double) freq);
}

__always_inline void TraceParser::processPerfCPUfreqEvent(TraceEvent &event)
{
	unsigned int cpu = perf_cpufreq_cpu(event);
	double time = event.time;
	unsigned int freq = perf_cpufreq_freq(event);

	cpuFreq[cpu].timev.push_back(time);
	cpuFreq[cpu].data.push_back((double) freq);
}

__always_inline void TraceParser::processFtraceCPUidleEvent(TraceEvent &event)
{
	unsigned int cpu = cpuidle_cpu(event);
	double time = event.time;
	unsigned int state = cpuidle_state(event);

	cpuIdle[cpu].timev.push_back(time);
	cpuIdle[cpu].data.push_back((double) state);
}

__always_inline void TraceParser::processPerfCPUidleEvent(TraceEvent &event)
{
	unsigned int cpu = perf_cpuidle_cpu(event);
	double time = event.time;
	unsigned int state = perf_cpuidle_state(event);

	cpuIdle[cpu].timev.push_back(time);
	cpuIdle[cpu].data.push_back((double) state);
}

#endif /* TRACEPARSER_H */
