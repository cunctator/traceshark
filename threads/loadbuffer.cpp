// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <new>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include "misc/tstring.h"
#include "threads/loadbuffer.h"
#include "vtl/error.h"

extern "C" {
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
}

LoadBuffer::LoadBuffer(unsigned int size):
	buffer(nullptr), bufSize(size), nRead(0), filePos(0),
	IOerror(false), IOerrno(0), state(LOADSTATE_EMPTY), eof(false)
{
	/*
	 * We need the extra byte to be able to set a null character in
	 * TraceFile::ReadNextWord() one byte out of bounds.
	 */
	memory = (char*) mmap(nullptr, 2 * size + 1, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == MAP_FAILED)
		mmap_err();
	readBegin = memory + size;
}

LoadBuffer::~LoadBuffer()
{
	if (munmap(memory, bufSize * 2 + 1) != 0)
		munmap_err();
}

/*
 * This function should be called from the IO thread until the function returns
 * true.
 */
bool LoadBuffer::produceBuffer(int fd, int64_t *filePosPtr, TString *lineBegin)
{
	ssize_t nRawBytes;
	char *c;

	waitForConsumptionComplete();

	nRead = lineBegin->len;
	if (nRead >= bufSize)
		abort();
	buffer = readBegin - lineBegin->len;
	strncpy(buffer, lineBegin->ptr, lineBegin->len);

	filePos = *filePosPtr;
	nRawBytes = read(fd, readBegin, bufSize);

	if (nRawBytes < 0) {
		IOerrno = errno;
		IOerror = true;
		nRawBytes = 0;
	} else {
		IOerror = false;
		IOerrno = 0;
	}

	if (nRawBytes == 0) {
		eof = true;
	} else {
		eof = false;
	}

	lineBegin->len = 0;

	for (c = readBegin + nRawBytes - 1; c >= readBegin; c--) {
		if (*c == '\n')
			break;
		lineBegin->len++;
	}

	if (lineBegin->len > 0) {
		c++;
		strncpy(lineBegin->ptr, c, lineBegin->len);
	}

	nRead += nRawBytes;
	nRead -= lineBegin->len;

	completeLoading();

	*filePosPtr += nRead;
	return eof;
}

/*
 * This should be called from the load thread before starting to process a
 * buffer.
 */
void LoadBuffer::beginProduceBuffer() {
	waitForConsumptionComplete();
}

/*
 * This should be called from the load hread when the processing of a buffer
 * has been completed.
 */
void LoadBuffer::endProduceBuffer() {
	completeLoading();
}

/*
 * This should be called from the reader thread before starting to process a
 * buffer.
 */
void LoadBuffer::beginTokenizeBuffer() {
	waitForLoadingComplete();
}

/*
 * This should be called from the reader processing thread when the processing
 * of a buffer has been completed.
 */
void LoadBuffer::endTokenizeBuffer() {
	completeTokenization();
}

/*
 * This should be called from the parser threa before starting to process a
 * buffer.
 */
void LoadBuffer::beginConsumeBuffer() {
	waitForTokenizationComplete();
}

/*
 * This should be called from the reader thread when the processing of a buffer
 * that has been completed
 */
void LoadBuffer::endConsumeBuffer() {
	completeConsumption();
}
