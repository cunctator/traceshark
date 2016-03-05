/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "genericparams.h"
#include "mm/mempool.h"
#include "cputask.h"
#include "tcolor.h"
#include "traceevent.h"
#include "traceline.h"
#include "grammarnode.h"
#include "migration.h"
#include "migrationarrow.h"
#include "task.h"
#include "threads/threadbuffer.h"
#include "traceshark.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"
#include "tlist.h"

#define FAKE_DELTA ((double) 0.00000005)

/* Macros for the heights of the scheduling graph */
#define FULL_HEIGHT  ((double) 1)
#define WAKEUP_HEIGHT ((double) 0.6)
#define WAKEUP_SIZE ((double) 0.4)
#define VERT_WAKEUP_HEIGHT ((double) 0.6)
#define SCHED_HEIGHT ((double) 0.5)
#define FLOOR_HEIGHT ((double) 0)
#define NR_TBUFFERS (3)
#define TBUFSIZE (256)

#define FULLDELAY (0.02)

class TraceFile;
class QCustomPlot;

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
	TList<TraceEvent> events;
	TraceEvent *findPreviousSchedEvent(double time, unsigned int pid,
					   int *index);
	TraceEvent *findPreviousWakeupEvent(int startidx, unsigned int pid,
					    int *index);
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
	int binarySearch(double time, int start, int end);
	int findIndexBefore(double time);
	void preparePreScan();
	void finalizePreScan();
	void fixLastEvent();
	bool parseBuffer(unsigned int index);
	__always_inline unsigned int generic_sched_switch_newpid(TraceEvent
								 &event);
	__always_inline unsigned int generic_sched_wakeup_pid(TraceEvent
							      &event);
	__always_inline bool __parseBuffer(tracetype_t ttppe,
					   unsigned int index);
	__always_inline bool parseFtraceBuffer(unsigned int index);
	__always_inline bool parsePerfBuffer(unsigned int index);
	__always_inline void __preScanEvent(tracetype_t ttype,
					    TraceEvent &event);
	__always_inline void preScanFtraceEvent(TraceEvent &event);
	__always_inline void preScanPerfEvent(TraceEvent &event);
	__always_inline bool parseLine(TraceLine* line, TraceEvent* event,
				       GrammarNode *root);
	__always_inline bool parseLineFtrace(TraceLine* line,
					     TraceEvent &event);
	__always_inline bool parseLinePerf(TraceLine *line, TraceEvent &event);
	bool parseLineBugFixup(TraceEvent* event);
	__always_inline Task *getTask(unsigned int pid);
	__always_inline double estimateWakeUpNew(CPU *eventCPU, double newTime,
						 double startTime);
	__always_inline double estimateWakeUp(Task *task, CPU *eventCPU,
					      double newTime, double startTime);
	void handleWrongTaskOnCPU(TraceEvent &event, unsigned int cpu,
				  CPU *eventCPU, unsigned int oldpid,
				  double oldtime);
	__always_inline void __processMigrationGeneric(tracetype_t ttype);
	void processMigrationFtrace();
	void processMigrationPerf();
	__always_inline void __processSwitchEvent(tracetype_t ttype,
						  TraceEvent &event);
	__always_inline void __processWakeupEvent(tracetype_t ttype,
						  TraceEvent &event);
	__always_inline void processCPUfreqEvent(tracetype_t ttype,
						 TraceEvent &event);
	__always_inline void processCPUidleEvent(tracetype_t ttype,
						 TraceEvent &event);
	__always_inline void processPerfCPUfreqEvent(TraceEvent &event);
	__always_inline void processPerfCPUidleEvent(TraceEvent &event);
	void addCpuFreqWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuIdleWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuSchedWork(unsigned int cpu,
			     QList<AbstractWorkItem*> &list);
	void scaleMigration();
	void _clearGrammarPools(GrammarNode *tree);
	void resetGrammarReapedFlag(GrammarNode *tree);
	void clearGrammarPools(GrammarNode *tree);
	void determineTraceType();
	__always_inline void __processSchedGeneric(tracetype_t ttype);
	void processSchedFtrace();
	void processSchedPerf();
	void processSchedAddTail();
	__always_inline void __processCPUfreq(tracetype_t ttype);
	void processCPUfreqFtrace();
	void processCPUfreqPerf();
	ThreadBuffer<TraceLine> **tbuffers;
	WorkThread<TraceParser> *parserThread;
	WorkQueue scalingQueue;
	GrammarNode *ftraceGrammarRoot;
	GrammarNode *perfGrammarRoot;
	TraceFile *traceFile;
	MemPool *ptrPool;
	MemPool *taskNamePool;
	MemPool *postEventPool;
	unsigned int maxCPU;
	unsigned int nrCPUs;
	double startTime;
	double endTime;
	unsigned long nrEvents;
	unsigned long nrFtraceEvents;
	unsigned long nrPerfEvents;
	TraceEvent fakeEvent;
	TString fakePostEventInfo;
	TraceEvent *prevEvent;
	char *infoBegin;
	bool prevLineIsEvent;
	double prevTime;
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
	return __parseBuffer(TRACE_TYPE_FTRACE, index);
}

