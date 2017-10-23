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
#include "mm/mempool.h"
#include "misc/traceshark.h"
#include "misc/tstring.h"

class StringPoolEntry {
public:
	StringPoolEntry *small;
	StringPoolEntry *large;
	StringPoolEntry *parent;
	TString *str;
	int height;
	__always_inline void setHeightFromChildren();
	__always_inline void swapEntries(StringPoolEntry *entry);
};

class StringPool
{
public:
	StringPool(unsigned int nr_pages = 256 * 10, unsigned int hSizeP = 256);
	~StringPool();
	__always_inline TString* allocString(const TString *str, uint32_t hval,
					     uint32_t cutoff);
	void clear();
	void reset();
private:
	__always_inline TString* allocUniqueString(const TString *str);
	MemPool *strPool;
	MemPool *charPool;
	MemPool *entryPool;
	StringPoolEntry **hashTable;
	unsigned int *usageTable;
	unsigned int hSize;
	void clearTable();
};

__always_inline TString* StringPool::allocString(const TString *str,
						 uint32_t hval,
						 uint32_t cutoff)
{
	TString *newstr;
	StringPoolEntry **aentry;
	StringPoolEntry *entry;
	StringPoolEntry *parent = nullptr;
	StringPoolEntry *grandParent;
	StringPoolEntry *smallChild;
	StringPoolEntry *largeChild;
	int cmp;
	int diff;
	int smallH;
	int largeH;
	int gHeight;

	hval = hval % hSize;

	if (cutoff != 0 && usageTable[hval] > cutoff) {
		newstr = allocUniqueString(str);
		return newstr;
	}

	aentry = hashTable + hval;
	entry = *aentry;

	while(entry != nullptr) {
		cmp = strcmp(str->ptr, entry->str->ptr);
		if (cmp == 0)
			return entry->str;
		parent = entry;
		if (cmp < 0)
			aentry = &entry->small;
		else
			aentry = &entry->large;
		entry = *aentry;
	}

	newstr = allocUniqueString(str);
	if (newstr == nullptr)
		return newstr;
	if (cutoff != 0)
		usageTable[hval]++;

	entry = (StringPoolEntry*) entryPool->allocObj();
	if (entry == nullptr)
		return nullptr;
	bzero(entry, sizeof(StringPoolEntry));
	entry->str = newstr;
	/* aentry is equal to &parent->[large|small] */
	*aentry = entry;

	entry->parent = parent;
	entry->height = 0;
	if (parent == nullptr)
		return newstr; /* Ok, this is the root node */
	if (parent->height > 0)
		return newstr; /* parent already has another node */
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
	return newstr;
rebalanceSmall:
	/* Do small rebalance here (case 1 and case 2) */
	if (entry == parent->small) {
		/* Case 1 */
		grandParent->small = entry;
		entry->parent = grandParent;

		parent->small = parent->large;
		parent->height--;

		parent->large = grandParent->large;
		if (parent->large != nullptr)
			parent->large->parent = parent;

		grandParent->large = parent;
		parent->swapEntries(grandParent);
	} else {
		/* Case 2 */
		smallChild = entry->small;
		largeChild = entry->large;
		gHeight = 0;
		if (grandParent->large != nullptr) {
			gHeight = grandParent->large->height + 1;
			grandParent->large->parent = entry;
		}

		parent->large = smallChild;
		if (smallChild != nullptr)
			smallChild->parent = parent;
		parent->height = grandParent->height - 1;

		entry->large = grandParent->large;
		entry->small = largeChild;
		if (largeChild != nullptr)
			largeChild->parent = entry;

		grandParent->large = entry;
		entry->parent = grandParent;

		entry->height = gHeight;
		entry->swapEntries(grandParent);
	}
	return newstr;
rebalanceLarge:
	/* Do large rebalance here */
	if (entry == parent->small) {
		/* Case 3 */
		smallChild = entry->small;
		largeChild = entry->large;
		gHeight = 0;
		if (grandParent->small != nullptr) {
			gHeight = grandParent->small->height + 1;
			grandParent->small->parent = entry;
		}

		parent->small = largeChild;
		if (largeChild != nullptr)
			largeChild->parent = parent;
		parent->height = grandParent->height - 1;

		entry->small = grandParent->small;
		entry->large = smallChild;
		if (smallChild != nullptr)
			smallChild->parent = entry;

		grandParent->small = entry;
		entry->parent = grandParent;

		entry->height = gHeight;
		entry->swapEntries(grandParent);
	} else {
		/* Case 4 */
		grandParent->large = entry;
		entry->parent = grandParent;

		parent->large = parent->small;
		parent->height--;

		parent->small = grandParent->small;
		if (parent->small != nullptr)
			parent->small->parent = parent;

		grandParent->small = parent;
		parent->swapEntries(grandParent);
	}
	return newstr;
}

__always_inline TString* StringPool::allocUniqueString(const TString *str)
{
	TString *newstr;

	newstr = (TString*) strPool->allocObj();
	if (newstr == nullptr)
		return nullptr;
	newstr->len = str->len;
	newstr->ptr = (char*) charPool->allocChars(str->len + 1);
	if (newstr->ptr == nullptr)
		return nullptr;
	strncpy(newstr->ptr, str->ptr, str->len + 1);
	return newstr;
}

__always_inline void StringPoolEntry::swapEntries(StringPoolEntry *entry)
{
	TString *tmp;
	tmp = entry->str;
	entry->str = str;
	str = tmp;
}

__always_inline void StringPoolEntry::setHeightFromChildren()
{
	int lh;
	int rh;

	lh = (small != nullptr) ? (small->height) : -1;
	rh = (large != nullptr) ? (large->height) : -1;
	height = TSMAX(lh, rh) + 1;
}



#endif /* STRINGPOOL_H */
