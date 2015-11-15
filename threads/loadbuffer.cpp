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

#include "loadbuffer.h"

extern "C" {
#include <unistd.h>
}

LoadBuffer::LoadBuffer(char *buf, unsigned int size):
	buffer(buf), bufSize(size) {
	/* Prevent consumer from consuming empty newly created buffer */
	productionComplete.lock();
}

LoadBuffer::~LoadBuffer()
{
	productionComplete.unlock();
}

/* This function should be called from the IO thread 
 * until the function returns true */
bool LoadBuffer::produceBuffer(int fd) {
	waitForConsumptionComplete();
	nRead = read(fd, buffer, bufSize);
	completeProduction();
	if (nRead <= 0)
		return true;
	return false;
}

/* This should be called from the data processing thread
 * before starting to process a buffer */
bool LoadBuffer::beginConsumeBuffer() {
	waitForProductionComplete();
	if (nRead <= 0)
		return true;
	return false;
}

/* This should be called from the data processing thread
 * when the processing of a buffer has been completed */
void LoadBuffer::endConsumeBuffer() {
	completeConsumption();
}
