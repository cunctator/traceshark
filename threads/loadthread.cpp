/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstdlib>
#include <cstring>
#include <QTextStream>

#include "misc/tstring.h"
#include "threads/loadbuffer.h"
#include "threads/loadthread.h"

extern "C" {
#include <sys/mman.h>
#include <unistd.h>
}

LoadThread::LoadThread(LoadBuffer **buffers, unsigned int nBuf, int myfd,
		       char *map)
	: TThread(QString("LoadThread")), loadBuffers(buffers), nBuffers(nBuf),
	  fd(myfd), mappedFile(map)
{}

void LoadThread::run()
{
	unsigned int i = 0;
	bool eof;
	char *filePos = mappedFile;
	TString lineBegin;
	size_t bufSize = loadBuffers[0]->bufSize;
	int uval, eval;
	QTextStream qout(stdout);

	lineBegin.ptr = (char*) mmap(nullptr, bufSize, PROT_READ|PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (lineBegin.ptr == MAP_FAILED)
		abort();
	lineBegin.len = 0;

	do {
		eof = loadBuffers[i]->produceBuffer(fd, &filePos, &lineBegin);
		i++;
		if (i == nBuffers)
			i = 0;
	} while(!eof);

	uval = close(fd);
	eval = errno;
	if (uval != 0) {
		qout << "Warning, error in " << __FUNCTION__
		     <<	"(), close() failed because: " << strerror(eval)
		     << "\n";
	}

	uval = munmap(lineBegin.ptr, bufSize);
	eval = errno;
	if (uval != 0) {
		qout << "Warning, error in " << __FUNCTION__
		     <<	"(), munmap() failed because: " << strerror(eval)
		     << "\n";
	}
}
