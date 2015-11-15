/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TRACEFILE_H
#define TRACEFILE_H

#include <QtGlobal>
#include <QVector>
#include <QDebug>

#include "threads/loadbuffer.h"
#include "mm/mempool.h"
#include "traceline.h"

class LoadThread;

class TraceFile
{
public:
	TraceFile(char *name, bool &ok, unsigned int bsize = 1024*1024,
		  unsigned int nrPoolsMAX = 1);
	~TraceFile();
	__always_inline unsigned int ReadLine(TraceLine* line,
					      unsigned int pool);
	__always_inline bool atEnd();
	__always_inline void clearPool(unsigned int pool);
private:
	__always_inline unsigned int nextBufferIdx(unsigned int n);
	__always_inline unsigned int ReadNextWord(char *word,
						  unsigned int maxstr);
	int fd;
	bool eof;
	unsigned int nRead;
	char *memory;

	unsigned lastBuf;
	unsigned lastPos;
	MemPool **strPool;
	MemPool **ptrPool;
	unsigned int nrPools;
	static const unsigned int MAXPTR = 640;
	static const unsigned int MAXSTR = 480;
	static const unsigned int NR_BUFFERS = 3;
	LoadBuffer *buffers[NR_BUFFERS];
	LoadThread *loadThread;
};

__always_inline unsigned int TraceFile::ReadNextWord(char *word,
						     unsigned int maxstr)
{
	unsigned int pos = lastPos;
	unsigned int nchar = 0;
	char c;
	bool e;

	maxstr--; /* Reserve space for null termination */

	if (buffers[lastBuf]->buffer[lastPos] == '\n') {
		lastPos++;
		if (lastPos >= nRead) {
			buffers[lastBuf]->endConsumeBuffer();
			lastBuf = nextBufferIdx(lastBuf);
			lastPos = 0;
			e = buffers[lastBuf]->beginConsumeBuffer();
			if (e) {
				buffers[lastBuf]->endConsumeBuffer();
				eof = e;
				word[nchar] = '\0';
				nRead = 0;
				return nchar;
			}
			nRead = (unsigned int) buffers[lastBuf]->nRead;
		}
		word[nchar] = '\0';
		return nchar;
	}

	for (c = buffers[lastBuf]->buffer[pos]; c == ' ';
	     c = buffers[lastBuf]->buffer[pos]) {
		pos++;
		if (pos >= nRead) {
			buffers[lastBuf]->endConsumeBuffer();
			lastBuf = nextBufferIdx(lastBuf);
			lastPos = 0;
			pos = lastPos;
			e = buffers[lastBuf]->beginConsumeBuffer();
			if (e) {
				buffers[lastBuf]->endConsumeBuffer();
				eof = e;
				word[nchar] = '\0';
			        nRead = 0;
				return nchar;
			}
			nRead = (unsigned int) buffers[lastBuf]->nRead;
		}
	}

	while (nchar < maxstr) {
		c = buffers[lastBuf]->buffer[pos];
		if (c == ' ' || c == '\n') {
			lastPos = pos;
			word[nchar] = '\0';
			return nchar;
		}
		word[nchar] = c;
		nchar++;
		pos++;
		if (pos >= nRead) {
			buffers[lastBuf]->endConsumeBuffer();
			lastBuf = nextBufferIdx(lastBuf);
			lastPos = 0;
			pos = lastPos;
			e = buffers[lastBuf]->beginConsumeBuffer();
			if (e) {
				buffers[lastBuf]->endConsumeBuffer();
				eof = e;
				word[nchar] = '\0';
				nRead = 0;
				return nchar;
			}
			nRead = (unsigned int) buffers[lastBuf]->nRead;
		}
	}

	lastPos = pos;
	word[nchar] = '\0';
	return nchar;
}

__always_inline unsigned int TraceFile::ReadLine(TraceLine* line,
						 unsigned int pool)
{
	unsigned int col;
	unsigned int n;

	line->strings = (TString*) ptrPool[pool]->preallocN(MAXPTR);

	for(col = 0; col < MAXPTR; col++) {
		line->strings[col].ptr = (char*)
			strPool[pool]->preallocChars(MAXSTR);
		n = ReadNextWord(line->strings[col].ptr, MAXSTR);
		if (n == 0)
			break;
		strPool[pool]->commitChars(n + 1 ); // + 1 for null termination
		line->strings[col].len = n;
	}
	if (col > 0)
		ptrPool[pool]->commitN(col);
	line->nStrings = col;
	return col;
}

__always_inline bool TraceFile::atEnd()
{
	return eof;
}

__always_inline unsigned int TraceFile::nextBufferIdx(unsigned int n)
{
	n++;
	if (n == NR_BUFFERS)
		n = 0;
	return n;
}

__always_inline void TraceFile::clearPool(unsigned int pool)
{
	ptrPool[pool]->reset();
	strPool[pool]->reset();
}

#endif
