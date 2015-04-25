/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2014-2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "tracefile.h"
#include "traceline.h"
#include "threads/loadthread.h"
#include "mm/mempool.h"
#include <QtGlobal>
#include <new>

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
}

TraceFile::TraceFile(char *name, bool &ok, unsigned int bsize)
{
	unsigned int i;
	char *m;
	fd = open(name, O_RDONLY);
	if (fd >= 0)
		ok = true;
	else
		ok = false;
	lastPos = 0;
	lastBuf = 0;
	eof = false;
	strPool = new MemPool(2048, 1);
	ptrPool = new MemPool(256, sizeof(TString));
	memory = new char[NR_BUFFERS * bsize];
	m = memory;
	for (i = 0; i < NR_BUFFERS; i++) {
		buffers[i] = new LoadBuffer(m, bsize);
		m += bsize;
	}
	loadThread = new LoadThread(buffers, NR_BUFFERS, fd);
	if (fd < 0) /* Don't start thread if open() failed earlier, we go*/
		return; /* this far in order to avoid problems in destructor */
	loadThread->start();
	eof = buffers[0]->beginConsumeBuffer();
	nRead = buffers[0]->nRead;
}

TraceFile::~TraceFile()
{
	unsigned int i;
	loadThread->wait();
	delete loadThread;
	delete[] memory;
	delete strPool;
	delete ptrPool;
	for (i = 0; i < NR_BUFFERS; i++) {
		delete buffers[i];
	}
}
