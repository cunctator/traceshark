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

#include <QString>
#include <QStringList>
#include <QVector>
#include <QList>

#include "mempool.h"
#include "task.h"
#include "traceevent.h"
#include "traceline.h"
#include "grammarnode.h"

class FtraceEvent;
class TraceFile;

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
	inline unsigned int getMaxCPU();
	inline double getStartTime();
	inline double getEndTime();
	inline unsigned long getNrEvents();
private:
	inline bool parseLine(TraceLine* line, TraceEvent* event);
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
	QMap<unsigned int, Task> *cpuTaskMaps;
};

inline bool FtraceParser::parseLine(TraceLine* line, TraceEvent* event)
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

inline unsigned int FtraceParser::getMaxCPU()
{
	return maxCPU;
}

inline double  FtraceParser::getStartTime()
{
	return startTime;
}

inline double FtraceParser::getEndTime()
{
	return endTime;
}

inline unsigned long int FtraceParser::getNrEvents()
{
	return nrEvents;
}

#endif