/* This parses a buffer */
__always_inline bool TraceParser::parsePerfBuffer(unsigned int index)
{
	return __parseBuffer(TRACE_TYPE_PERF, index);
}

/* This parses a buffer */
__always_inline bool TraceParser::__parseBuffer(tracetype_t ttype,
						unsigned int index)
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
		TraceEvent &event = events.preAlloc();
		event.argc = 0;
		event.argv = (TString**) ptrPool->preallocN(256);
		if (ttype == TRACE_TYPE_FTRACE)
			parseLineFtrace(line, event);
		else if (ttype == TRACE_TYPE_PERF)
			parseLinePerf(line, event);
	}
	tbuf->endConsumeBuffer();
	return false;
}
		
__always_inline void TraceParser::__preScanEvent(tracetype_t ttype,
						 TraceEvent &event)
{
	int state;
	unsigned int cpu;
	unsigned int freq;
	unsigned int dest;
	unsigned int orig;

	if (event.cpu > maxCPU)
		maxCPU = event.cpu;

	switch (event.type) {
	case CPU_IDLE:
		if (!cpuidle_args_ok(ttype, event))
			break;
		state = cpuidle_state(ttype, event);
		cpu = cpuidle_cpu(ttype, event);
		if (cpu > maxCPU)
			maxCPU = cpu;
		if (state < minIdleState)
			minIdleState = state;
		if (state > maxIdleState)
			maxIdleState = state;
		break;
	case CPU_FREQUENCY:
		if (!cpufreq_args_ok(ttype, event))
			break;
		cpu = cpufreq_cpu(ttype, event);
		freq = cpufreq_freq(ttype, event);
		if (freq > maxFreq)
			maxFreq = freq;
		if (freq < minFreq)
			minFreq = freq;
		if (cpu > maxCPU)
			maxCPU = cpu;
		if (cpu <= HIGHEST_CPU_EVER && startFreq[cpu] < 0) {
			startFreq[cpu] = freq;
		}
		break;
	case SCHED_MIGRATE_TASK:
		dest = sched_migrate_destCPU(ttype, event);
		orig = sched_migrate_origCPU(ttype, event);
		if (dest > maxCPU)
			maxCPU = dest;
		if (orig > maxCPU)
			maxCPU = dest;
		nrMigrateEvents++;
		break;
	default:
		break;
	}
}

__always_inline void TraceParser::preScanFtraceEvent(TraceEvent &event)
{
	__preScanEvent(TRACE_TYPE_FTRACE, event);
}

__always_inline void TraceParser::preScanPerfEvent(TraceEvent &event)
{
	__preScanEvent(TRACE_TYPE_PERF, event);
}

__always_inline bool TraceParser::parseLine(TraceLine* line, TraceEvent* event,
					    GrammarNode *root)
{
	unsigned int i,j;
	GrammarNode *node = root;
	bool retval = root->isLeaf;
	event->argc = 0;

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

__always_inline bool TraceParser::parseLineFtrace(TraceLine* line,
						  TraceEvent &event)
{
	if (parseLine(line, &event, ftraceGrammarRoot)) {
		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < prevTime) {
			if (!parseLineBugFixup(&event))
				return true;
		}
		prevTime = event.time;
		ptrPool->commitN(event.argc);
		event.postEventInfo = NULL;
		events.commit();
		nrFtraceEvents++;
		preScanFtraceEvent(event);
		/* probably not necessary because ftrace traces doesn't
		 * have backtraces and stuff but do it anyway */
		prevLineIsEvent = true;
		return true;
	}
	return false;
}

__always_inline bool TraceParser::parseLinePerf(TraceLine* line,
						TraceEvent & event)
{
	if (parseLine(line, &event, perfGrammarRoot)) {
		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < prevTime) {
			if (!parseLineBugFixup(&event))
				return true;
		}
		prevTime = event.time;
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
		return true;
	} else {
		if (prevLineIsEvent) {
			infoBegin = line->begin;
			prevLineIsEvent = false;
		}
		return false;
	}
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

__always_inline unsigned int TraceParser::generic_sched_switch_newpid(TraceEvent
								      &event)
{
	if (!tracetype_is_valid(traceType))
		return UINT_MAX;
	return sched_switch_newpid(traceType, event);
}

