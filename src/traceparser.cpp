/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "ftraceparams.h"
#include "traceparser.h"
#include "tracefile.h"
#include "grammarroot.h"
#include "namepidnode.h"
#include "cpunode.h"
#include "threads/threadbuffer.h"
#include "timenode.h"
#include "eventnode.h"
#include "argnode.h"
#include "namenode.h"
#include "pidnode.h"
#include "perfeventnode.h"
#include "traceshark.h"
#include "threads/workthread.h"
#include "threads/workitem.h"
#include "threads/workqueue.h"

bool TraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	unsigned int i = 0;
	unsigned int curbuf = 0;
	bool ok = false;

	if (traceFile != NULL)
		return ok;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ok,
				  1024*1024, NR_TBUFFERS);

	if (!ok) {
		delete traceFile;
		traceFile = NULL;
		return ok;
	}

	for (i = 0; i < NR_TBUFFERS; i++)
		tbuffers[i] = new ThreadBuffer<TraceLine>(TBUFSIZE,
							  NR_TBUFFERS);
	parserThread->start();

	i = 0;
	tbuffers[curbuf]->beginProduceBuffer();
	while(!traceFile->atEnd()) {
		TraceLine *line = &tbuffers[curbuf]->buffer[i];
		quint32 n = traceFile->ReadLine(line, 0);
		nr += n;
		i++;
		if (i == (TBUFSIZE - 1)) {
			tbuffers[curbuf]->endProduceBuffer(i);
			curbuf++;
			if (curbuf == NR_TBUFFERS)
				curbuf = 0;
			i = 0;
			tbuffers[curbuf]->beginProduceBuffer();
			traceFile->clearPool(curbuf);
		}
	}
	tbuffers[curbuf]->endProduceBuffer(i);
	/* We must send an empty buffer at the end to signal that we are EOF */
	if (i != 0) {
		curbuf++;
		if (curbuf == NR_TBUFFERS)
			curbuf = 0;
		tbuffers[curbuf]->beginProduceBuffer();
		tbuffers[curbuf]->endProduceBuffer(0);
	}

	parserThread->wait();

	for (i = 0; i < NR_TBUFFERS; i++)
		delete tbuffers[i];

	QTextStream(stdout) << nr << "\n";
	return true;
}

bool TraceParser::isOpen()
{
	return (traceFile != NULL);
}

void TraceParser::close()
{
	if (traceFile != NULL) {
		events.clear();
		delete traceFile;
		traceFile = NULL;
	}
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
	migrations.clear();
	migrationArrows.clear();
	ptrPool->reset();
	taskNamePool->reset();
	clearGrammarPools(ftraceGrammarRoot);
	traceType = TRACE_TYPE_NONE;
}

TraceParser::TraceParser()
	: cpuTaskMaps(NULL), cpuFreq(NULL), cpuIdle(NULL),
	  traceType(TRACE_TYPE_NONE), black(0, 0, 0), white(255, 255, 255),
	  CPUs(NULL)
{
	traceFile = NULL;
	ptrPool = new MemPool(16384, sizeof(TString*));
	taskNamePool = new MemPool(16384, sizeof(char));

	createFtraceGrammarTree();
	createPerfGrammarTree();

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<TraceParser>
		(this, &TraceParser::parseThread);
}

void TraceParser::createFtraceGrammarTree()
{
	ArgNode *ftraceArgNode;
	EventNode *ftraceEventNode;
	TimeNode *ftraceTimeNode;
	CpuNode *ftraceCpuNode;
	NamePidNode *ftraceNamePidNode;

	ftraceArgNode = new ArgNode("ftraceArgNode");
	ftraceArgNode->nChildren = 1;
	ftraceArgNode->children[0] = ftraceArgNode;
	ftraceArgNode->isLeaf = true;

	ftraceEventNode = new EventNode("ftraceEventnode");
	ftraceEventNode->nChildren = 1;
	ftraceEventNode->children[0] = ftraceArgNode;
	ftraceEventNode->isLeaf = true;

	ftraceTimeNode = new TimeNode("ftraceTimeNode");
	ftraceTimeNode->nChildren = 1;
	ftraceTimeNode->children[0] = ftraceEventNode;
	ftraceTimeNode->isLeaf = false;

	ftraceCpuNode = new CpuNode("ftraceCpuNode");
	ftraceCpuNode->nChildren = 1;
	ftraceCpuNode->children[0] = ftraceTimeNode;
	ftraceCpuNode->isLeaf = false;

	ftraceNamePidNode = new NamePidNode("ftraceNamePidNode");
	ftraceNamePidNode->nChildren = 1;
	ftraceNamePidNode->children[0] = ftraceCpuNode;
	ftraceNamePidNode->isLeaf = false;

	ftraceGrammarRoot = new GrammarRoot("ftraceRootNode");
	ftraceGrammarRoot->nChildren = 1;
	ftraceGrammarRoot->children[0] = ftraceNamePidNode;
	ftraceGrammarRoot->isLeaf = false;
}

