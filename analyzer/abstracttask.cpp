// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2018-2022
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "analyzer/abstracttask.h"
#include "analyzer/traceanalyzer.h"
#include "ui/taskgraph.h"
#include "vtl/tlist.h"

#define SCHED_HEIGHT ((double) 0.5)
#define FLOOR_HEIGHT ((double) 0)

#define ABSTRACT_TASK_TIME_ZERO vtl::Time(0, 6)

AbstractTask::AbstractTask() :
	pid(0), accTime(), accPct(0), cursorTime(), cursorPct(0), isNew(true),
	offset(0), scale(0), graph(nullptr), events(nullptr)
{}

AbstractTask::~AbstractTask()
{
	if (graph != nullptr)
		delete graph;
}

bool AbstractTask::doScale()
{
	int i;
	int s = schedData.size();
	double schedScale = scale * SCHED_HEIGHT;
	scaledSchedData.resize(s);
	for (i = 0; i < s; i++)
		scaledSchedData[i] = schedData.read(i) * schedScale + offset;
	return false; /* No error */
}

bool AbstractTask::doStats()
{
	int startidx, endidx;
	int i;
	vtl::Time start, end;
	vtl::Time startIdxTime, endIdxTime;
	vtl::Time delta;
	int s = schedEventIdx.size();
	unsigned int prevState;
	vtl::Time prevTime;
	vtl::Time t;
	unsigned int state;

	accTime = ABSTRACT_TASK_TIME_ZERO;
	accPct = 0;

	if (s < 1) {
		return false;
	}

	start = startTime;
	end = endTime;
	delta = end - start;

	vtl::Time firstTime = (*events)[schedEventIdx[0]].time;

	if (s < 2) {
		if (firstTime < start) {
			if (schedData.read(0) == SCHED_BIT) {
				accTime = delta;
				accPct = 10000;
			}
			return false;
		}
		if (firstTime > end) {
			return false;
		}
		if (schedData.read(0) == SCHED_BIT) {
			accTime = end - firstTime;
			accPct = (unsigned)
				(10000 * (accTime.toDouble() / delta.toDouble()
					  + 0.00005));
		}
		return false;
	}

	startidx = 0;
	endidx = s - 1;

	startIdxTime = (*events)[schedEventIdx[startidx]].time;
	endIdxTime = (*events)[schedEventIdx[endidx]].time;

	if (startIdxTime >= end || endIdxTime <= start)
		return false;

	if (startIdxTime < start)
		vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d",
			  __FILE__, __LINE__);

	prevTime = start;
	prevState = schedData.read(0);

	/* Todo fix this */
	for (i = startidx; i <= endidx; i++) {
		t = (*events)[schedEventIdx[i]].time;
		state = schedData.read(i);
		if (SCHED_BIT == prevState) {
			accTime += t - prevTime;
		}
		prevTime = t;
		prevState = state;
	}

	if (prevTime < end && prevState == SCHED_BIT) {
		accTime += end - prevTime;
	}
	accPct = (unsigned) (10000 * (accTime.toDouble() / delta.toDouble()
				      + 0.00005));
	return false;
}

bool AbstractTask::doStatsTimeLimited()
{
	int startidx, endidx;
	int i;
	vtl::Time startIdxTime, endIdxTime;
	vtl::Time delta;
	int s = schedEventIdx.size();
	unsigned int prevState;
	vtl::Time prevTime;
	vtl::Time t;
	unsigned int state;

	cursorTime = ABSTRACT_TASK_TIME_ZERO;
	cursorPct = 0;

	if (s < 1) {
		return false;
	}


	const vtl::Time &start = lowerTimeLimit;
	const vtl::Time &end = higherTimeLimit;
	delta = end - start;

	vtl::Time firstTime = (*events)[schedEventIdx[0]].time;

	if (s < 2) {
		if (firstTime < start) {
			if (schedData.read(0) == SCHED_BIT) {
				cursorTime = delta;
				cursorPct = 10000;
			}
			return false;
		}
		if (firstTime > end) {
			return false;
		}
		if (schedData.read(0) == SCHED_BIT) {
			cursorTime = end - firstTime;
			cursorPct = (unsigned)
				(10000 * (cursorTime.toDouble()
					  / delta.toDouble() + 0.00005));
		}
		return false;
	}

	vtl::Time lastTime = (*events)[schedEventIdx[s - 1]].time;


	if (lastTime < start)
		return false;

	if (firstTime > start) {
		/* Normal case, do nothing */
	}

	if (lastTime < end) {
		/* Normal case, Do nothing */
	}

	if (firstTime > end)
		return false;

	startidx = findLower(start);
	endidx = findLower(end);

	startIdxTime = (*events)[schedEventIdx[startidx]].time;

	if (startIdxTime >= end)
		return false;

	if (startIdxTime < start)
		prevTime = start;
	else
		prevTime = startIdxTime;
	prevState = schedData.read(startidx);

	for (i = startidx + 1; i <= endidx; i++) {
		t = (*events)[schedEventIdx[i]].time;
		state = schedData.read(i);
		if (SCHED_BIT == prevState) {
			cursorTime += t - prevTime;
		}
		prevTime = t;
		prevState = state;
	}

	if (prevTime < end && prevState == SCHED_BIT) {
		cursorTime += end - prevTime;
	}
	cursorPct = (unsigned) (10000 * (cursorTime.toDouble()
					 / delta.toDouble() + 0.00005));
	return false;
}

