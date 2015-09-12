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
	StringPool(unsigned int nr_pages = 256*10, unsigned int hSizeP = 256);
	~StringPool();
	__always_inline TString* allocString(const TString *str, uint32_t hval);
	void clear();
	void reset();
private:
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
						 uint32_t hval)
{
	TString *newstr;
	StringPoolEntry **aentry;
	StringPoolEntry *entry;
	StringPoolEntry *parent = NULL;
	StringPoolEntry *grandParent;
	StringPoolEntry *smallChild;
	StringPoolEntry *largeChild;
	int cmp;
	int diff;
	int smallH;
	int largeH;
	int gHeight;

	hval = hval % hSize;

	if (usageTable[hval] > 100) {
		newstr = (TString*) strPool->allocObj();
		if (newstr == NULL)
			return NULL;
		newstr->len = str->len;
		newstr->ptr = (char*) charPool->allocChars(str->len + 1);
		if (newstr->ptr == NULL)
			return NULL;
		strncpy(newstr->ptr, str->ptr, str->len + 1);
		return newstr;
	}

	aentry = hashTable + hval;
iterate:
	entry = *aentry;
	if (entry == NULL) {
		usageTable[hval]++;
		newstr = (TString*) strPool->allocObj();
		if (newstr == NULL)
			return NULL;
		newstr->len = str->len;
		newstr->ptr = (char*) charPool->allocChars(str->len + 1);
		if (newstr->ptr == NULL)
			return NULL;
		strncpy(newstr->ptr, str->ptr, str->len + 1);
		entry = (StringPoolEntry*) entryPool->allocObj();
		if (entry == NULL)
			return NULL;
		bzero(entry, sizeof(StringPoolEntry));
		entry->str = newstr;
		*aentry = entry; /* aentry is equal to &parent->[large|small] */

		entry->parent = parent;
		entry->height = 0;
		if (parent == NULL)
			return newstr; /* Ok, this is the root node */
		if (parent->height > 0)
			return newstr; /* parent already has another node */
		parent->height = 1;
		grandParent = parent->parent;
		/* update heights and find offending node */
		while(grandParent != NULL) {
			smallH = grandParent->small == NULL ?
				-1 : grandParent->small->height;
			largeH = grandParent->large == NULL ?
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
			if (parent->large != NULL)
				parent->large->parent = parent;

			grandParent->large = parent;
			parent->swapEntries(grandParent);
		} else {
			/* Case 2 */
			smallChild = entry->small;
			largeChild = entry->large;
			gHeight = 0;
			if (grandParent->large != NULL) {
				gHeight = grandParent->large->height + 1;
				grandParent->large->parent = entry;
			}

			parent->large = smallChild;
			if (smallChild != NULL)
				smallChild->parent = parent;
			parent->height = grandParent->height - 1;

			entry->large = grandParent->large;
			entry->small = largeChild;
			if (largeChild != NULL)
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
			if (grandParent->small != NULL) {
			        gHeight = grandParent->small->height + 1;
				grandParent->small->parent = entry;
			}

			parent->small = largeChild;
			if (largeChild != NULL)
				largeChild->parent = parent;
			parent->height = grandParent->height - 1;

			entry->small = grandParent->small;
			entry->large = smallChild;
			if (smallChild != NULL)
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
			if (parent->small != NULL)
				parent->small->parent = parent;

			grandParent->small = parent;
			parent->swapEntries(grandParent);
		}
		return newstr;
	}
	/* Using strncmp here would lose performance and we know that the
	 * strings are null terminated */
	cmp = strcmp(str->ptr, entry->str->ptr);
	if (cmp == 0)
		return entry->str;
	parent = entry;
	if (cmp < 0) {
		aentry = &entry->small;
		goto iterate;
	}
	/*  cmp must be > 0, since not 0 and not < 0 */
	aentry = &entry->large;
	goto iterate;
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

	lh = (small != NULL) ? (small->height) : -1;
	rh = (large != NULL) ? (large->height) : -1;
	height = TSMAX(lh, rh) + 1;
}



#endif /* STRINGPOOL_H */
