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
#include "src/tstring.h"

class StringPoolEntry {
public:
	StringPoolEntry *small;
	StringPoolEntry *large;
	TString *str;
};

union value32 {
	uint32_t word32;
	uint8_t word8[4];
};

#define SPROL32(VALUE, N) \
	((VALUE << N) | (VALUE >> (32 - N)))

/* This is totally mumbo jumbo, should probably be replaced with something
 * more scientific */
__always_inline uint32_t StringHashFuncSimple32(const TString *str)
{
	union value32 uvalue;

	if (str->len < 1)
		return 0;

	uvalue.word8[0] = str->ptr[str->len - 1];
	uvalue.word8[1] = str->ptr[str->len / 2];
	uvalue.word8[2] = str->ptr[str->len / 3];
	uvalue.word8[4] = str->ptr[str->len / 4];
	//uvalue.word32 = SPROL32(uvalue.word32, len % 32);
	return uvalue.word32;
}

class StringPool
{
public:
	StringPool(unsigned int nr_pages = 256*10, unsigned int hSizeP = 256);
	~StringPool();
	__always_inline TString* allocString(const TString *str, uint32_t hval);
	void clear();
	void reset();
	__always_inline uint16_t hashFuncSimple(char *str, unsigned int len);
	__always_inline uint16_t hashFunc(char *str, unsigned int len);
private:
	MemPool *strPool;
	MemPool *charPool;
	MemPool *entryPool;
	StringPoolEntry** hashTable;
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
	int cmp;

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
		entry = (StringPoolEntry*) entryPool->allocObj();
		if (entry == NULL)
			return NULL;
		bzero(entry, sizeof(StringPoolEntry));
		entry->str = newstr;
		*aentry = entry;
		return newstr;
	}
	/* Would be difficult to use strncmp here, string should be null
	 * terminated... */
	cmp = strcmp(str->ptr, entry->str->ptr);
	if (cmp == 0)
		return entry->str;
	if (cmp < 0) {
		aentry = &entry->small;
		goto iterate;
	}
	/*  cmp must be > 0, since not 0 and not < 0 */
	aentry = &entry->large;
	goto iterate;
}

#endif /* STRINGPOOL_H */
