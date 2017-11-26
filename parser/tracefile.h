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

#ifndef TRACEFILE_H
#define TRACEFILE_H

#include <QtGlobal>
#include <QVector>
#include <QDebug>

#include "threads/loadbuffer.h"
#include "threads/threadbuffer.h"
#include "mm/mempool.h"
#include "parser/traceline.h"
#include "misc/traceshark.h"

class LoadThread;

class TraceFile
{
public:
	TraceFile(char *name, bool &ok, unsigned int bsize = 1024 * 1024);
	~TraceFile();
	__always_inline unsigned int
		ReadLine(TraceLine *line, ThreadBuffer<TraceLine> *tbuffer);
	__always_inline bool atEnd() const;
	__always_inline bool getBufferSwitch() const;
	__always_inline void clearBufferSwitch();
	char *mappedFile;
	unsigned long fileSize;
	__always_inline LoadBuffer *getLoadBuffer(int index) const;
private:
	__always_inline unsigned int nextBufferIdx(unsigned int n);
	__always_inline unsigned int
		ReadNextWord(char **word, ThreadBuffer<TraceLine> *tbuffer);
	__always_inline bool
		CheckBufferSwitch(unsigned int pos,
				  ThreadBuffer<TraceLine> *tbuffer);
	int fd;
	bool bufferSwitch;
	unsigned int nRead;
	unsigned lastBuf;
	unsigned lastPos;
	bool endOfLine;
	static const unsigned int MAX_NR_STRINGS = 128;
	static const unsigned int NR_BUFFERS = 4;
	LoadBuffer *loadBuffers[NR_BUFFERS];
	LoadThread *loadThread;
};


__always_inline bool TraceFile::CheckBufferSwitch(unsigned int pos,
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

__always_inline unsigned int
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

__always_inline unsigned int
TraceFile::ReadLine(TraceLine *line, ThreadBuffer<TraceLine> *tbuffer)
{
	unsigned int col;
	unsigned int n;
	/* This is a setup needed by ReadNextWord() */
	endOfLine = false;

	line->strings = (TString*) tbuffer->strPool->preallocN(MAX_NR_STRINGS);
	line->begin = tbuffer->loadBuffer->filePos + lastPos;

	for(col = 0; col < MAX_NR_STRINGS; col++) {
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

__always_inline bool TraceFile::getBufferSwitch() const
{
	return bufferSwitch;
}

__always_inline void TraceFile::clearBufferSwitch()
{
	bufferSwitch = false;
}

__always_inline unsigned int TraceFile::nextBufferIdx(unsigned int n)
{
	n++;
	if (n == NR_BUFFERS)
		n = 0;
	return n;
}

__always_inline LoadBuffer *TraceFile::getLoadBuffer(int index) const
{
	return loadBuffers[index];
}

#endif
