// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
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
#include <limits>

#include "vtl/avltree.h"
#include "vtl/compiler.h"
#include "vtl/time.h"
#include "vtl/tlist.h"

#include "analyzer/abstracttask.h"
#include "analyzer/cpu.h"
#include "analyzer/cpufreq.h"
#include "analyzer/cpuidle.h"
#include "analyzer/cputask.h"
#include "analyzer/filterstate.h"
#include "analyzer/latency.h"
#include "analyzer/migration.h"
#include "analyzer/regexfilter.h"
#include "analyzer/task.h"
#include "analyzer/tcolor.h"
#include "misc/traceshark.h"
#include "mm/mempool.h"
#include "parser/genericparams.h"
#include "parser/traceevent.h"
#include "parser/traceparser.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"
#include "ui/migrationarrow.h"

/*
 * Ftrace and perf only record sched_switch events, there is no record of when
 * the old task was schedule out, and when the new task was scheduled. To solve
 * this we simply assume that the old task was scheduled out FAKE_DELTA before
 * the sched_switch event and the new task was scheduled FAKE_DELTA after the
 * sched_switch event.
 */
#define FAKE_DELTA (vtl::Time(20))

/* Macros for the heights of the scheduling graph */
#define FULL_HEIGHT  ((double) 1)
#define DELAY_HEIGHT ((double) 0.6)
#define DELAY_SIZE ((double) 0.4)
/*
 * This delay (20 ms) rerpresents the "full height" of the error graphs that
 * are used to display delays in the CPU scheduling graphs
 */
#define DELAY_MAX ((double) 0.020)

class TraceFile;
class QCustomPlot;
class SettingStore;

class TraceAnalyzer
{
public:
	typedef enum : int {
		EXPORT_TYPE_ALL = 0,
		EXPORT_TYPE_CPU_CYCLES
	} exporttype_t;

	typedef enum : int {
		LATENCY_WAKEUP = 0,
		LATENCY_SCHED,
	} latencytype_t;

	typedef enum ExportFormat : int {
		EXPORT_ASCII = 0,
		EXPORT_CSV,
		EXPORT_NR,
	} exportformat_t;

