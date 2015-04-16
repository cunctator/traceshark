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

#include "mempool.h"
#include "task.h"
#include "tcolor.h"
#include "traceevent.h"
#include "traceline.h"
#include "grammarnode.h"
#include "migration.h"

class FtraceEvent;
class TraceFile;
class CpuFreq;
class CpuIdle;

class FtraceParser
{
public:
	FtraceParser();
	~FtraceParser();
	void DeleteGrammarTree(GrammarNode* node);
	bool open(const QString &fileName);
	bool isOpen();
	void close();
	bool parse(void);
	void preScan();
	void processMigration();
	void processSched();
	void processCPUfreq();
	QVector<TraceEvent> events;
	QVector<TraceLine> lines;
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
	__always_inline void preScanEvent(TraceEvent &event);
	__always_inline bool parseLine(TraceLine* line, TraceEvent* event);
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

#endif
