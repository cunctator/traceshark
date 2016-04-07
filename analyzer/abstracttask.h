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

#ifndef ABSTRACTTASK_H
#define ABSTRACTTASK_H

#include <QVector>

class TaskGraph;

class AbstractTask {
public:
	AbstractTask();
	unsigned int pid; /* is really tid as all other pids here */
	QVector<double> schedTimev;
	QVector<double> schedData;
	QVector<double> scaledSchedData;
	QVector<double> wakeTimev;
	QVector<double> wakeDelay;
	QVector<double> wakeHeight;
	QVector<double> wakeZero;
	QVector<double> runningTimev;
	QVector<double> runningData;
	QVector<double> scaledRunningData;
	bool isNew; /* Only used during extraction */
	/* These are for scaling purposes */
	double offset;
	double scale;
	bool doScale();
	bool doScaleWakeup();
	bool doScaleRunning();
	TaskGraph *graph;
};

#endif /* ABSTRACTTASK_H */
