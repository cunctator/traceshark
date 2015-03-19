/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl
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

#include "mempool.h"

extern "C" {
#include <sys/mman.h>
}

MemPool::MemPool(unsigned long long size, unsigned long objsize)
{
	poolSize = size;
	objSize = objsize;
	next = NULL;
	if (!newmap()) {
		used = 0;
		poolSize = 0;
	}
}

MemPool::~MemPool()
{
	int i;
	int len = exhaustList.size();
	for (i = 0; i < len; i++) {
		munmap(exhaustList[i], poolSize);
	}
}
