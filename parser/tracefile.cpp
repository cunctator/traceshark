// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
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
#include "misc/chunk.h"
#include "misc/errors.h"
#include "vtl/compiler.h"
#include "vtl/error.h"
#include <QtGlobal>
#include <new>

extern "C" {
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
}

vtl_always_inline static int clib_close(int fd)
{
	return close(fd);
}

TraceFile::TraceFile(char *name, int &ts_errno, unsigned int bsize)
	: fd_is_open(false), bufferSwitch(false), nRead(0), lastBuf(0),
	  lastPos(0), endOfLine(false), mappedFile(nullptr), fileSize(0)
{
	unsigned int i;

	fd = open(name, O_RDONLY);
	if (fd >= 0) {
		fd_is_open = true;
		fileInfo.saveStat(fd, &ts_errno);
		fileSize = fileInfo.getFileSize();
	} else {
		if (errno != 0)
			ts_errno = errno;
		else
			ts_errno = - TS_ERROR_ERROR;
	}

	for (i = 0; i < NR_BUFFERS; i++) {
		loadBuffers[i] = new LoadBuffer(bsize);
	}
	loadThread = new LoadThread(loadBuffers, NR_BUFFERS, fd);
	/*
	 * Don't start thread if something failed earlier, we go this far in
	 * order to avoid problems in the destructor
	 */
	buffer = (char *) mmap(nullptr, BUFFER_SIZE, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (buffer == MAP_FAILED)
		mmap_err();
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
	if (munmap(buffer, BUFFER_SIZE) != 0)
		munmap_err();
}

void TraceFile::close(int *ts_errno)
{
	*ts_errno = 0;
	freeMmap();
	if (!fd_is_open)
		return;
	fd_is_open = false;
	if (clib_close(fd) != 0) {
		if (errno != 0)
			*ts_errno = errno;
		else
			*ts_errno = - TS_ERROR_ERROR;
	}
}

bool TraceFile::isIntact(int *ts_errno)
{
	bool intact;

	intact = fileInfo.cmpStat(fd, ts_errno);
	if (*ts_errno != 0)
		return false;

	return intact;
}

bool TraceFile::allocMmap()
{
	mappedFile = (char*) mmap(nullptr, fileSize, PROT_READ,
				  MAP_PRIVATE, fd, 0);
	if (mappedFile == MAP_FAILED) {
		/*
		 * It is normal for this mmap() to fail on 32-bit platforms, in
		 * that case we record the failure by setting mappedFile to
		 * nullptr, then readChunk() and getChunkArray() will use
		 * the fallback readChunk_() and getChunkArray_() functions,
		 * which uses lseek64() and read() to obtain the desired chunks.
		 */
		mappedFile = nullptr;
		return false;
	}
	return true;
}

void TraceFile::freeMmap()
{
	if (mappedFile == nullptr)
		return;
	if (munmap(mappedFile, fileSize) != 0)
		munmap_err();
	mappedFile = nullptr;
}

void TraceFile::readChunk(const Chunk *chunk, char *buf, int size,
			  int *ts_errno)
{
	int64_t s;
	if (mappedFile == nullptr) {
		readChunk_(chunk, buf, size, ts_errno);
		return;
	}
	s = TSMIN(size, chunk->len);
	if (chunk->offset + s > fileSize) {
		*ts_errno = - TS_ERROR_EOF;
		return;
	}
	strncpy(buf, mappedFile + chunk->offset, chunk->len);
}

QByteArray TraceFile::getChunkArray(const Chunk *chunk, int *ts_errno)
{
	char *buf;
	QByteArray rval;

	if (mappedFile == nullptr) {
		return getChunkArray_(chunk, ts_errno);
	}

	if (chunk->offset + chunk->len > fileSize) {
		*ts_errno = - TS_ERROR_EOF;
		return rval;
	}

	if (chunk->len <= BUFFER_SIZE) {
		buf = buffer;
	} else {
		buf = new char[chunk->len];
	}

	strncpy(buf, mappedFile + chunk->offset, chunk->len);
	rval = QByteArray(buf, chunk->len);

	if (buf != buffer) {
		delete[] buf;
	}

	return rval;
}
