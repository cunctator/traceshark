/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
ema *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STRINGPOOL_H
#define STRINGPOOL_H

#include <cstdint>
#include "mempool.h"
#include "src/traceshark.h"
#include "src/tstring.h"

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
	char *curPtr;
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
		cmp = TString::strcmp(str, entry->str);
		if (cmp == 0)
			return entry->str;
		parent = entry;
		if (cmp < 0) {
			aentry = &entry->small;
			entry = *aentry;
			continue;
		}
		/*  cmp must be > 0, since not 0 and not < 0 */
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
	*aentry = entry; /* aentry is equal to &parent->[large|small] */

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
