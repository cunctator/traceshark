/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include <cstdint>
#include <cstring>
#include "mm/mempool.h"
#include "misc/traceshark.h"
#include "misc/tstring.h"
#include "vtl/avltree.h"
#include "vtl/tlist.h"

class __DummySP {};

class PoolBundleSP {
public:
	MemPool *charPool;
	MemPool *nodePool;
};

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
		strncpy(node->key.ptr, key.ptr, key.len + 1);
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

#define AVLTREE_SIZE (sizeof(vtl::AVLTree<TString, __DummySP, \
vtl::AVLBALANCE_USEPOINTERS, AVLAllocatorSP<TString, __DummySP>, \
			     AVLCompareSP<TString>>))
#define TSTRING_PTR_SIZE (sizeof(TString*))
#define TYPICAL_CACHE_LINE_SIZE (64)

#define SP_CACHE_SIZE (TYPICAL_CACHE_LINE_SIZE - TSTRING_PTR_SIZE - \
		       AVLTREE_SIZE)

class StringPoolEntry {
	friend class StringPool;
public:
StringPoolEntry(void *data): cachePtr(nullptr), avlTree(data) {
		bzero(cache, SP_CACHE_SIZE + 1);
	}
protected:
	char cache[SP_CACHE_SIZE];
	TString *cachePtr;
	vtl::AVLTree<TString, __DummySP, vtl::AVLBALANCE_USEPOINTERS,
		AVLAllocatorSP<TString, __DummySP>, AVLCompareSP<TString>>
		avlTree;
};

class StringPool
{
public:
	StringPool(unsigned int nr_pages = 256 * 10, unsigned int hSizeP = 256);
	~StringPool();
	__always_inline const TString *allocString(const TString *str,
						   uint32_t hval,
						   uint32_t cutoff);
	void clear();
	void reset();
private:
	__always_inline const TString *allocUniqueString(const TString *str);
	MemPool *coldCharPool;
	MemPool *strPool;
	PoolBundleSP avlPools;
	StringPoolEntry **hashTable;
	unsigned int *countAllocs;
	unsigned int *countReuse;
	unsigned int hSize;
	void clearTable();
	vtl::TList<StringPoolEntry*> deleteList;
};

__always_inline const TString *StringPool::allocString(const TString *str,
						       uint32_t hval,
						       uint32_t cutoff)
{
	StringPoolEntry *entry;
	bool isNew;

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
				strncpy(entry->cache, refStr.ptr, refStr.len);
				entry->cachePtr = &refStr;
			}
			if (cutoff != 0)
				countReuse[hval]++;
		}
		return &refStr;
	} else {
		entry = new StringPoolEntry(&avlPools);
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

__always_inline const TString *StringPool::allocUniqueString(const TString *str)
{
	TString *newstr;

	newstr = (TString*) strPool->allocObj();
	if (newstr == nullptr)
		return nullptr;
	newstr->len = str->len;
	newstr->ptr = (char*) coldCharPool->allocChars(str->len + 1);
	if (newstr->ptr == nullptr)
		return nullptr;
	strncpy(newstr->ptr, str->ptr, str->len + 1);
	return newstr;
}




#endif /* STRINGPOOL_H */
