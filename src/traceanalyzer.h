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

#ifndef TRACEANALYZER_H
#define TRACEANALYZER_H

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
#include "migration.h"
#include "migrationarrow.h"
#include "task.h"
#include "traceparser.h"
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

#define FULLDELAY (0.02)

class TraceFile;
class QCustomPlot;

class TraceAnalyzer
{
public:
	TraceAnalyzer();
	~TraceAnalyzer();
	bool open(const QString &fileName);
	bool isOpen();
	void close();
	void processTrace();
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
	__always_inline tracetype_t getTraceType();
	void setSchedOffset(unsigned int cpu, double offset);
	void setSchedScale(unsigned int cpu, double scale);
	void setCpuIdleOffset(unsigned int cpu, double offset);
	void setCpuIdleScale(unsigned int cpu, double scale);
	void setCpuFreqOffset(unsigned int cpu, double offset);
	void setCpuFreqScale(unsigned int cpu, double scale);
	void setMigrationOffset(double offset);
	void setMigrationScale(double scale);
	void doScale();
	void setQCustomPlot(QCustomPlot *plot);
	__always_inline Task *findTask(unsigned int pid);
	QMap<unsigned int, CPUTask> *cpuTaskMaps;
	QMap<unsigned int, Task> taskMap;
	CpuFreq *cpuFreq;
	CpuIdle *cpuIdle;
	QList<Migration> migrations;
	QList<MigrationArrow*> migrationArrows;
private:
	TraceParser *parser;
	void prepareDataStructures();
	bool processMigration();
	bool processSched();
	bool processCPUfreq();
	int binarySearch(double time, int start, int end);
	void colorizeTasks();
	int findIndexBefore(double time);
	__always_inline unsigned int generic_sched_switch_newpid(TraceEvent
								 &event);
	__always_inline unsigned int generic_sched_wakeup_pid(TraceEvent
							      &event);
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
	WorkItem<TraceAnalyzer> *schedItem;
	WorkItem<TraceAnalyzer> *migItem;
	WorkItem<TraceAnalyzer> *freqItem;
	WorkQueue processingQueue;
	WorkQueue scalingQueue;
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
	MemPool *taskNamePool;
	QCustomPlot *customPlot;
};

__always_inline double TraceAnalyzer::estimateWakeUpNew(CPU *eventCPU,
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

__always_inline double TraceAnalyzer::estimateWakeUp(Task *task,
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

__always_inline unsigned int TraceAnalyzer::generic_sched_switch_newpid(TraceEvent
								      &event)
{
	if (!tracetype_is_valid(getTraceType()))
		return UINT_MAX;
	return sched_switch_newpid(getTraceType(), event);
}

__always_inline unsigned int TraceAnalyzer::generic_sched_wakeup_pid(TraceEvent
								   &event)
{
	if (!tracetype_is_valid(getTraceType()))
		return UINT_MAX;
	return sched_wakeup_pid(getTraceType(), event);
}

__always_inline unsigned int TraceAnalyzer::getMaxCPU()
{
	return parser->maxCPU;
}

__always_inline unsigned int TraceAnalyzer::getNrCPUs()
{
	return parser->nrCPUs;;
}

__always_inline double  TraceAnalyzer::getStartTime()
{
	return parser->startTime;
}

__always_inline double TraceAnalyzer::getEndTime()
{
	return parser->endTime;
}

__always_inline unsigned long int TraceAnalyzer::getNrEvents()
{
	return parser->nrEvents;
}

__always_inline int TraceAnalyzer::getMinIdleState()
{
	return parser->minIdleState;
}

__always_inline int TraceAnalyzer::getMaxIdleState()
{
	return parser->minIdleState;
}

__always_inline int TraceAnalyzer::getNrMigrateEvents()
{
	return parser->nrMigrateEvents;
}

__always_inline QColor TraceAnalyzer::getTaskColor(unsigned int pid)
{
	TColor taskColor = colorMap.value(pid, black);
	return taskColor.toQColor();
}


__always_inline Task *TraceAnalyzer::getTask(unsigned int pid)
{
	Task *task = &taskMap[pid]; /* Modifiable reference */ ;
	if (task->isNew) { /* true means task is newly constructed above */
		task->pid = pid;
		task->isNew = false;
	}
	return task;
}

__always_inline tracetype_t TraceAnalyzer::getTraceType()
{
	return parser->traceType;
}

__always_inline Task *TraceAnalyzer::findTask(unsigned int pid)
{
	DEFINE_TASKMAP_ITERATOR(iter) = taskMap.find(pid);
	if (iter == taskMap.end())
		return NULL;
	else
		return &iter.value();
}

__always_inline void TraceAnalyzer::__processMigrationGeneric(tracetype_t ttype)
{
	unsigned long i;
	Migration m;
	for (i = 0; i < getNrEvents(); i++) {
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

__always_inline void TraceAnalyzer::__processSwitchEvent(tracetype_t ttype,
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

	if (cpu > getMaxCPU())
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
		cpuTask->timev.append(getStartTime());
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
		delay = estimateWakeUpNew(eventCPU, newtime, getStartTime());
	} else
		delay = estimateWakeUp(task, eventCPU, newtime, getStartTime());

	cpuTask = &cpuTaskMaps[cpu][newpid]; /* Modifiable reference */
	if (cpuTask->isNew) { /* true means task is newly constructed above */
		cpuTask->pid = newpid;
		cpuTask->isNew = false;

		cpuTask->timev.append(getStartTime());
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

__always_inline void TraceAnalyzer::__processWakeupEvent(tracetype_t ttype,
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

__always_inline void TraceAnalyzer::processCPUfreqEvent(tracetype_t ttype,
							TraceEvent &event)
{
	unsigned int cpu = cpufreq_cpu(ttype, event);
	double time = event.time;
	unsigned int freq = cpufreq_freq(ttype, event);

	cpuFreq[cpu].timev.append(time);
	cpuFreq[cpu].data.append((double) freq);
}

__always_inline void TraceAnalyzer::processCPUidleEvent(tracetype_t ttype,
							TraceEvent &event)
{
	unsigned int cpu = cpuidle_cpu(ttype, event);
	double time = event.time;
	unsigned int state = cpuidle_state(ttype, event);

	cpuIdle[cpu].timev.append(time);
	cpuIdle[cpu].data.append((double) state);
}

__always_inline void TraceAnalyzer::__processSchedGeneric(tracetype_t ttype)
{
	unsigned long i;
	for (i = 0; i < getNrEvents(); i++) {
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

__always_inline void TraceAnalyzer::__processCPUfreq(tracetype_t ttype)
{
	unsigned int i;
	for (i = 0; i < getNrEvents(); i++) {
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

#endif /* TRACEANALYZER_H */
