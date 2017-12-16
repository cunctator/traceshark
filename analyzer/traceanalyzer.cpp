/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <climits>
#include <cstdlib>

#include <QtGlobal>
#include <QList>
#include <QString>
#include <QTextStream>

#include "vtl/tlist.h"

#include "analyzer/cpufreq.h"
#include "analyzer/cpuidle.h"
#include "parser/genericparams.h"
#include "analyzer/traceanalyzer.h"
#include "parser/traceparser.h"
#include "misc/traceshark.h"
#include "threads/workthread.h"
#include "threads/workitem.h"
#include "threads/workqueue.h"

TraceAnalyzer::TraceAnalyzer()
	: cpuTaskMaps(nullptr), cpuFreq(nullptr), cpuIdle(nullptr),
	  black(0, 0, 0), white(255, 255, 255), migrationOffset(0),
	  migrationScale(0), maxCPU(0), nrCPUs(0), endTime(0), startTime(0),
	  maxFreq(0), minFreq(0), maxIdleState(0), minIdleState(0),
	  CPUs(nullptr), customPlot(nullptr)
{
	taskNamePool = new StringPool(16384, 256);
	parser = new TraceParser(&events);
	filterState.disableAll();
	OR_filterState.disableAll();
}

TraceAnalyzer::~TraceAnalyzer()
{
	TraceAnalyzer::close();
	delete parser;
	delete taskNamePool;
}

bool TraceAnalyzer::open(const QString &fileName)
{
	bool retval = parser->open(fileName);
	if (retval)
		prepareDataStructures();
	return retval;
}

void TraceAnalyzer::prepareDataStructures()
{
	cpuTaskMaps = new vtl::AVLTree<unsigned int, CPUTask,
				       vtl::AVLBALANCE_USEPOINTERS>
		[NR_CPUS_ALLOWED];
	cpuFreq = new CpuFreq[NR_CPUS_ALLOWED];
	cpuIdle = new CpuIdle[NR_CPUS_ALLOWED];
	CPUs = new CPU[NR_CPUS_ALLOWED];
	schedOffset.resize(0);
	schedOffset.resize(NR_CPUS_ALLOWED);
	schedScale.resize(0);
	schedScale.resize(NR_CPUS_ALLOWED);
	cpuIdleOffset.resize(0);
	cpuIdleOffset.resize(NR_CPUS_ALLOWED);
	cpuIdleScale.resize(0);
	cpuIdleScale.resize(NR_CPUS_ALLOWED);
	cpuFreqOffset.resize(0);
	cpuFreqOffset.resize(NR_CPUS_ALLOWED);
	cpuFreqScale.resize(0);
	cpuFreqScale.resize(NR_CPUS_ALLOWED);
}

bool TraceAnalyzer::isOpen()
{
	return parser->isOpen();
}

void TraceAnalyzer::close()
{
	if (cpuTaskMaps != nullptr) {
		delete[] cpuTaskMaps;
		cpuTaskMaps = nullptr;
	}
	if (cpuFreq != nullptr) {
		delete[] cpuFreq;
		cpuFreq = nullptr;
	}
	if (cpuIdle != nullptr) {
		delete[] cpuIdle;
		cpuIdle = nullptr;
	}
	if (CPUs != nullptr) {
		delete[] CPUs;
		CPUs = nullptr;
	}

	DEFINE_TASKMAP_ITERATOR(iter) = taskMap.begin();
	while (iter != taskMap.end()) {
		Task *task = iter.value().task;
		delete task;
		iter++;
	}

	taskMap.clear();
	events.clear();
	disableAllFilters();
	migrations.clear();
	migrationArrows.clear();
	colorMap.clear();
	parser->close();
	taskNamePool->clear();
}

void TraceAnalyzer::resetProperties()
{
	maxCPU = 0;
	startTime = 0;
	endTime = 0;
	minFreq = UINT_MAX;
	maxFreq = 0;
	minIdleState = INT_MAX;
	maxIdleState = INT_MIN;
}

void TraceAnalyzer::processTrace()
{
	resetProperties();
	/*
	 * We do the processing from the main thread, since otherwise
	 * we would have to wait for it
	 */
	threadProcess();
	colorizeTasks();
}

void TraceAnalyzer::threadProcess()
{
	parser->waitForTraceType();
	switch (getTraceType()) {
	case TRACE_TYPE_FTRACE:
		processFtrace();
		break;
	case TRACE_TYPE_PERF:
		processPerf();
		break;
	default:
		break;
	}
	processSchedAddTail();
	processFreqAddTail();
}

