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

#ifndef LOADBUFFER_H
#define LOADBUFFER_H

#include <QMutex>

extern "C" {
#include <unistd.h>
}

/* This class is a load buffer for two threads where one is a producer, i.e.
 * IO thread, and the other is a consumer, i.e. data processing thread.
 */
class LoadBuffer
{
public:
	LoadBuffer(char *buf, unsigned int size);
	~LoadBuffer();
	char *buffer;
	size_t bufSize;
	ssize_t nRead;
	bool produceBuffer(int fd);
	bool beginConsumeBuffer();
	void endConsumeBuffer();
private:
	__always_inline void waitForProductionComplete();
	__always_inline void completeProduction();
	__always_inline void waitForConsumptionComplete();
	__always_inline void completeConsumption();
	QMutex consumptionComplete; /* I hate when userspace polls a mutex */
	QMutex productionComplete;  /* so let's use two of them */
};

__always_inline void LoadBuffer::waitForProductionComplete() {
	productionComplete.lock();
}

__always_inline void LoadBuffer::completeProduction() {
	productionComplete.unlock();
}

__always_inline void LoadBuffer::waitForConsumptionComplete() {
	consumptionComplete.lock();
}

__always_inline void LoadBuffer::completeConsumption() {
	consumptionComplete.unlock();
}

#endif /* LOADBUFFER */
