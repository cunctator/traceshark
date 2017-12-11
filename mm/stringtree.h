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

#ifndef STRINGTREE_H
#define STRINGTREE_H

#include <cstdint>
#include <cstring>
#include "mm/mempool.h"
#include "misc/traceshark.h"
#include "misc/tstring.h"
#include "parser/traceevent.h"
#include "vtl/avltree.h"
#include "vtl/tlist.h"

class PoolBundleST {
public:
	MemPool *charPool;
	MemPool *nodePool;
};

#define __STRINGTREE_ITERATOR(name) \
vtl::AVLTree<TString, event_t, false, AVLAllocatorST<TString, event_t>, \
AVLCompareST<TString>>::iterator

template <class T>
class AVLCompareST {
public:
	__always_inline static int compare(const T &a, const T &b) {
		return strcmp(a.ptr, b.ptr);
	}
};

template <class T, class U>
class AVLAllocatorST {
public:
	AVLAllocatorST(void *data) {
		PoolBundleST *pb = (PoolBundleST*) data;
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
		 * Do nothing because the pools are owned by StringTree. This
		 * is only called from StringTree, via AVLTree, when the object
		 * is cleared.
		 */
		return 0;
	}
private:
	PoolBundleST pools;
};

#define ST_AVLTREE_SIZE (sizeof(vtl::AVLTree<TString, event_t, false, \
AVLAllocatorST<TString, event_t>, AVLCompareST<TString>>))
#define ST_TSTRING_PTR_SIZE (sizeof(TString*))
#define ST_TYPICAL_CACHE_LINE_SIZE (32)

#define ST_CACHE_SIZE (ST_TYPICAL_CACHE_LINE_SIZE - ST_TSTRING_PTR_SIZE - \
		       ST_AVLTREE_SIZE)

class StringTreeEntry {
	friend class StringTree;
public:
StringTreeEntry(void *data): avlTree(data) { }
protected:
	vtl::AVLTree<TString, event_t, false, AVLAllocatorST<TString,
		event_t>, AVLCompareST<TString>> avlTree;
};

class StringTree
{
public:
	StringTree(unsigned int nr_pages = 256 * 10, unsigned int hSizeP = 256,
		   unsigned int table_size = 4096);
	~StringTree();
	__always_inline const TString *stringLookup(event_t value) const;
	__always_inline event_t searchAllocString(const TString *str,
						  uint32_t hval,
						  event_t newval);
	__always_inline event_t getMaxEvent() const;
	void clear();
	void reset();
private:
	PoolBundleST avlPools;
	StringTreeEntry **hashTable;
	unsigned int hSize;
	unsigned int tableSize;
	event_t maxEvent;
	TString **stringTable;
	void clearTable();
	vtl::TList<StringTreeEntry*> deleteList;
};

__always_inline const TString *StringTree::stringLookup(event_t value) const
{
	if (value < 0 || value > maxEvent)
		return nullptr;
	return stringTable[value];
}

__always_inline event_t StringTree::searchAllocString(const TString *str,
						      uint32_t hval,
						      event_t newval)
{
	StringTreeEntry *entry;
	bool isNew;

	hval = hval % hSize;

	if (hashTable[hval] != nullptr) {
		entry = hashTable[hval];
		__STRINGTREE_ITERATOR(iter) iter =
			entry->avlTree.findInsert(*str, isNew);
		if (isNew) {
			event_t &e = iter.value();
			e = newval;
			stringTable[newval] = &iter.key();
			maxEvent = newval;
			return newval;
		} else {
			return iter.value();
		}
	} else {
		entry = new StringTreeEntry(&avlPools);
		hashTable[hval] = entry;
		deleteList.append(entry);
		__STRINGTREE_ITERATOR(iter) iter =
			entry->avlTree.findInsert(*str, isNew);
		event_t &e = iter.value();
		e = newval;
		stringTable[newval] = &iter.key();
		maxEvent = newval;
		return newval;
	}
}

__always_inline event_t StringTree::getMaxEvent() const
{
	return maxEvent;
}

#endif /* STRINGTREE_H */
