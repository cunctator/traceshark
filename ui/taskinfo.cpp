/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include <QHBoxLayout>
#include <QIcon>
#include <QMap>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QString>

#include "analyzer/task.h"
#include "ui/taskgraph.h"
#include "ui/taskinfo.h"
#include "misc/resources.h"
#include "misc/traceshark.h"
#include "qcustomplot/qcustomplot.h"
#include "vtl/error.h"

#define DEFINE_PIDMAP_ITERATOR(name) \
	QMap<int, TaskGraph*>::iterator name

#define FIND_TOOLTIP \
	"Find the wakeup of this task that precedes the active cursor"
#define REMOVE_TASK_TOOLTIP \
	"Remove the unified graph for this task"

TaskInfo::TaskInfo(QWidget *parent):
	QWidget(parent), taskGraph(nullptr)
{
	QHBoxLayout *layout  = new QHBoxLayout(this);
	QLabel *colonLabel = new QLabel(tr(":"));

	nameLine = new QLineEdit(this);
	pidLine = new QLineEdit(this);

	nameLine->setReadOnly(true);
	pidLine->setReadOnly(true);

	layout->addWidget(nameLine);
	layout->addWidget(colonLabel);
	layout->addWidget(pidLine);

	createActions();
	createToolBar();

	layout->addWidget(taskToolBar);
	layout->addStretch();

	tsconnect(addToLegendAction, triggered(), this,
		addToLegendTriggered());
	tsconnect(addTaskGraphAction, triggered(), this,
		addTaskGraphTriggered());
	tsconnect(removeTaskGraphAction, triggered(), this,
		removeTaskGraphTriggered());
	tsconnect(clearAction, triggered(), this, clearTriggered());
	tsconnect(findAction, triggered(), this, findTriggered());
}

void TaskInfo::createActions()
{
	addTaskGraphAction = new QAction(tr("Add task graph"), this);
	addTaskGraphAction->setIcon(QIcon(RESSRC_PNG_ADD_TASK));
	/* addTaskGraphAction->setShortcuts(I_dont_know); */
	addTaskGraphAction->setToolTip(tr("Add a unified graph for this task"));

	addToLegendAction = new QAction(tr("Add task to the legend"), this);
	addToLegendAction->setIcon(QIcon(RESSRC_PNG_ADD_TO_LEGEND));
	/* addToLegendAction->setShortcuts(I_dont_know); */
	addToLegendAction->setToolTip(tr("Add this task to the legend"));

	clearAction = new QAction(tr("Clear the legend"), this);
	clearAction->setIcon(QIcon(RESSRC_PNG_CLEAR_LEGEND));
	/* clearAction->setShortcuts(I_dont_know) */
	clearAction->setToolTip(tr("Remove all tasks from the legend"));

	findAction = new QAction(tr("Find wakeup"), this);
	findAction->setIcon(QIcon(RESSRC_PNG_FIND_WAKEUP));
	/* findAction->setShortCuts(I_dont_know) */
	findAction->setToolTip(tr(FIND_TOOLTIP));

	removeTaskGraphAction = new QAction(tr("Remove task graph"), this);
	removeTaskGraphAction->setIcon(QIcon(RESSRC_PNG_REMOVE_TASK));
	/* removeTaskGraphAction->setShortCuts(I_dont_know) */
	removeTaskGraphAction->setToolTip(tr(REMOVE_TASK_TOOLTIP));

	setTraceActionsEnabled(false);
}

void TaskInfo::createToolBar()
{
	taskToolBar = new QToolBar(tr("Task Toolbar"), this);
	taskToolBar->addAction(addToLegendAction);
	taskToolBar->addAction(clearAction);
	taskToolBar->addAction(findAction);
	taskToolBar->addAction(addTaskGraphAction);
	taskToolBar->addAction(removeTaskGraphAction);
}

void TaskInfo::setTraceActionsEnabled(bool e)
{
	addToLegendAction->setEnabled(e);
	clearAction->setEnabled(e);
	findAction->setEnabled(e);
	addTaskGraphAction->setEnabled(e);
	removeTaskGraphAction->setEnabled(e);
}

TaskInfo::~TaskInfo()
{
}


void TaskInfo::setTaskGraph(TaskGraph *graph)
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

void TaskInfo::removeTaskGraph()
{
	taskGraph = nullptr;
	nameLine->setText(tr(""));
	pidLine->setText(tr(""));
}

void TaskInfo::clear()
{
	removeTaskGraph();
	legendPidMap.clear();
}

void TaskInfo::addToLegendTriggered()
{
	if (taskGraph == nullptr)
		return;

	addTaskGraphToLegend(taskGraph);
}


void TaskInfo::addTaskGraphToLegend(TaskGraph *graph)
{
	QObject *obj;
	QCustomPlot *plot;
	Task *task;

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

	legendPidMap[task->pid] = graph;
	graph->addToLegend();
	obj = graph->getQCPGraph()->parent();
	plot = qobject_cast<QCustomPlot *>(obj);
	if (plot != nullptr)
		plot->replot();
}


void TaskInfo::clearTriggered()
{
	QCustomPlot *plot = nullptr;
	QObject *obj;
	DEFINE_PIDMAP_ITERATOR(iter) = legendPidMap.begin();
	while(iter != legendPidMap.end()) {
		TaskGraph *&graph = iter.value();
		graph->removeFromLegend();
		if (plot == nullptr) {
			obj = graph->getQCPGraph()->parent();
			plot = qobject_cast<QCustomPlot *>(obj);
		}
		iter++;
	}
	if (plot != nullptr)
		plot->replot();
	legendPidMap.clear();
}

void TaskInfo::findTriggered()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit findWakeup(task->pid);
}

void TaskInfo::addTaskGraphTriggered()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit addTaskGraph(task->pid);
}

void TaskInfo::removeTaskGraphTriggered()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit removeTaskGraph(task->pid);
}

void TaskInfo::checkGraphSelection()
{
	if (taskGraph == nullptr)
		return;
	if (taskGraph->getQCPGraph()->selected())
		return;
	removeTaskGraph();
}

void TaskInfo::pidRemoved(int pid)
{
	if (legendPidMap.contains(pid))
		legendPidMap.remove(pid);
	else
		vtl::warnx("Unexpected state in %s:%d", __FILE__, __LINE__);
}