void TraceAnalyzer::processSchedAddTail()
{
	/* Add the "tail" to all tasks, i.e. extend them until endTime */
	unsigned int cpu;
	for (cpu = 0; cpu < getNrCPUs(); cpu++) {
		DEFINE_CPUTASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
		while (iter != cpuTaskMaps[cpu].end()) {
			CPUTask &task = iter.value();
			double d;
			double lastTime;
			int lastIndex = task.schedTimev.size() - 1;
			iter++;
			/*
			 * I think that this should not happen, there should
			 * always be some data in a CPUTask at this point but
			 * check anyway
			 */
			if (lastIndex < 0)
				continue;
			/* Check if tail is necessary */
			lastTime = task.schedTimev[lastIndex];
			if (lastTime >= getEndTime())
				continue;
			d = task.schedData[task.schedData.size() - 1];
			task.schedTimev.append(getEndTime());
			task.schedData.append(d);
		}
	}

	DEFINE_TASKMAP_ITERATOR(iter) = taskMap.begin();
	while (iter != taskMap.end()) {
		Task &task = *iter.value().task;
		double d;
		double lastTime;
		int s = task.schedTimev.size();
		iter++;
		task.generateDisplayName();
		if (s <= 0)
			continue;
		lastTime = task.schedTimev[s - 1];
		if (lastTime >= getEndTime() || task.exitStatus == STATUS_FINAL)
			continue;
		d = task.schedData[task.schedData.size() - 1];
		task.schedTimev.append(getEndTime());
		task.schedData.append(d);
	}
}

void TraceAnalyzer::processFreqAddTail()
{
	unsigned int cpu;

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		if (!cpuFreq[cpu].data.isEmpty()) {
			double freq = cpuFreq[cpu].data.last();
			cpuFreq[cpu].data.append(freq);
			cpuFreq[cpu].timev.append(endTime);
		}
	}
}

/*
 * This function is supposed to be called seldom, thus it's ok to not have it
 * as optimized as the other functions, e.g. in terms of inlining
 */
void TraceAnalyzer::handleWrongTaskOnCPU(TraceEvent &/*event*/,
					 unsigned int cpu,
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
		Q_ASSERT(!cpuTask->schedTimev.isEmpty());
		prevtime = cpuTask->schedTimev.last();
		faketime = prevtime + FAKE_DELTA;
		cpuTask->schedTimev.append(faketime);
		cpuTask->schedData.append(FLOOR_HEIGHT);
		task = findTask(epid);
		Q_ASSERT(task != nullptr);
		task->lastSleepEntry = faketime;
		task->schedTimev.append(faketime);
		task->schedData.append(FLOOR_HEIGHT);
	}

	if (oldpid != 0) {
		cpuTask = &cpuTaskMaps[cpu][oldpid];
		if (cpuTask->isNew) {
			cpuTask->pid = oldpid;
		}
		cpuTask->isNew = false;
		faketime = oldtime - FAKE_DELTA;
		cpuTask->schedTimev.append(faketime);
		cpuTask->schedData.append(SCHED_HEIGHT);

		task = &taskMap[oldpid].getTask();
		if (task->isNew) {
			task->pid = oldpid;
		}
		task->isNew = false;
		task->schedTimev.append(faketime);
		task->schedData.append(SCHED_HEIGHT);
	}
}

