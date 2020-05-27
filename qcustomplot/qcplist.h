// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2019-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef QCPLIST_H
#define QCPLIST_H

#include <QMultiMap>
#include <QList>

#include <cstdint>
#include <cstdlib>

extern "C" {
#include <unistd.h>
}

#ifdef __has_attribute
#define qcp_has_attribute(x) __has_attribute(x)
#else
#define qcp_has_attribute(x) 0
#endif

#ifdef __always_inline
#define qcp_always_inline __always_inline

#elif defined(__inline__)
#define qcp_always_inline __inline__

#elif qcp_has_attribute(always_inline)
#define qcp_always_inline __inline __attribute__((always_inline))

#elif qcp_has_attribute(__always_inline__)
#define qcp_always_inline __inline __attribute__((__always_inline__))

#else
#define qcp_always_inline __inline

#endif /* #ifdef __always_inline  */

template<class T>
class QCPListElement {
public:
	QCPListElement();
	QCPListElement(const T &v);
	QCPListElement<T> *next;
	QCPListElement<T> *prev;
	T value;
	/*
	 * pos is used to keep track of the order of elements in case the same
	 * value has been inserted more than once, since QCPList<T>::removeOne()
	 * is required to remove the first occurrence of the value, in order to
	 * keep compatibility with QList<T>
	 */
	long long pos;
};

template<class T>
QCPListElement<T>::QCPListElement():
next(nullptr), prev(nullptr), value(T()), pos(0)
{}

template<class T>
QCPListElement<T>::QCPListElement(const T &v):
next(nullptr), prev(nullptr), value(v), pos(0)
{}

template<class T>
class QCPList {
public:
	QCPList();
	~QCPList();
	class iterator {
		friend class QCPList<T>;
	public:
		iterator operator++(int);
		iterator operator--(int);
		T &operator*();
		T *operator->();
		bool operator==(iterator i);
		bool operator!=(iterator i);
	protected:
		QCPListElement<T> *ptr;
	};
	class const_iterator {
		friend class QCPList<T>;
	public:
		const_iterator operator++(int);
		const_iterator operator--(int);
		const T &operator*();
		const T *operator->();
		bool operator==(const_iterator i);
		bool operator!=(const_iterator i);
	protected:
		const QCPListElement<T> *ptr;
	};
	class reverse_iterator {
		friend class QCPList<T>;
	public:
		reverse_iterator operator++(int);
		reverse_iterator operator--(int);
		T &operator*();
		T *operator->();
		bool operator==(reverse_iterator i);
		bool operator!=(reverse_iterator i);
	protected:
		QCPListElement<T> *ptr;
	};
	class const_reverse_iterator {
		friend class QCPList<T>;
	public:
		const_reverse_iterator operator++(int);
		const_reverse_iterator operator--(int);
		const T &operator*();
		const T *operator->();
		bool operator==(const_reverse_iterator i);
		bool operator!=(const_reverse_iterator i);
	protected:
		const QCPListElement<T> *ptr;
	};
	void append(const T &value);
	const T &at(int index) const;
	void prepend(const T &value);
	bool removeOne(const T &value);
	void removeOne(iterator i);
	void removeOne(reverse_iterator i);
	bool isEmpty() const;
	const T &last() const;
	T &last();
	bool contains(const T &value) const;
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
	T &operator[](int index);
	const T &operator[](int index) const;
private:
	qcp_always_inline T &_at(int index) const;
	void deleteAll();
	void removeElement(QCPListElement<T> *e);
	int mysize;
	QCPListElement<T> *newElement(const T &value);
	QMultiMap<T, QCPListElement<T>*> elementMap;
	QCPListElement<T> head;
};

template<class T>
QCPList<T>::QCPList():
mysize(0)
{
	head.next = &head;
	head.prev = &head;
	head.pos = 0LL;
}

template<class T>
QCPList<T>::~QCPList()
{
	deleteAll();
}

