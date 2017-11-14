/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
#include <QTextStream>
#include <limits>

#include "analyzer/cpu.h"
#include "analyzer/cpufreq.h"
#include "analyzer/cpuidle.h"
#include "analyzer/filterstate.h"
#include "parser/genericparams.h"
#include "mm/mempool.h"
#include "analyzer/cputask.h"
#include "analyzer/tcolor.h"
#include "parser/traceevent.h"
#include "analyzer/migration.h"
#include "ui/migrationarrow.h"
#include "analyzer/task.h"
#include "parser/traceparser.h"
#include "misc/traceshark.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"
#include "misc/tlist.h"

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
	TList <const TraceEvent*> filteredEvents;
	const TraceEvent *findPreviousSchedEvent(double time, unsigned int pid,
						 int *index) const;
	const TraceEvent *findPreviousWakeupEvent(int startidx,
						  unsigned int pid,
						  int *index) const;
	const TraceEvent *findFilteredEvent(int index, int *filterIndex);
	__always_inline unsigned int getMaxCPU() const;
	__always_inline unsigned int getNrCPUs() const;
	__always_inline double getStartTime() const;
	__always_inline double getEndTime() const;
	__always_inline int getMinIdleState() const;
	__always_inline int getMaxIdleState() const;
	__always_inline CPUTask *findCPUTask(unsigned int pid,
					     unsigned int cpu);
	__always_inline QColor getTaskColor(unsigned int pid) const;
	__always_inline tracetype_t getTraceType() const;
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
	QMap<unsigned int, TaskHandle> taskMap;
	CpuFreq *cpuFreq;
	CpuIdle *cpuIdle;
	QList<Migration> migrations;
	QList<MigrationArrow*> migrationArrows;
	void createPidFilter(QMap<unsigned int, unsigned int> &map,
			     bool orlogic, bool inclusive);
	void createEventFilter(QMap<event_t, event_t> &map, bool orlogic);
	void disableFilter(FilterState::filter_t filter);
	void addPidToFilter(unsigned int pid);
	void removePidFromFilter(unsigned int pid);
	void disableAllFilters();
	bool isFiltered();
	bool filterActive(FilterState::filter_t filter);
private:
	TraceParser *parser;
	void prepareDataStructures();
	void resetProperties();
	void threadProcess();
	int binarySearch(double time, int start, int end) const;
	int binarySearchFiltered(double time, int start, int end) const;
	void colorizeTasks();
	int findIndexBefore(double time) const;
	int findFilteredIndexBefore(double time) const;
	__always_inline unsigned int
		generic_sched_switch_newpid(const TraceEvent &event) const;
	__always_inline unsigned int
		generic_sched_wakeup_pid(const TraceEvent &event) const;
	__always_inline double estimateWakeUpNew(const CPU *eventCPU,
						 double newTime,
						 double startTime,
						 bool &valid) const;
	__always_inline double estimateWakeUp(const Task *task,
					      double newTime,
					      bool &valid) const;
	void handleWrongTaskOnCPU(TraceEvent &event, unsigned int cpu,
				  CPU *eventCPU, unsigned int oldpid,
				  double oldtime);
	__always_inline void __processSwitchEvent(tracetype_t ttype,
						  TraceEvent &event);
	__always_inline void __processWakeupEvent(tracetype_t ttype,
						  TraceEvent &event);
	__always_inline void __processCPUfreqEvent(tracetype_t ttype,
						   TraceEvent &event);
	__always_inline void __processCPUidleEvent(tracetype_t ttype,
						   TraceEvent &event);
	__always_inline void __processMigrateEvent(tracetype_t ttype,
						   TraceEvent &event);
	__always_inline void __processForkEvent(tracetype_t ttype,
						TraceEvent &event);
	__always_inline void __processExitEvent(tracetype_t ttype,
						TraceEvent &event);
	void addCpuFreqWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuIdleWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuSchedWork(unsigned int cpu,
			     QList<AbstractWorkItem*> &list);
	void scaleMigration();
	void processSchedAddTail();
	void processFreqAddTail();
	__always_inline void __processGeneric(tracetype_t ttype);
	__always_inline void updateMaxCPU(unsigned int cpu);
	__always_inline void updateMaxFreq(unsigned int freq);
	__always_inline void updateMinFreq(unsigned int freq);
	__always_inline void updateMaxIdleState(int state);
	__always_inline void updateMinIdleState(int state);
	void processFtrace();
	void processPerf();
	void processAllFilters();
	__always_inline
		bool __processPidFilter(const TraceEvent &event,
					QMap<unsigned int, unsigned int> &map,
					bool inclusive);
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
	unsigned int maxCPU;
	unsigned int nrCPUs;
	double endTime;
	double startTime;
	unsigned int maxFreq;
	unsigned int minFreq;
	int maxIdleState;
	int minIdleState;
	CPU *CPUs;
	StringPool *taskNamePool;
	QCustomPlot *customPlot;
	FilterState filterState;
	FilterState OR_filterState;
	QMap<unsigned int, unsigned int> filterPidMap;
	QMap<unsigned int, unsigned int> OR_filterPidMap;
	QMap<event_t, event_t> filterEventMap;
	QMap<event_t, event_t> OR_filterEventMap;
	bool pidFilterInclusive;
	bool OR_pidFilterInclusive;
};

