// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef THREADBUFFER_H
#define THREADBUFFER_H

#include <cstdint>

#include <QMutex>
#include <QWaitCondition>
#include "misc/tstring.h"
#include "mm/mempool.h"
#include "threads/loadbuffer.h"
#include "vtl/tlist.h"

/*
 * This class is a load buffer for two threads where one is a producer and the
 * other is a consumer. The synchronization functions have not been designed
 * for scenarios with multiple consumers or producers.
 */
template<class T>
class ThreadBuffer
{
public:
	ThreadBuffer(unsigned int nr);
	~ThreadBuffer();
	unsigned int nrBuffers;
	vtl::TList<T> list;
	MemPool *strPool;
	void beginProduceBuffer();
	void endProduceBuffer();
	void beginConsumeBuffer();
	void endConsumeBuffer();
	LoadBuffer *loadBuffer;
private:
	__always_inline void waitForProductionComplete();
	__always_inline void completeProduction();
	__always_inline void waitForConsumptionComplete();
	__always_inline void completeConsumption();
	bool isEmpty;
	QMutex mutex;
	QWaitCondition consumptionComplete;
	QWaitCondition productionComplete;
};

template<class T>
__always_inline void ThreadBuffer<T>::waitForProductionComplete() {
	mutex.lock();
	while(isEmpty) {
		/*
		 * Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting.
		 */
		productionComplete.wait(&mutex);
	}
}

template<class T>
__always_inline void ThreadBuffer<T>::completeProduction() {
	isEmpty = false;
	productionComplete.wakeOne();
	mutex.unlock();
}

template<class T>
__always_inline void ThreadBuffer<T>::waitForConsumptionComplete() {
	mutex.lock();
	while(!isEmpty) {
		/*
		 * Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting.
		 */
		consumptionComplete.wait(&mutex);
	}
}

template<class T>
__always_inline void ThreadBuffer<T>::completeConsumption() {
	isEmpty = true;
	list.softclear();
	consumptionComplete.wakeOne();
	mutex.unlock();
}

template<class T>ThreadBuffer<T>::ThreadBuffer(unsigned int nr):
nrBuffers(nr), loadBuffer(nullptr), isEmpty(true)
{
	strPool = new MemPool(4096, sizeof(TString));
}

template<class T>
ThreadBuffer<T>::~ThreadBuffer()
{
	delete strPool;
}

/*
 * This should be called from the producer thread before starting to fill a
 * buffer.
 */
template<class T>
void ThreadBuffer<T>::beginProduceBuffer() {
	waitForConsumptionComplete();
	loadBuffer->beginTokenizeBuffer();
	strPool->reset();
	list.softclear();
}

/*
 * This should be called from the data processing thread when the buffer is
 * ready to be consumed.
 */
template<class T>
void ThreadBuffer<T>::endProduceBuffer() {
	loadBuffer->endTokenizeBuffer();
	completeProduction();
}


/*
 * This should be called from the data processing thread
 * before starting to process a buffer.
 */
template<class T>
void ThreadBuffer<T>::beginConsumeBuffer() {
	waitForProductionComplete();
	loadBuffer->beginConsumeBuffer();
}

/*
 * This should be called from the data processing thread
 * when the processing of a buffer has been completed.
 */
template<class T>
void ThreadBuffer<T>::endConsumeBuffer() {
	loadBuffer->endConsumeBuffer();
	completeConsumption();
}

#endif /* THREADBUFFER */
