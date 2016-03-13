/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef INDEXWATCHER_H
#define INDEXWATCHER_H

#include <QMutex>
#include <QWaitCondition>

class IndexWatcher
{
public:
	IndexWatcher(int bSize = 100);
	void setBatchSize(int bSize);
	__always_inline void waitForNextBatch(bool &eof, int &index);
	__always_inline void sendNextIndex(int index);
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

__always_inline void IndexWatcher::waitForNextBatch(bool &eof, int &index)
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

__always_inline void IndexWatcher::sendNextIndex(int index)
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