template<class T>
void QCPList<T>::clear()
{
	elementMap.clear();
	deleteAll();
	mysize = 0;
}

template<class T>
void QCPList<T>::deleteAll()
{
	QCPListElement<T> *i = head.next;
	QCPListElement<T> *p;
	while (i != &head) {
		p = i;
		i = i->next;
		delete p;
	}
	head.next = &head;
	head.prev = &head;
}

template<class T>
void QCPList<T>::append(const T&value)
{
	QCPListElement<T> *elem = newElement(value);
	elem->prev = head.prev;
	elem->pos = head.prev->pos + 1;
	elem->next = &head;
	head.prev = elem;
	elem->prev->next = elem;
}

template<class T>
T &QCPList<T>::_at(int i) const
{
	int j;
	QCPListElement<T> *elem = head.next;

	for (j = 0; j < i; j++)
		elem = elem->next;

	return elem->value;
}

template<class T>
const T &QCPList<T>::at(int i) const
{
	return _at(i);
}

template<class T>
void QCPList<T>::prepend(const T &value)
{
	QCPListElement<T> *elem = newElement(value);
	elem->next = head.next;
	elem->pos = head.next->pos - 1;
	elem->prev = &head;
	head.next = elem;
	elem->next->prev = elem;
}

template<class T>
QCPListElement<T> *QCPList<T>::newElement(const T &value)
{
	QCPListElement<T> *elem = new QCPListElement<T>(value);
	elementMap.insert(value, elem);
	mysize++;
	return elem;
}

template<class T>
bool QCPList<T>::removeOne(const T &value)
{
	long long min = LLONG_MAX;
	QCPListElement<T> *minElement = nullptr;
	QCPListElement<T> *e;

	QList<QCPListElement<T>*> values = elementMap.values(value);
	typename QList<QCPListElement<T>*>::iterator iter;
	for (iter = values.begin(); iter != values.end(); iter++) {
		e = *iter;
		if (e->pos < min) {
			minElement = e;
			min = e->pos;
		}
	}
	if (minElement != nullptr) {
		removeElement(minElement);
		return true;
	}
	return false;
}

template<class T>
void QCPList<T>::removeOne(iterator i)
{
	removeElement(i.ptr);
}

template<class T>
void QCPList<T>::removeOne(reverse_iterator i)
{
	removeElement(i.ptr);
}

template<class T>
void QCPList<T>::removeElement(QCPListElement<T> *e)
{
	elementMap.remove(e->value, e);
	e->next->prev = e->prev;
	e->prev->next = e->next;
	delete e;
	mysize--;
}

template<class T>
bool QCPList<T>::isEmpty() const
{
	return head.next == &head;
}

template<class T>
const T &QCPList<T>::last() const
{
	return head.prev->value;
}

template<class T>
T &QCPList<T>::last()
{
	return head.prev->value;
}

template<class T>
bool QCPList<T>::contains(const T &value) const
{
	return elementMap.contains(value);
}

template<class T>
int QCPList<T>::size() const
{
	return mysize;
}

template<class T>
typename QCPList<T>::iterator QCPList<T>::begin()
{
	typename QCPList<T>::iterator r;
	r.ptr = head.next;
	return r;
}


template<class T>
typename QCPList<T>::iterator QCPList<T>::end()
{
	typename QCPList<T>::iterator r;
	r.ptr = &head;
	return r;
}