	TraceAnalyzer(const SettingStore *sstore);
	~TraceAnalyzer();
	int open(const QString &fileName);
	bool isOpen() const;
	void close(int *ts_errno);
	void processTrace();
	const TraceEvent *findPreviousSchedEvent(const vtl::Time &time,
						 int pid,
						 int *index) const;
	const TraceEvent *findNextSchedSleepEvent(const vtl::Time &time,
						  int pid,
						  int *index) const;
	const TraceEvent *findPreviousWakEvent(int startidx,
					       int pid,
					       event_t wanted,
					       int *index) const;
	const TraceEvent *findWakingEvent(const TraceEvent *wakeup,
					  int *index) const;
	const TraceEvent *findFilteredEvent(int index, int *filterIndex) const;
	vtl_always_inline unsigned int getMaxCPU() const;
	vtl_always_inline unsigned int getNrCPUs() const;
	vtl_always_inline unsigned int getNrSchedLatencies() const;
	vtl_always_inline unsigned int getNrWakeLatencies() const;
	vtl_always_inline vtl::Time getStartTime() const;
	vtl_always_inline vtl::Time getEndTime() const;
	vtl_always_inline int getMinIdleState() const;
	vtl_always_inline int getMaxIdleState() const;
	vtl_always_inline unsigned int getTimePrecision() const;
	vtl_always_inline CPUTask *findCPUTask(int pid,
					     unsigned int cpu);
	vtl_always_inline QColor getTaskColor(int pid) const;
	vtl_always_inline tracetype_t getTraceType() const;
	void setSchedOffset(unsigned int cpu, double offset);
	void setSchedScale(unsigned int cpu, double scale);
	void setCpuIdleOffset(unsigned int cpu, double offset);
	void setCpuIdleScale(unsigned int cpu, double scale);
	void setCpuFreqOffset(unsigned int cpu, double offset);
	void setCpuFreqScale(unsigned int cpu, double scale);
	void setMigrationOffset(double offset);
	void setMigrationScale(double scale);
	bool enableMigrations();
	void doScale();
	void doStats();
	void doLimitedStats();
	void doLatencyStats();
	void setQCustomPlot(QCustomPlot *plot);
	vtl_always_inline Task *findTask(int pid);
	void createPidFilter(QMap<int, int> &map,
			     bool orlogic, bool inclusive);
	bool updatePidFilter(bool inclusive);
	void createCPUFilter(QMap<unsigned, unsigned> &map, bool orlogic);
	void createEventFilter(QMap<event_t, event_t> &map, bool orlogic);
	void createTimeFilter(const vtl::Time &low,
			      const vtl::Time &high, bool orlogic);
	int createRegexFilter(RegexFilter &regexFilter, bool orlogic);
	void disableFilter(FilterState::filter_t filter);
	void addPidToFilter(int pid);
	void removePidFromFilter(int pid);
	void disableAllFilters();
	bool isFiltered() const;
	bool filterActive(FilterState::filter_t filter) const;
	bool exportTraceFile(const char *fileName, int *ts_errno,
			     exporttype_t export_type);
	bool exportLatencies(exportformat_t format, latencytype_t type,
			     const char *fileName, int *ts_errno);
	TraceFile *getTraceFile();
	vtl::TList<TraceEvent> *events;
	vtl::TList<const TraceEvent*> filteredEvents;
	vtl::TList<Latency> schedLatencies;
	vtl::TList<Latency> wakeLatencies;
	vtl::AVLTree<int, CPUTask, vtl::AVLBALANCE_USEPOINTERS>
		*cpuTaskMaps;
	vtl::AVLTree<int, TaskHandle> taskMap;
	CpuFreq *cpuFreq;
	CpuIdle *cpuIdle;
	QList<Migration> migrations;
private:
	TraceParser *parser;
	void prepareDataStructures();
	void resetProperties();
	void threadProcess();
	int binarySearch(const vtl::Time &time, int start, int end) const;
	int binarySearchFiltered(const vtl::Time &time, int start, int end)
		const;
	void colorizeTasks();
	event_t determineCPUEvent(bool &ok);
	int findIndexBefore(const vtl::Time &time) const;
	int findIndexAfter(const vtl::Time &time) const;
	int findFilteredIndexBefore(const vtl::Time &time) const;
	vtl_always_inline int
		generic_sched_switch_newpid(const TraceEvent &event) const;
	vtl_always_inline int
		generic_sched_switch_oldpid(const TraceEvent &event) const;
	vtl_always_inline taskstate_t
		generic_sched_switch_state(const TraceEvent &event) const;
	vtl_always_inline int
		generic_sched_wakeup_pid(const TraceEvent &event) const;
	vtl_always_inline int
		generic_sched_waking_pid(const TraceEvent &event) const;
	vtl_always_inline
	vtl::Time estimateSchedDelayNew(const CPU *eventCPU,
					const vtl::Time &newTime,
					const vtl::Time &startTime,
					bool &valid) const;
	vtl_always_inline vtl::Time estimateSchedDelay(const Task *task,
						       const vtl::Time &newTime,
						       bool &valid) const;
	vtl_always_inline vtl::Time estimateWakeDelay(const Task *task,
						      const vtl::Time &newTime,
						      bool &valid) const;
	void handleWrongTaskOnCPU(const TraceEvent &event, unsigned int cpu,
				  CPU *eventCPU, int oldpid,
				  const vtl::Time &oldtime,
				  int idx);
	vtl_always_inline void processSwitchEvent(tracetype_t ttype,
						  const TraceEvent &event,
						  int idx);
	vtl_always_inline void processWakeupEvent(tracetype_t ttype,
						  const TraceEvent &event,
						  int idx);
	vtl_always_inline void processCPUfreqEvent(tracetype_t ttype,
						   const TraceEvent &event,
						   int idx);
	vtl_always_inline void processCPUidleEvent(tracetype_t ttype,
						   const TraceEvent &event,
						   int idx);
	vtl_always_inline void processMigrateEvent(tracetype_t ttype,
						   const TraceEvent &event,
						   int idx);
	vtl_always_inline void processForkEvent(tracetype_t ttype,
						const TraceEvent &event,
						int idx);
	vtl_always_inline void processExitEvent(tracetype_t ttype,
						const TraceEvent &event,
						int idx);
	void addCpuFreqWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuIdleWork(unsigned int cpu,
			    QList<AbstractWorkItem*> &list);
	void addCpuSchedWork(unsigned int cpu,
			     QList<AbstractWorkItem*> &list);
	void scaleMigration();
	void processSchedAddTail();
	void processFreqAddTail();
	unsigned int guessTimePrecision();
	vtl_always_inline void processGeneric(tracetype_t ttype);
	vtl_always_inline void updateMaxCPU(unsigned int cpu);
	vtl_always_inline void updateMaxFreq(unsigned int freq);
	vtl_always_inline void updateMinFreq(unsigned int freq);
	vtl_always_inline void updateMaxIdleState(int state);
	vtl_always_inline void updateMinIdleState(int state);
	void processFtrace();
	void processPerf();
	void processAllFilters();
	vtl_always_inline
		bool processPidFilter(const TraceEvent &event,
				      QMap<int, int> &map,
				      bool inclusive);
	vtl_always_inline bool processRegexFilter(const TraceEvent &event,
						  const RegexFilter &regex);
	int compileRegex(RegexFilter &filter);
	void freeRegex(RegexFilter &filter);
	int writePerfEvent(char *wb, int *space, const TraceEvent *eptr,
				  int *ts_errno);
	int writeLatency(char *wb, int *space, const Latency *lptr, int size,
			 const char *sep, int *ts_errno);
	WorkQueue processingQueue;
	WorkQueue scalingQueue;
	WorkQueue statsQueue;
	WorkQueue statsLimitedQueue;
	vtl::AVLTree <int, TColor> colorMap;
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
	vtl::Time endTime;
	vtl::Time startTime;
	double endTimeDbl;
	double startTimeDbl;
	int endTimeIdx;
	unsigned int maxFreq;
	unsigned int minFreq;
	int maxIdleState;
	int minIdleState;
	unsigned int timePrecision;
	CPU *CPUs;
	StringPool<> *taskNamePool;
	QCustomPlot *customPlot;
	FilterState filterState;
	FilterState OR_filterState;
	QMap<int, int> filterPidMap;
	QMap<int, int> OR_filterPidMap;
	QMap<unsigned, unsigned> filterCPUMap;
	QMap<unsigned, unsigned> OR_filterCPUMap;
	QMap<event_t, event_t> filterEventMap;
	QMap<event_t, event_t> OR_filterEventMap;
	RegexFilter filterRegex;
	RegexFilter OR_filterRegex;
	bool pidFilterInclusive;
	bool OR_pidFilterInclusive;
	vtl::Time filterTimeLow;
	vtl::Time filterTimeHigh;
	vtl::Time OR_filterTimeLow;
	vtl::Time OR_filterTimeHigh;
	static const char spaceStr[];
	static const int spaceStrLen;
	static const char * const cpuevents[];
	static const int CPUEVENTS_NR;
	const SettingStore *setstor;
};

