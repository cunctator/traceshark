// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QIcon>
#include <QHBoxLayout>
#include <QMap>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QString>
#include <QToolBar>
#include <QWidget>

#include "analyzer/task.h"
#include "ui/taskgraph.h"
#include "ui/tasktoolbar.h"
#include "misc/maplist.h"
#include "misc/resources.h"
#include "misc/traceshark.h"
#include "ui/qcustomplot.h"
#include "vtl/error.h"

#define DEFINE_PIDMAP_ITERATOR(name) \
	MapList<int, TaskGraph*>::iterator name

#define DEFINE_CONST_PIDMAP_ITERATOR(name) \
	MapList<int, TaskGraph*>::const_iterator name

TaskToolBar::TaskToolBar(const QString &title, QWidget *parent):
	QToolBar(title, parent), taskGraph(nullptr)
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout  = new QHBoxLayout(widget);
	QLabel *colonLabel = new QLabel(tr(":"));

	nameLine = new QLineEdit();
	pidLine = new QLineEdit();

	nameLine->setReadOnly(true);
	pidLine->setReadOnly(true);

	layout->addWidget(nameLine);
	layout->addWidget(colonLabel);
	layout->addWidget(pidLine);
	addWidget(widget);
}


TaskToolBar::~TaskToolBar()
{}

void TaskToolBar::setTaskGraph(TaskGraph *graph)
{
	TaskGraph *legendGraph;
	Task *task = graph->getTask();
	if (task == nullptr)
		return;
	QString nameStr = task->getLastName();
	QString pidStr = QString::number(task->pid);
	nameLine->setText(nameStr);
	pidLine->setText(pidStr);
	/*
	 * taskGraph will be used for displaying the legend in case the user
	 * pushes that button. For that reason we will check if this TaskGraph
	 * has a pointer to another TaskGraph that is to be used for legend
	 * displaying purposes. In practice this happens when a unfied TaskGraph
	 * is set here, because that one might get deleted by user action, it
	 * instead has a pointer to a per CPU TaskGraph.
	 */
	legendGraph = graph->getTaskGraphForLegend();
	taskGraph = legendGraph != nullptr ? legendGraph : graph;
}

void TaskToolBar::removeTaskGraph()
{
	taskGraph = nullptr;
	nameLine->setText(tr(""));
	pidLine->setText(tr(""));
}

void TaskToolBar::clear()
{
	bool before, after;

	removeTaskGraph();
	before = legendPidMap.isEmpty();
	legendPidMap.clear();
	after = legendPidMap.isEmpty();
	if (before != after)
		emit LegendEmptyChanged(after);
}

void TaskToolBar::addCurrentTaskToLegend()
{
	if (taskGraph == nullptr)
		return;

	addTaskGraphToLegend(taskGraph);
}


void TaskToolBar::addTaskGraphToLegend(TaskGraph *graph)
{
	Task *task;
	bool before, after;

	task = graph->getTask();
	if (task == nullptr)
		return;

	/*
	 * We use the legendPidMap to keep track of pids that have been added
	 * to the legend, otherwise it could happen that someone added the same
	 * task multiple times to the legend and that looks a bit crazy. While
	 * QCustomPlot prevents the same the same QCPGraph object being
	 * added twice we can have multiple identical objects, since tasks can
	 * migrate between CPUs.
	 */
	if (legendPidMap.contains(task->pid))
		return;

	before = legendPidMap.isEmpty();
	legendPidMap.append(task->pid, graph);
	after = legendPidMap.isEmpty();

	graph->addToLegend();
	if (before != after)
		emit LegendEmptyChanged(after);
}


void TaskToolBar::clearLegend()
{
	bool before, after;
	QCustomPlot *plot = nullptr;
	QObject *obj;
	DEFINE_PIDMAP_ITERATOR(iter) = legendPidMap.begin();
	while(iter != legendPidMap.end()) {
		TaskGraph *graph = iter.value();
		graph->removeFromLegend();
		if (plot == nullptr) {
			obj = graph->getQCPGraph()->parent();
			plot = qobject_cast<QCustomPlot *>(obj);
		}
		iter++;
	}
	if (plot != nullptr)
		plot->replot();
	before = legendPidMap.isEmpty();
	legendPidMap.clear();
	after = legendPidMap.isEmpty();
	if (before != after)
		emit LegendEmptyChanged(after);
}

bool TaskToolBar::checkGraphSelection()
{
	if (taskGraph == nullptr)
		return false;
	if (taskGraph->getQCPGraph()->selected())
		return false;
	removeTaskGraph();
	return true;
}

void TaskToolBar::pidRemoved(int pid)
{
	bool before, after;

	before = legendPidMap.isEmpty();
	if (legendPidMap.contains(pid))
		legendPidMap.remove(pid);
	else
		vtl::warnx("Unexpected state in %s:%d", __FILE__, __LINE__);
	after = legendPidMap.isEmpty();
	if (before != after)
		emit LegendEmptyChanged(after);
}

int TaskToolBar::getPid()
{
	Task *task;

	if (taskGraph == nullptr)
		return 0;

	task = taskGraph->getTask();
	if (task == nullptr)
		return 0;

	return task->pid;
}

void TaskToolBar::addStretch()
{
	QWidget *widget = new QWidget();
	QHBoxLayout *layout  = new QHBoxLayout(widget);
	layout->addStretch();
	addWidget(widget);
}

QList<int> TaskToolBar::legendPidList() const
{
	DEFINE_CONST_PIDMAP_ITERATOR(iter);
	QList<int> rlist;

	for (iter = legendPidMap.cbegin(); iter != legendPidMap.cend(); iter++)
		rlist.append(iter.key());
	return rlist;
}

bool TaskToolBar::legendContains(int pid) const
{
	return legendPidMap.contains(pid);
}
