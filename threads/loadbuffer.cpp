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

#include <new>
#include <cassert>
#include <cstring>
#include "misc/tstring.h"
#include "threads/loadbuffer.h"

extern "C" {
#include <unistd.h>
#include <sys/mman.h>
}

LoadBuffer::LoadBuffer(unsigned int size):
	bufSize(size), state(LOADSTATE_EMPTY)
{
	/* We need the extra byte to be able to set a null character in
	 * TraceFile::ReadNextWord() one byte out of bounds */
	memory = (char*) mmap(nullptr, 2 * size + 1, PROT_READ | PROT_WRITE,
			      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	assert(memory != MAP_FAILED);
	readBegin = memory + size;
}

LoadBuffer::~LoadBuffer()
{
	int uval;
	uval = munmap(memory, bufSize * 2 + 1);
	assert(uval == 0);
}

/* This function should be called from the IO thread until the function returns
 * true */
bool LoadBuffer::produceBuffer(int fd, char** filePosPtr, TString *lineBegin)
{
	ssize_t nRawBytes;
	char *c;

	waitForConsumptionComplete();

	nRead = lineBegin->len;
	assert(nRead < bufSize);
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

/* This should be called from the load thread before starting to process a
 * buffer */
void LoadBuffer::beginProduceBuffer() {
	waitForConsumptionComplete();
}

/* This should be called from the load hread when the processing of a buffer
 * has been completed */
void LoadBuffer::endProduceBuffer() {
	completeLoading();
}

/* This should be called from the reader thread before starting to process a
 * buffer */
void LoadBuffer::beginTokenizeBuffer() {
	waitForLoadingComplete();
}

/* This should be called from the reader processing thread when the processing
 * of a buffer has been completed */
void LoadBuffer::endTokenizeBuffer() {
	completeTokenization();
}

/* This should be called from the parser threa before starting to process a
 * buffer */
void LoadBuffer::beginConsumeBuffer() {
	waitForTokenizationComplete();
}

/* This should be called from the reader thread when the processing of a buffer
 * that has been completed */
void LoadBuffer::endConsumeBuffer() {
	completeConsumption();
}
