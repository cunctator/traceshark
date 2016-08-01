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

#include "analyzer/task.h"
#include "ui/taskgraph.h"
#include "ui/taskinfo.h"
#include "misc/traceshark.h"
#include "qcustomplot/qcustomplot.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QMap>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QString>

#define DEFINE_PIDMAP_ITERATOR(name) \
	QMap<unsigned int, TaskGraph*>::iterator name

#define ADD_TO_LEGEND_RESOURCE ":/traceshark/images/addtolegend30x30.png"
#define ADD_TASK_RESOURCE ":/traceshark/images/addtask30x30.png"
#define CLEAR_LEGEND_RESOURCE ":/traceshark/images/clearlegend30x30.png"
#define FIND_WAKEUP_RESOURCE ":/traceshark/images/wakeup30x30.png"
#define REMOVE_TASK_RESOURCE ":/traceshark/images/removetask30x30.png"
#define TASKSELECTOR_RESOURCE ":/traceshark/images/taskselector30x30.png"

#define FIND_TOOLTIP \
	"Find the wakeup of this task that precedes the active cursor"
#define REMOVE_TASK_TOOLTIP \
	"Remove the unified graph for this task"
#define TASKSELECTOR_TOOLTIP \
	"View a list of all tasks and it's possible to select one"

TaskInfo::TaskInfo(QWidget *parent):
	QWidget(parent), taskGraph(nullptr)
{
	QPixmap addPM(QLatin1String(ADD_TO_LEGEND_RESOURCE));
	QPixmap addTaskPM(QLatin1String(ADD_TASK_RESOURCE));
	QPixmap clearPM(QLatin1String(CLEAR_LEGEND_RESOURCE));
	QPixmap findPM(QLatin1String(FIND_WAKEUP_RESOURCE));
	QPixmap removeTaskPM(QLatin1String(REMOVE_TASK_RESOURCE));
	QPixmap taskSelectorPM(QLatin1String(TASKSELECTOR_RESOURCE));
	QIcon findIcon(findPM);
	QIcon addIcon(addPM);
	QIcon addTaskIcon(addTaskPM);
	QIcon clearIcon(clearPM);
	QIcon removeTaskIcon(removeTaskPM);
	QIcon taskSelectorIcon(taskSelectorPM);
	QHBoxLayout *layout  = new QHBoxLayout(this);
	QLabel *colonLabel = new QLabel(tr(":"));
	QPushButton *addButton = new QPushButton(addIcon, tr(""), this);
	QPushButton *addTaskButton = new QPushButton(addTaskIcon, tr(""), this);
	QPushButton *clearButton = new QPushButton(clearIcon, tr(""), this);
	QPushButton *findButton = new QPushButton(findIcon, tr(""), this);
	QPushButton *removeTaskButton = new QPushButton(removeTaskIcon, tr(""),
							this);
	QPushButton *taskSelectorButton = new  QPushButton(taskSelectorIcon,
							   tr(""),
							   this);

	addButton->setToolTip(tr("Add this task to the legend"));
	addButton->setIconSize(addPM.size());

	addTaskButton->setToolTip(tr("Add a unified graph for this task"));
	addTaskButton->setIconSize(addTaskPM.size());

	clearButton->setToolTip(tr("Remove all tasks from the legend"));
	clearButton->setIconSize(clearPM.size());

	findButton->setToolTip(tr(FIND_TOOLTIP));
	findButton->setIconSize(findPM.size());

	removeTaskButton->setToolTip(tr(REMOVE_TASK_TOOLTIP));
	removeTaskButton->setIconSize(removeTaskPM.size());

	taskSelectorButton->setToolTip(tr(TASKSELECTOR_TOOLTIP));
	taskSelectorButton->setIconSize(taskSelectorPM.size());

	nameLine = new QLineEdit(this);
	pidLine = new QLineEdit(this);

	nameLine->setReadOnly(true);
	pidLine->setReadOnly(true);

	layout->addWidget(nameLine);
	layout->addWidget(colonLabel);
	layout->addWidget(pidLine);
	layout->addWidget(taskSelectorButton);
	layout->addWidget(addButton);
	layout->addWidget(clearButton);
	layout->addWidget(findButton);
	layout->addWidget(addTaskButton);
	layout->addWidget(removeTaskButton);
	layout->addStretch();

	tsconnect(addButton, clicked(), this, addToLegendClicked());
	tsconnect(addTaskButton, clicked(), this, addTaskGraphClicked());
	tsconnect(removeTaskButton, clicked(), this, removeTaskGraphClicked());
	tsconnect(clearButton, clicked(), this, clearClicked());
	tsconnect(findButton, clicked(), this, findClicked());
	sigconnect(taskSelectorButton, clicked(), this, requestTaskSelector());
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
	/* taskGraph will be used for displaying the legend in case the user
	 * pushes that button. For that reason we will check if this TaskGraph
	 * has a pointer to another TaskGraph that is to be used for legend
	 * displaying purposes. In practice this happens when a unfied TaskGraph
	 * is set here, because that one might get deleted by user action, it
	 * instead has a pointer to a per CPU TaskGraph */
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

void TaskInfo::addToLegendClicked()
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

	/* We use the legendPidMap to keep track of pids that have been added
	 * to the legend, otherwise it could happen that someone added the same
	 * task multiple times to the legend and that looks a bit crazy. While
	 * QCustomPlot prevents the same the same LegendGraph object being
	 * added twice we can have multiple identical objects, since tasks can
	 * migrate between CPUs */
	if (legendPidMap.contains(task->pid))
		return;

	legendPidMap[task->pid] = graph;
	graph->addToLegend();
	obj = graph->parent();
	plot = qobject_cast<QCustomPlot *>(obj);
	if (plot != nullptr)
		plot->replot();
}


void TaskInfo::clearClicked()
{
	QCustomPlot *plot = nullptr;
	QObject *obj;
	DEFINE_PIDMAP_ITERATOR(iter) = legendPidMap.begin();
	while(iter != legendPidMap.end()) {
		TaskGraph *&graph = iter.value();
		graph->removeFromLegend();
		if (plot == nullptr) {
			obj = graph->parent();
			plot = qobject_cast<QCustomPlot *>(obj);
		}
		iter++;
	}
	if (plot != nullptr)
		plot->replot();
	legendPidMap.clear();
}

void TaskInfo::findClicked()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit findWakeup(task->pid);
}

void TaskInfo::addTaskGraphClicked()
{
	Task *task;

	if (taskGraph == nullptr)
		return;

	task = taskGraph->getTask();
	if (task == nullptr)
		return;

	emit addTaskGraph(task->pid);
}

void TaskInfo::removeTaskGraphClicked()
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
	if (taskGraph->selected())
		return;
	removeTaskGraph();
}

void TaskInfo::pidRemoved(unsigned int pid)
{
	legendPidMap.remove(pid);
}
