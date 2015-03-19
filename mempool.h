/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl
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
#include <QDebug>

extern "C" {
#include <sys/mman.h>
}

class MemPool
{
public:
	MemPool(unsigned long long size = 1024*1024*10,
		unsigned long objsize = 64);
	~MemPool();
	inline void* AllocObj();
	inline void* AllocN(unsigned long n);
	inline void* PreAllocN(unsigned long n);
	inline bool CommitN(unsigned long n);
	inline void* AllocBytes(quint32 bytes);
	inline void* AllocChars(quint32 chars);
	inline void* PreAllocBytes(quint32 maxbytes);
	inline void* PreAllocChars(quint32 maxchars);
	inline bool CommitBytes(quint32 nrbytes);
	inline bool CommitChars(quint32 nrbytes);
	void Reset();
private:
	quint8 *memory;
	quint8 *next;
	unsigned long long poolSize;
	unsigned long long used;
	unsigned long objSize;
	QVector <void*> exhaustList;
	inline bool newmap();
	inline bool AddMemory();
};
#endif /* MEMPOOL_H */

inline void* MemPool::AllocObj()
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
		if (used == 0 || !AddMemory())
			return NULL;
		retries++;
	} while(retries < 2);
	return NULL;
}

inline void* MemPool::AllocN(unsigned long n)
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
		if (used == 0 || !AddMemory())
			return NULL;
		retries++;
	} while(retries < 2);
	return NULL;
}

inline void* MemPool::PreAllocN(unsigned long n)
{
	unsigned long chunk = objSize * n;
	int retries = 0;
	unsigned long long maxused = used + chunk;

	do {
		if (maxused < poolSize)
			return next;
		if (used == 0)
			return NULL;
		if (!AddMemory())
			return NULL;
		maxused = chunk; /* used will be 0 here */
		retries++;
	} while(retries < 2);
	return NULL;
}

inline bool MemPool::CommitN(unsigned long n)
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

inline void* MemPool::AllocBytes(quint32 bytes)
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
		if (used == 0 || !AddMemory())
			return NULL;
		retries++;
	} while(retries < 2);
	return NULL;
}

inline void* MemPool::AllocChars(quint32 chars)
{
	return AllocBytes(sizeof(char) * chars);
}

inline void* MemPool::PreAllocBytes(quint32 bytes)
{
	unsigned long long maxused = used + bytes;
	int retries = 0;

	do {
		if (maxused < poolSize)
			return next;
		if (used == 0)
			return NULL;
		if (!AddMemory())
			return NULL;
		maxused = bytes; /* used will be 0 here */
		retries++;
	} while(retries < 2);
	return NULL;
}

inline bool MemPool::CommitBytes(quint32 bytes)
{
	used += bytes;
	if (used >= poolSize) {
		next = NULL;
		return false;
	}
	next += bytes;
	return true;
}

inline void* MemPool::PreAllocChars(quint32 maxchars)
{
	return PreAllocBytes(sizeof(char) * maxchars);
}

inline bool MemPool::CommitChars(quint32 chars)
{
	return CommitBytes(sizeof(char) * chars);
}

inline bool MemPool::AddMemory()
{
	exhaustList.append(memory);
	if (newmap())
		return true;
	exhaustList.removeLast();
	return false;
}

inline bool MemPool::newmap()
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
	qDebug() << "mmap() failed\n";
	return false;
}
