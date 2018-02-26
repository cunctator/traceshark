/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

TraceFile::TraceFile(char *name, int &ts_errno, unsigned int bsize)
	: mappedFile(nullptr), fileSize(0), bufferSwitch(false), nRead(0),
	  lastBuf(0), lastPos(0), endOfLine(false)
{
	unsigned int i;
	struct stat sbuf;
	bool succ;

	fd = open(name, O_RDONLY);
	succ = fd >= 0;

	ts_errno = succ ? 0:errno;

	if (succ) {
		if (fstat(fd, &sbuf) != 0)
			ts_errno = errno;
		else {
			fileSize = sbuf.st_size;
			mappedFile = (char*) mmap(nullptr, fileSize, PROT_READ,
						  MAP_PRIVATE, fd, 0);
			if (mappedFile == MAP_FAILED) {
				ts_errno = errno;
				mappedFile = nullptr;
			}
		}
	}

	for (i = 0; i < NR_BUFFERS; i++) {
		loadBuffers[i] = new LoadBuffer(bsize);
	}
	loadThread = new LoadThread(loadBuffers, NR_BUFFERS, fd, mappedFile);
	/*
	 * Don't start thread if something failed earlier, we go this far in
	 * order to avoid problems in the destructor
	 */
	if (ts_errno != 0)
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