vtl_always_inline
vtl::Time TraceAnalyzer::estimateSchedDelayNew(const CPU *eventCPU,
					       const vtl::Time &newTime,
					       const vtl::Time &startTime,
					       bool &valid) const
{
	vtl::Time delay;

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

vtl_always_inline
vtl::Time TraceAnalyzer::estimateSchedDelay(const Task *task,
					    const vtl::Time &newTime,
					    bool &valid) const
{
	vtl::Time delay;

	/* Is this reasonable ? */
	if (task->lastRunnable_status == RUN_STATUS_INVALID ||
	    task->lastRunnable < task->lastSleepEntry) {
		valid = false;
		return 0;
	}

	delay = newTime - task->lastRunnable;
	valid = true;
	return delay;
}

vtl_always_inline
vtl::Time TraceAnalyzer::estimateWakeDelay(const Task *task,
					   const vtl::Time &newTime,
					   bool &valid) const
{
	vtl::Time delay;

	/* Is this reasonable ? */
	if (task->lastRunnable_status != RUN_STATUS_WAKEUP ||
	    task->lastRunnable < task->lastSleepEntry) {
		valid = false;
		return 0;
	}

	delay = newTime - task->lastRunnable;
	valid = true;
	return delay;
}

vtl_always_inline int
TraceAnalyzer::generic_sched_switch_newpid(const TraceEvent &event) const
{
	sched_switch_handle_t handle;
	tracetype_t ttype = getTraceType();

	if (!tracetype_is_valid(ttype))
		return INT_MAX;
	if (!sched_switch_parse(ttype, event, handle))
		return INT_MAX;
	return sched_switch_handle_newpid(ttype, event, handle);;
}

vtl_always_inline int
TraceAnalyzer::generic_sched_switch_oldpid(const TraceEvent &event) const
{
	sched_switch_handle_t handle;
	tracetype_t ttype = getTraceType();

	if (!tracetype_is_valid(ttype))
		return INT_MAX;
	if (!sched_switch_parse(ttype, event, handle))
		return INT_MAX;
	return sched_switch_handle_oldpid(ttype, event, handle);;
}

vtl_always_inline taskstate_t
TraceAnalyzer::generic_sched_switch_state(const TraceEvent &event) const
{
	sched_switch_handle_t handle;
	tracetype_t ttype = getTraceType();

	if (!tracetype_is_valid(ttype))
		return 0;
	if (!sched_switch_parse(ttype, event, handle))
		return 0;
	return sched_switch_handle_state(ttype, event, handle);
}

vtl_always_inline int
TraceAnalyzer::generic_sched_wakeup_pid(const TraceEvent &event) const
{
	tracetype_t ttype = getTraceType();

	if (!tracetype_is_valid(ttype))
		return INT_MAX;
	if (!sched_wakeup_args_ok(ttype, event))
		return INT_MAX;
	return sched_wakeup_pid(ttype, event);
}

vtl_always_inline int
TraceAnalyzer::generic_sched_waking_pid(const TraceEvent &event) const
{
	tracetype_t ttype = getTraceType();

	if (!tracetype_is_valid(ttype))
		return INT_MAX;
	if (!sched_waking_args_ok(ttype, event))
		return INT_MAX;
	return sched_waking_pid(ttype, event);
}

vtl_always_inline unsigned int TraceAnalyzer::getMaxCPU() const
{
	return maxCPU;
}

vtl_always_inline unsigned int TraceAnalyzer::getNrCPUs() const
{
	return nrCPUs;
}

vtl_always_inline unsigned int TraceAnalyzer::getNrSchedLatencies() const
{
	return schedLatencies.size();
}

vtl_always_inline unsigned int TraceAnalyzer::getNrWakeLatencies() const
{
	return wakeLatencies.size();
}

vtl_always_inline vtl::Time TraceAnalyzer::getStartTime() const
{
	return startTime;
}

vtl_always_inline vtl::Time TraceAnalyzer::getEndTime() const
{
	return endTime;
}

vtl_always_inline int TraceAnalyzer::getMinIdleState() const
{
	return minIdleState;
}

vtl_always_inline int TraceAnalyzer::getMaxIdleState() const
{
	return maxIdleState;
}

vtl_always_inline unsigned int TraceAnalyzer::getTimePrecision() const
{
	return timePrecision;
}

vtl_always_inline QColor TraceAnalyzer::getTaskColor(int pid) const
{
	TColor taskColor = colorMap.value(pid, black);
	return taskColor.toQColor();
}

vtl_always_inline CPUTask *TraceAnalyzer::findCPUTask(int pid,
						    unsigned int cpu)
{
	if (cpuTaskMaps[cpu].contains(pid))
		return &cpuTaskMaps[cpu][pid];
	else
		return nullptr;
}

vtl_always_inline tracetype_t TraceAnalyzer::getTraceType() const
{
	return parser->traceType;
}

vtl_always_inline Task *TraceAnalyzer::findTask(int pid)
{
	DEFINE_TASKMAP_ITERATOR(iter) = taskMap.find(pid);
	if (iter == taskMap.end())
		return nullptr;
	else
		return iter.value().task;
}

vtl_always_inline
void TraceAnalyzer::processMigrateEvent(tracetype_t ttype,
					const TraceEvent &event,
					int /* idx */)
{
	Migration m;
	unsigned int oldcpu;
	unsigned int newcpu;

	if (!sched_migrate_args_ok(ttype, event))
		return;

	oldcpu = sched_migrate_origCPU(ttype, event);
	newcpu = sched_migrate_destCPU(ttype, event);

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

vtl_always_inline void TraceAnalyzer::processForkEvent(tracetype_t ttype,
						       const TraceEvent &event,
						       int idx)
{
	Migration m;
	const char *childname;

	if (!sched_process_fork_args_ok(ttype, event))
		return;

	m.pid = sched_process_fork_childpid(ttype, event);
	m.oldcpu = -1;
	m.newcpu = event.cpu;
	m.time = event.time;
	migrations.append(m);

	Task *task = &taskMap[m.pid].getTask();
	if (task->isNew) {
		/* This should be very likely for a task that just forked !*/
		task->isNew = false;
		task->lastRunnable_status = RUN_STATUS_INVALID;
		task->pid = m.pid;
		task->events = events;
		task->schedTimev.append(event.time.toDouble());
		task->schedData.append(FLOOR_BIT);
		task->schedEventIdx.append(idx);
		childname = sched_process_fork_childname_strdup(ttype, event,
								taskNamePool);
		task->checkName(childname, true);
	}
}

vtl_always_inline void TraceAnalyzer::processExitEvent(tracetype_t ttype,
						       const TraceEvent &event,
						       int /* idx */)
{
	Migration m;

	if (!sched_process_exit_args_ok(ttype, event))
		return;

	m.pid = sched_process_exit_pid(ttype, event);
	m.oldcpu = event.cpu;
	m.newcpu = -1;
	m.time = event.time;
	migrations.append(m);

	Task *task = &taskMap[m.pid].getTask();
	if (task->isNew) {
		task->pid = m.pid;
		task->events = events;
	}
	task->exitStatus = STATUS_EXITCALLED;
}

vtl_always_inline
void TraceAnalyzer::processSwitchEvent(tracetype_t ttype,
				       const TraceEvent &event,
				       int idx)
{
	sched_switch_handle_t handle;
	unsigned int cpu = event.cpu;
	vtl::Time oldtime = event.time - FAKE_DELTA;
	vtl::Time newtime = event.time + FAKE_DELTA;
	vtl::Time midtime = event.time;
	double oldtimeDbl, newtimeDbl;
	int oldpid;
	int newpid;
	CPUTask *cpuTask;
	Task *task;
	vtl::Time delay;
	bool delayOK;
	vtl::Time wakedelay;
	bool wakedelayOK;
	CPU *eventCPU = &CPUs[cpu];
	taskstate_t state;
	const char *name;
	bool runnable;
	bool preempted;
	bool uint;

	if (!sched_switch_parse(ttype, event, handle))
		return;

	oldpid = sched_switch_handle_oldpid(ttype, event, handle);
	newpid = sched_switch_handle_newpid(ttype, event, handle);

	if (!isValidCPU(cpu))
		return;

	/*
	 * This is done to update the names of existing tasks. Here we will
	 * accept the negative pids of ghost processes. The idea is that they
	 * get added to the taskMap, so that they can be selected by the user
	 * when filtering. I never liked negative pids but the reality is that
	 * some kernel versions have them, at least 4.14.x does use a negative
	 * pid in the final switch event when an exited process is being
	 * switched out after exit has been called.
	 */
	if (event.pid != 0) {
		task = &taskMap[event.pid].getTask();
		task->checkName(event.taskName->ptr);
		if (task->isNew) {
			task->pid = event.pid;
			task->events = events;
			task->lastRunnable_status = RUN_STATUS_INVALID;
		}
	}

	if (eventCPU->pidOnCPU != oldpid && eventCPU->hasBeenScheduled)
		handleWrongTaskOnCPU(event, cpu, eventCPU, oldpid, oldtime,
				     idx);

	/*
	 * Sometimes there are consecutive switch events with exactly the same
	 * timestamp. We guard against this by checking lastSched. If it has
	 * happened, or the timestamps are too close for the FAKE_DELTA to work,
	 * then we simply add time equal to two FAKE_DELTAs to the lastSched
	 * time and assumes that this is a more accurate time than the
	 * event.time!
	 */
	if (eventCPU->lastSched >= oldtime && eventCPU->hasBeenScheduled) {
		midtime = eventCPU->lastSched + FAKE_DELTA * 2;
		oldtime = midtime - FAKE_DELTA;
		newtime = midtime + FAKE_DELTA;
	}

	if (oldpid <= 0) {
		eventCPU->lastExitIdle = oldtime;
		/*
		 * We don't care about the idle task. Neither do we care if the
		 * pid is negative. I am not aware of any kernel version that
		 * would have a negative oldpid but let's include that case as
		 * well.
		 */
		goto skip;
	}

	oldtimeDbl = oldtime.toDouble();

	/* Handle the outgoing task */
	cpuTask = &cpuTaskMaps[cpu][oldpid];
	task = &taskMap[oldpid].getTask();
	state = sched_switch_handle_state(ttype, event, handle);

	/* First handle the global task */
	if (task->isNew) {
		/* true means task is newly constructed above */
		task->pid = oldpid;
		task->isNew = false;
		task->events = events;
		name = sched_switch_handle_oldname_strdup(ttype,
							  event,
							  taskNamePool,
							  handle);
		task->checkName(name);

		/* Apparently this task was running when we started tracing */
		task->schedTimev.append(startTimeDbl);
		task->schedData.append(SCHED_BIT);
		task->schedEventIdx.append(0);

		task->schedTimev.append(oldtimeDbl);
		task->schedData.append(FLOOR_BIT);
		task->schedEventIdx.append(idx);
	}
	if (task->exitStatus == STATUS_EXITCALLED)
		task->exitStatus = STATUS_FINAL;
	task->schedTimev.append(oldtimeDbl);
	task->schedData.append(FLOOR_BIT);
	task->schedEventIdx.append(idx);

	runnable = task_state_is_runnable(state);

	if (runnable) {
		preempted = task_state_is_flag_set(state, TASK_FLAG_PREEMPT);
		if (preempted) {
			task->preemptedTimev.append(oldtimeDbl);
		} else {
			task->runningTimev.append(oldtimeDbl);
		}
		task->lastRunnable = midtime;
		task->lastRunnable_idx = idx;
		task->lastRunnable_status = RUN_STATUS_SCHED;
	} else {
		task->lastSleepEntry = oldtime;
		uint = task_state_is_flag_set(state, TASK_FLAG_UNINTERRUPTIBLE);
		if (uint)
			task->uninterruptibleTimev.append(oldtimeDbl);
		task->lastRunnable_status = RUN_STATUS_INVALID;
	}

	/* ... then handle the per CPU task */
	if (cpuTask->isNew) {
		/* true means task is newly constructed above */
		cpuTask->pid = oldpid;
		cpuTask->isNew = false;
		cpuTask->events = events;

		/* Apparently this task was on CPU when we started tracing */
		cpuTask->schedTimev.append(startTimeDbl);
		cpuTask->schedData.append(SCHED_BIT);
		cpuTask->schedEventIdx.append(0);
	}
	cpuTask->schedTimev.append(oldtimeDbl);
	cpuTask->schedData.append(FLOOR_BIT);
	cpuTask->schedEventIdx.append(idx);
	if (runnable) {
		if (preempted) {
			cpuTask->preemptedTimev.append(oldtimeDbl);
		} else {
			cpuTask->runningTimev.append(oldtimeDbl);
		}
	} else {
		if (uint)
			cpuTask->uninterruptibleTimev.append(oldtimeDbl);
	}

skip:
	if (newpid <= 0) {
		eventCPU->lastEnterIdle = newtime;
		/*
		 * We don't care about the idle task. Neither do we care if the
		 * pid is negative. I am not aware of any kernel version that
		 * would have a negative newpid but let's include that case as
		 * well.
		 */
		goto out;
	}

	newtimeDbl = newtime.toDouble();

	/* Handle the incoming task */
	task = &taskMap[newpid].getTask();
	if (task->isNew) {
		task->pid = newpid;
		task->isNew = false;
		task->events = events;
		name = sched_switch_handle_newname_strdup(ttype,
							  event,
							  taskNamePool,
							  handle);
		if (name != nullptr)
			task->checkName(name);
		delay = estimateSchedDelayNew(eventCPU, midtime, startTime,
					      delayOK);

		task->schedTimev.append(startTimeDbl);
		task->schedData.append(FLOOR_BIT);
		task->schedEventIdx.append(0);

		/*
		 * In this case, we will not record a wakeup latency because we
		 * do not know whether or not this task has been woken up or if
		 * it was runnable when scheduled out.
		 */
		wakedelayOK = false;
	} else {
		delay = estimateSchedDelay(task, midtime, delayOK);
		wakedelay = estimateWakeDelay(task, midtime, wakedelayOK);
	}

	double delayDbl;

	if (delayOK) {
		Latency &slatency = schedLatencies.increase();

		delayDbl = delay.toDouble();
		task->delayTimev.append(newtimeDbl);
		task->delay.append(delayDbl);

		slatency.delay = delay;
		slatency.pid = newpid;
		slatency.time = midtime;
		slatency.sched_idx = idx;
		/*
		 * We trust this task->lastRunnable_idx because in this case
		 * estimateSchedDelay() or estimateSchedDelayNew() found wakeup
		 * to be OK.
		 */
		slatency.runnable_idx = task->lastRunnable_idx;
	}

	double wakedelayDbl;

	if (wakedelayOK) {
		Latency &wlatency = wakeLatencies.increase();

		wakedelayDbl = wakedelay.toDouble();
		task->wakeTimev.append(newtimeDbl);
		task->wakeDelay.append(wakedelayDbl);

		wlatency.delay = wakedelay;
		wlatency.pid = newpid;
		wlatency.time = midtime;
		wlatency.sched_idx = idx;
		/*
		 * We trust this task->lastRunnable_idx to be pointing to a
		 * valid wakeup because in this case estimateWakeDelay() above
		 * found wakeup to be OK.
		 */
		wlatency.runnable_idx = task->lastRunnable_idx;
	}

	task->schedTimev.append(newtimeDbl);
	task->schedData.append(SCHED_BIT);
	task->schedEventIdx.append(idx);

	cpuTask = &cpuTaskMaps[cpu][newpid];
	if (cpuTask->isNew) {
		/* true means task is newly constructed above */
		cpuTask->pid = newpid;
		cpuTask->isNew = false;
		cpuTask->events = events;

		cpuTask->schedTimev.append(startTimeDbl);
		cpuTask->schedData.append(FLOOR_BIT);
		cpuTask->schedEventIdx.append(idx);
	}

	if (delayOK) {
		cpuTask->delayTimev.append(newtimeDbl);
		cpuTask->delay.append(delayDbl);
	}

	if (wakedelayOK) {
		cpuTask->wakeTimev.append(newtimeDbl);
		cpuTask->wakeDelay.append(wakedelayDbl);
	}

	cpuTask->schedTimev.append(newtimeDbl);
	cpuTask->schedData.append(SCHED_BIT);
	cpuTask->schedEventIdx.append(idx);

out:
	eventCPU->hasBeenScheduled = true;
	eventCPU->pidOnCPU = newpid;
	eventCPU->lastSched = newtime;
	eventCPU->lastSchedIdx = idx;
	return;
}

vtl_always_inline
void TraceAnalyzer::processWakeupEvent(tracetype_t ttype,
				       const TraceEvent &event,
				       int idx)
{
	int pid;
	Task *task;
	vtl::Time time;
	const char *name;

	if (!sched_wakeup_args_ok(ttype, event))
		return;

	/* Only interested in success */
	if (!sched_wakeup_success(ttype, event))
		return;

	time = event.time;
	pid = sched_wakeup_pid(ttype, event);

	/* Handle the woken up task */
	task = &taskMap[pid].getTask();
	task->lastRunnable = time;
	task->lastRunnable_idx = idx;
	task->lastRunnable_status = RUN_STATUS_WAKEUP;
	if (task->isNew) {
		task->pid = pid;
		task->isNew = false;
		task->events = events;
		name = sched_wakeup_name_strdup(ttype, event, taskNamePool);
		if (name != nullptr)
			task->checkName(name);
		task->schedTimev.append(startTimeDbl);
		task->schedData.append(FLOOR_BIT);
		task->schedEventIdx.append(0);
	}
}

vtl_always_inline
void TraceAnalyzer::processCPUfreqEvent(tracetype_t ttype,
					const TraceEvent &event,
					int /* idx */)
{
	unsigned int cpu;
	unsigned int freq;
	vtl::Time time = event.time;

	if(!cpufreq_args_ok(ttype, event))
		return;

	cpu = cpufreq_cpu(ttype, event);
	freq = cpufreq_freq(ttype, event);

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
	cpuFreq[cpu].timev.append(time.toDouble());
	cpuFreq[cpu].data.append((double) freq);
}

vtl_always_inline
void TraceAnalyzer::processCPUidleEvent(tracetype_t ttype,
					const TraceEvent &event,
					int /* idx */)
{
	unsigned int cpu;
	double time;
	unsigned int state;

	if (!cpuidle_args_ok(ttype, event))
		return;

	cpu = cpuidle_cpu(ttype, event);
	time = event.time.toDouble();
	state = cpuidle_state(ttype, event) + 1;

	if (!isValidCPU(cpu))
		return;

	updateMaxCPU(cpu);
	updateMaxIdleState(state);
	updateMinIdleState(state);

	cpuIdle[cpu].timev.append(time);
	cpuIdle[cpu].data.append((double) state);
}

vtl_always_inline void TraceAnalyzer::updateMaxCPU(unsigned int cpu)
{
	if (cpu > maxCPU)
		maxCPU = cpu;
}

vtl_always_inline void TraceAnalyzer::updateMaxFreq(unsigned int freq)
{
	if (freq > maxFreq)
		maxFreq = freq;
}

vtl_always_inline void TraceAnalyzer::updateMinFreq(unsigned int freq)
{
	if (freq < minFreq)
		minFreq = freq;
}

vtl_always_inline void TraceAnalyzer::updateMaxIdleState(int state)
{
	if (state > maxIdleState)
		maxIdleState = state;
}

vtl_always_inline void TraceAnalyzer::updateMinIdleState(int state)
{
	if (state < minIdleState)
		minIdleState = state;
}

vtl_always_inline void TraceAnalyzer::processGeneric(tracetype_t ttype)
{
	int i;
	bool eof = false;
	int indexReady = 0;
	int prevIndex = 0;

	while (!eof && indexReady <= 0)
		parser->waitForNextBatch(eof, indexReady);

	if (indexReady <= 0)
		return;

	startTime = (*events)[0].time;
	AbstractTask::setStartTime(startTime);
	startTimeDbl = startTime.toDouble();

	while(true) {
		for (i = prevIndex; i < indexReady; i++) {
			TraceEvent &event = (*events)[i];
			if (!isValidCPU(event.cpu))
				continue;
			updateMaxCPU(event.cpu);
			switch (event.type) {
			case CPU_FREQUENCY:
				processCPUfreqEvent(ttype, event, i);
				break;
			case CPU_IDLE:
				processCPUidleEvent(ttype, event, i);
				break;
			case SCHED_MIGRATE_TASK:
				processMigrateEvent(ttype, event, i);
				break;
			case SCHED_SWITCH:
				processSwitchEvent(ttype, event, i);
				break;
			case SCHED_WAKEUP:
			case SCHED_WAKEUP_NEW:
				processWakeupEvent(ttype, event, i);
				break;
			case SCHED_PROCESS_FORK:
				processForkEvent(ttype, event, i);
				break;
			case SCHED_PROCESS_EXIT:
				processExitEvent(ttype, event, i);
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
	endTime = events->last().time;
	endTimeIdx = events->size() - 1;
	AbstractTask::setEndTime(endTime);
	endTimeDbl = endTime.toDouble();
	nrCPUs = maxCPU + 1;
	timePrecision = guessTimePrecision();
}

vtl_always_inline
bool TraceAnalyzer::processPidFilter(const TraceEvent &event,
				     QMap<int, int> &map,
				     bool inclusive)
{
	sched_switch_handle sw_handle;
	DEFINE_FILTER_PIDMAP_ITERATOR(iter);
	iter = map.find(event.pid);
	if (iter == map.end()) {
		tracetype_t ttype = getTraceType();
		int pid = INT_MAX;
		if (!inclusive)
			return true;
		switch (event.type) {
		case SCHED_WAKEUP:
		case SCHED_WAKEUP_NEW:
			if (!sched_wakeup_args_ok(ttype, event))
				return true;
			pid = sched_wakeup_pid(ttype, event);
			break;
		case SCHED_WAKING:
			if (!sched_waking_args_ok(ttype, event))
				return true;
			pid = sched_waking_pid(ttype, event);
			break;
		case SCHED_PROCESS_FORK:
			if (!sched_process_fork_args_ok(ttype, event))
				return true;
			pid = sched_process_fork_childpid(ttype, event);
			break;
		case SCHED_SWITCH:
			if (!sched_switch_parse(ttype, event, sw_handle))
				return true;
			pid = sched_switch_handle_newpid(ttype, event,
							 sw_handle);
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

vtl_always_inline
bool TraceAnalyzer::processRegexFilter(const TraceEvent &event,
				       const RegexFilter &regex)
{
	const QVector<Regex> &rvec = regex.regvec;
	int i, j;
	bool sum = true;
	bool value;
	int pidx = 0;
	bool pvalue = false;
	int pos;

	for (i = 0; i < rvec.size(); i++) {
		const Regex &regex = rvec[i];
		value = false;
		switch (regex.posType) {
		case Regex::POS_NONE:
			for (j = 0; j < event.argc; j++) {
				value = !regexec(&regex.regex,
						 event.argv[j]->ptr,
						 0, NULL, 0);
				if (value) {
					pidx = j;
					break;
				}
			}
			break;
		case Regex::POS_ABSOLUTE:
			if (regex.pos < 0 || regex.pos > event.argc - 1)
				value = false;
			else {
				value = !regexec(&regex.regex,
						 event.argv[regex.pos]->ptr,
						 0, NULL, 0);
				if (value) {
					pidx = regex.pos;
					break;
				}
			}
			break;
		case Regex::POS_RELATIVE:
			if (!pvalue)
				break;
			pos = pidx + regex.pos;
			if (pos < 0 || pos > event.argc - 1)
				value = false;
			else {
				value = !regexec(&regex.regex,
						 event.argv[pos]->ptr,
						 0, NULL, 0);
				if (value) {
					pidx = pos;
					break;
				}
			}
		default:
			break;
		}
		if (regex.inverted)
			value = !value;
		pvalue = value;
		switch (regex.logic) {
		case TShark::LOGIC_OR:
			sum = sum || value;
			break;
		case TShark::LOGIC_NOR:
			sum = !(sum || value);
			break;
		case TShark::LOGIC_XOR:
			sum = (sum != value);
			break;
		case TShark::LOGIC_XNOR:
			sum = !(sum != value);
			break;
		case TShark::LOGIC_AND:
			sum = sum && value;
			break;
		case TShark::LOGIC_NAND:
			sum = !(sum && value);
			break;
		default:
			Q_ASSERT(false);
			break;
		}
	}
	return sum;
}

#endif /* TRACEANALYZER_H */