__always_inline unsigned int TraceParser::generic_sched_wakeup_pid(TraceEvent
								   &event)
{
	if (!tracetype_is_valid(traceType))
		return UINT_MAX;
	return sched_wakeup_pid(traceType, event);
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

__always_inline void TraceParser::__processMigrationGeneric(tracetype_t ttype)
{
	unsigned long i;
	Migration m;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		switch (event.type) {
		case SCHED_MIGRATE_TASK:
			if (!sched_migrate_args_ok(ttype, event))
				break;
			m.pid = sched_migrate_pid(ttype, event);
			m.oldcpu = sched_migrate_origCPU(ttype, event);
			m.newcpu = sched_migrate_destCPU(ttype, event);
			m.time = event.time;
			migrations.append(m);
			break;
		case SCHED_PROCESS_FORK:
			if (!sched_process_fork_args_ok(ttype, event))
				break;
			m.pid = sched_process_fork_childpid(ttype, event);
			m.oldcpu = -1;
			m.newcpu = event.cpu;
			m.time = event.time;
			migrations.append(m);
			break;
		case SCHED_PROCESS_EXIT:
			if (!sched_process_exit_args_ok(ttype, event))
				break;
			m.pid = sched_process_exit_pid(ttype, event);
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



__always_inline void TraceParser::__processSwitchEvent(tracetype_t ttype,
						       TraceEvent &event)
{
	unsigned int cpu = event.cpu;
	double oldtime = event.time - FAKE_DELTA;
	double newtime = event.time + FAKE_DELTA;
	unsigned int oldpid = sched_switch_oldpid(ttype, event);
	unsigned int newpid = sched_switch_newpid(ttype, event);
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
		taskstate_t state = sched_switch_state(ttype, event);

		/* Apparenly this task was on CPU when we started tracing */
		cpuTask->timev.append(startTime);
		cpuTask->data.append(SCHED_HEIGHT);

		if (state == TASK_STATE_RUNNABLE) {
			cpuTask->runningTimev.append(oldtime);
			cpuTask->runningData.append(FLOOR_HEIGHT);
			task->lastWakeUP = oldtime;
		}

		cpuTask->timev.append(oldtime);
		cpuTask->data.append(FLOOR_HEIGHT);
		cpuTask->name = sched_switch_oldname_strdup(ttype, event,
							    taskNamePool);
	} else {
		taskstate_t state = sched_switch_state(ttype, event);

		cpuTask->timev.append(oldtime);
		cpuTask->data.append(FLOOR_HEIGHT);

		if (state == TASK_STATE_RUNNABLE) {
			cpuTask->runningTimev.append(oldtime);
			cpuTask->runningData.append(FLOOR_HEIGHT);
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

		cpuTask->timev.append(startTime);
		cpuTask->data.append(FLOOR_HEIGHT);

		cpuTask->wakeTimev.append(newtime);
		cpuTask->wakeDelay.append(delay);

		cpuTask->timev.append(newtime);
		cpuTask->data.append(SCHED_HEIGHT);

		cpuTask->name = sched_switch_newname_strdup(ttype, event,
							    taskNamePool);
	} else {
		cpuTask->wakeTimev.append(newtime);
		cpuTask->wakeDelay.append(delay);

		cpuTask->timev.append(newtime);
		cpuTask->data.append(SCHED_HEIGHT);
	}
out:
	eventCPU->hasBeenScheduled = true;
	eventCPU->pidOnCPU = newpid;
	eventCPU->lastSched = newtime;
	return;
}

__always_inline void TraceParser::__processWakeupEvent(tracetype_t ttype,
						     TraceEvent &event)
{
	unsigned int pid;
	Task *task;
	double time;

	/* Only interested in success */
	if (!sched_wakeup_success(ttype, event))
		return;

	time = event.time;
	pid = sched_wakeup_pid(ttype, event);

	/* Handle the woken up task */
	task = getTask(pid);
	task->lastWakeUP = time;
}

__always_inline void TraceParser::processCPUfreqEvent(tracetype_t ttype,
						      TraceEvent &event)
{
	unsigned int cpu = cpufreq_cpu(ttype, event);
	double time = event.time;
	unsigned int freq = cpufreq_freq(ttype, event);

	cpuFreq[cpu].timev.append(time);
	cpuFreq[cpu].data.append((double) freq);
}

__always_inline void TraceParser::processCPUidleEvent(tracetype_t ttype,
						      TraceEvent &event)
{
	unsigned int cpu = cpuidle_cpu(ttype, event);
	double time = event.time;
	unsigned int state = cpuidle_state(ttype, event);

	cpuIdle[cpu].timev.append(time);
	cpuIdle[cpu].data.append((double) state);
}

__always_inline void TraceParser::__processSchedGeneric(tracetype_t ttype)
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		switch (event.type) {
		case SCHED_SWITCH:
			if (sched_switch_args_ok(ttype, event))
				__processSwitchEvent(ttype, event);
			break;
		case SCHED_WAKEUP:
		case SCHED_WAKEUP_NEW:
			if (sched_wakeup_args_ok(ttype, event))
				__processWakeupEvent(ttype, event);
			break;
		default:
			break;
		}
	}
	processSchedAddTail();
}

__always_inline void TraceParser::__processCPUfreq(tracetype_t ttype)
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
			if (cpufreq_args_ok(ttype, event))
				processCPUidleEvent(ttype, event);
			break;
		case CPU_FREQUENCY:
			if (cpuidle_args_ok(ttype, event))
				processCPUfreqEvent(ttype, event);
			break;
		default:
			break;
		}
	}
}

#endif /* TRACEPARSER_H */