__always_inline double TraceAnalyzer::estimateWakeUpNew(const CPU *eventCPU,
							double newTime,
							double startTime,
							bool &valid) const
{
	double delay;

	if (!eventCPU->hasBeenScheduled)
		goto regular;

	if (eventCPU->lastEnterIdle < eventCPU->lastExitIdle) {
		valid = false;
		return 0;
	}
regular:
	valid = true;
	delay = newTime - startTime;
	return delay;
}

__always_inline double TraceAnalyzer::estimateWakeUp(const Task *task,
						     double newTime,
						     bool &valid) const
{
	double delay;

	/* Is this reasonable ? */
	if (task->lastWakeUP < task->lastSleepEntry) {
		valid = false;
		return 0;
	}

	delay = newTime - task->lastWakeUP;
	valid = true;
	return delay;
}

__always_inline unsigned int
TraceAnalyzer::generic_sched_switch_newpid(const TraceEvent &event) const
{
	if (!tracetype_is_valid(getTraceType()))
		return UINT_MAX;
	return sched_switch_newpid(getTraceType(), event);
}

__always_inline unsigned int
TraceAnalyzer::generic_sched_wakeup_pid(const TraceEvent &event) const
{
	if (!tracetype_is_valid(getTraceType()))
		return UINT_MAX;
	return sched_wakeup_pid(getTraceType(), event);
}

__always_inline unsigned int TraceAnalyzer::getMaxCPU() const
{
	return maxCPU;
}

__always_inline unsigned int TraceAnalyzer::getNrCPUs() const
{
	return nrCPUs;
}

__always_inline double  TraceAnalyzer::getStartTime() const
{
	return startTime;
}

__always_inline double TraceAnalyzer::getEndTime() const
{
	return endTime;
}

__always_inline int TraceAnalyzer::getMinIdleState() const
{
	return minIdleState;
}

__always_inline int TraceAnalyzer::getMaxIdleState() const
{
	return maxIdleState;
}

__always_inline QColor TraceAnalyzer::getTaskColor(unsigned int pid) const
{
	TColor taskColor = colorMap.value(pid, black);
	return taskColor.toQColor();
}

__always_inline CPUTask *TraceAnalyzer::findCPUTask(unsigned int pid,
						    unsigned int cpu)
{
	if (cpuTaskMaps[cpu].contains(pid))
		return &cpuTaskMaps[cpu][pid];
	else
		return nullptr;
}

__always_inline tracetype_t TraceAnalyzer::getTraceType() const
{
	return parser->traceType;
}

__always_inline Task *TraceAnalyzer::findTask(unsigned int pid)
{
	DEFINE_TASKMAP_ITERATOR(iter) = taskMap.find(pid);
	if (iter == taskMap.end())
		return nullptr;
	else
		return iter.value().task;
}

