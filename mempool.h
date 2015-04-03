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
	inline void* allocObj();
	inline void* allocN(unsigned long n);
	inline void* preallocN(unsigned long n);
	inline bool commitN(unsigned long n);
	inline void* allocBytes(quint32 bytes);
	inline void* allocChars(quint32 chars);
	inline void* preallocBytes(quint32 maxbytes);
	inline void* preallocChars(quint32 maxchars);
	inline bool commitBytes(quint32 nrbytes);
	inline bool commitChars(quint32 nrbytes);
	void reset();
private:
	quint8 *memory;
	quint8 *next;
	unsigned long long poolSize;
	unsigned long long used;
	unsigned long objSize;
	QVector <void*> exhaustList;
	inline bool newMap();
	inline bool addMemory();
};
#endif /* MEMPOOL_H */

inline void* MemPool::allocObj()
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

inline void* MemPool::allocN(unsigned long n)
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

inline void* MemPool::preallocN(unsigned long n)
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

inline bool MemPool::commitN(unsigned long n)
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

inline void* MemPool::allocBytes(quint32 bytes)
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

inline void* MemPool::allocChars(quint32 chars)
{
	return allocBytes(sizeof(char) * chars);
}

inline void* MemPool::preallocBytes(quint32 bytes)
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

inline bool MemPool::commitBytes(quint32 bytes)
{
	used += bytes;
	if (used >= poolSize) {
		next = NULL;
		return false;
	}
	next += bytes;
	return true;
}

inline void* MemPool::preallocChars(quint32 maxchars)
{
	return preallocBytes(sizeof(char) * maxchars);
}

inline bool MemPool::commitChars(quint32 chars)
{
	return commitBytes(sizeof(char) * chars);
}

inline bool MemPool::addMemory()
{
	exhaustList.append(memory);
	if (newMap())
		return true;
	exhaustList.removeLast();
	return false;
}

inline bool MemPool::newMap()
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
