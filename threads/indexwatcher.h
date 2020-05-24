// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016, 2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef INDEXWATCHER_H
#define INDEXWATCHER_H

#include <QMutex>
#include <QWaitCondition>

#include "vtl/compiler.h"

class IndexWatcher
{
public:
	IndexWatcher(int bSize = 100);
	void setBatchSize(int bSize);
	vtl_always_inline void waitForNextBatch(bool &eof, int &index);
	vtl_always_inline void sendNextIndex(int index);
	void sendEOF();
	void reset();
private:
	int batchSize;
	bool isEOF;
	/* This is the highest index posted by the producer */
	int postedIndex;
	/* This is the higher index being received by the consumer */
	int receivedIndex;
	QMutex mutex;
	QWaitCondition batchCompleted;
};

vtl_always_inline void IndexWatcher::waitForNextBatch(bool &eof, int &index)
{
	mutex.lock();
	while(!isEOF && postedIndex - receivedIndex < batchSize) {
		batchCompleted.wait(&mutex);
	}
	receivedIndex = postedIndex;
	index = postedIndex;
	eof = isEOF;
	mutex.unlock();
}

vtl_always_inline void IndexWatcher::sendNextIndex(int index)
{
	mutex.lock();
	if (index <= postedIndex)
		goto out;
	postedIndex = index;
	if (postedIndex - receivedIndex >= batchSize)
		batchCompleted.wakeAll();
out:
	mutex.unlock();
}

#endif /* INDEXWATCHER_H */