void TraceParser::createPerfGrammarTree()
{
	ArgNode *perfArgNode;
	PerfEventNode *perfEventNode;
	TimeNode *perfTimeNode;
	CpuNode *perfCpuNode;
	PidNode *perfPidNode;
	NameNode *perfNameNode;

	perfArgNode = new ArgNode("perfArgNode");
	perfArgNode->nChildren = 1;
	perfArgNode->children[0] = perfArgNode;
	perfArgNode->isLeaf = true;

	perfEventNode = new PerfEventNode("perfEventNode");
	perfEventNode->nChildren = 1;
	perfEventNode->children[0] = perfArgNode;
	perfEventNode->isLeaf = true;

	perfTimeNode = new TimeNode("perfTimeNode");
	perfTimeNode->nChildren = 1;
	perfTimeNode->children[0] = perfEventNode;
	perfTimeNode->isLeaf = false;

	perfCpuNode = new CpuNode("perfCpuNode");
	perfCpuNode->nChildren = 1;
	perfCpuNode->children[0] = perfTimeNode;
	perfCpuNode->isLeaf = false;

	perfPidNode = new PidNode("perfPidNode");
	perfPidNode->nChildren = 1;
	perfPidNode->children[0] = perfCpuNode;
	perfPidNode->isLeaf = false;

	perfNameNode = new NameNode("perfNameNode");
	perfNameNode->nChildren = 1;
	perfNameNode->children[0] = perfPidNode;
	perfNameNode->isLeaf = false;

	perfGrammarRoot = new GrammarRoot("perfNameNode");
	perfGrammarRoot->nChildren = 1;
	perfGrammarRoot->children[0] = perfNameNode;
	perfGrammarRoot->isLeaf = false;
}

TraceParser::~TraceParser()
{
	TraceParser::close();
	DeleteGrammarTree(ftraceGrammarRoot);
	DeleteGrammarTree(perfGrammarRoot);
	delete ptrPool;
	delete taskNamePool;
	delete[] tbuffers;
	delete parserThread;
}

void TraceParser::DeleteGrammarTree(GrammarNode* node) {
	unsigned int i;
	for (i = 0; i < node->nChildren; i++) {
		/* Delete subtree unless it's a node being it's own child */
		if (node->children[i] != node)
			DeleteGrammarTree(node->children[i]);
	}
	delete node;
}

/* This function does prescanning as well, to determine number of events,
 * number of CPUs, max/min CPU frequency etc */
void TraceParser::parseThread()
{
	unsigned int i = 0;
	preparePreScan();
	events.clear();

	while(true) {
		if (parseBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
		determineTraceType();
		if (traceType == TRACE_TYPE_FTRACE) {
			goto ftrace;
		}
		if (traceType == TRACE_TYPE_PERF) {
			goto perf;
		}
	}
	/* Something went wrong if we get here, we better pretend that
	 * we were not able to read any events */
	nrEvents = 0;
	events.clear();
	return;

	/* The purpose of jumping to these loops is to  be able to use the
	 * (hopefully faster) specialized parse functions */
ftrace:
	while(true) {
		if (parseFtraceBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
	finalizePreScan();
	return;

perf:
	while(true) {
		if (parsePerfBuffer(i))
			break;
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
	finalizePreScan();
}

void TraceParser::preparePreScan()
{
	nrEvents = 0;
	nrFtraceEvents = 0;
	nrPerfEvents = 0;
	maxCPU = 0;
	startTime = 0;
	endTime = 0;
	minFreq = UINT_MAX;
	maxFreq = 0;
	minIdleState = INT_MAX;
	maxIdleState = INT_MIN;
	nrMigrateEvents = 0;
	startFreq.fill(-1, HIGHEST_CPU_EVER + 1);
}

void TraceParser::finalizePreScan()
{
	lastEvent = nrEvents - 1;
	if (nrEvents >= 2) {
		startTime = events[0].time;
		endTime = events[lastEvent].time;
	}

	nrCPUs = maxCPU + 1;
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
	startFreq.resize(nrCPUs);
}

void TraceParser::preScan()
{
}

void TraceParser::parse()
{
}

/* This parses a buffer regardless if it's perf or ftrace */
bool TraceParser::parseBuffer(unsigned int index)
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
			nrFtraceEvents++;
			preScanFtraceEvent(event);
		}
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
			nrPerfEvents++;
			preScanPerfEvent(event);
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}

bool TraceParser::processMigration()
{
	if (traceType == TRACE_TYPE_FTRACE) {
		processMigrationFtrace();
	} else if (traceType == TRACE_TYPE_PERF) {
		processMigrationPerf();
	}
	return false;
}

void TraceParser::processMigrationFtrace()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (sched_migrate(event)) {
			Migration m;
			m.pid = sched_migrate_pid(event);
			m.oldcpu = sched_migrate_origCPU(event);
			m.newcpu = sched_migrate_destCPU(event);
			m.time = event.time;
			migrations.push_back(m);
		} else if (sched_process_fork(event)) {
			Migration m;
			m.pid = sched_process_fork_childpid(event);
			m.oldcpu = -1;
			m.newcpu = event.cpu;
			m.time = event.time;
			migrations.push_back(m);
		} else if (sched_process_exit(event)) {
			Migration m;
			m.pid = sched_process_exit_pid(event);
			m.oldcpu = event.cpu;
			m.newcpu = -1;
			m.time = event.time;
			migrations.push_back(m);
		}
	}
}

