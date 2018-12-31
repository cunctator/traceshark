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

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <QList>

extern "C" {
#include <sys/mman.h>
}

#include "vtl/compiler.h"
#include "vtl/error.h"

class MemPool
{
public:
	MemPool(unsigned int nr_pages = 256 * 10,
		unsigned int objsize = 64);
	~MemPool();
	__always_inline void* allocObj();
	__always_inline void* allocN(unsigned int n);
	__always_inline void* preallocN(unsigned int n);
	__always_inline bool commitN(unsigned int n);
	__always_inline void* allocBytes(unsigned int bytes);
	__always_inline void* allocChars(unsigned int chars);
	__always_inline void* preallocBytes(unsigned int maxbytes);
	__always_inline void* preallocChars(unsigned int maxchars);
	__always_inline bool commitBytes(unsigned int nrbytes);
	__always_inline bool commitChars(unsigned int nrbytes);
	void reset();
private:
	quint8 *memory;
	quint8 *next;
	unsigned long long poolSize;
	unsigned long long used;
	unsigned int objSize;
	QList <void*> exhaustList;
	__always_inline void newMap();
	void addMemory();
};

__always_inline void* MemPool::allocObj()
{
	return allocBytes(objSize);
}

__always_inline void* MemPool::allocN(unsigned int n)
{
	return allocBytes(objSize * n);
}

__always_inline void* MemPool::preallocN(unsigned int n)
{
	return preallocBytes(objSize * n);
}

__always_inline bool MemPool::commitN(unsigned int n)
{
	return commitBytes(objSize * n);
}

__always_inline void* MemPool::allocBytes(unsigned int bytes)
{
	quint8* ptr;
	int retries = 0;

	do {
		used += bytes;
		if (used < poolSize) {
			ptr = next;
			next += bytes;
			return ptr;
		}
		used -= bytes;
		if (used == 0)
			return nullptr;
		addMemory();
		retries++;
	} while(retries < 2);
	return nullptr;
}

__always_inline void* MemPool::allocChars(unsigned int chars)
{
	return allocBytes(sizeof(char) * chars);
}

__always_inline void* MemPool::preallocBytes(unsigned int bytes)
{
	unsigned long long maxused = used + bytes;
	int retries = 0;

	do {
		if (maxused < poolSize)
			return next;
		if (used == 0)
			return nullptr;
		addMemory();
		maxused = bytes; /* used will be 0 here */
		retries++;
	} while(retries < 2);
	return nullptr;
}

__always_inline bool MemPool::commitBytes(unsigned int bytes)
{
	used += bytes;
	if (used >= poolSize) {
		next = nullptr;
		return false;
	}
	next += bytes;
	return true;
}

__always_inline void* MemPool::preallocChars(unsigned int maxchars)
{
	return preallocBytes(sizeof(char) * maxchars);
}

__always_inline bool MemPool::commitChars(unsigned int chars)
{
	return commitBytes(sizeof(char) * chars);
}

__always_inline void MemPool::newMap()
{
	quint8 *ptr;
	ptr = (quint8*) mmap(nullptr, (size_t) poolSize, PROT_READ|PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (likely(ptr != MAP_FAILED)) {
		memory = ptr;
		next = ptr;
		used = 0ULL;
		return;
	}
	mmap_err();
}

#endif /* MEMPOOL_H */