template<class T>
typename QCPList<T>::iterator QCPList<T>::iterator::operator++(int)
{
	typename QCPList<T>::iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T>
typename QCPList<T>::iterator QCPList<T>::iterator::operator--(int)
{
	typename QCPList<T>::iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T>
bool QCPList<T>::iterator::operator==(iterator i)
{
	return ptr == i.ptr;
}

template<class T>
bool QCPList<T>::iterator::operator!=(iterator i)
{
	return ptr != i.ptr;
}

template<class T>
T &QCPList<T>::iterator::operator*()
{
	return ptr->value;
}

template<class T>
T *QCPList<T>::iterator::operator->()
{
	return &ptr->value;
}

template<class T>
typename QCPList<T>::const_iterator QCPList<T>::cbegin() const
{
	typename QCPList<T>::const_iterator r;
	r.ptr = head.next;
	return r;
}


template<class T>
typename QCPList<T>::const_iterator QCPList<T>::cend() const
{
	typename QCPList<T>::const_iterator r;
	r.ptr = &head;
	return r;
}

template<class T>
typename QCPList<T>::const_iterator QCPList<T>::const_iterator::operator++(int)
{
	typename QCPList<T>::const_iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T>
typename QCPList<T>::const_iterator QCPList<T>::const_iterator::operator--(int)
{
	typename QCPList<T>::const_iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T>
bool QCPList<T>::const_iterator::operator==(const_iterator i)
{
	return ptr == i.ptr;
}

template<class T>
bool QCPList<T>::const_iterator::operator!=(const_iterator i)
{
	return ptr != i.ptr;
}

template<class T>
const T &QCPList<T>::const_iterator::operator*()
{
	return ptr->value;
}

template<class T>
const T *QCPList<T>::const_iterator::operator->()
{
	return &ptr->value;
}

template<class T>
typename QCPList<T>::reverse_iterator QCPList<T>::rbegin()
{
	typename QCPList<T>::reverse_iterator r;
	r.ptr = head.prev;
	return r;
}

template<class T>
typename QCPList<T>::reverse_iterator QCPList<T>::rend()
{
	typename QCPList<T>::reverse_iterator r;
	r.ptr = &head;
	return r;
}

template<class T>
typename QCPList<T>::reverse_iterator
QCPList<T>::reverse_iterator::operator++(int)
{
	typename QCPList<T>::reverse_iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T>
typename QCPList<T>::reverse_iterator
QCPList<T>::reverse_iterator::operator--(int)
{
	typename QCPList<T>::reverse_iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T>
bool QCPList<T>::reverse_iterator::operator==(reverse_iterator i)
{
	return ptr == i.ptr;
}

template<class T>
bool QCPList<T>::reverse_iterator::operator!=(reverse_iterator i)
{
	return ptr != i.ptr;
}

template<class T>
T &QCPList<T>::reverse_iterator::operator*()
{
	return ptr->value;
}

template<class T>
T *QCPList<T>::reverse_iterator::operator->()
{
	return &ptr->value;
}

template<class T>
typename QCPList<T>::const_reverse_iterator QCPList<T>::crbegin() const
{
	typename QCPList<T>::const_reverse_iterator r;
	r.ptr = head.prev;
	return r;
}

template<class T>
typename QCPList<T>::const_reverse_iterator QCPList<T>::crend() const
{
	typename QCPList<T>::const_reverse_iterator r;
	r.ptr = &head;
	return r;
}

template<class T>
typename QCPList<T>::const_reverse_iterator
QCPList<T>::const_reverse_iterator::operator++(int)
{
	typename QCPList<T>::const_reverse_iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

template<class T>
typename QCPList<T>::const_reverse_iterator
QCPList<T>::const_reverse_iterator::operator--(int)
{
	typename QCPList<T>::const_reverse_iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

template<class T>
bool QCPList<T>::const_reverse_iterator::operator==(const_reverse_iterator i)
{
	return ptr == i.ptr;
}

template<class T>
bool QCPList<T>::const_reverse_iterator::operator!=(const_reverse_iterator i)
{
	return ptr != i.ptr;
}

template<class T>
const T &QCPList<T>::const_reverse_iterator::operator*()
{
	return ptr->value;
}

template<class T>
const T *QCPList<T>::const_reverse_iterator::operator->()
{
	return &ptr->value;
}

template<class T>
T &QCPList<T>::operator[](int i)
{
	return _at(i);
}

template<class T>
const T &QCPList<T>::operator[](int i) const
{
	return _at(i);
}

#endif /* QCPLIST_H  */
