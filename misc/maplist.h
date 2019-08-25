// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2019 Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MAPLIST_H
#define _MAPLIST_H

#include <QMultiMap>
#include <QList>

#include <cstdint>
#include <cstdlib>

template<class T, class U>
class MapListElement {
public:
	MapListElement();
	MapListElement(const T &k, const U &v);
	MapListElement<T, U> *next;
	MapListElement<T, U> *prev;
	T key;
	U value;
};

template<class T, class U>
MapListElement<T, U>::MapListElement():
	next(nullptr), prev(nullptr), key(T()), value(U())
{}

template<class T, class U>
MapListElement<T, U>::MapListElement(const T &k, const U &v):
	next(nullptr), prev(nullptr), key(k), value(v)
{}

template<class T, class U>
class MapList {
public:
	MapList();
	~MapList();
	class iterator {
		friend class MapList<T, U>;
	public:
		iterator operator++(int);
		iterator operator--(int);
		U &operator*() const;
		U *operator->() const;
		const T &key() const;
		U &value() const;
		bool operator==(iterator i) const;
		bool operator!=(iterator i) const;
	protected:
		MapListElement<T, U> *ptr;
	};
	class const_iterator {
		friend class MapList<T, U>;
	public:
		const_iterator operator++(int);
		const_iterator operator--(int);
		const U &operator*() const;
		const U *operator->() const;
		const T &key() const;
		const U &value() const;
		bool operator==(const_iterator i) const;
		bool operator!=(const_iterator i) const;
	protected:
		const MapListElement<T, U> *ptr;
	};
	class reverse_iterator {
		friend class MapList<T, U>;
	public:
		reverse_iterator operator++(int);
		reverse_iterator operator--(int);
		U &operator*() const;
		U *operator->() const;
		const T &key() const;
		U &value() const;
		bool operator==(reverse_iterator i) const;
		bool operator!=(reverse_iterator i) const;
	protected:
		MapListElement<T, U> *ptr;
	};
	class const_reverse_iterator {
		friend class MapList<T, U>;
	public:
		const_reverse_iterator operator++(int);
		const_reverse_iterator operator--(int);
		const U &operator*() const;
		const U *operator->() const;
		const T &key() const;
		const U &value() const;
		bool operator==(const_reverse_iterator i) const;
		bool operator!=(const_reverse_iterator i) const;
	protected:
		const MapListElement<T, U> *ptr;
	};
	void append(const T &key, const U &value);
	void prepend(const T &key, const U &value);
	bool remove(const T &key);
	void remove(iterator i);
	void remove(reverse_iterator i);
	bool isEmpty() const;
	const U &lastValue() const;
	const U &last() const;
	U &last();
	bool contains(const T &key) const;
	int size() const;
	void clear();
	iterator begin();
	iterator end();
	const_iterator cbegin() const;
	const_iterator cend() const;
	reverse_iterator rbegin();
	reverse_iterator rend();
	const_reverse_iterator crbegin() const;
	const_reverse_iterator crend() const;
private:
	void deleteAll();
	void removeElement(MapListElement<T, U> *e);
	int mysize;
	MapListElement<T, U> *newElement(const T &key, const U &value);
	QMap<T, MapListElement<T, U>*> elementMap;
	MapListElement<T, U> head;
};

template<class T, class U>
MapList<T, U>::MapList():
mysize(0)
{
	head.next = &head;
	head.prev = &head;
}

template<class T, class U>
MapList<T, U>::~MapList()
{
	deleteAll();
}

template<class T, class U>
void MapList<T, U>::clear()
{
	elementMap.clear();
	deleteAll();
	mysize = 0;
}

template<class T, class U>
void MapList<T, U>::deleteAll()
{
	MapListElement<T, U> *i = head.next;
	MapListElement<T, U> *p;
	while (i != &head) {
		p = i;
		i = i->next;
		delete p;
	}
	head.next = &head;
	head.prev = &head;
}

