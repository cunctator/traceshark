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

#include "loadbuffer.h"
#include "loadthread.h"

LoadThread::LoadThread(LoadBuffer **buffers, unsigned int nBuf, int myfd)
	: loadBuffers(buffers), nBuffers(nBuf), fd(myfd)
{}

void LoadThread::run()
{
	unsigned int i = 0;
	bool eof;
	do {
		eof = loadBuffers[i]->produceBuffer(fd);
		i = (i + 1) % nBuffers;
	} while(!eof);
}
