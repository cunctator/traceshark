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

#ifndef TRACEFILE_H
#define TRACEFILE_H

#include <QtGlobal>
#include <QVector>
#include <QDebug>

class MemPool;
class TraceLine;

class TraceFile
{
public:
	TraceFile(char *name, bool &ok, quint32 bsize = 1024*1024);
	~TraceFile();
	quint32 ReadLine(TraceLine* line);
	bool atEnd();
private:
	int fd;
	bool eof;
	quint32 bufSize;
	quint32 nRead;
	char *memory;
	char *buffer[2];
	quint32 lastBuf;
	quint32 lastPos;
	MemPool *strPool;
	MemPool *ptrPool;
	static const quint32 MAXPTR = 640;
	static const quint32 MAXSTR = 480;
	inline quint32 ReadNextWord(char *word, quint32 maxstr);
	ssize_t Read(int fd, void *buf, size_t count);
};

inline quint32 TraceFile::ReadNextWord(char *word, quint32 maxstr)
{
	quint32 pos = lastPos;
	ssize_t n;
	quint32 nchar = 0;
	char c;

	maxstr--; /* Reserve space for null termination */

	if (buffer[lastBuf][lastPos] == '\n') {
		lastPos++;
		if (lastPos >= nRead) {
			lastBuf = (lastBuf + 1) % 2;
			lastPos = 0;
			n = Read(fd, buffer[lastBuf], bufSize);
			if (n <= 0) {
				eof = true;
				word[nchar] = '\0';
				nRead = 0;
				return nchar;
			}
			nRead = (quint32) n;
		}
		word[nchar] = '\0';
		return nchar;
	}


	for (c = buffer[lastBuf][pos]; c == ' '; c = buffer[lastBuf][pos]) {
		pos++;
		if (pos >= nRead) {
			lastBuf = (lastBuf + 1) % 2;
			lastPos = 0;
			pos = lastPos;
			n = Read(fd, buffer[lastBuf], bufSize);
			if (n <= 0) {
				eof = true;
				word[nchar] = '\0';
			        nRead = 0;
				return nchar;
			}
			nRead = (quint32) n;
		}
	}

	while (nchar < maxstr) {
		c = buffer[lastBuf][pos];
		if (c == ' ' || c == '\n') {
			lastPos = pos;
			word[nchar] = '\0';
			return nchar;
		}
		word[nchar] = c;
		nchar++;
		pos++;
		if (pos >= nRead) {
			lastBuf = (lastBuf + 1) % 2;
			lastPos = 0;
			pos = lastPos;
			n = Read(fd, buffer[lastBuf], bufSize);
			if (n <= 0) {
				eof = true;
				word[nchar] = '\0';
				nRead = 0;
				return nchar;
			}
			nRead = (quint32) n;
		}
	}

	lastPos = pos;
	word[nchar] = '\0';
	return nchar;
}

#endif
