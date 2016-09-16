/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstdlib>
#include <cstring>
#include <QTextStream>

#include "misc/tstring.h"
#include "threads/loadbuffer.h"
#include "threads/loadthread.h"

extern "C" {
#include <sys/mman.h>
}

LoadThread::LoadThread(LoadBuffer **buffers, unsigned int nBuf, int myfd,
		       char *map)
	: TThread(QString("LoadThread")), loadBuffers(buffers), nBuffers(nBuf),
	  fd(myfd), mappedFile(map)
{}

void LoadThread::run()
{
	unsigned int i = 0;
	bool eof;
	char *filePos = mappedFile;
	TString lineBegin;
	size_t bufSize = loadBuffers[0]->bufSize;
	int uval, eval;
	QTextStream qout(stdout);

	lineBegin.ptr = (char*) mmap(nullptr, bufSize, PROT_READ|PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (lineBegin.ptr == MAP_FAILED)
		abort();
	lineBegin.len = 0;

	do {
		eof = loadBuffers[i]->produceBuffer(fd, &filePos, &lineBegin);
		i++;
		if (i == nBuffers)
			i = 0;
	} while(!eof);

	uval = munmap(lineBegin.ptr, bufSize);
	eval = errno;
	if (uval != 0) {
		qout << "Warning, error in " << __FUNCTION__
		     <<	"(), munmap() failed because: " << strerror(eval)
		     << "\n";
	}
}
