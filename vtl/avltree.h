
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

#ifndef __AVLTREE_H
#define __AVLTREE_H

#include <utility>
#include <cstdlib>

namespace vtl {

#define __AVLTREEMAX(A, B) ((A) >= (B) ? A:B)

template <class T>
class AVLSampleCompare {
public:
	__always_inline static int compare(const T &a, const T &b) {
		if (a < b)
			return -1;
		if (a > b)
			return 1;
		return 0;
	}
};

template <class T>
class AVLDefaultCompare {
};

/*
 * This is a type trait class that determines whether a class has the
 * compare(U, U) function.
 */
template <typename T, typename U>
class has_compare {
private:
	typedef char Yes;
	typedef Yes No[2];
	template<typename C> static auto Test(void*)
		-> decltype(int{std::declval<C const>().compare(U(), U())},
			    Yes{});

	template<typename> static No& Test(...);
public:
	static bool const value = sizeof(Test<T>(0)) == sizeof(Yes);
};

template<bool b> class __AVLFindSelect;
template<bool b> class __AVLFindValueSelect;

template <class T, class U, typename CF = AVLDefaultCompare<T>>
class AVLTree
{
	friend class __AVLFindSelect<false>;
	friend class __AVLFindValueSelect<false>;
	friend class __AVLFindSelect<true>;
	friend class __AVLFindValueSelect<true>;
protected:
	class AVLNode;
public:
	class iterator {
		friend class AVLTree<T, U, CF>;
	public:
		iterator();
		__always_inline const T &key() const;
		__always_inline U &value() const;
		__always_inline bool atEnd() const;
		__always_inline void next();
		__always_inline void prev();
		__always_inline iterator &operator++();
		__always_inline iterator operator++(int);
		__always_inline iterator &operator--();
		__always_inline iterator operator--(int);
		__always_inline bool operator!=(const iterator &other) const;
		__always_inline bool operator==(const iterator &other) const;
	protected:
		AVLNode *pos;
	};
	AVLTree(CF cf);
	AVLTree();
	~AVLTree();
	__always_inline void insert(const T &key, const U &value);
	__always_inline U &findValue(const T &key);
	__always_inline U value(const T &key, const U &defaultValue = U())
		const;
	__always_inline bool contains(const T &key) const;
	__always_inline bool isEmpty() const;
	__always_inline int size() const;
	__always_inline U &operator[](const T &key);
	__always_inline iterator find(const T &key) const;

