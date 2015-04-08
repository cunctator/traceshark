/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef MEMPOOL_H
#define MEMPOOL_H

#include <QVector>
#include <QTextStream>

extern "C" {
#include <sys/mman.h>
}

class MemPool
{
public:
	MemPool(unsigned int nr_pages = 256*10,
		unsigned long objsize = 64);
	~MemPool();
	__always_inline void* allocObj();
	__always_inline void* allocN(unsigned long n);
	__always_inline void* preallocN(unsigned long n);
	__always_inline bool commitN(unsigned long n);
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
	unsigned long objSize;
	QVector <void*> exhaustList;
	__always_inline bool newMap();
	bool addMemory();
};

__always_inline void* MemPool::allocObj()
{
	quint8 *ptr;
	int retries = 0;

	do {
		used += objSize;
		if (used < poolSize) {
			ptr = next;
			next += objSize;
			return ptr;
		}
		used -= objSize;
		if (used == 0 || !addMemory())
			return NULL;
		retries++;
	} while(retries < 2);
	return NULL;
}

__always_inline void* MemPool::allocN(unsigned long n)
{
	quint8 *ptr;
	unsigned long chunk = objSize * n;
	int retries = 0;

	do {
		used += chunk;
		if (used < poolSize) {
			ptr = next;
			next += chunk;
			return ptr;
		}
		used -= chunk;
		if (used == 0 || !addMemory())
			return NULL;
		retries++;
	} while(retries < 2);
	return NULL;
}

__always_inline void* MemPool::preallocN(unsigned long n)
{
	unsigned long chunk = objSize * n;
	int retries = 0;
	unsigned long long maxused = used + chunk;

	do {
		if (maxused < poolSize)
			return next;
		if (used == 0)
			return NULL;
		if (!addMemory())
			return NULL;
		maxused = chunk; /* used will be 0 here */
		retries++;
	} while(retries < 2);
	return NULL;
}

__always_inline bool MemPool::commitN(unsigned long n)
{
	unsigned long chunk = objSize * n;

	used += chunk;
	if (used >= poolSize) {
		next = NULL;
		return false;
	}
	next += chunk;
	return true;
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
		if (used == 0 || !addMemory())
			return NULL;
		retries++;
	} while(retries < 2);
	return NULL;
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
			return NULL;
		if (!addMemory())
			return NULL;
		maxused = bytes; /* used will be 0 here */
		retries++;
	} while(retries < 2);
	return NULL;
}

__always_inline bool MemPool::commitBytes(unsigned int bytes)
{
	used += bytes;
	if (used >= poolSize) {
		next = NULL;
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

__always_inline bool MemPool::newMap()
{
	quint8 *ptr;
	ptr = (quint8*) mmap(NULL, (size_t) poolSize, PROT_READ|PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr != MAP_FAILED) {
		memory = ptr;
		next = ptr;
		used = 0ULL;
		return true;
	}
	QTextStream(stderr) << "mmap() failed\n";
	return false;
}

#endif /* MEMPOOL_H */
