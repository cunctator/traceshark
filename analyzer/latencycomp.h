// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2021  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef LATENCYCOMPFUNC_H
#define LATENCYCOMPFUNC_H

#include "analyzer/latency.h"
#include "analyzer/task.h"
#include "analyzer/traceanalyzer.h"

class LatencyCompFunc
{
public:
	LatencyCompFunc(Latency::compare_t cmp,
			Latency::order_t ord,
			TraceAnalyzer *azr) :
		compare(cmp), order(ord), analyzer(azr) {}
	int operator() (const Latency &ll, const Latency &rl) {
		int r;

		if (compare == Latency::CMP_NAME) {
			r = pidToName(ll.pid).compare(pidToName(rl.pid));
			if (r == 0)
				r = ll.place - rl.place;
		} else if (compare == Latency::CMP_TIME) {
			r = ll.time.compare(rl.time);
			if (r == 0)
				r = ll.place - rl.place;
		} else if (compare == Latency::CMP_DELAY) {
			/*
			 * Here we should really compare the delay first but
			 * since that since place has been generated with the
			 * Latency::CMP_CREATE_PLACE below, it is not necessary.
			 */
			r = ll.place - rl.place;
		} else if (compare == Latency::CMP_PLACE) {
			r = ll.place - rl.place;
		} else if (compare == Latency::CMP_CREATE_PLACE) {
			/*
			 * This is only needed for the purpose of sorting the
			 * latency array when we create the place member.
			 */
			r = ll.delay.rcompare(rl.delay);
			if (r == 0) {
				r = ll.time.compare(rl.time);
				if (r == 0)
					r = pidToName(ll.pid).compare(
						pidToName(rl.pid));
			}
		} else {
			return 0;
		}

		if (order == Latency::ORDER_REVERSE)
			r = -r;
		return r;
	}

private:
	const QString &pidToName(int pid) {
		Task *task = analyzer->findTask(pid);

		if (task == nullptr)
			return dummystr;
		else
			return *task->displayName;
	}
	Latency::compare_t compare;
	Latency::order_t order;
	TraceAnalyzer *analyzer;
	static const QString dummystr;
};

#endif
