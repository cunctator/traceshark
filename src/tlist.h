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

#ifndef TLIST_H
#define TLIST_H

#include "traceshark.h"

extern "C" {
#include <sys/mman.h>
}

#define TLIST_MAP_NR_ELEMENTS (0x100000)
#define TLIST_MAP_ELEMENT_MASK (TLIST_MAP_NR_ELEMENTS - 1)
#define TLIST_MAP_MASK (0xffffffff - TLIST_MAP_ELEMENT_MASK)

template<class T>
class TList
{
public:
	TList();
	~TList();
	__always_inline void append(const T &element);
	__always_inline T& increase();
	__always_inline T& preAlloc();
	__always_inline void commit();
	__always_inline T value(unsigned int index);
	__always_inline const T& at(unsigned int index);
	__always_inline T& last();
	__always_inline unsigned int size();
	void clear();
	__always_inline T& operator[](unsigned int index);
private:
	__always_inline unsigned int mapFromIndex(unsigned int index);
	__always_inline unsigned int mapIndexFromIndex(unsigned int index);
	void clearAll();
	void setupMem();
	void addMem();
	void decMem();
	unsigned int nrMaps;
	unsigned int nrElements;
	T **mapArray;
};

template<class T>
TList<T>::TList():
nrMaps(0), nrElements(0)
{
	setupMem();
}

template<class T>
TList<T>::~TList()
{
	clearAll();
}

template<class T>
__always_inline unsigned int TList<T>::mapFromIndex(unsigned int index)
{
	return (index >> 20);
}

template<class T>
__always_inline unsigned int TList<T>::mapIndexFromIndex(unsigned int index)
{
	return (index & TLIST_MAP_ELEMENT_MASK);
}

template<class T>
void TList<T>::setupMem()
{
	unsigned int maxNrMaps = mapFromIndex(TLIST_MAP_MASK) + 1;
	mapArray = (T**) mmap(NULL, (size_t) maxNrMaps * sizeof(T*),
			     PROT_READ | PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mapArray == MAP_FAILED)
		mapArray = NULL; /* let's die from null pointer if failed */
	addMem();
}

template<class T>
void TList<T>::addMem()
{
	mapArray[nrMaps] = (T*) mmap(NULL, (size_t) TLIST_MAP_NR_ELEMENTS *
				sizeof(T), PROT_READ | PROT_WRITE,
				MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (mapArray[nrMaps] == MAP_FAILED)
		mapArray[nrMaps] = NULL; /* let's die from null pointer if...*/
	nrMaps++;
}

template<class T>
void TList<T>::decMem()
{
	nrMaps--;
	munmap(mapArray[nrMaps], TLIST_MAP_NR_ELEMENTS * sizeof(T));
}

template<class T>
void TList<T>::clearAll()
{
	unsigned int maxNrMaps = mapFromIndex(TLIST_MAP_MASK) + 1;
	unsigned int i;
	for (i = 0; i < nrMaps; i++)
		munmap(mapArray[i], TLIST_MAP_NR_ELEMENTS * sizeof(T));
	munmap(mapArray, maxNrMaps * sizeof(T*));
	nrMaps = 0;
	nrElements = 0;
}

template<class T>
__always_inline T& TList<T>::increase()
{
	unsigned int map = mapFromIndex(nrElements);
	unsigned int mapIndex = mapIndexFromIndex(nrElements);
	nrElements++;
	if (map == nrMaps)
		addMem();
	return mapArray[map][mapIndex];
}

template<class T>
__always_inline T& TList<T>::preAlloc()
{
	unsigned int map = mapFromIndex(nrElements);
	unsigned int mapIndex = mapIndexFromIndex(nrElements);
	if (map == nrMaps)
		addMem();
	return mapArray[map][mapIndex];
}

template<class T>
__always_inline void TList<T>::commit()
{
	nrElements++;
}

template<class T>
__always_inline void TList<T>::append(const T &element)
{
	unsigned int map = mapFromIndex(nrElements);
	unsigned int mapIndex = mapIndexFromIndex(nrElements);
	nrElements++;
	if (map == nrMaps)
		addMem();
	mapArray[map][mapIndex] = element;
}

template<class T>
__always_inline const T& TList<T>::at(unsigned int index)
{
	unsigned int map = mapFromIndex(index);
	unsigned int mapIndex = mapIndexFromIndex(index);
	return mapArray[map][mapIndex];
}

template<class T>
__always_inline T TList<T>::value(unsigned int index)
{
	if (index >= nrElements) {
		T dvalue;
		return dvalue;
	}
	return at(index);
}

template<class T>
__always_inline T& TList<T>::last()
{
	unsigned int index = nrElements - 1;
	unsigned int map = mapFromIndex(index);
	unsigned int mapIndex = mapIndexFromIndex(index);
	return mapArray[map][mapIndex];
	
}

template<class T>
__always_inline unsigned int TList<T>::size()
{
	return nrElements;
}

template<class T>
void TList<T>::clear()
{
	clearAll();
	setupMem();
}

template<class T>
__always_inline T& TList<T>::operator[](unsigned int index)
{
	unsigned int map = mapFromIndex(index);
	unsigned int mapIndex = mapIndexFromIndex(index);
	return mapArray[map][mapIndex];
}


#endif /* TLIST_H */