__always_inline void TraceAnalyzer::__processMigrateEvent(tracetype_t ttype,
							  TraceEvent &event)
{
	Migration m;
	unsigned int oldcpu = sched_migrate_origCPU(ttype, event);
	unsigned int newcpu = sched_migrate_destCPU(ttype, event);

	if (!isValidCPU(oldcpu) || !isValidCPU(newcpu))
		return;

	updateMaxCPU(oldcpu);
	updateMaxCPU(newcpu);

	m.pid = sched_migrate_pid(ttype, event);
	m.oldcpu = oldcpu;
	m.newcpu = newcpu;
	m.time = event.time;
	migrations.append(m);
}

__always_inline void TraceAnalyzer::__processForkEvent(tracetype_t ttype,
						       TraceEvent &event)
{
	Migration m;
	char *childname;

	m.pid = sched_process_fork_childpid(ttype, event);
	m.oldcpu = -1;
	m.newcpu = event.cpu;
	m.time = event.time;
	migrations.append(m);

	Task *task = &taskMap[m.pid].getTask();
	if (task->isNew) {
		/* This should be very likely for a task that just forked !*/
		task->isNew = false;
		task->pid = m.pid;
		task->schedTimev.append(event.time);
		task->schedData.append(FLOOR_HEIGHT);
		childname = sched_process_fork_childname_strdup(ttype, event,
								taskNamePool);
		task->checkName(childname, true);
	}
}

