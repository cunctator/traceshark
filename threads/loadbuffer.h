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

#ifndef LOADBUFFER_H
#define LOADBUFFER_H

#include <QMutex>
#include <QWaitCondition>

extern "C" {
#include <unistd.h>
}

class TString;

/* This class is a load buffer for three threads where one is a loader, i.e.
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
	char *filePos;
	bool IOerror;
	int IOerrno;
	bool produceBuffer(int fd, char ** filePosPtr, TString *lineBegin);
	void beginProduceBuffer();
	void endProduceBuffer();
	void beginTokenizeBuffer();
	void endTokenizeBuffer();
	void beginConsumeBuffer();
	void endConsumeBuffer();
	__always_inline bool isEOF() const;
private:
	__always_inline void waitForLoadingComplete();
	__always_inline void completeLoading();
	__always_inline void waitForTokenizationComplete();
	__always_inline void completeTokenization();
	__always_inline void waitForConsumptionComplete();
	__always_inline void completeConsumption();
	typedef enum {
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

__always_inline void LoadBuffer::waitForLoadingComplete() {
	mutex.lock();
	while(state != LOADSTATE_LOADED) {
		/* Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting */
		loadingComplete.wait(&mutex);
	}
}

__always_inline void LoadBuffer::completeLoading() {
	state = LOADSTATE_LOADED;
	loadingComplete.wakeOne();
	mutex.unlock();
}

__always_inline void LoadBuffer::waitForTokenizationComplete() {
	mutex.lock();
	while(state != LOADSTATE_TOKENIZED) {
		/* Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting */
		parsingComplete.wait(&mutex);
	}
}

__always_inline void LoadBuffer::completeTokenization() {
	state = LOADSTATE_TOKENIZED;
	parsingComplete.wakeOne();
	mutex.unlock();
}


__always_inline void LoadBuffer::waitForConsumptionComplete() {
	mutex.lock();
	while(state != LOADSTATE_EMPTY) {
		/* Note that this implicitely unlocks the mutex while waiting
		 * and relocks it when done waiting */
		consumptionComplete.wait(&mutex);
	}
}

__always_inline void LoadBuffer::completeConsumption() {
	state = LOADSTATE_EMPTY;
	consumptionComplete.wakeOne();
	mutex.unlock();
}

__always_inline bool LoadBuffer::isEOF() const {
	return eof;
}

#endif /* LOADBUFFER */
