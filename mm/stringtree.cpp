/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
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

#include <cstring>
#include "mm/stringtree.h"

#define MAX(A, B) ((A) >= (B) ? A:B)
#define MIN(A, B) ((A) < (B) ? A:B)

StringTree::StringTree(unsigned int nr_pages, unsigned int hSizeP)
{
	unsigned int entryPages, strPages;

	if (hSizeP == 0)
		hSize = 1;
	else
		hSize = hSizeP;

	entryPages = 2 * hSize * sizeof(StringTreeEntry) / 4096;
	entryPages = MAX(1, entryPages);
	strPages = 2* hSize * sizeof(TString) / 4096;
	strPages = MAX(16, strPages);

	strPool = new MemPool(strPages, sizeof(TString));
	charPool = new MemPool(nr_pages, 1);
	entryPool = new MemPool(entryPages, sizeof(StringTreeEntry));

	hashTable = new StringTreeEntry*[hSize];
	clearTable();
}

StringTree::~StringTree()
{
	delete charPool;
	delete strPool;
	delete entryPool;
	delete[] hashTable;
}

void StringTree::clearTable()
{
	bzero(hashTable, hSize * sizeof(StringTreeEntry*));
}

void StringTree::clear()
{
	clearTable();
	strPool->reset();
	entryPool->reset();
	charPool->reset();
}

void StringTree::reset()
{
	clear();
}
