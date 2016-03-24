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

#include "task.h"
#include "taskgraph.h"
#include "taskinfo.h"
#include "traceshark.h"
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
#define CLEAR_LEGEND_RESOURCE ":/traceshark/images/clearlegend30x30.png"
#define FIND_WAKEUP_RESOURCE ":/traceshark/images/wakeup30x30.png"

TaskInfo::TaskInfo(QWidget *parent):
	QWidget(parent), taskGraph(NULL)
{
	QPixmap addPM(QLatin1String(ADD_TO_LEGEND_RESOURCE));
	QPixmap clearPM(QLatin1String(CLEAR_LEGEND_RESOURCE));
	QPixmap findPM(QLatin1String(FIND_WAKEUP_RESOURCE));
	QIcon findIcon(findPM);
	QIcon addIcon(addPM);
	QIcon clearIcon(clearPM);
	QHBoxLayout *layout  = new QHBoxLayout(this);
	QLabel *colonLabel = new QLabel(tr(":"));
	QPushButton *addButton = new QPushButton(addIcon, tr(""), this);
	QPushButton *clearButton = new QPushButton(clearIcon, tr(""), this);
	QPushButton *findButton = new QPushButton(findIcon, tr(""), this);

	addButton->setToolTip(tr("Add to legend"));
	addButton->setIconSize(addPM.size());

	clearButton->setToolTip(tr("Clear the legend"));
	clearButton->setIconSize(clearPM.size());

	findButton->setToolTip(tr("Find previous wakeup"));
	findButton->setIconSize(findPM.size());

	nameLine = new QLineEdit(this);
	pidLine = new QLineEdit(this);

	nameLine->setReadOnly(true);
	pidLine->setReadOnly(true);

	layout->addWidget(nameLine);
	layout->addWidget(colonLabel);
	layout->addWidget(pidLine);
	layout->addWidget(addButton);
	layout->addWidget(clearButton);
	layout->addWidget(findButton);
	layout->addStretch();

	tsconnect(addButton, clicked(), this, addClicked());
	tsconnect(clearButton, clicked(), this, clearClicked());
	tsconnect(findButton, clicked(), this, findClicked());
}

TaskInfo::~TaskInfo()
{
}


void TaskInfo::setTaskGraph(TaskGraph *graph)
{
	Task *task = graph->getTask();
	if (task == NULL)
		return;
	QString nameStr = task->getLastName();
	QString pidStr = QString::number(task->pid);
	nameLine->setText(nameStr);
	pidLine->setText(pidStr);
	taskGraph = graph;
}

void TaskInfo::removeTaskGraph()
{
	taskGraph = NULL;
	nameLine->setText(tr(""));
	pidLine->setText(tr(""));
}

void TaskInfo::clear()
{
	removeTaskGraph();
	legendPidMap.clear();
}

void TaskInfo::addClicked()
{
	QObject *obj;
	QCustomPlot *plot;
	Task *task;

	if (taskGraph == NULL)
		return;

	task = taskGraph->getTask();
	if (task == NULL)
		return;

	/* We use the legendPidMap to keep track of pids that have been added
	 * to the legend, otherwise it could happen that someone added the same
	 * task multiple times to the legend and that looks a bit crazy. While
	 * QCustomPlot prevents the same the same LegendGraph object being
	 * added twice we can have multiple identical objects, since tasks can
	 * migrate between CPUs */
	if (legendPidMap.contains(task->pid))
		return;

	legendPidMap[task->pid] = taskGraph;
	taskGraph->addToLegend();
	obj = taskGraph->parent();
	plot = qobject_cast<QCustomPlot *>(obj);
	if (plot != NULL)
		plot->replot();
}

void TaskInfo::clearClicked()
{
	QCustomPlot *plot = NULL;
	QObject *obj;
	DEFINE_PIDMAP_ITERATOR(iter) = legendPidMap.begin();
	while(iter != legendPidMap.end()) {
		TaskGraph *&graph = iter.value();
		graph->removeFromLegend();
		if (plot == NULL) {
			obj = graph->parent();
			plot = qobject_cast<QCustomPlot *>(obj);
		}
		iter++;
	}
	if (plot != NULL)
		plot->replot();
	legendPidMap.clear();
}

void TaskInfo::findClicked()
{
	Task *task;

	if (taskGraph == NULL)
		return;

	task = taskGraph->getTask();
	if (task == NULL)
		return;

	emit findWakeup(task->pid);
}

void TaskInfo::checkGraphSelection()
{
	if (taskGraph == NULL)
		return;
	if (taskGraph->selected())
		return;
	removeTaskGraph();
}

void TaskInfo::pidRemoved(unsigned int pid)
{
	legendPidMap.remove(pid);
}
