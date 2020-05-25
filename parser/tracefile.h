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

#ifndef TRACEFILE_H
#define TRACEFILE_H

#include <cstdio>
#include <cstdint>

#include <QByteArray>
#include <QtGlobal>
#include <QVector>
#include <QDebug>

#include "threads/loadbuffer.h"
#include "threads/threadbuffer.h"
#include "mm/mempool.h"
#include "parser/fileinfo.h"
#include "parser/traceline.h"
#include "misc/chunk.h"
#include "misc/errors.h"
#include "misc/osapi.h"
#include "misc/traceshark.h"
#include "vtl/compiler.h"

class LoadThread;
class TraceFile
{
public:
	TraceFile(char *name, int &ts_errno, unsigned int bsize = 1024 * 1024);
	~TraceFile();
	void close(int *ts_errno);
	vtl_always_inline unsigned int
		ReadLine(TraceLine *line, ThreadBuffer<TraceLine> *tbuffer);
	vtl_always_inline bool atEnd() const;
	vtl_always_inline bool getBufferSwitch() const;
	vtl_always_inline void clearBufferSwitch();
	FileInfo fileInfo;
	vtl_always_inline LoadBuffer *getLoadBuffer(int index) const;
	QByteArray getChunkArray(const Chunk *chunk,
						 int *ts_errno);
	bool isIntact(int *ts_errno);
	void readChunk(const Chunk *chunk, char *buf, int size,
				       int *ts_errno);
	vtl_always_inline int64_t getFileSize();
	bool allocMmap();
	void freeMmap();
private:
	vtl_always_inline QByteArray getChunkArray_(const Chunk *chunk,
						    int *ts_errno);
	vtl_always_inline void readChunk_(const Chunk *chunk, char *buf,
					  int size, int *ts_errno);
	vtl_always_inline unsigned int nextBufferIdx(unsigned int n);
	vtl_always_inline unsigned int
		ReadNextWord(char **word, ThreadBuffer<TraceLine> *tbuffer);
	vtl_always_inline bool
		CheckBufferSwitch(unsigned int pos,
				  ThreadBuffer<TraceLine> *tbuffer);
	int fd;
	bool fd_is_open;
	bool bufferSwitch;
	unsigned int nRead;
	unsigned lastBuf;
	unsigned lastPos;
	bool endOfLine;
	char *mappedFile;
	int64_t fileSize;
	static const unsigned int NR_BUFFERS = 4;
	LoadBuffer *loadBuffers[NR_BUFFERS];
	LoadThread *loadThread;
	char *buffer;
	static const int BUFFER_SIZE = 131072;
};


vtl_always_inline bool TraceFile::CheckBufferSwitch(unsigned int pos,
						    ThreadBuffer<TraceLine>
						    *tbuffer)
{
	LoadBuffer *loadBuffer = tbuffer->loadBuffer;
	if (unlikely(pos >= loadBuffer->nRead)) {
		lastPos = 0;
		bufferSwitch = true;
		endOfLine = true;
		return true;
	}
	return false;
}

vtl_always_inline unsigned int
TraceFile::ReadNextWord(char **word, ThreadBuffer<TraceLine> *tbuffer)
{
	unsigned int pos = lastPos;
	unsigned int nchar = 0;
	char c;
	char *buffer = tbuffer->loadBuffer->buffer;

	if (endOfLine)
		return 0;

	for (c = buffer[pos]; c == ' '; c = buffer[pos]) {
		pos++;
		if (unlikely(CheckBufferSwitch(pos, tbuffer)))
			return 0;
	}

	if (c == '\n') {
		pos++;
		if (likely(!CheckBufferSwitch(pos, tbuffer)))
			lastPos = pos;
		return 0;
	}

	*word = buffer + pos;
	pos++;

	while (true) {
		nchar++;
		if (unlikely(CheckBufferSwitch(pos, tbuffer)))
			break;
		c = buffer[pos];
		if (c == ' ' || c == '\n')
			break;
		pos++;
	}
	if (c == '\n')
		endOfLine = true;
	/*
	 * This can be out otside of the buffer, in case hit the break
	 * above but we have that spare page
	 */
	buffer[pos] = '\0';
	pos++;
	if (unlikely(CheckBufferSwitch(pos, tbuffer)))
		return nchar;
	lastPos = pos;
	return nchar;
}