	void clear();
	__always_inline iterator begin() const;
	__always_inline iterator end() const;
protected:
	__always_inline U &__findValue(const T &key);
	__always_inline U &__findValueCmp(const T &key);
	__always_inline iterator __findCmp(const T &key) const;
	__always_inline iterator __find(const T &key) const;
	class AVLNode {
	public:
		AVLNode();
		__always_inline void stealParent(AVLNode *newChild,
						 AVLNode **rootptr);
		__always_inline void setHeightFromChildren();
		class AVLNode *small;
		class AVLNode *large;
		class AVLNode *parent;
		int height;
		T key;
		U value;
	};
private:
	void deleteNode(AVLNode *node);
	__always_inline U &addValue(const T &key, AVLNode *&parent,
				    AVLNode **&aentry);
	class AVLNode *root;
	int _size;
};

template <bool b>
	class __AVLFindSelect {
public:
	template <typename T, typename U, typename CF>
		__always_inline
		static typename AVLTree<T, U, CF>::iterator
		find(const AVLTree<T, U, CF> &obj, const T &key) {
		return obj.__find(key);
	}
};

template <>
	class __AVLFindSelect<true> {
public:
	template <typename T, typename U, typename CF>
		__always_inline
		static typename AVLTree<T, U, CF>::iterator
		find(const AVLTree<T, U, CF> &obj, const T &key) {
		return obj.__findCmp(key);
	}
};

template<bool b>
	class __AVLFindValueSelect {
public:
	template <typename T, typename U, typename CF>
		__always_inline
		static U &findValue(AVLTree<T, U, CF> &obj,
				    const T &key) {
		return obj.__findValue(key);
	}
};

template<>
	class __AVLFindValueSelect<true> {
public:
	template <typename T, typename U, typename CF>
		__always_inline
		static U &findValue(AVLTree<T, U, CF> &obj,
				    const T &key) {
		return obj.__findValueCmp(key);
	}
};

template <class T, class U, typename CF>
AVLTree<T, U, CF>::iterator::iterator():
pos(nullptr)
{}

template <class T, class U, typename CF>
__always_inline const T &AVLTree<T, U, CF>::iterator::key() const
{
	return pos->key;
}

template <class T, class U, typename CF>
__always_inline U &AVLTree<T, U, CF>::iterator::value() const
{
	return pos->value;
}

template <class T, class U, typename CF>
__always_inline bool AVLTree<T, U, CF>::iterator::atEnd() const
{
	return pos == nullptr;
}

template <class T, class U, typename CF>
__always_inline	void AVLTree<T, U, CF>::iterator::next()
{
	AVLNode *prev;
	if (pos->large != nullptr) {
		pos = pos->large;
		while (pos->small != nullptr)
			pos = pos->small;
		return;
	}
	while (pos->parent != nullptr) {
		prev = pos;
		pos = pos->parent;
		if (pos->small == prev)
			return;
	}
	pos = nullptr;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	&AVLTree<T, U, CF>::iterator::operator++()
{
	next();
	return *this;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	AVLTree<T, U, CF>::iterator::operator++(int)
{
	iterator prev = *this;
	next();
	return prev;
}

template <class T, class U, typename CF>
__always_inline	void AVLTree<T, U, CF>::iterator::prev()
{
	AVLNode *prev;
	if (pos->small != nullptr) {
		pos = pos->small;
		return;
	}
	while (pos->parent != nullptr) {
		prev = pos;
		pos = pos->parent;
		if (pos->large == prev)
			return;
	}
	pos = nullptr;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	&AVLTree<T, U, CF>::iterator::operator--()
{
	prev();
	return *this;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	AVLTree<T, U, CF>::iterator::operator--(int)
{
	iterator prev = *this;
	prev();
	return prev;
}

template <class T, class U, typename CF>
__always_inline
 bool AVLTree<T, U, CF>::iterator::operator!=(const iterator &other)
	const
{
	return pos != other.pos;
}

template <class T, class U, typename CF>
__always_inline
bool AVLTree<T, U, CF>::iterator::operator==(const iterator &other)
	const
{
	return pos == other.pos;
}

template <class T, class U, typename CF>
AVLTree<T, U, CF>::AVLTree():
root(nullptr), _size(0)
{}

template <class T, class U, typename CF>
AVLTree<T, U, CF>::~AVLTree()
{
	clear();
}

template <class T, class U, typename CF>
__always_inline void AVLTree<T, U, CF>::insert(const T &key, const U &value)
{
	U &ref = findValue(key);
	ref = value;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	AVLTree<T, U, CF>::__findCmp(const T &key) const
{
	AVLNode *entry = root;
	iterator iter;
	while (entry != nullptr) {
		int cmp = CF::compare(key, entry->key);
		if (cmp == 0)
			break;
		if (cmp < 0)
			entry = entry->small;
		else
			entry = entry->large;
	}
	iter.pos = entry;
	return iter;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	AVLTree<T, U, CF>::__find(const T &key) const
{
	AVLNode *entry = root;
	iterator iter;
	while (entry != nullptr) {
		if (key == entry->key)
			break;
		if (key < entry->key)
			entry = entry->small;
		else
			entry = entry->large;
	}
	iter.pos = entry;
	return iter;
}

template <class T, class U, typename CF>
__always_inline	typename AVLTree<T, U, CF>::iterator
	AVLTree<T, U, CF>::find(const T &key) const
{
	return __AVLFindSelect<has_compare<CF, T>::value>::find(*this, key);
}


template <class T, class U, typename CF>
__always_inline	U &AVLTree<T, U, CF>::findValue(const T &key)
{
	return __AVLFindValueSelect<has_compare<CF, T>::value>::findValue(*this,
									  key);
}


template <class T, class U, typename CF>
__always_inline	U &AVLTree<T, U, CF>::__findValue(const T &key)
{
	AVLNode **aentry;
	AVLNode *entry;
	AVLNode *parent = nullptr;

	aentry = &root;
	entry = root;

	while (entry != nullptr) {
		if (key == entry->key)
			return entry->value;
		parent = entry;
		if (key < entry->key)
			aentry = &entry->small;
		else
			aentry = &entry->large;
		entry = *aentry;
	}
	return addValue(key, parent, aentry);
}

template <class T, class U, typename CF>
__always_inline	U &AVLTree<T, U, CF>::__findValueCmp(const T &key)
{
	AVLNode **aentry;
	AVLNode *entry;
	AVLNode *parent = nullptr;

	aentry = &root;
	entry = root;

	while (entry != nullptr) {
		int cmp = CF::compare(key, entry->key);
		if (cmp == 0)
			return entry->value;
		parent = entry;
		if (cmp < 0)
			aentry = &entry->small;
		else
			aentry = &entry->large;
		entry = *aentry;
	}
	return addValue(key, parent, aentry);
}

template <class T, class U, typename CF>
	__always_inline	U &AVLTree<T, U, CF>::addValue(const T &key,
						       AVLNode *&parent,
						       AVLNode **&aentry)
{
	AVLNode *entry;
	AVLNode *newentry;
	AVLNode *sibling;
	AVLNode *grandParent;
	AVLNode *smallChild;
	AVLNode *largeChild;
	int diff;
	int smallH;
	int largeH;

	_size++;
	newentry = new AVLNode;
	entry = newentry;
	*aentry = entry;

	entry->key = key;
	entry->parent = parent;
	entry->height = 0;
	if (parent == nullptr)
		return newentry->value;

	if (parent->height > 0)
		return newentry->value;

	parent->height = 1;
	grandParent = parent->parent;

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
	return newentry->value;
rebalanceSmall:
	/* Do small rebalance here (case 1 and case 2) */
	if (entry == parent->small) {
		/* Case 1 */
		sibling = parent->large;

		grandParent->stealParent(parent, &root);
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

		grandParent->stealParent(entry, &root);
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
	return newentry->value;
rebalanceLarge:
	/* Do large rebalance here */
	if (entry == parent->small) {
		/* Case 3 */
		smallChild = entry->small;
		largeChild = entry->large;

		grandParent->stealParent(entry, &root);
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

		grandParent->stealParent(parent, &root);
		parent->small = grandParent;
		grandParent->parent = parent;
		grandParent->large = sibling;
		grandParent->height--;
		if (sibling != nullptr)
			sibling->parent = grandParent;
	}
	return newentry->value;
}

template <class T, class U, typename CF>
__always_inline	U AVLTree<T, U, CF>::value(const T &key,
						    const U &defaultValue)
	const
{
	iterator iter = find(key);
	if (iter.pos == nullptr)
		return defaultValue;
	return iter.value();
}

template <class T, class U, typename CF>
__always_inline	bool AVLTree<T, U, CF>::contains(const T &key) const
{
	iterator iter = find(key);
	return iter.pos != nullptr;
}

template <class T, class U, typename CF>
__always_inline	bool AVLTree<T, U, CF>::isEmpty() const
{
	return _size == 0;
}

template <class T, class U, typename CF>
__always_inline	int AVLTree<T, U, CF>::size() const
{
	return _size;
}

template <class T, class U, typename CF>
__always_inline	U &AVLTree<T, U, CF>::operator[](const T &key)
{
	return findValue(key);
}

template <class T, class U, typename CF>
AVLTree<T, U, CF>::AVLNode::AVLNode():
small(nullptr), large(nullptr)
{}

template <class T, class U, typename CF>
__always_inline void AVLTree<T, U, CF>::AVLNode::stealParent(
	AVLNode *newChild,
	AVLNode **rootptr)
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

template <class T, class U, typename CF>
__always_inline
void AVLTree<T, U, CF>::AVLNode::setHeightFromChildren()
{
	int lh;
	int rh;

	lh = (small != nullptr) ? (small->height) : -1;
	rh = (large != nullptr) ? (large->height) : -1;
	height = __AVLTREEMAX(lh, rh) + 1;
}

template <class T, class U, typename CF>
__always_inline typename AVLTree<T, U, CF>::iterator AVLTree<T, U, CF>::begin()
	const
{
	iterator _begin;
	_begin.pos = root;
	if (_begin.pos == nullptr)
		return _begin;
	while (_begin.pos->small != nullptr)
		_begin.pos = _begin.pos->small;
	return _begin;
}

template <class T, class U, typename CF>
__always_inline typename AVLTree<T, U, CF>::iterator AVLTree<T, U, CF>::end()
	const
{
	iterator _end;
	return _end;
}

template <class T, class U, typename CF>
void AVLTree<T, U, CF>::deleteNode(AVLNode *node) {
	if (node->small != nullptr)
		deleteNode(node->small);
	if (node->large != nullptr)
		deleteNode(node->large);
	delete node;
}

template <class T, class U, typename CF>
void AVLTree<T, U, CF>::clear()
{
	if (root != nullptr) {
		deleteNode(root);
		root = nullptr;
		_size = 0;
	}
}

}

#endif /* __AVLTREE_H */