bool AbstractTask::doScaleRunning()
{
	fillDataVector(runningTimev, scaledRunningData, nullptr,
		       FLOOR_HEIGHT);
	return false; /* No error */
}

bool AbstractTask::doScalePreempted()
{
	fillDataVector(preemptedTimev, scaledPreemptedData, nullptr,
		       FLOOR_HEIGHT);
	return false; /* No error */
}

bool AbstractTask::doScaleUnint()
{
	fillDataVector(uninterruptibleTimev, scaledUninterruptibleData,
		       nullptr,  FLOOR_HEIGHT);
	return false; /* No error */
}

bool AbstractTask::doScaleDelay()
{
	fillDataVector(delay, delayHeight, &delayZero, DELAY_HEIGHT);
	fillDataVector(wakeDelay, wakeHeight, &wakeZero, DELAY_HEIGHT);
	return false; /* No error */
}

void  AbstractTask::fillDataVector(QVector<double> &timev,
				   QVector<double> &data,
				   QVector<double> *zerov,
				   double height)
{
	int s = timev.size();
	double scaledHeight = height * scale + offset;
	data.fill(scaledHeight, s);
	if (zerov != nullptr)
		zerov->fill(0, s);
}

void AbstractTask::setCursorTime(enum TShark::CursorIdx cursor,
				 const vtl::Time &time)
{
	cursorValues[cursor] = time;
	const vtl::Time &time1 = cursorValues[TShark::RED_CURSOR];
	const vtl::Time &time2 = cursorValues[TShark::BLUE_CURSOR];

	if (time1 < time2) {
		lowerTimeLimit = time1;
		higherTimeLimit = time2;
	} else {
		lowerTimeLimit = time2;
		higherTimeLimit = time1;
	}
}

void AbstractTask::setStartTime(const vtl::Time &time)
{
	startTime = time;
}

void AbstractTask::setEndTime(const vtl::Time &time)
{
	endTime = time;
}

int AbstractTask::binarySearch_(const vtl::Time &time, int lowerIdx,
				int higherIdx)
{
	int pivot = (lowerIdx + higherIdx) / 2;
	int width = higherIdx - lowerIdx;
	const vtl::Time &pTime = (*events)[schedEventIdx[pivot]].time;
	bool pSmaller = pTime < time;

	if (width < 2) {
		return pSmaller ? higherIdx : lowerIdx;
	}

	lowerIdx = pSmaller ? pivot : lowerIdx;
	higherIdx = pSmaller ? higherIdx : pivot;

	return binarySearch_(time, lowerIdx, higherIdx);
}

int AbstractTask::binarySearch(const vtl::Time &time)
{
	int s = schedEventIdx.size();

	return binarySearch_(time, 0, s - 1);
}


int AbstractTask::findLower(const vtl::Time &time)
{
	int idxmax = schedEventIdx.size() - 1;
	int idx = binarySearch_(time, 0, idxmax);

	vtl::Time idxtime = (*events)[schedEventIdx[idx]].time;
	/*
	 * In normal circumstances this could only do one loop iteration but
	 * if we have many identical timestamps, we could end up in a situation
	 * where we need to do several. In other words, we assume here that the
	 * clock is possibly not always strictly monotonic, even if it probably
	 * is that in practice
	 */
	while (idxtime > time && idx > 0) {
		idx--;
		idxtime = (*events)[schedEventIdx[idx]].time;
	}
	return idx;
}

int AbstractTask::findHigher(const vtl::Time &time)
{
	int idxmax = schedEventIdx.size() - 1;
	int idx = binarySearch_(time, 0, idxmax);

	vtl::Time idxtime = (*events)[schedEventIdx[idx]].time;
	/*
	 * In normal circumstances this could only do one loop iteration but
	 * if we have many identical timestamps, we could end up in a situation
	 * where we need to do several. In other words, we assume here that the
	 * clock is possibly not always strictly monotonic, even if it probably
	 * is that in practice
	 */
	while (idxtime < time && idx < idxmax) {
		idx++;
		idxtime = (*events)[schedEventIdx[idx]].time;
	}
	return idx;
}

vtl::Time AbstractTask::startTime;
vtl::Time AbstractTask::endTime;
vtl::Time AbstractTask::lowerTimeLimit;
vtl::Time AbstractTask::higherTimeLimit;
vtl::Time AbstractTask::cursorValues[TShark::NR_CURSORS];
