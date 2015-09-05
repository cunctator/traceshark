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

#ifndef STRINGTREE_H
#define STRINGTREE_H

#include <cstdint>
#include "mempool.h"
#include "src/traceshark.h"
#include "src/tstring.h"
#include "src/traceevent.h"

enum StringTreeColor { SP_RED, SP_BLACK };

class StringTreeEntry {
public:
	StringTreeEntry *small;
	StringTreeEntry *large;
	StringTreeEntry *parent;
	TString *str;
	event_t eventType;
	enum StringTreeColor color;
};

using namespace TraceShark;

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

/* For now this will use the RB-tree that is copied from the StringPool 
 * class but over time it would make sense to change this to an AVL-tree,
 * since this data structure is meant to store event names and the number
 * of event names is extremely small compared to the total number of events.
 * Events are only inserted once, meaning that the number of events names 
 * equals the number of inserts and the number of events equals the number
 * of lookups */
__always_inline TString* StringTree::searchAllocString(const TString *str,
						       uint32_t hval,
						       event_t *foundval,
						       event_t newval)
{
	TString *newstr;
	StringTreeEntry **aentry;
	StringTreeEntry *entry;
	StringTreeEntry *parent = NULL;
	StringTreeEntry *pSibling;
	StringTreeEntry *grandParent;
	int cmp;
	
	*foundval = newval;
	hval = hval % hSize;
	aentry = hashTable + hval;
iterate:
	entry = *aentry;
	if (entry == NULL) {
		newstr = (TString*) strPool->allocObj();
		if (newstr == NULL)
			return NULL;
		newstr->len = str->len;
		newstr->ptr = (char*) charPool->allocChars(str->len + 1);
		if (newstr->ptr == NULL)
			return NULL;
		strncpy(newstr->ptr, str->ptr, str->len + 1);
		entry = (StringTreeEntry*) entryPool->allocObj();
		if (entry == NULL)
			return NULL;
		bzero(entry, sizeof(StringTreeEntry));
		entry->str = newstr;
		entry->eventType = newval;
		*aentry = entry;
		entry->parent = parent;
		if (parent == NULL) {
			/* Ok, this is the root node */
			entry->color = SP_BLACK;
			return newstr;
		}
		entry->color = SP_RED;
		if (parent->color == SP_BLACK)
			return newstr;
	recheck:
		/* Now we now that the parent is red and we need to reshuffle */
		grandParent = parent->parent;
		pSibling = grandParent->small == parent ?
			grandParent->large:grandParent->small;
		if (pSibling == NULL || pSibling->color == SP_BLACK) {
			/* Case 2a */
			/* Reshuffle */
			entry->parent = grandParent;
			if (grandParent->small == parent) {
				if (parent->small == entry) {
					SwapEntries(parent, grandParent);
					grandParent->small = entry;
					grandParent->large = parent;
					parent->small = parent->large;
					parent->large = pSibling;
					if (pSibling != NULL)
						pSibling->parent = parent;
				} else { /* parent->large == entry */
					SwapEntries(grandParent, entry);
					parent->large = entry->small;
					if (parent->large != NULL)
						parent->large->parent = parent;
					entry->small = entry->large;
					grandParent->large = entry;
					entry->large = pSibling;
					if (pSibling != NULL)
						pSibling->parent = entry;
				}
			} else { /* grandParent->large == parent */
				if (parent->small == entry) {
					SwapEntries(grandParent, entry);
					parent->small = entry->large;
					if (parent->small != NULL)
						parent->small->parent = parent;
					entry->large = entry->small;
					grandParent->small = entry;
					entry->small = pSibling;
					if (pSibling != NULL)
						pSibling->parent = entry;
				} else {  /* parent->large == entry */
					SwapEntries(parent, grandParent);
					grandParent->large = entry;
					grandParent->small = parent;
					parent->large = parent->small;
					parent->small = pSibling;
					if (pSibling != NULL)
						pSibling->parent = parent;
				}
			}
			return newstr;
		}
		// Q_ASSERT(pSibling->color == SP_RED);
		/* Do recolor */
		parent->color = SP_BLACK;
		pSibling->color = SP_BLACK;
		/* Don't recolor grandParent if it's the root */
		if (grandParent->parent != NULL) {
			grandParent->color = SP_RED;
			if (grandParent->parent->color == SP_RED) {
				/* Ok, we have created a red violation */
				entry = grandParent;
				parent = entry->parent;
				goto recheck;
			}
		}
		return newstr;
	}
	/* Would be difficult to use strncmp here, string should be null
	 * terminated... */
	cmp = TSstrcmp(str, entry->str);
	if (cmp == 0) {
		*foundval = entry->eventType;
		return entry->str;
	}
	parent = entry;
	if (cmp < 0) {
		aentry = &entry->small;
		goto iterate;
	}
	/*  cmp must be > 0, since not 0 and not < 0 */
	aentry = &entry->large;
	goto iterate;
}

__always_inline void StringTree::SwapEntries(StringTreeEntry *a,
					     StringTreeEntry *b)
{
	TString *tmp;
	event_t et;
	tmp = a->str;
	et = a->eventType;
	a->str = b->str;
	a->eventType = b->eventType;
	b->str = tmp;
	b->eventType = et;
}

#endif /* STRINGTREE_H */
