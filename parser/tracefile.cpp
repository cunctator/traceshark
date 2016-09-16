/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "parser/tracefile.h"
#include "parser/traceline.h"
#include "threads/loadthread.h"
#include "mm/mempool.h"
#include <QtGlobal>
#include <new>

extern "C" {
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
}

TraceFile::TraceFile(char *name, bool &ok, unsigned int bsize)
	: mappedFile(nullptr), fileSize(0), bufferSwitch(false), nRead(0),
	  lastBuf(0), lastPos(0), endOfLine(false)
{
	unsigned int i;
	struct stat sbuf;

	fd = open(name, O_RDONLY);
	if (fd >= 0)
		ok = true;
	else
		ok = false;

	if (ok) {
		if (fstat(fd, &sbuf) != 0)
			ok = false;
		else {
			fileSize = sbuf.st_size;
			mappedFile = (char*) mmap(nullptr, fileSize, PROT_READ,
						  MAP_PRIVATE, fd, 0);
			if (mappedFile == MAP_FAILED) {
				ok = false;
				mappedFile = nullptr;
			}
		}
	}

	for (i = 0; i < NR_BUFFERS; i++) {
		loadBuffers[i] = new LoadBuffer(bsize);
	}
	loadThread = new LoadThread(loadBuffers, NR_BUFFERS, fd, mappedFile);
	/* Don't start thread if something failed earlier, we go this far in
	 * order to avoid problems in the destructor */
	if (!ok)
		return;
	loadThread->start();
}

TraceFile::~TraceFile()
{
	unsigned int i;
	loadThread->wait();
	delete loadThread;
	for (i = 0; i < NR_BUFFERS; i++)
		delete loadBuffers[i];
	if (mappedFile != nullptr)
		munmap(mappedFile, fileSize);
}
