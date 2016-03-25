/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
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

#include "cputask.h"
#include "traceanalyzer.h"

/* This delays (20 ms) rerpresents the "full length" of the error */
#define WAKEUP_MAX ((double) 0.020)

bool CPUTask::doScale() {
	int i;
	int s = schedData.size();
	scaledSchedData.resize(s);
	for (i = 0; i < s; i++)
		scaledSchedData[i] = schedData[i] * scale + offset;
	return false; /* No error */
}

bool CPUTask::doScaleWakeup() {
	int s = wakeDelay.size();
	int i;
	/* Create the dummy vector needed for horizontal display */
	double scaledHeight = WAKEUP_HEIGHT * scale + offset;
	wakeZero.fill(0, s);
	wakeHeight.fill(scaledHeight, s);
	/* Compute a scaled delay vector needed for vertical display */
	verticalDelay.resize(s);
	double maxsize = WAKEUP_SIZE * scale;
	double factor = maxsize / WAKEUP_MAX;
	for (i = 0; i < s; i++)
		verticalDelay[i] = TSMIN(factor * wakeDelay[i], maxsize);

	return false; /* No error */
}

bool CPUTask::doScaleRunning() {
	int i;
	int s = runningData.size();
	scaledRunningData.resize(s);
	for (i = 0; i < s; i++)
		scaledRunningData[i] = runningData[i] * scale + offset;
	return false; /* No error */
}