void TraceAnalyzer::colorizeTasks()
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
	struct drand48_data rdata;
	unsigned int i, j;
	QList<TColor> colorList;
	const TColor black(0, 0, 0);
	const TColor white(255, 255, 255);
	TColor gray;
	TColor tmp;
	long int rnd = 0;
	QTextStream qout(stdout);

	srand48_r(290876, &rdata);

	for (cpu = 0; cpu <= getMaxCPU(); cpu++) {
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
	s = 0.95 * cbrt( (1 / nf) * (255 * 255 * 255 ));
	s = TSMIN(s, 128.0);
	s = TSMAX(s, 1.0);
retry:
	step = (unsigned int) s;
	for (red = 0; red < 256; red += step) {
		for (green = 0; green < 256; green += step)  {
			for (blue = 0; blue < 256; blue += step) {
				TColor color(red, green, blue);
				if (color.SqDistance(black) < 10000)
					continue;
				if (color.SqDistance(white) < 12000)
					continue;
				gray = TColor(red, red, red);
				if (color.SqDistance(gray) < 2500)
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
			qout << "retrying colors...\n";
			goto retry;
		}
	}

	/*
	 * Randomize the order by swapping every element with a random
	 * element
	 */
	for (i = 0; i < ncolor; i++) {
		lrand48_r(&rdata, &rnd);
		j = rnd % ncolor;
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


int TraceAnalyzer::binarySearch(double time, int start, int end) const
{
	int pivot = (end + start) / 2;
	if (pivot == start)
		return pivot;
	if (time < events.at(pivot).time)
		return binarySearch(time, start, pivot);
	else
		return binarySearch(time, pivot, end);
}

int TraceAnalyzer::binarySearchFiltered(double time, int start, int end) const
{
	int pivot = (end + start) / 2;
	if (pivot == start)
		return pivot;
	if (time < filteredEvents.at(pivot)->time)
		return binarySearchFiltered(time, start, pivot);
	else
		return binarySearchFiltered(time, pivot, end);
}

int TraceAnalyzer::findIndexBefore(double time) const
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

	while (c > 0 && events.at(c).time >= time)
		c--;
	return c;
}

int TraceAnalyzer::findFilteredIndexBefore(double time) const
{
	if (filteredEvents.size() < 1)
		return -1;

	int end = filteredEvents.size() - 1;

	/* Basic sanity checks */
	if (time > filteredEvents.at(end)->time)
		return end;
	if (time < filteredEvents.at(0)->time)
		return 0;

	int c = binarySearchFiltered(time, 0, end);

	while (c > 0 && filteredEvents.at(c)->time >= time)
		c--;
	return c;
}

const TraceEvent *TraceAnalyzer::findPreviousSchedEvent(double time,
							unsigned int pid,
							int *index) const
{
	int start = findIndexBefore(time);
	int i;

	if (start < 0)
		return nullptr;

	for (i = start; i >= 0; i--) {
		const TraceEvent &event = events[i];
		if (event.type == SCHED_SWITCH  &&
		    generic_sched_switch_newpid(event) == pid) {
			if (index != nullptr)
				*index = i;
			return &event;
		}
	}
	return nullptr;
}

const TraceEvent *TraceAnalyzer::findFilteredEvent(int index,
						   int *filterIndex)
{
	TraceEvent *eptr = &events[index];
	double time = eptr->time;
	int s = filteredEvents.size();
	int i;
	int start = findFilteredIndexBefore(time);

	if (start < 0)
		return nullptr;

	for (i = start; i < s; i++) {
		const TraceEvent *cptr = filteredEvents[i];
		if (cptr == eptr) {
			*filterIndex = i;
			return cptr;
		}
		if (cptr->time > time)
			break;
	}
	return nullptr;
}

const TraceEvent *TraceAnalyzer::findPreviousWakeupEvent(int startidx,
							 unsigned int pid,
							 int *index) const
{
	int i;

	if (startidx < 0 || startidx >= (int) events.size())
		return nullptr;

	for (i = startidx; i >= 0; i--) {
		const TraceEvent &event = events[i];
		if ((event.type == SCHED_WAKEUP ||
		     event.type == SCHED_WAKEUP_NEW) &&
				generic_sched_wakeup_pid(event) == pid) {
			if (index != nullptr)
				*index = i;
			return &event;
		}
	}
	return nullptr;
}



void TraceAnalyzer::setSchedOffset(unsigned int cpu, double offset)
{
	schedOffset[cpu] = offset;
}

void TraceAnalyzer::setSchedScale(unsigned int cpu, double scale)
{
	schedScale[cpu] = scale;
}

void TraceAnalyzer::setCpuIdleOffset(unsigned int cpu, double offset)
{
	cpuIdleOffset[cpu] = offset;
}

void TraceAnalyzer::setCpuIdleScale(unsigned int cpu, double scale)
{
	cpuIdleScale[cpu] = scale / maxIdleState;
}

void TraceAnalyzer::setCpuFreqOffset(unsigned int cpu, double offset)
{
	cpuFreqOffset[cpu] = offset;
}

void TraceAnalyzer::setCpuFreqScale(unsigned int cpu, double scale)
{
	cpuFreqScale[cpu] = scale / maxFreq;
}

void TraceAnalyzer::setMigrationOffset(double offset)
{
	migrationOffset = offset;
}

void TraceAnalyzer::setMigrationScale(double scale)
{
	migrationScale = scale;
}

void TraceAnalyzer::setQCustomPlot(QCustomPlot *plot)
{
	customPlot = plot;
}

void TraceAnalyzer::addCpuFreqWork(unsigned int cpu,
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

void TraceAnalyzer::addCpuIdleWork(unsigned int cpu,
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

void TraceAnalyzer::addCpuSchedWork(unsigned int cpu,
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
		taskItem = new WorkItem<CPUTask>(&task,
						 &CPUTask::doScalePreempted);
		list.append(taskItem);
		iter++;
	}
}

/*
 * This function must be called from the application mainthread because it
 * creates objects that are children customPlot, which is created by the
 * mainthread
 */
void TraceAnalyzer::scaleMigration()
{
	MigrationArrow *a;
	QList<Migration>::iterator iter;
	double unit = migrationScale / getNrCPUs();
	for (iter = migrations.begin(); iter != migrations.end(); iter++) {
		Migration &m = *iter;
		double s = migrationOffset + (m.oldcpu + 1) * unit;
		double e = migrationOffset + (m.newcpu + 1) * unit;
		QColor color = getTaskColor(m.pid);
		a = new MigrationArrow(s, e, m.time, color, customPlot);
		migrationArrows.append(a);
	}
}

void TraceAnalyzer::doScale()
{
	QList<AbstractWorkItem*> workList;
	unsigned int cpu;
	int i, s;

	for (cpu = 0; cpu <= getMaxCPU(); cpu++) {
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
	/* Migration scaling is done from the mainthread */
	scaleMigration();
	scalingQueue.wait();
	for (i = 0; i < s; i++)
		delete workList[i];
}

void TraceAnalyzer::processFtrace()
{
	__processGeneric(TRACE_TYPE_FTRACE);
}

void TraceAnalyzer::processPerf()
{
	__processGeneric(TRACE_TYPE_PERF);
}

void TraceAnalyzer::processAllFilters()
{
	unsigned int i;
	unsigned int s = events.size();
	const TraceEvent *eptr;

	filteredEvents.clear();

	for (i = 0; i < s; i++) {
		const TraceEvent &event = events[i];
		eptr = &event;
		/* OR filters */
		if (OR_filterState.isEnabled(FilterState::FILTER_PID) &&
		    !__processPidFilter(event, OR_filterPidMap,
					OR_pidFilterInclusive)) {
			filteredEvents.append(eptr);
			continue;
		}
		if (OR_filterState.isEnabled(FilterState::FILTER_EVENT)) {
			DEFINE_FILTER_EVENTMAP_ITERATOR(iter);
			iter = OR_filterEventMap.find(event.type);
			if (iter != OR_filterEventMap.end()) {
				filteredEvents.append(eptr);
				continue;
			}
		}
		if (OR_filterState.isEnabled(FilterState::FILTER_TIME)) {
			if (event.time >= OR_filterTimeLow &&
			    event.time <= OR_filterTimeHigh) {
				filteredEvents.append(eptr);
				continue;
			}
		}
		/* AND filters */
		if (filterState.isEnabled(FilterState::FILTER_PID) &&
		    __processPidFilter(event, filterPidMap,
				       pidFilterInclusive)) {
			continue;
		}
		if (filterState.isEnabled(FilterState::FILTER_EVENT)) {
			DEFINE_FILTER_EVENTMAP_ITERATOR(iter);
			iter = filterEventMap.find(event.type);
			if (iter == filterEventMap.end())
				continue;
		}
		if (filterState.isEnabled(FilterState::FILTER_TIME)) {
			if (event.time < filterTimeLow ||
			    event.time > filterTimeHigh)
				continue;
		}
		if (filterState.isEnabled(FilterState::FILTER_CPU)) {
			/* Add CPU nr filtering here */
		}
		if (filterState.isEnabled(FilterState::FILTER_ARG)) {
			/* Add argument filtering here */
		}
		filteredEvents.append(eptr);
	}
}

void TraceAnalyzer::createPidFilter(QMap<unsigned int, unsigned int> &map,
				    bool orlogic, bool inclusive)
{
	/*
	 * An empty map is interpreted to mean that no filtering is desired,
	 * a map of the same size as the taskMap should mean that the user
	 * wants to filter on all pids, which is the same as no filtering
	 */
	if (map.isEmpty() || map.size() == taskMap.size()) {
		if (filterState.isEnabled(FilterState::FILTER_PID))
			disableFilter(FilterState::FILTER_PID);
		return;
	}

	if (orlogic) {
		OR_pidFilterInclusive = inclusive;
		OR_filterPidMap = map;
		OR_filterState.enable(FilterState::FILTER_PID);
	} else {
		pidFilterInclusive = inclusive;
		filterPidMap = map;
		filterState.enable(FilterState::FILTER_PID);
	}
	if (filterState.isEnabled())
		processAllFilters();
}

void TraceAnalyzer::createEventFilter(QMap<event_t, event_t> &map,
				      bool orlogic)
{
	/*
	 * An empty map is interpreted to mean that no filtering is desired,
	 * a map of the same size as the taskMap should mean that the user
	 * wants to filter on all pids, which is the same as no filtering
	 */
	if (map.isEmpty() || map.size() == TraceEvent::getNrEvents()) {
		if (filterState.isEnabled(FilterState::FILTER_EVENT))
			disableFilter(FilterState::FILTER_EVENT);
		return;
	}

	if (orlogic) {
		OR_filterEventMap = map;
		OR_filterState.enable(FilterState::FILTER_EVENT);
	} else {
		filterEventMap = map;
		filterState.enable(FilterState::FILTER_EVENT);
	}
	/* No need to process filters if we only have OR-filters */
	if (filterState.isEnabled())
		processAllFilters();
}

void TraceAnalyzer::createTimeFilter(double low, double high, bool
				     orlogic)
{
	double start = getStartTime();
	double end = getEndTime();

	if (low < start && high > end)
		return;

	if (orlogic) {
		OR_filterTimeLow = low;
		OR_filterTimeHigh = high;
		OR_filterState.enable(FilterState::FILTER_TIME);
	} else {
		filterTimeLow = low;
		filterTimeHigh = high;
		filterState.enable(FilterState::FILTER_TIME);
	}
	/* No need to process filters if we only have OR-filters */
	if (filterState.isEnabled())
		processAllFilters();
}

void TraceAnalyzer::disableFilter(FilterState::filter_t filter)
{
	filterState.disable(filter);
	OR_filterState.disable(filter);
	switch (filter) {
	case FilterState::FILTER_PID:
		filterPidMap.clear();
		OR_filterPidMap.clear();
		break;
	case FilterState::FILTER_EVENT:
		filterEventMap.clear();
		break;
	case FilterState::FILTER_TIME:
		/* We need to do nothing */
		break;
	case FilterState::FILTER_CPU:
		break;
	case FilterState::FILTER_ARG:
		break;
	default:
		break;
	}
	if (filterState.isEnabled())
		processAllFilters();
	else
		filteredEvents.clear();
}

void TraceAnalyzer::addPidToFilter(unsigned int pid) {
	DEFINE_FILTER_PIDMAP_ITERATOR(iter);

	iter = filterPidMap.find(pid);
	if (iter != filterPidMap.end()) {
		if (filterState.isEnabled(FilterState::FILTER_PID))
			return;
		goto epilogue;
	}
	filterPidMap[pid] = pid;

epilogue:
	/* Disable filter if we are filtering on all pids */
	if (filterPidMap.size() == taskMap.size()) {
		disableFilter(FilterState::FILTER_PID);
		return;
	}

	filterState.enable(FilterState::FILTER_PID);
	processAllFilters();
}

void TraceAnalyzer::removePidFromFilter(unsigned int pid) {
	DEFINE_FILTER_PIDMAP_ITERATOR(iter);

	if (!filterState.isEnabled(FilterState::FILTER_PID))
		return;

	iter = filterPidMap.find(pid);
	if (iter == filterPidMap.end()) {
		return;
	}

	iter = filterPidMap.erase(iter);
	if (filterPidMap.isEmpty()) {
		disableFilter(FilterState::FILTER_PID);
		return;
	}
	processAllFilters();
}

void TraceAnalyzer::disableAllFilters()
{
	filterState.disableAll();
	OR_filterState.disableAll();

	filterPidMap.clear();
	OR_filterPidMap.clear();

	filterEventMap.clear();
	OR_filterEventMap.clear();

	filteredEvents.clear();
}

bool TraceAnalyzer::isFiltered()
{
	/*
	 * No need to consider us filtered if we only have OR-filters, so we
	 * only check the normal (AND) filters
	 */
	return filterState.isEnabled();
}

bool TraceAnalyzer::filterActive(FilterState::filter_t filter)
{
	return filterState.isEnabled(filter) ||
		OR_filterState.isEnabled(filter);
}

double TraceAnalyzer::getStartTime()
{
	double rval = 0;

	if (events.size() > 0)
		rval = events[0].time;
	return rval;
}

double TraceAnalyzer::getEndTime()
{
	double rval = 0;
	unsigned int s = events.size();

	if (s > 0)
		rval = events[s - 1].time;
	return rval;
}