void TraceParser::processMigrationPerf()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (perf_sched_migrate(event)) {
			Migration m;
			m.pid = perf_sched_migrate_pid(event);
			m.oldcpu = perf_sched_migrate_origCPU(event);
			m.newcpu = perf_sched_migrate_destCPU(event);
			m.time = event.time;
			migrations.push_back(m);
		} else if (perf_sched_process_fork(event)) {
			Migration m;
			m.pid = perf_sched_process_fork_childpid(event);
			m.oldcpu = -1;
			m.newcpu = event.cpu;
			m.time = event.time;
			migrations.push_back(m);
		} else if (perf_sched_process_exit(event)) {
			Migration m;
			m.pid = perf_sched_process_exit_pid(event);
			m.oldcpu = event.cpu;
			m.newcpu = -1;
			m.time = event.time;
			migrations.push_back(m);
		}
	}
}

bool TraceParser::processSched()
{
	if (traceType == TRACE_TYPE_FTRACE) {
		processSchedFtrace();
	} else if (traceType == TRACE_TYPE_PERF) {
		processSchedPerf();
	}
	return false;
}

void TraceParser::processSchedFtrace()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (sched_switch(event)) {
			processFtraceSwitchEvent(event);
		} else if (sched_wakeup(event)) {
			processFtraceWakeupEvent(event);
		}
	}
	processSchedAddTail();
}

void TraceParser::processSchedPerf()
{
	unsigned long i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		if (perf_sched_switch(event)) {
			processPerfSwitchEvent(event);
		} else if (sched_wakeup(event)) {
			processPerfWakeupEvent(event);
		}
	}
	processSchedAddTail();
}

void TraceParser::processSchedAddTail()
{
	/* Add the "tail" to all tasks, i.e. extend them until endTime */
	unsigned int cpu;
	for (cpu = 0; cpu < nrCPUs; cpu++) {
		DEFINE_CPUTASKMAP_ITERATOR(iter) = cpuTaskMaps[cpu].begin();
		while (iter != cpuTaskMaps[cpu].end()) {
			CPUTask &task = iter.value();
			double d;
			iter++;
			/* Check if tail is necessary */
			if (task.timev[task.timev.size() - 1] >= endTime)
				continue;
			d = task.data[task.data.size() - 1];
			task.timev.push_back(endTime);
			task.data.push_back(d);
		}
	}
}

bool TraceParser::processCPUfreq()
{
	unsigned int cpu;

	if (traceType == TRACE_TYPE_NONE)
		return true;

	if (traceType != TRACE_TYPE_FTRACE && traceType != TRACE_TYPE_PERF)
		return false;

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		if (startFreq[cpu] > 0) {
			cpuFreq[cpu].timev.push_back(startTime);
			cpuFreq[cpu].data.push_back(startFreq[cpu]);
		}
	}

	if (traceType == TRACE_TYPE_FTRACE)
		_processCPUfreqFtrace();
	else
		_processCPUfreqPerf();

	for (cpu = 0; cpu <= maxCPU; cpu++) {
		if (!cpuFreq[cpu].data.isEmpty()) {
			double freq = cpuFreq[cpu].data.last();
			cpuFreq[cpu].data.push_back(freq);
			cpuFreq[cpu].timev.push_back(endTime);
		}
	}
	return false;
}

