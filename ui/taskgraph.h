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

#ifndef TASKGRAPH_H
#define TASKGRAPH_H

#include <QString>
#include "qcustomplot/qcustomplot.h"

class LegendGraph;
class Task;

class TaskGraph : public QCPGraph
{
	Q_OBJECT
public:
	TaskGraph(QCPAxis *keyAxis, QCPAxis *valueAxis);
	virtual ~TaskGraph();
	void setTask(Task *newTask);
	Task *getTask();
	TaskGraph *getTaskGraphForLegend();
	void setTaskGraphForLegend(TaskGraph *legendTaskGraph);
	void setPen(const QPen &pen);
	bool addToLegend();
	bool removeFromLegend();
private:
	Task *task;
	TaskGraph *taskGraph;
	LegendGraph *legendGraph;
};

#endif /* TASKGRAPH_H */
