// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef LOADBUFFER_H
#define LOADBUFFER_H

#include <cstdint>

#include <QMutex>
#include <QWaitCondition>

extern "C" {
#include <unistd.h>
}

#include "vtl/compiler.h"

class TString;

/*
 * This class is a load buffer for three threads where one is a loader, i.e.
 * IO thread, and the second is a tokenizer, and the third is a consumer, which
 * probably is a grammar processing thread. The synchronization functions have
 * not been designed for scenarios with more than one thread per category
 */
class LoadBuffer
{
public:
	LoadBuffer(unsigned int size);
	~LoadBuffer();
	char *buffer;
	char *memory;
	char *readBegin;
	size_t bufSize;
	size_t nRead;
	int64_t filePos;
	bool IOerror;
	int IOerrno;
	bool produceBuffer(int fd, int64_t *filePosPtr, TString *lineBegin);
	void beginProduceBuffer();
	void endProduceBuffer();
	void beginTokenizeBuffer();
	void endTokenizeBuffer();
	void beginConsumeBuffer();
	void endConsumeBuffer();
	vtl_always_inline bool isEOF() const;
private:
	vtl_always_inline void waitForLoadingComplete();
	vtl_always_inline void completeLoading();
	vtl_always_inline void waitForTokenizationComplete();
	vtl_always_inline void completeTokenization();
	vtl_always_inline void waitForConsumptionComplete();
	vtl_always_inline void completeConsumption();
	typedef enum : int {
		LOADSTATE_EMPTY = 0,
		LOADSTATE_LOADED,
		LOADSTATE_TOKENIZED
	} loadbufferstate_t;
	loadbufferstate_t state;
	QMutex mutex;
	QWaitCondition consumptionComplete;
	QWaitCondition loadingComplete;
	QWaitCondition parsingComplete;
	bool eof;
};

vtl_always_inline void LoadBuffer::waitForLoadingComplete() {
	mutex.lock();
	while(state != LOADSTATE_LOADED) {
		/*
		 * Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting.
		 */
		loadingComplete.wait(&mutex);
	}
}

vtl_always_inline void LoadBuffer::completeLoading() {
	state = LOADSTATE_LOADED;
	loadingComplete.wakeOne();
	mutex.unlock();
}

vtl_always_inline void LoadBuffer::waitForTokenizationComplete() {
	mutex.lock();
	while(state != LOADSTATE_TOKENIZED) {
		/*
		 * Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting.
		 */
		parsingComplete.wait(&mutex);
	}
}

vtl_always_inline void LoadBuffer::completeTokenization() {
	state = LOADSTATE_TOKENIZED;
	parsingComplete.wakeOne();
	mutex.unlock();
}


vtl_always_inline void LoadBuffer::waitForConsumptionComplete() {
	mutex.lock();
	while(state != LOADSTATE_EMPTY) {
		/*
		 * Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting.
		 */
		consumptionComplete.wait(&mutex);
	}
}

vtl_always_inline void LoadBuffer::completeConsumption() {
	state = LOADSTATE_EMPTY;
	consumptionComplete.wakeOne();
	mutex.unlock();
}

vtl_always_inline bool LoadBuffer::isEOF() const {
	return eof;
}

#endif /* LOADBUFFER */
