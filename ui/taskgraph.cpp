// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "qcustomplot/qcustomplot.h"
#include "ui/taskgraph.h"
#include "analyzer/task.h"

QMap<QCPGraph *, TaskGraph *> TaskGraph::graphDir;

TaskGraph::TaskGraph(QCustomPlot *parent, unsigned int cpu_,
		     enum GraphType g):
	plot(parent), task(nullptr), taskGraph(nullptr), cpu(cpu_),
	graph_type(g)
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
}

void TaskGraph::destroy()
{
	plot->removeGraph(graph);
	plot->removeGraph(legendGraph);
	graphDir.remove(graph);
	graphDir.remove(legendGraph);
	delete this;
}

void TaskGraph::setTask(Task *newTask)
{
	QString name = *newTask->displayName;
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
	QMap<QCPGraph *, TaskGraph *>::iterator i = graphDir.find(g);
	if (i == graphDir.end())
		return nullptr;
	return i.value();
}

void TaskGraph::clearMap()
{
	graphDir.clear();
}

QCPGraph *TaskGraph::getQCPGraph()
{
	return graph;
}

unsigned int TaskGraph::getCPU()
{
	return cpu;
}

int TaskGraph::getPid()
{
	if (task != nullptr)
		return task->pid;
	return 0;
}

enum TaskGraph::GraphType TaskGraph::getGraphType()
{
	return graph_type;
}
