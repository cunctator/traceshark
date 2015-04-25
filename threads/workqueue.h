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

#ifndef WORKQUEUE_H
#define WORKQUEUE_H

#include <QList>
#include <QMutex>
#include "workitem.h"
#include "workthread.h"

class WorkQueue {
	friend class WorkThread<WorkQueue>;
public:
	WorkQueue();
	~WorkQueue();
	void addWorkItem(AbstractWorkItem *item);
	void addDefaultWorkItem(AbstractWorkItem *item);
	void setWorkItemsDefault();
	bool start();
protected:
	void ThreadRun();
private:
	QList<AbstractWorkItem*> queue;
	QList<AbstractWorkItem*> defaultQueue;
	QMutex queueMutex;
	QMutex errorMutex;
	bool error;
	WorkThread<WorkQueue> *threads;
	int nrThreads;
};

#endif /* WORKQUEUE_H */