vtl_always_inline unsigned int
TraceFile::ReadLine(TraceLine *line, ThreadBuffer<TraceLine> *tbuffer)
{
	unsigned int col;
	unsigned int n;
	/* This is a setup needed by ReadNextWord() */
	endOfLine = false;

	line->strings = (TString*)
		tbuffer->strPool->preallocN(EVENT_MAX_NR_ARGS);
	line->begin = tbuffer->loadBuffer->filePos + lastPos;

	for(col = 0; col < EVENT_MAX_NR_ARGS; col++) {
		n = ReadNextWord(&line->strings[col].ptr, tbuffer);
		if (n == 0)
			break;
		line->strings[col].len = n;
	}
	if (col > 0)
		tbuffer->strPool->commitN(col);
	line->nStrings = col;
	return col;
}

vtl_always_inline bool TraceFile::getBufferSwitch() const
{
	return bufferSwitch;
}

vtl_always_inline void TraceFile::clearBufferSwitch()
{
	bufferSwitch = false;
}

vtl_always_inline unsigned int TraceFile::nextBufferIdx(unsigned int n)
{
	n++;
	if (n == NR_BUFFERS)
		n = 0;
	return n;
}

vtl_always_inline LoadBuffer *TraceFile::getLoadBuffer(int index) const
{
	return loadBuffers[index];
}

vtl_always_inline QByteArray TraceFile::getChunkArray_(const Chunk *chunk,
						       int *ts_errno)
{
	char *buf;
	size_t count;
	char *b;
	ssize_t r;
	QByteArray rval;

	if (chunk->len <= BUFFER_SIZE) {
		buf = buffer;
	} else {
		buf = new char[chunk->len];
	}

	if (lseek64(fd, chunk->offset, SEEK_SET) != chunk->offset) {
		if (errno != 0)
			*ts_errno = errno;
		else
			*ts_errno = - TS_ERROR_ERROR;
		goto out;
	}

	count = chunk->len;
	b = buf;

	while (count > 0) {
		r = read(fd, b, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			if (errno != 0)
				*ts_errno = errno;
			else
				*ts_errno = - TS_ERROR_ERROR;
			goto out;
		}
		if (r == 0) {
			*ts_errno = - TS_ERROR_EOF;
			goto out;
		}
		b += r;
		count -= r;
	}
	rval = QByteArray(buf, chunk->len);
	*ts_errno = 0;

out:
	if (buf != buffer) {
		delete[] buf;
	}

	return rval;
}

vtl_always_inline void TraceFile::readChunk_(const Chunk *chunk, char *buf,
					     int size, int *ts_errno)
{
	size_t count;
	char *b;
	ssize_t r;

	if (lseek64(fd, chunk->offset, SEEK_SET) != chunk->offset) {
		if (errno != 0)
			*ts_errno = errno;
		else
			*ts_errno = - TS_ERROR_ERROR;
		return;
	}

	count = TSMIN(chunk->len, size);
	b = buf;

	while (count > 0) {
		r = read(fd, b, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			if (errno != 0)
				*ts_errno = errno;
			else
				*ts_errno = - TS_ERROR_ERROR;
			return;
		}
		if (r == 0) {
			*ts_errno = - TS_ERROR_EOF;
			return;
		}
		b += r;
		count -= r;
	}
	*ts_errno = 0;
}

int64_t TraceFile::getFileSize()
{
	return fileSize;
}

#endif
