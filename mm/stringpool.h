// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include <cstdint>
#include <cstring>
#include "mm/mempool.h"
#include "misc/traceshark.h"
#include "misc/tstring.h"
#include "vtl/avltree.h"
#include "vtl/tlist.h"

#define STRINGPOOL_MAX(A, B) ((A) >= (B) ? A:B)
#define STRINGPOOL_MIN(A, B) ((A) < (B) ? A:B)

class __DummySP {};

class PoolBundleSP {
public:
	MemPool *charPool;
	MemPool *nodePool;
};

template<typename HashFunc>
class StringPool;

#define __STRINGPOOL_ITERATOR(name) \
vtl::AVLTree<TString, __DummySP, vtl::AVLBALANCE_USEPOINTERS, \
AVLAllocatorSP<TString, __DummySP>, AVLCompareSP<TString>>::iterator

template <class T>
class AVLCompareSP {
public:
	__always_inline static int compare(const T &a, const T &b) {
		return strcmp(a.ptr, b.ptr);
	}
};

template <class T, class U>
class AVLAllocatorSP {
public:
	AVLAllocatorSP(void *data) {
		PoolBundleSP *pb = (PoolBundleSP*) data;
		pools = *pb;
	}
	__always_inline vtl::AVLNode<T, U> *alloc(const T &key) {
		vtl::AVLNode<T, U> *node = (vtl::AVLNode<T, U> *)
			pools.nodePool->allocObj();
		node->key.len = key.len;
		node->key.ptr = (char*) pools.charPool->allocChars(key.len + 1);
		strcpy(node->key.ptr, key.ptr);
		return node;
	}
	__always_inline int clear() {
		/*
		 * Do nothing because the pools are owned by StringPool. This
		 * is only called from StringPool, via AVLTree, when the object
		 * is cleared.
		 */
		return 0;
	}
private:
	PoolBundleSP pools;
};

#define AVLTREE_SIZE ((int)sizeof(vtl::AVLTree<TString, __DummySP,	 \
vtl::AVLBALANCE_USEPOINTERS, AVLAllocatorSP<TString, __DummySP>, \
			     AVLCompareSP<TString>>))
#define TSTRING_PTR_SIZE ((int)sizeof(TString*))
#define TYPICAL_CACHE_LINE_SIZE (64)

#define SP_CACHE_SIZE (TYPICAL_CACHE_LINE_SIZE - TSTRING_PTR_SIZE - \
		       AVLTREE_SIZE)

template<typename HashFunc>
class StringPoolEntry {
	friend class StringPool<HashFunc>;
public:
StringPoolEntry(void *data): cachePtr(nullptr), avlTree(data) {
		bzero(cache, SP_CACHE_SIZE);
	}
protected:
	char cache[SP_CACHE_SIZE];
	TString *cachePtr;
	vtl::AVLTree<TString, __DummySP, vtl::AVLBALANCE_USEPOINTERS,
		AVLAllocatorSP<TString, __DummySP>, AVLCompareSP<TString>>
		avlTree;
};

class StringPoolDefaultHashFunc {
public:
	__always_inline uint32_t operator()(const TString *str) const
	{
		return TShark::StrHash32(str);
	}
};

template<typename HashFunc = StringPoolDefaultHashFunc>
class StringPool
{
public:
	StringPool(unsigned int nr_pages = 256 * 10, unsigned int hSizeP = 256);
	~StringPool();
	__always_inline const TString *allocString(const TString *str,
						   uint32_t cutoff);
	void clear();
	void reset();
private:
	__always_inline const TString *allocUniqueString(const TString *str);
	MemPool *coldCharPool;
	MemPool *strPool;
	PoolBundleSP avlPools;
	StringPoolEntry<HashFunc> **hashTable;
	unsigned int *countAllocs;
	unsigned int *countReuse;
	unsigned int hSize;
	void clearTable();
	vtl::TList<StringPoolEntry<HashFunc>*> deleteList;
	HashFunc hFunc;
};

