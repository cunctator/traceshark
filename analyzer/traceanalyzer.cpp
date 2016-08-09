/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include <cstdlib>
#include <QtGlobal>
#include <QList>
#include <QString>
#include "analyzer/cpufreq.h"
#include "analyzer/cpuidle.h"
#include "parser/genericparams.h"
#include "analyzer/traceanalyzer.h"
#include "parser/traceparser.h"
#include "misc/traceshark.h"
#include "threads/workthread.h"
#include "threads/workitem.h"
#include "threads/workqueue.h"
#include "misc/tlist.h"

TraceAnalyzer::TraceAnalyzer()
	: cpuTaskMaps(nullptr), cpuFreq(nullptr), cpuIdle(nullptr),
	  black(0, 0, 0), white(255, 255, 255),
	  CPUs(nullptr)
{
	taskNamePool = new MemPool(16384, sizeof(char));
	parser = new TraceParser(&events);
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
	cpuTaskMaps = new QMap<unsigned int, CPUTask>[NR_CPUS_ALLOWED];
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
	taskMap.clear();
	events.clear();
	migrations.clear();
	migrationArrows.clear();
	colorMap.clear();
	parser->close();
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
	/* We do the processing from the main thread, since otherwise
	 * we would have to wait for it */
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
			/* I think that this should not happen, there should
			 * always be some data in a CPUTask at this point but
			 * check anyway */
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
		Task &task = iter.value();
		double d;
		double lastTime;
		int s = task.schedTimev.size();
		iter++;
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

/* This function is supposed to be called seldom, thus it's ok to not have it
 * as optimized as the other functions, e.g. in terms of inlining */
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

		task = &taskMap[oldpid];
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
	unsigned int seed = 290876;
	unsigned int i, j;
	QList<TColor> colorList;
	const TColor black(0, 0, 0);
	const TColor white(255, 255, 255);
	TColor tmp;

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
	s = cbrt( (1 / nf) * (255 * 255 * 255 ));
	s = TSMIN(s, 128.0);
	s = TSMAX(s, 1.0);
retry:
	step = (unsigned int) s;
	for (red = 0; red < 256; red += step) {
		for (green = 0; green < 256; green += step)  {
			for (blue = 0; blue < 256; blue += step) {
				TColor color(red, green, blue);
				if (color.SqDistance(black) < 1000)
					continue;
				if (color.SqDistance(white) < 10000)
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
			goto retry;
		}
	}

	/* Randomize the order by swapping every element with a random
	 * element */
	for (i = 0; i < ncolor; i++) {
		j = rand_r(&seed) % ncolor;
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


int TraceAnalyzer::binarySearch(double time, int start, int end)
{
	int pivot = (end + start) / 2;
	if (pivot == start)
		return pivot;
	if (time < events.at(pivot).time)
		return binarySearch(time, start, pivot);
	else
		return binarySearch(time, pivot, end);
}

int TraceAnalyzer::findIndexBefore(double time)
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

	if (events.at(c).time >= time)
		c--;
	return c;
}

TraceEvent *TraceAnalyzer::findPreviousSchedEvent(double time,
						unsigned int pid,
						int *index)
{
	int start = findIndexBefore(time);
	int i;

	if (start < 0)
		return nullptr;

	for (i = start; i >= 0; i--) {
		TraceEvent &event = events[i];
		if (event.type == SCHED_SWITCH  &&
		    generic_sched_switch_newpid(event) == pid) {
			if (index != nullptr)
				*index = i;
			return &event;
		}
	}
	return nullptr;
}

TraceEvent *TraceAnalyzer::findPreviousWakeupEvent(int startidx,
						 unsigned int pid,
						 int *index)
{
	int i;

	if (startidx < 0 || startidx >= (int) events.size())
		return nullptr;

	for (i = startidx; i >= 0; i--) {
		TraceEvent &event = events[i];
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
		customPlot->addItem(a);
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
	scaleMigration(); /* Migration scaling is done from the mainthread */
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
