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
#include "loadthread.h"

LoadThread::LoadThread(LoadBuffer **buffers, unsigned int nBuf, int myfd,
		       char *map)
	: loadBuffers(buffers), nBuffers(nBuf), fd(myfd), mappedFile(map)
{}

void LoadThread::run()
{
	unsigned int i = 0;
	bool eof;
	char *filePos = mappedFile;

	do {
		eof = loadBuffers[i]->produceBuffer(fd, &filePos);
		i++;
		if (i == nBuffers)
			i = 0;
	} while(!eof);
}