void TraceParser::_processCPUfreqFtrace()
{
	unsigned int i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		/*
		 * I expect this loop to be so fast in comparison
		 * to the other functions that will be running in parallel
		 * that it's acceptable to piggy back cpuidle events here */
		if (cpuidle_event(event))
			processFtraceCPUidleEvent(event);
		else if (cpufreq_event(event))
			processFtraceCPUfreqEvent(event);
	}
}

void TraceParser::_processCPUfreqPerf()
{
	unsigned int i;
	for (i = 0; i < nrEvents; i++) {
		TraceEvent &event = events[i];
		/*
		 * I expect this loop to be so fast in comparison
		 * to the other functions that will be running in parallel
		 * that it's acceptable to piggy back cpuidle events here */
		if (perf_cpuidle_event(event))
			processPerfCPUidleEvent(event);
		else if (perf_cpufreq_event(event))
			processPerfCPUfreqEvent(event);
	}
}

void TraceParser::colorizeTasks()
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

	for (cpu = 0; cpu < maxCPU; cpu++) {
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
				colorList.push_back(color);
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

void TraceParser::setSchedOffset(unsigned int cpu, double offset)
{
	schedOffset[cpu] = offset;
}

void TraceParser::setSchedScale(unsigned int cpu, double scale)
{
	schedScale[cpu] = scale;
}

void TraceParser::setCpuIdleOffset(unsigned int cpu, double offset)
{
	cpuIdleOffset[cpu] = offset;
}

void TraceParser::setCpuIdleScale(unsigned int cpu, double scale)
{
	cpuIdleScale[cpu] = scale / maxIdleState;
}

void TraceParser::setCpuFreqOffset(unsigned int cpu, double offset)
{
	cpuFreqOffset[cpu] = offset;
}

void TraceParser::setCpuFreqScale(unsigned int cpu, double scale)
{
	cpuFreqScale[cpu] = scale / maxFreq;
}

void TraceParser::setMigrationOffset(double offset)
{
	migrationOffset = offset;
}

void TraceParser::setMigrationScale(double scale)
{
	migrationScale = scale;
}

void TraceParser::setQCustomPlot(QCustomPlot *plot)
{
	customPlot = plot;
}

void TraceParser::addCpuFreqWork(unsigned int cpu,
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

void TraceParser::addCpuIdleWork(unsigned int cpu,
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

void TraceParser::addCpuSchedWork(unsigned int cpu,
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
void TraceParser::scaleMigration()
{
	MigrationArrow *a;
	QList<Migration>::iterator iter;
	double unit = migrationScale / nrCPUs;
	for (iter = migrations.begin(); iter != migrations.end(); iter++) {
		Migration &m = *iter;
		double s = migrationOffset + (m.oldcpu + 1) * unit;
		double e = migrationOffset + (m.newcpu + 1) * unit;
		QColor color = getTaskColor(m.pid);
		a = new MigrationArrow(s, e, m.time, color, customPlot);
		customPlot->addItem(a);
		migrationArrows.push_back(a);
	}
}

void TraceParser::doScale()
{
	QList<AbstractWorkItem*> workList;
	unsigned int cpu;
	int i, s;

	for (cpu = 0; cpu <= maxCPU; cpu++) {
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

bool TraceParser::parseLineBugFixup(TraceEvent* event, double prevtime)
{
	double corrtime = event->time + 0.9;
	double delta = corrtime - prevtime;
	bool retval = false;

	if (delta >= (double)0 && delta < 0.00001) {
		event->time = corrtime;
		retval = true;
	}
	return retval;
}

void TraceParser::clearGrammarPools(GrammarNode *tree)
{
	unsigned int i;
	tree->clearStringPool();
	for (i = 0; i < tree->nChildren; i++) {
		/* Clear subtree unless it's a node being it's own child */
		if (tree->children[i] != tree)
			clearGrammarPools(tree->children[i]);
	}
}

void TraceParser::determineTraceType()
{
	if (nrFtraceEvents > 0 && nrPerfEvents == 0) {
		traceType = TRACE_TYPE_FTRACE;
		return;
	} else if (nrFtraceEvents  == 0 && nrPerfEvents > 0) {
		traceType = TRACE_TYPE_PERF;
		return;
	}
	traceType = TRACE_TYPE_NONE;
}
