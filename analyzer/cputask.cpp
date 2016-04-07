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

#include "analyzer/cputask.h"
#include "analyzer/traceanalyzer.h"

/* This delays (20 ms) rerpresents the "full length" of the error */
#define WAKEUP_MAX ((double) 0.020)

CPUTask::CPUTask() :
	AbstractTask()
{}

bool CPUTask::doScaleWakeup() {
	int s = wakeDelay.size();
	int i;
	/* Compute a scaled delay vector needed for vertical display */
	verticalDelay.resize(s);
	double maxsize = WAKEUP_SIZE * scale;
	double factor = maxsize / WAKEUP_MAX;
	for (i = 0; i < s; i++)
		verticalDelay[i] = TSMIN(factor * wakeDelay[i], maxsize);

	return AbstractTask::doScaleWakeup();
}
