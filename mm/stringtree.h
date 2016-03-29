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
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef STRINGTREE_H
#define STRINGTREE_H

#include <cstdint>
#include "mempool.h"
#include "src/traceshark.h"
#include "src/tstring.h"
#include "src/traceevent.h"

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
	StringTree(unsigned int nr_pages = 256*10, unsigned int hSizeP = 256);
	~StringTree();
	__always_inline TString* searchAllocString(const TString *str,
						   uint32_t hval,
						   event_t *foundval,
						   event_t newval);
	void clear();
	void reset();
	__always_inline void SwapEntries(StringTreeEntry *a,
					 StringTreeEntry *b);
private:
	MemPool *strPool;
	MemPool *charPool;
	MemPool *entryPool;
	StringTreeEntry **hashTable;
	unsigned int hSize;
	char *curPtr;
	void clearTable();
};

/* We use and  AVL-tree here, since this data structure is meant to store
 * event names and the number of event names is extremely small compared to
 * the total number of events. The number of different events names equals the
 * number of inserts and the number of events equals the number
 * of lookups */
__always_inline TString* StringTree::searchAllocString(const TString *str,
						       uint32_t hval,
						       event_t *foundval,
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

	*foundval = newval;
	hval = hval % hSize;
	aentry = hashTable + hval;
	rootptr = aentry;
	entry = *aentry;

	while (entry != nullptr) {
		/* Using strncmp here would lose performance and we know that
		 * the strings are null terminated */
		cmp = strcmp(str->ptr, entry->str->ptr);
		if (cmp == 0) {
			*foundval = entry->eventType;
			return entry->str;
		}
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

	newstr = (TString*) strPool->allocObj();
	if (newstr == nullptr)
		return nullptr;
	newstr->len = str->len;
	newstr->ptr = (char*) charPool->allocChars(str->len + 1);
	if (newstr->ptr == nullptr)
		return nullptr;
	strncpy(newstr->ptr, str->ptr, str->len + 1);
	entry = (StringTreeEntry*) entryPool->allocObj();
	if (entry == nullptr)
		return nullptr;
	bzero(entry, sizeof(StringTreeEntry));
	entry->str = newstr;
	entry->eventType = newval;
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
	return newstr;
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
	return newstr;
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
