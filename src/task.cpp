/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "traceanalyzer.h"
#include "traceshark.h"

TaskName::TaskName():
	str(nullptr), prev(nullptr)
{}

Task::Task():
	taskName(nullptr), exitStatus(STATUS_ALIVE), isNew(true), lastWakeUP(0),
	graph(nullptr), wakeUpGraph(nullptr), runningGraph(nullptr)
{}

Task::~Task()
{
	TaskName *prev;
	TaskName *current = taskName;
	while(current != nullptr) {
		prev = current->prev;
		delete current;
		current = prev;
	}
}

void Task::addName(char *name)
{
	TaskName *newName = new TaskName();
	newName->str = name;
	newName->prev = taskName;
	taskName = newName;
}

QString Task::getDisplayName()
{
	QString nameStr;

	if (taskName != nullptr) {
		nameStr += QString(taskName->str);
		if (taskName->prev != nullptr) {
			nameStr += QString("(")
				+ QString(taskName->prev->str)
				+ QString(")");
			if (taskName->prev->prev != nullptr)
				nameStr += QString("...");
		}
	}
	return nameStr;
}

QString Task::getLastName()
{
	QString empty;

	if (taskName != nullptr)
		return QString(taskName->str);
	return empty;
}

/* The member functions below should be moved to a superclass, that do not
 * exist at the time of writing, so that they can be shared with thos in the
 * CPUTask class */

bool Task::doScale() {
	int i;
	int s = schedData.size();
	scaledSchedData.resize(s);
	for (i = 0; i < s; i++)
		scaledSchedData[i] = schedData[i] * scale + offset;
	return false; /* No error */
}

bool Task::doScaleWakeup() {
	int s = wakeDelay.size();
#if 0
	int i;
#endif
	/* Create the dummy vector needed for horizontal display */
	double scaledHeight = WAKEUP_HEIGHT * scale + offset;
	wakeZero.fill(0, s);
	wakeHeight.fill(scaledHeight, s);
	/* Compute a scaled delay vector needed for vertical display */
#if 0
	verticalDelay.resize(s);
	double maxsize = WAKEUP_SIZE * scale;
	double factor = maxsize / WAKEUP_MAX;
	for (i = 0; i < s; i++)
		verticalDelay[i] = TSMIN(factor * wakeDelay[i], maxsize);
#endif

	return false; /* No error */
}

bool Task::doScaleRunning() {
	int i;
	int s = runningData.size();
	scaledRunningData.resize(s);
	for (i = 0; i < s; i++)
		scaledRunningData[i] = runningData[i] * scale + offset;
	return false; /* No error */
}
