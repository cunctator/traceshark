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

#include "workqueue.h"
#include <QThread>

#define DEFAULT_NR_CPUS (6) /* Isn't this what most people are running now? */

WorkQueue::WorkQueue():
	error(false)
{
	int cpus;
	cpus = QThread::idealThreadCount();
	nrHostCPUs = cpus > 0 ? cpus:DEFAULT_NR_CPUS;
}

void WorkQueue::addWorkItem(AbstractWorkItem *item) {
	queueMutex.lock();
	queue.append(item);
	queueMutex.unlock();
}

bool WorkQueue::start() {
	int i;
	int qs = queue.size();
	nrThreads = TSMIN(qs, nrHostCPUs);
	threads = new WorkThread<WorkQueue>[nrThreads]();

	for (i = 0; i < nrThreads; i++)
		threads[i].setObjFn(this, &WorkQueue::ThreadRun);

	for (i = 0; i < nrThreads; i++)
		threads[i].start();

	for (i = 0; i < nrThreads; i++)
		threads[i].wait();

	delete[] threads;
	return error;
}

void WorkQueue::ThreadRun() {
	AbstractWorkItem *current;
	bool empty;

	do {
		bool rval = false;
		queueMutex.lock();
		empty = queue.isEmpty();
		if (!empty)
			current = queue.takeFirst();
		queueMutex.unlock();
		if (!empty)
			rval = current->__runWork();
		if (rval) {
			errorMutex.lock();
			error = rval;
			errorMutex.unlock();
		}
	} while(!empty);
}
