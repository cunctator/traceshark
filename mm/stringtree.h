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
#include "mm/mempool.h"
#include "misc/traceshark.h"
#include "misc/tstring.h"
#include "parser/traceevent.h"

class StringTreeEntry {
public:
	StringTreeEntry *small;
	StringTreeEntry *large;
	StringTreeEntry *parent;
	TString *str;
	event_t eventType;
	int height;
	__always_inline void stealParent(StringTreeEntry *newChild,
					 StringTreeEntry **rootptr);
	__always_inline void setHeightFromChildren();
};

class StringTree
{
public:
	StringTree(unsigned int nr_pages = 256 * 10, unsigned int hSizeP = 256,
		   unsigned int table_size = 4096);
	~StringTree();
	__always_inline TString *stringLookup(event_t value) const;
	__always_inline event_t searchAllocString(const TString *str,
						  uint32_t hval,
						  event_t newval);
	void clear();
	void reset();
private:
	__always_inline void SwapEntries(StringTreeEntry *a,
					 StringTreeEntry *b);
	MemPool *strPool;
	MemPool *charPool;
	MemPool *entryPool;
	StringTreeEntry **hashTable;
	unsigned int hSize;
	TString **stringTable;
	int tableSize;
	void clearTables();
};


__always_inline TString *StringTree::stringLookup(event_t value) const
{
	if (value < 0 || value >= tableSize)
		return nullptr;
	return stringTable[value];
}


/*
 * We use and  AVL-tree here, since this data structure is meant to store
 * event names and the number of event names is extremely small compared to
 * the total number of events. The number of different events names equals the
 * number of inserts and the number of events equals the number
 * of lookups
 */
__always_inline event_t StringTree::searchAllocString(const TString *str,
						      uint32_t hval,
						      event_t newval)
{
	TString *newstr;
	StringTreeEntry **aentry;
	StringTreeEntry **rootptr;
	StringTreeEntry *entry;
	StringTreeEntry *parent = nullptr;
	StringTreeEntry *sibling;
	StringTreeEntry *grandParent;
	StringTreeEntry *smallChild;
	StringTreeEntry *largeChild;
	int cmp;
	int diff;
	int smallH;
	int largeH;

	hval = hval % hSize;
	aentry = hashTable + hval;
	rootptr = aentry;
	entry = *aentry;

	while (entry != nullptr) {
		/*
		 * Using strncmp here would lose performance and we know that
		 * the strings are null terminated
		 */
		cmp = strcmp(str->ptr, entry->str->ptr);
		if (cmp == 0)
			return entry->eventType;
		parent = entry;
		if (cmp < 0)
			aentry = &entry->small;
		else
			aentry = &entry->large;
		entry = *aentry;
	}

	newstr = (TString*) strPool->allocObj();
	if (newstr == nullptr)
		return EVENT_ERROR;
	newstr->len = str->len;
	newstr->ptr = (char*) charPool->allocChars(str->len + 1);
	if (newstr->ptr == nullptr)
		return EVENT_ERROR;
	strncpy(newstr->ptr, str->ptr, str->len + 1);
	entry = (StringTreeEntry*) entryPool->allocObj();
	if (entry == nullptr)
		return EVENT_ERROR;
	bzero(entry, sizeof(StringTreeEntry));
	entry->str = newstr;
	entry->eventType = newval;
	/* aentry is equal to &parent->[large|small] */
	*aentry = entry;

	entry->parent = parent;
	entry->height = 0;
	if (parent == nullptr || parent->height > 0) {
		/* This is the root node or parent already has another node */
		stringTable[newval] = newstr;
		return newval;
	}
	parent->height = 1;
	grandParent = parent->parent;
	/* update heights and find offending node */
	while(grandParent != nullptr) {
		smallH = grandParent->small == nullptr ?
			-1 : grandParent->small->height;
		largeH = grandParent->large == nullptr ?
			-1 : grandParent->large->height;
		diff = smallH - largeH;
		if (diff == 0)
			break;
		if (diff > 1)
			goto rebalanceSmall;
		if (diff < -1)
			goto rebalanceLarge;
		grandParent->height = parent->height + 1;
		entry = parent;
		parent = grandParent;
		grandParent = grandParent->parent;
	}
	stringTable[newval] = newstr;
	return newval;
rebalanceSmall:
	/* Do small rebalance here (case 1 and case 2) */
	if (entry == parent->small) {
		/* Case 1 */
		sibling = parent->large;

		grandParent->stealParent(parent, rootptr);
		parent->large = grandParent;
		grandParent->parent = parent;
		grandParent->small = sibling;
		grandParent->height--;
		if (sibling != nullptr)
			sibling->parent = grandParent;
	} else {
		/* Case 2 */
		smallChild = entry->small;
		largeChild = entry->large;

		grandParent->stealParent(entry, rootptr);
		entry->small = parent;
		entry->large = grandParent;
		entry->height = grandParent->height;

		grandParent->parent = entry;
		grandParent->small = largeChild;
		grandParent->setHeightFromChildren(); // Fixme: faster

		parent->parent = entry;
		parent->large = smallChild;
		parent->setHeightFromChildren();

		if (largeChild != nullptr)
			largeChild->parent = grandParent;
		if (smallChild != nullptr)
			smallChild->parent = parent;
	}
	stringTable[newval] = newstr;
	return newval;
rebalanceLarge:
	/* Do large rebalance here */
	if (entry == parent->small) {
		/* Case 3 */
		smallChild = entry->small;
		largeChild = entry->large;

		grandParent->stealParent(entry, rootptr);
		entry->small = grandParent;
		entry->large = parent;
		entry->height = grandParent->height;

		grandParent->parent = entry;
		grandParent->large = smallChild;
		grandParent->setHeightFromChildren(); // Fixme: faster

		parent->parent = entry;
		parent->small = largeChild;
		parent->setHeightFromChildren();

		if (largeChild != nullptr)
			largeChild->parent = parent;
		if (smallChild != nullptr)
			smallChild->parent = grandParent;
	} else {
		/* Case 4 */
		sibling = parent->small;

		grandParent->stealParent(parent, rootptr);
		parent->small = grandParent;
		grandParent->parent = parent;
		grandParent->large = sibling;
		grandParent->height--;
		if (sibling != nullptr)
			sibling->parent = grandParent;
	}
	stringTable[newval] = newstr;
	return newval;
}

__always_inline void StringTree::SwapEntries(StringTreeEntry *a,
					     StringTreeEntry *b)
{
	TString *tmp;
	event_t et;
	int height;
	tmp = a->str;
	et = a->eventType;
	height = a->height;
	a->str = b->str;
	a->eventType = b->eventType;
	a->height = b->height;
	b->str = tmp;
	b->eventType = et;
	b->height = height;
}

__always_inline void StringTreeEntry::stealParent(StringTreeEntry *newChild,
						  StringTreeEntry **rootptr)
{
	newChild->parent = parent;
	if (parent == nullptr) {
		/* Oops this is a root node */
		*rootptr = newChild;
		return;
	}
	if (parent->small == this)
		parent->small = newChild;
	else
		parent->large = newChild;
}

__always_inline void StringTreeEntry::setHeightFromChildren()
{
	int lh;
	int rh;

	lh = (small != nullptr) ? (small->height) : -1;
	rh = (large != nullptr) ? (large->height) : -1;
	height = TSMAX(lh, rh) + 1;
}

#endif /* STRINGTREE_H */
