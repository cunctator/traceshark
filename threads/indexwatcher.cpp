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

#include "indexwatcher.h"

IndexWatcher::IndexWatcher(int bSize) :
	batchSize(bSize), isEOF(false), postedIndex(0), receivedIndex(0)
{}

void IndexWatcher::setBatchSize(int bSize)
{
	batchSize = bSize;
}

void IndexWatcher::sendEOF()
{
	mutex.lock();
	isEOF = true;
	batchCompleted.wakeAll();
	mutex.unlock();
}

void IndexWatcher::reset()
{
	mutex.lock();
	isEOF = false;
	postedIndex = 0;
	receivedIndex = 0;
	mutex.unlock();
}
