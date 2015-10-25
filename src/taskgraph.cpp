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

#include "taskgraph.h"

TaskGraph::TaskGraph(QCPAxis *keyAxis, QCPAxis *valueAxis):
	QCPGraph(keyAxis, valueAxis), task(NULL) {}

void TaskGraph::setTask(CPUTask *newtask)
{
	QString name = QString(newtask->name) + QString(":") +
		QString::number(newtask->pid);
	QCPGraph::setName(name);
	task = newtask;
}	

CPUTask *TaskGraph::getTask()
{
	return task;
}