__always_inline void TraceAnalyzer::__processExitEvent(tracetype_t ttype,
						       TraceEvent &event)
{
	Migration m;

	m.pid = sched_process_exit_pid(ttype, event);
	m.oldcpu = event.cpu;
	m.newcpu = -1;
	m.time = event.time;
	migrations.append(m);

	Task *task = &taskMap[m.pid].getTask();
	if (task->isNew)
		task->pid = m.pid;
	task->exitStatus = STATUS_EXITCALLED;
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
	bool delayOK;
	CPU *eventCPU = &CPUs[cpu];
	taskstate_t state;
	char *name;
	bool runnable;
	bool preempted;

	if (!isValidCPU(cpu))
		return;

	/* This is done to update the names of existing tasks */
	if (event.pid != 0) {
		task = &taskMap[event.pid].getTask();
		task->checkName(event.taskName->ptr);
		if (task->isNew)
			task->pid = event.pid;
	}

	if (eventCPU->pidOnCPU != oldpid && eventCPU->hasBeenScheduled)
		handleWrongTaskOnCPU(event, cpu, eventCPU, oldpid, oldtime);

	if (oldpid == 0) {
		eventCPU->lastExitIdle = oldtime;
		goto skip; /* We don't care about the idle task */
	}

	/* Handle the outgoing task */
	cpuTask = &cpuTaskMaps[cpu][oldpid];
	task = &taskMap[oldpid].getTask();
	state = sched_switch_state(ttype, event);

	/* First handle the global task */
	if (task->isNew) {
		/* true means task is newly constructed above */
		task->pid = oldpid;
		task->isNew = false;
		name = sched_switch_oldname_strdup(ttype, event, taskNamePool);
		task->checkName(name);

		/* Apparently this task was running when we started tracing */
		task->schedTimev.append(startTime);
		task->schedData.append(SCHED_HEIGHT);

		task->schedTimev.append(oldtime);
		task->schedData.append(FLOOR_HEIGHT);
	}
	if (task->exitStatus == STATUS_EXITCALLED)
		task->exitStatus = STATUS_FINAL;
	task->schedTimev.append(oldtime);
	task->schedData.append(FLOOR_HEIGHT);

	runnable = task_state_is_runnable(state);

	if (runnable) {
		preempted = task_state_is_flag_set(state, TASK_FLAG_PREEMPT);
		if (preempted) {
			task->preemptedTimev.append(oldtime);
			task->preemptedData.append(FLOOR_HEIGHT);
		} else {
			task->runningTimev.append(oldtime);
			task->runningData.append(FLOOR_HEIGHT);
		}
		task->lastWakeUP = oldtime;
	} else {
		task->lastSleepEntry = oldtime;
	}

	/* ... then handle the per CPU task */
	if (cpuTask->isNew) {
		/* true means task is newly constructed above */
		cpuTask->pid = oldpid;
		cpuTask->isNew = false;

		/* Apparently this task was on CPU when we started tracing */
		cpuTask->schedTimev.append(startTime);
		cpuTask->schedData.append(SCHED_HEIGHT);
	}
	cpuTask->schedTimev.append(oldtime);
	cpuTask->schedData.append(FLOOR_HEIGHT);
	if (runnable) {
		if (preempted) {
			cpuTask->preemptedTimev.append(oldtime);
			cpuTask->preemptedData.append(FLOOR_HEIGHT);
		} else {
			cpuTask->runningTimev.append(oldtime);
			cpuTask->runningData.append(FLOOR_HEIGHT);
		}
	}

skip:
	if (newpid == 0) {
		eventCPU->lastEnterIdle = newtime;
		goto out; /* We don't care about the idle task */
	}

	/* Handle the incoming task */
	task = &taskMap[newpid].getTask();
	if (task->isNew) {
		task->pid = newpid;
		task->isNew = false;
		name = sched_switch_newname_strdup(ttype, event, taskNamePool);
		if (name != nullptr)
			task->checkName(name);
		delay = estimateWakeUpNew(eventCPU, newtime, startTime,
					  delayOK);

		task->schedTimev.append(startTime);
		task->schedData.append(FLOOR_HEIGHT);
	} else
		delay = estimateWakeUp(task, newtime, delayOK);

	if (delayOK) {
		task->wakeTimev.append(newtime);
		task->wakeDelay.append(delay);
	}

	task->schedTimev.append(newtime);
	task->schedData.append(SCHED_HEIGHT);

	cpuTask = &cpuTaskMaps[cpu][newpid];
	if (cpuTask->isNew) {
		/* true means task is newly constructed above */
		cpuTask->pid = newpid;
		cpuTask->isNew = false;

		cpuTask->schedTimev.append(startTime);
		cpuTask->schedData.append(FLOOR_HEIGHT);
	}

	if (delayOK) {
		cpuTask->wakeTimev.append(newtime);
		cpuTask->wakeDelay.append(delay);
	}

	cpuTask->schedTimev.append(newtime);
	cpuTask->schedData.append(SCHED_HEIGHT);

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
	char *name;

	/* Only interested in success */
	if (!sched_wakeup_success(ttype, event))
		return;

	time = event.time;
	pid = sched_wakeup_pid(ttype, event);

	/* Handle the woken up task */
	task = &taskMap[pid].getTask();
	task->lastWakeUP = time;
	if (task->isNew) {
		task->pid = pid;
		task->isNew = false;
		name = sched_wakeup_name_strdup(ttype, event, taskNamePool);
		if (name != nullptr)
			task->checkName(name);
		task->schedTimev.append(startTime);
		task->schedData.append(FLOOR_HEIGHT);
	}
}

__always_inline void TraceAnalyzer::__processCPUfreqEvent(tracetype_t ttype,
							  TraceEvent &event)
{
	unsigned int cpu = cpufreq_cpu(ttype, event);
	double time = event.time;
	unsigned int freq = cpufreq_freq(ttype, event);

	if (!isValidCPU(cpu))
		return;

	updateMaxCPU(cpu);
	updateMaxFreq(freq);
	updateMinFreq(freq);

	/*
	 * If this is the first cpufreq event of the CPU, we will insert it as
	 * a start frequency for that CPU
	 */
	if (cpuFreq[cpu].timev.isEmpty())
		time = startTime;
	cpuFreq[cpu].timev.append(time);
	cpuFreq[cpu].data.append((double) freq);
}

__always_inline void TraceAnalyzer::__processCPUidleEvent(tracetype_t ttype,
							  TraceEvent &event)
{
	unsigned int cpu = cpuidle_cpu(ttype, event);
	double time = event.time;
	unsigned int state = cpuidle_state(ttype, event) + 1;

	if (!isValidCPU(cpu))
		return;

	updateMaxCPU(cpu);
	updateMaxIdleState(state);
	updateMinIdleState(state);

	cpuIdle[cpu].timev.append(time);
	cpuIdle[cpu].data.append((double) state);
}