template<class T, class U>
void MapList<T, U>::append(const T &key, const U &value)
{
	MapListElement<T, U> *elem = newElement(key, value);
	elem->prev = head.prev;
	elem->next = &head;
	head.prev = elem;
	elem->prev->next = elem;
}

template<class T, class U>
void MapList<T, U>::prepend(const T &key, const U &value)
{
	MapListElement<T, U> *elem = newElement(key, value);
	elem->next = head.next;
	elem->prev = &head;
	head.next = elem;
	elem->next->prev = elem;
}

template<class T, class U>
MapListElement<T, U> *MapList<T, U>::newElement(const T &key, const U &value)
{
	MapListElement<T, U> *elem = new MapListElement<T, U>(key, value);
	elementMap.insert(key, elem);
	mysize++;
	return elem;
}

template<class T, class U>
bool MapList<T, U>::remove(const T &key)
{
	MapListElement<T, U> *element;

	typename QMap<T, MapListElement<T, U>*>::iterator iter;
	iter = elementMap.find(key);
	if (iter == elementMap.end())
		return false;

	element = iter.value();
	removeElement(element);
	return true;
}

template<class T, class U>
void MapList<T, U>::remove(iterator i)
{
	removeElement(i.ptr);
}

template<class T, class U>
void MapList<T, U>::remove(reverse_iterator i)
{
	removeElement(i.ptr);
}

template<class T, class U>
void MapList<T, U>::removeElement(MapListElement<T, U> *e)
{
	elementMap.remove(e->key);
	e->next->prev = e->prev;
	e->prev->next = e->next;
	delete e;
	mysize--;
}

template<class T, class U>
bool MapList<T, U>::isEmpty() const
{
	return head.next == &head;
}

template<class T, class U>
const U &MapList<T, U>::lastValue() const
{
	return head.prev->value;
}

template<class T, class U>
U &MapList<T, U>::last()
{
	return head.prev->value;
}

template<class T, class U>
const U &MapList<T, U>::last() const
{
	return head.prev->value;
}

template<class T, class U>
bool MapList<T, U>::contains(const T &key) const
{
	return elementMap.contains(key);
}

template<class T, class U>
int MapList<T, U>::size() const
{
	return mysize;
}

template<class T, class U>
typename MapList<T, U>::iterator MapList<T, U>::begin()
{
	typename MapList<T, U>::iterator r;
	r.ptr = head.next;
	return r;
}

template<class T, class U>
typename MapList<T, U>::iterator MapList<T, U>::end()
{
	typename MapList<T, U>::iterator r;
	r.ptr = &head;
	return r;
}

