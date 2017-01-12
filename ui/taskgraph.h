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
#include <QVector>
#include <QHash>

class LegendGraph;
class Task;
class QCustomPlot;
class QCPGraph;

class TaskGraph
{
public:
	TaskGraph(QCustomPlot *parent);
	virtual ~TaskGraph();
	void setTask(Task *newTask);
	Task *getTask();
	TaskGraph *getTaskGraphForLegend();
	void setTaskGraphForLegend(TaskGraph *legendTaskGraph);
	void setPen(const QPen &pen);
	bool addToLegend();
	bool removeFromLegend() const;
	void setData(const QVector<double > &keys,
		     const QVector<double> &values,
		     bool alreadySorted = false);
	static TaskGraph *fromQCPGraph(QCPGraph *g);
	QCPGraph *getQCPGraph();
private:
	Task *task;
	TaskGraph *taskGraph;
	QCPGraph *graph;
	QCPGraph *legendGraph;
	static QHash<QCPGraph *, TaskGraph *> graphDir;
};

#endif /* TASKGRAPH_H */