__always_inline void TraceAnalyzer::updateMaxCPU(unsigned int cpu)
{
	if (cpu > maxCPU)
		maxCPU = cpu;
}

__always_inline void TraceAnalyzer::updateMaxFreq(unsigned int freq)
{
	if (freq > maxFreq)
		maxFreq = freq;
}

__always_inline void TraceAnalyzer::updateMinFreq(unsigned int freq)
{
	if (freq < minFreq)
		minFreq = freq;
}

__always_inline void TraceAnalyzer::updateMaxIdleState(int state)
{
	if (state > maxIdleState)
		maxIdleState = state;
}

__always_inline void TraceAnalyzer::updateMinIdleState(int state)
{
	if (state < minIdleState)
		minIdleState = state;
}

__always_inline void TraceAnalyzer::__processGeneric(tracetype_t ttype)
{
	int i;
	bool eof = false;
	int indexReady = 0;
	int prevIndex = 0;

	while (!eof && indexReady <= 0)
		parser->waitForNextBatch(eof, indexReady);

	if (indexReady <= 0)
		return;

	startTime = events[0].time;

	while(true) {
		for (i = prevIndex; i < indexReady; i++) {
			TraceEvent &event = events[i];
			if (!isValidCPU(event.cpu))
				continue;
			updateMaxCPU(event.cpu);
			switch (event.type) {
			case CPU_FREQUENCY:
				if (cpufreq_args_ok(ttype, event))
					__processCPUfreqEvent(ttype, event);
				break;
			case CPU_IDLE:
				if (cpuidle_args_ok(ttype, event))
					__processCPUidleEvent(ttype, event);
				break;
			case SCHED_MIGRATE_TASK:
				if (sched_migrate_args_ok(ttype, event))
					__processMigrateEvent(ttype, event);
				break;
			case SCHED_SWITCH:
				if (sched_switch_args_ok(ttype, event))
					__processSwitchEvent(ttype, event);
				break;
			case SCHED_WAKEUP:
			case SCHED_WAKEUP_NEW:
				if (sched_wakeup_args_ok(ttype, event))
					__processWakeupEvent(ttype, event);
				break;
			case SCHED_PROCESS_FORK:
				if (sched_process_fork_args_ok(ttype, event))
					__processForkEvent(ttype, event);
				break;
			case SCHED_PROCESS_EXIT:
				if (sched_process_exit_args_ok(ttype, event))
					__processExitEvent(ttype, event);
				break;
			default:
				break;
			}
		}
		if (eof)
			break;
		prevIndex = indexReady;
		parser->waitForNextBatch(eof, indexReady);
	}
	endTime = events.last().time;
	nrCPUs = maxCPU + 1;
}

__always_inline
bool TraceAnalyzer::__processPidFilter(const TraceEvent &event,
				       QMap<unsigned int, unsigned int> &map,
				       bool inclusive)
{
	DEFINE_FILTER_PIDMAP_ITERATOR(iter);
	iter = map.find(event.pid);
	if (iter == map.end()) {
		tracetype_t ttype = getTraceType();
		unsigned int pid = UINT_MAX;
		if (!inclusive)
			return true;
		switch (event.type) {
		case SCHED_WAKEUP:
		case SCHED_WAKEUP_NEW:
			if (!sched_wakeup_args_ok(ttype, event))
				return true;
			pid = sched_wakeup_pid(ttype, event);
			break;
		case SCHED_PROCESS_FORK:
			if (!sched_process_fork_args_ok(ttype, event))
				return true;
			pid = sched_process_fork_childpid(ttype, event);
			break;
		case SCHED_SWITCH:
			if (!sched_switch_args_ok(ttype, event))
				return true;
			pid = sched_switch_newpid(ttype, event);
			if (pid == 0)
				return true;
			break;
		default:
			return true;
		}
		iter = filterPidMap.find(pid);
		if (iter == filterPidMap.end())
			return true;
	}
	return false;
}

#endif /* TRACEANALYZER_H */
