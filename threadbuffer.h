/*
 * Traceshark - a visualizer for visualizing ftrace traces
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

#ifndef THREADBUFFER_H
#define THREADBUFFER_H

#include <QMutex>

/* This class is a load buffer for two threads where one is a producer and the
 * other is a consumer.
 */
template<class T>
class ThreadBuffer
{
public:
	ThreadBuffer(unsigned int bufsize, unsigned int nr);
	unsigned int bufSize;
	unsigned int nrBuffers;
	unsigned int nRead;
	T *buffer;
	void beginProduceBuffer();
	void endProduceBuffer(unsigned int s);
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

template<class T>
__always_inline void ThreadBuffer<T>::waitForProductionComplete() {
	productionComplete.lock();
}

template<class T>
__always_inline void ThreadBuffer<T>::completeProduction() {
	productionComplete.unlock();
}

template<class T>
__always_inline void ThreadBuffer<T>::waitForConsumptionComplete() {
	consumptionComplete.lock();
}

template<class T>
__always_inline void ThreadBuffer<T>::completeConsumption() {
	consumptionComplete.unlock();
}

template<class T>
ThreadBuffer<T>::ThreadBuffer(unsigned int bufsize, unsigned int nr):
	bufSize(bufsize), nrBuffers(nr), nRead(0)
{
	buffer = new T[bufsize];
	/* Prevent consumer from consuming empty newly created buffer */
	productionComplete.lock();
}

/* This should be called from the producer thread
 * before starting to fill a buffer */
template<class T>
void ThreadBuffer<T>::beginProduceBuffer() {
	waitForConsumptionComplete();
}

/* This should be called from the data processing thread
 * when the butter is ready to be consumed  */
template<class T>
void ThreadBuffer<T>::endProduceBuffer(unsigned int s) {
	nRead = s;
	completeProduction();
}


/* This should be called from the data processing thread
 * before starting to process a buffer */
template<class T>
bool ThreadBuffer<T>::beginConsumeBuffer() {
	waitForProductionComplete();
	if (nRead == 0)
		return true;
	return false;
}

/* This should be called from the data processing thread
 * when the processing of a buffer has been completed */
template<class T>
void ThreadBuffer<T>::endConsumeBuffer() {
	completeConsumption();
}

#endif /* THREADBUFFER */
