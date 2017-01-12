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

#include "qcustomplot/qcustomplot.h"
#include "ui/taskgraph.h"
#include "analyzer/task.h"

QHash<QCPGraph *, TaskGraph *> TaskGraph::graphDir;

TaskGraph::TaskGraph(QCustomPlot *parent):
	task(nullptr), taskGraph(nullptr)
{
	graph = parent->addGraph(parent->xAxis, parent->yAxis);
	graphDir[graph] = this;
	graph->setAdaptiveSampling(true);
	graph->setLineStyle(QCPGraph::lsStepLeft);
	legendGraph = parent->addGraph(parent->xAxis, parent->yAxis);
	graphDir[legendGraph] = this;
}

TaskGraph::~TaskGraph()
{
	legendGraph->removeFromLegend();
	delete legendGraph;
}

void TaskGraph::setTask(Task *newTask)
{
	QString name = newTask->getDisplayName();
	name += QString(":") + QString::number(newTask->pid);
	graph->setName(name);
	legendGraph->setName(name);
	task = newTask;
}	

Task *TaskGraph::getTask()
{
	return task;
}

void TaskGraph::setPen(const QPen &pen)
{
	graph->setPen(pen);

	QPen legendPen(pen);
	legendPen.setWidth(5);
	legendGraph->setPen(legendPen);
}

bool TaskGraph::addToLegend()
{
	return legendGraph->addToLegend();
}

bool TaskGraph::removeFromLegend() const
{
	return legendGraph->removeFromLegend();
}

TaskGraph *TaskGraph::getTaskGraphForLegend()
{
	return taskGraph;
}

void TaskGraph::setTaskGraphForLegend(TaskGraph *legendTaskGraph)
{
	taskGraph = legendTaskGraph;
}

void TaskGraph::setData(const QVector<double > &keys,
			const QVector<double> &values,
			bool alreadySorted)
{
	graph->setData(keys, values, alreadySorted);
}

TaskGraph *TaskGraph::fromQCPGraph(QCPGraph *g)
{
	QHash<QCPGraph *, TaskGraph *>::iterator i = graphDir.find(g);
	if (i == graphDir.end())
		return nullptr;
	return i.value();
}

QCPGraph *TaskGraph::getQCPGraph()
{
	return graph;
}