template<class T, class U>
typename MapList<T, U>::iterator MapList<T, U>::iterator::operator++(int)
{
	typename MapList<T, U>::iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
typename MapList<T, U>::iterator MapList<T, U>::iterator::operator--(int)
{
	typename MapList<T, U>::iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
bool MapList<T, U>::iterator::operator==(iterator i) const
{
	return ptr == i.ptr;
}

template<class T, class U>
bool MapList<T, U>::iterator::operator!=(iterator i) const
{
	return ptr != i.ptr;
}

template<class T, class U>
U &MapList<T, U>::iterator::operator*() const
{
	return ptr->value;
}

template<class T, class U>
U *MapList<T, U>::iterator::operator->() const
{
	return &ptr->value;
}

template<class T, class U>
const T &MapList<T, U>::iterator::key() const
{
	return ptr->key;
}

template<class T, class U>
U &MapList<T, U>::iterator::value() const
{
	return ptr->value;
}

template<class T, class U>
typename MapList<T, U>::const_iterator MapList<T, U>::cbegin() const
{
	typename MapList<T, U>::const_iterator r;
	r.ptr = head.next;
	return r;
}


template<class T, class U>
typename MapList<T, U>::const_iterator MapList<T, U>::cend() const
{
	typename MapList<T, U>::const_iterator r;
	r.ptr = &head;
	return r;
}

template<class T, class U>
typename MapList<T, U>::const_iterator MapList<T, U>::const_iterator::operator++(int)
{
	typename MapList<T, U>::const_iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
typename MapList<T, U>::const_iterator MapList<T, U>::const_iterator::operator--(int)
{
	typename MapList<T, U>::const_iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
bool MapList<T, U>::const_iterator::operator==(const_iterator i) const
{
	return ptr == i.ptr;
}

template<class T, class U>
bool MapList<T, U>::const_iterator::operator!=(const_iterator i) const
{
	return ptr != i.ptr;
}

template<class T, class U>
const U &MapList<T, U>::const_iterator::operator*() const
{
	return ptr->value;
}

template<class T, class U>
const U *MapList<T, U>::const_iterator::operator->() const
{
	return &ptr->value;
}

template<class T, class U>
const T &MapList<T, U>::const_iterator::key() const
{
	return ptr->key;
}

template<class T, class U>
const U &MapList<T, U>::const_iterator::value() const
{
	return ptr->value;
}

template<class T, class U>
typename MapList<T, U>::reverse_iterator MapList<T, U>::rbegin()
{
	typename MapList<T, U>::reverse_iterator r;
	r.ptr = head.prev;
	return r;
}

template<class T, class U>
typename MapList<T, U>::reverse_iterator MapList<T, U>::rend()
{
	typename MapList<T, U>::reverse_iterator r;
	r.ptr = &head;
	return r;
}

template<class T, class U>
typename MapList<T, U>::reverse_iterator
MapList<T, U>::reverse_iterator::operator++(int)
{
	typename MapList<T, U>::reverse_iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
typename MapList<T, U>::reverse_iterator
MapList<T, U>::reverse_iterator::operator--(int)
{
	typename MapList<T, U>::reverse_iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
bool MapList<T, U>::reverse_iterator::operator==(reverse_iterator i) const
{
	return ptr == i.ptr;
}

template<class T, class U>
bool MapList<T, U>::reverse_iterator::operator!=(reverse_iterator i) const
{
	return ptr != i.ptr;
}

template<class T, class U>
U &MapList<T, U>::reverse_iterator::operator*() const
{
	return ptr->value;
}

template<class T, class U>
U *MapList<T, U>::reverse_iterator::operator->() const
{
	return &ptr->value;
}

template<class T, class U>
const T &MapList<T, U>::reverse_iterator::key() const
{
	return ptr->key;
}

template<class T, class U>
U &MapList<T, U>::reverse_iterator::value() const
{
	return ptr->value;
}

template<class T, class U>
typename MapList<T, U>::const_reverse_iterator MapList<T, U>::crbegin() const
{
	typename MapList<T, U>::const_reverse_iterator r;
	r.ptr = head.prev;
	return r;
}

template<class T, class U>
typename MapList<T, U>::const_reverse_iterator MapList<T, U>::crend() const
{
	typename MapList<T, U>::const_reverse_iterator r;
	r.ptr = &head;
	return r;
}

template<class T, class U>
typename MapList<T, U>::const_reverse_iterator
MapList<T, U>::const_reverse_iterator::operator++(int)
{
	typename MapList<T, U>::const_reverse_iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
typename MapList<T, U>::const_reverse_iterator
MapList<T, U>::const_reverse_iterator::operator--(int)
{
	typename MapList<T, U>::const_reverse_iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T, class U>
bool MapList<T, U>::const_reverse_iterator::operator==(const_reverse_iterator i)
	const
{
	return ptr == i.ptr;
}

template<class T, class U>
bool MapList<T, U>::const_reverse_iterator::operator!=(const_reverse_iterator i)
	const
{
	return ptr != i.ptr;
}

template<class T, class U>
const U &MapList<T, U>::const_reverse_iterator::operator*() const
{
	return ptr->value;
}

template<class T, class U>
const U *MapList<T, U>::const_reverse_iterator::operator->() const
{
	return &ptr->value;
}

template<class T, class U>
const T &MapList<T, U>::const_reverse_iterator::key() const
{
	return ptr->key;
}

template<class T, class U>
const U &MapList<T, U>::const_reverse_iterator::value() const
{
	return ptr->value;
}

#endif /* MAPLIST_H  */
