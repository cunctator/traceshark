// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef ABSTRACTTASK_H
#define ABSTRACTTASK_H

#include <QVector>
#include "vtl/bitvector.h"

#include "vtl/time.h"
#include "misc/traceshark.h"

class TaskGraph;
class TraceEvent;
class TraceAnalyzer;

#define SCHED_BIT 0x1
#define FLOOR_BIT 0x0

namespace vtl {
	template<class T> class TList;
}

class AbstractTask {
	friend class StatsModel;
	friend class StatsLimitedModel;
	friend class TraceAnalyzer;
public:
	AbstractTask();
	~AbstractTask();

	/* is really tid as all other pids here */
	int pid;

	QVector<double> schedTimev;
	QVector<int>    schedEventIdx;
	vtl::BitVector  schedData;
	QVector<double> scaledSchedData;
	QVector<double> wakeTimev;
	QVector<double> wakeDelay;
	QVector<double> wakeHeight;
	QVector<double> wakeZero;
	QVector<double> preemptedTimev;
	QVector<double> runningTimev;
	QVector<double> uninterruptibleTimev;
	QVector<double> scaledPreemptedData;
	QVector<double> scaledRunningData;
	QVector<double> scaledUninterruptibleData;

	vtl::Time accTime;             /* Total time consumption        */
	unsigned  accPct;              /* Percentage of the above       */
	vtl::Time cursorTime;          /* Consumed time between cursors */
	unsigned  cursorPct;           /* Percentage of the above       */

	/* Only used during extraction */
	bool isNew;

	/* These are for scaling purposes */
	double offset;
	double scale;

	bool doScale();
	bool doStats();
	bool doStatsTimeLimited();
	bool doScaleWakeup();
	bool doScaleRunning();
	bool doScalePreempted();
	bool doScaleUnint();

	static void setCursorTime(enum TShark::CursorIdx cursor,
				  const vtl::Time &time);
	static void setStartTime(const vtl::Time &time);
	static void setEndTime(const vtl::Time &time);

	TaskGraph *graph;

private:
	int _binarySearch(const vtl::Time &time, int lowerIdx,
					  int higherIdx);
	__always_inline int binarySearch(const vtl::Time &time);
	int findLower(const vtl::Time &time);
	int findHigher(const vtl::Time &time);
	bool fillDataVector(QVector<double> &timev, QVector<double> &data,
			    QVector<double> *zerov, double height);
protected:
	static vtl::Time lowerTimeLimit;
	static vtl::Time higherTimeLimit;
	static vtl::Time startTime;
	static vtl::Time endTime;
	static vtl::Time cursorValues[];
	vtl::TList<TraceEvent> *events;
};

#endif /* ABSTRACTTASK_H */

