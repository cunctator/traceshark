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

#ifndef TASK_H
#define TASK_H

#include <QVector>

class Task {
public:
	Task(): isNew(true) {}
	char *name;
	unsigned int pid; /* is really tid as all other pids here */
	QVector<double> timev;
	QVector<double> data;
	QVector<double> scaledData;
	QVector<double> wakeTimev;
	QVector<double> wakeData;
	QVector<double> scaledWakeData;
	QVector<double> runningTimev;
	QVector<double> runningData;
	QVector<double> scaledRunningData;
	bool isNew; /* Only used during extraction */
	double lastWakeUP;        /* Only used during extraction */
	double offset;
	double scale;
	bool doScale();
};

#endif
