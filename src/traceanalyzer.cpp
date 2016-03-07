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
#include <QTextStream>
#include "cpufreq.h"
#include "cpuidle.h"
#include "genericparams.h"
#include "traceanalyzer.h"
#include "traceparser.h"
#include "traceshark.h"
#include "threads/workthread.h"
#include "threads/workitem.h"
#include "threads/workqueue.h"
#include "tlist.h"

TraceAnalyzer::TraceAnalyzer()
	: cpuTaskMaps(NULL), cpuFreq(NULL), cpuIdle(NULL),
	  black(0, 0, 0), white(255, 255, 255),
	  CPUs(NULL)
{
	taskNamePool = new MemPool(16384, sizeof(char));
	schedItem = new WorkItem<TraceAnalyzer> (this,
					       &TraceAnalyzer::processSched);
	migItem = new WorkItem<TraceAnalyzer> (this,
					     &TraceAnalyzer::processMigration);
	freqItem = new WorkItem<TraceAnalyzer> (this,
					      &TraceAnalyzer::processCPUfreq);
	processingQueue.addDefaultWorkItem(schedItem);
	processingQueue.addDefaultWorkItem(migItem);
	processingQueue.addDefaultWorkItem(freqItem);
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
	unsigned int nrCPUs = getNrCPUs();
	cpuTaskMaps = new QMap<unsigned int, CPUTask>[nrCPUs];
	cpuFreq = new CpuFreq[nrCPUs];
	cpuIdle = new CpuIdle[nrCPUs];
	CPUs = new CPU[nrCPUs];
	schedOffset.resize(0);
	schedOffset.resize(nrCPUs);
	schedScale.resize(0);
	schedScale.resize(nrCPUs);
	cpuIdleOffset.resize(0);
	cpuIdleOffset.resize(nrCPUs);
	cpuIdleScale.resize(0);
	cpuIdleScale.resize(nrCPUs);
	cpuFreqOffset.resize(0);
	cpuFreqOffset.resize(nrCPUs);
	cpuFreqScale.resize(0);
	cpuFreqScale.resize(nrCPUs);
}

bool TraceAnalyzer::isOpen()
{
	return parser->isOpen();
}

void TraceAnalyzer::close()
{
	if (cpuTaskMaps != NULL) {
		delete[] cpuTaskMaps;
		cpuTaskMaps = NULL;
	}
	if (cpuFreq != NULL) {
		delete[] cpuFreq;
		cpuFreq = NULL;
	}
	if (cpuIdle != NULL) {
		delete[] cpuIdle;
		cpuIdle = NULL;
	}
	if (CPUs != NULL) {
		delete[] CPUs;
		CPUs = NULL;
	}
	events.clear();
	migrations.clear();
	migrationArrows.clear();
	colorMap.clear();
	parser->close();
	taskNamePool->reset();
}

void TraceAnalyzer::processTrace()
{
	QTextStream qout(stdout);
	quint64 start, process, colorize;

	qout.setRealNumberPrecision(6);
	qout.setRealNumberNotation(QTextStream::FixedNotation);

	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	processingQueue.setWorkItemsDefault();
	processingQueue.start();
	processingQueue.wait();

	process = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	qout << "processing took " << (double) (process - start) / 1000 <<
		" s\n";
	qout.flush();

	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	colorizeTasks();
	colorize = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	qout << "colorize() took " << (double) (colorize - start) / 1000 <<
		" s\n";
}

bool TraceAnalyzer::processMigration()
{
	if (getTraceType() == TRACE_TYPE_FTRACE) {
		processMigrationFtrace();
	} else if (getTraceType() == TRACE_TYPE_PERF) {
		processMigrationPerf();
	}
	return false;
}

bool TraceAnalyzer::processSched()
{
	switch (getTraceType()) {
	case TRACE_TYPE_FTRACE:
		processSchedFtrace();
		break;
	case TRACE_TYPE_PERF:
		processSchedPerf();
		break;
	default:
		break;
	}
	return false;
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
			iter++;
			/* Check if tail is necessary */
			if (task.timev[task.timev.size() - 1] >= getEndTime())
				continue;
			d = task.data[task.data.size() - 1];
			task.timev.append(getEndTime());
			task.data.append(d);
		}
	}
}

/* This function is supposed to be called seldom, thus it's ok to not have it
 * as optimized as the other functions, e.g. in terms of inlining */
void TraceAnalyzer::handleWrongTaskOnCPU(TraceEvent &event, unsigned int cpu,
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
		Q_ASSERT(!cpuTask->timev.isEmpty());
		prevtime = cpuTask->timev.last();
		faketime = prevtime + FAKE_DELTA;
		cpuTask->timev.append(faketime);
		cpuTask->data.append(FLOOR_HEIGHT);
		task = getTask(epid);
		task->lastWakeUP = faketime;
	}

	if (oldpid != 0) {
		cpuTask = &cpuTaskMaps[cpu][oldpid];
		if (cpuTask->isNew) {
			cpuTask->pid = oldpid;
			cpuTask->name =
				sched_switch_oldname_strdup(getTraceType(),
							    event,
							    taskNamePool);
		}
		cpuTask->isNew = false;
		faketime = oldtime - FAKE_DELTA;
		cpuTask->timev.append(faketime);
		cpuTask->data.append(SCHED_HEIGHT);
	}
}

bool TraceAnalyzer::processCPUfreq()
{
	unsigned int cpu;

	if (getTraceType() == TRACE_TYPE_NONE)
		return true;

	if (!tracetype_is_valid(getTraceType()))
		return false;

	for (cpu = 0; cpu <= getMaxCPU(); cpu++) {
		if (parser->startFreq[cpu] > 0) {
			cpuFreq[cpu].timev.append(getStartTime());
			cpuFreq[cpu].data.append(parser->startFreq[cpu]);
		}
	}

	if (getTraceType() == TRACE_TYPE_FTRACE)
		processCPUfreqFtrace();
	else
		processCPUfreqPerf();

	for (cpu = 0; cpu <= getMaxCPU(); cpu++) {
		if (!cpuFreq[cpu].data.isEmpty()) {
			double freq = cpuFreq[cpu].data.last();
			cpuFreq[cpu].data.append(freq);
			cpuFreq[cpu].timev.append(getEndTime());
		}
	}
	return false;
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
	cpuIdleScale[cpu] = scale / getMaxIdleState();
}

void TraceAnalyzer::setCpuFreqOffset(unsigned int cpu, double offset)
{
	cpuFreqOffset[cpu] = offset;
}

void TraceAnalyzer::setCpuFreqScale(unsigned int cpu, double scale)
{
	cpuFreqScale[cpu] = scale / parser->maxFreq;
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

void TraceAnalyzer::processSchedFtrace()
{
	__processSchedGeneric(TRACE_TYPE_FTRACE);
}

void TraceAnalyzer::processSchedPerf()
{
	__processSchedGeneric(TRACE_TYPE_PERF);
}

void TraceAnalyzer::processCPUfreqFtrace()
{
	__processCPUfreq(TRACE_TYPE_FTRACE);
}

void TraceAnalyzer::processCPUfreqPerf()
{
	__processCPUfreq(TRACE_TYPE_PERF);
}

void TraceAnalyzer::processMigrationFtrace()
{
	__processMigrationGeneric(TRACE_TYPE_FTRACE);
}

void TraceAnalyzer::processMigrationPerf()
{
	__processMigrationGeneric(TRACE_TYPE_PERF);
}