template<typename HashFunc>
__always_inline
const TString *StringPool<HashFunc>::allocString(const TString *str,
						 uint32_t cutoff)
{
	StringPoolEntry<HashFunc> *entry;
	bool isNew;
	uint32_t hval = hFunc(str);

	hval = hval % hSize;

	if (cutoff != 0 && countAllocs[hval] > cutoff &&
	    countAllocs[hval] > countReuse[hval]) {
		const TString *newstr = allocUniqueString(str);
		return newstr;
	}

	if (hashTable[hval] != nullptr) {
		entry = hashTable[hval];
		if (entry->cachePtr != nullptr &&
		    strcmp(entry->cache, str->ptr) == 0) {
			if (cutoff != 0)
				countReuse[hval]++;
			return entry->cachePtr;
		}
		__STRINGPOOL_ITERATOR(iter) iter =
			entry->avlTree.findInsert(*str, isNew);
		TString &refStr = iter.key();
		if (isNew) {
			if (cutoff != 0)
				countAllocs[hval]++;
		} else {
			if (refStr.len < SP_CACHE_SIZE) {
				strcpy(entry->cache, refStr.ptr);
				entry->cachePtr = &refStr;
			}
			if (cutoff != 0)
				countReuse[hval]++;
		}
		return &refStr;
	} else {
		entry = new StringPoolEntry<HashFunc>(&avlPools);
		hashTable[hval] = entry;
		deleteList.append(entry);
		__STRINGPOOL_ITERATOR(iter) iter =
			entry->avlTree.findInsert(*str, isNew);
		if (cutoff != 0)
			countAllocs[hval]++;
		TString &refStr = iter.key();
		return &refStr;
	}
}

template<typename HashFunc>
__always_inline
const TString *StringPool<HashFunc>::allocUniqueString(const TString *str)
{
	TString *newstr;

	newstr = (TString*) strPool->allocObj();
	if (newstr == nullptr)
		return nullptr;
	newstr->len = str->len;
	newstr->ptr = (char*) coldCharPool->allocChars(str->len + 1);
	if (newstr->ptr == nullptr)
		return nullptr;
	strcpy(newstr->ptr, str->ptr);
	return newstr;
}

template<typename HashFunc>
StringPool<HashFunc>::StringPool(unsigned int nr_pages, unsigned int hSizeP)
{
	unsigned int entryPages, strPages;

	if (hSizeP == 0)
		hSize = 1;
	else
		hSize = hSizeP;

	entryPages = 2 * hSize *
		sizeof(vtl::AVLNode<TString, __DummySP>) / 4096;
	entryPages = STRINGPOOL_MAX(1, entryPages);
	strPages = 2* hSize * sizeof(TString) / 4096;
	strPages = STRINGPOOL_MAX(16, strPages);

	coldCharPool = new MemPool(nr_pages, 1);
	strPool = new MemPool(nr_pages, sizeof(TString));

	avlPools.charPool = new MemPool(nr_pages, sizeof(char));
	avlPools.nodePool = new MemPool(entryPages, sizeof(vtl::AVLNode<TString,
							   __DummySP>));
	hashTable = new StringPoolEntry<HashFunc>*[hSize];
	countAllocs = new unsigned int[hSize];
	countReuse = new unsigned int[hSize];
	clearTable();
}

template<typename HashFunc>
StringPool<HashFunc>::~StringPool()
{
	unsigned int i, s;
	delete coldCharPool;
	delete strPool;
	delete avlPools.charPool;
	delete avlPools.nodePool;
	delete[] hashTable;
	delete[] countAllocs;
	delete[] countReuse;
	s = deleteList.size();
	for (i = 0; i < s; i++) {
		delete deleteList[i];
	}
}

template<typename HashFunc>
void StringPool<HashFunc>::clearTable()
{
	bzero(hashTable, hSize * sizeof(StringPoolEntry<HashFunc>*));
	bzero(countAllocs, hSize * sizeof(unsigned int));
	bzero(countReuse, hSize * sizeof(unsigned int));
}

template<typename HashFunc>
void StringPool<HashFunc>::clear()
{
	unsigned int s, i;
	clearTable();
	coldCharPool->reset();
	strPool->reset();
	avlPools.nodePool->reset();
	avlPools.charPool->reset();
	s = deleteList.size();
	for (i = 0; i < s; i++) {
		delete deleteList[i];
	}
	deleteList.clear();
}

template<typename HashFunc>
void StringPool<HashFunc>::reset()
{
	clear();
}

#endif /* STRINGPOOL_H */
