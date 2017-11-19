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

#define __AVLTREEMAX(A, B) ((A) >= (B) ? A:B)

template <class T, class U>
class AVLTree
{
private:
	class AVLNode;
public:
	class iterator
	{
		friend class AVLTree<T, U>;
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
private:
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
	void deleteNode(AVLNode *node);
	class AVLNode *root;
	int _size;
};

template <class T, class U>
	AVLTree<T, U>::iterator::iterator():
	pos(nullptr)
{}

template <class T, class U>
__always_inline const T &AVLTree<T, U>::iterator::key() const
{
	return pos->key;
}

template <class T, class U>
__always_inline U &AVLTree<T, U>::iterator::value() const
{
	return pos->value;
}

template <class T, class U>
__always_inline bool AVLTree<T, U>::iterator::atEnd() const
{
	return pos == nullptr;
}

template <class T, class U>
__always_inline	void AVLTree<T, U>::iterator::next()
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

template <class T, class U>
__always_inline	typename AVLTree<T, U>::iterator
	&AVLTree<T, U>::iterator::operator++()
{
	next();
	return *this;
}

template <class T, class U>
__always_inline	typename AVLTree<T, U>::iterator
	AVLTree<T, U>::iterator::operator++(int)
{
	iterator prev = *this;
	next();
	return prev;
}

template <class T, class U>
__always_inline	void AVLTree<T, U>::iterator::prev()
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

template <class T, class U>
__always_inline	typename AVLTree<T, U>::iterator
	&AVLTree<T, U>::iterator::operator--()
{
	prev();
	return *this;
}

template <class T, class U>
__always_inline	typename AVLTree<T, U>::iterator
	AVLTree<T, U>::iterator::operator--(int)
{
	iterator prev = *this;
	prev();
	return prev;
}

template <class T, class U>
__always_inline
	bool AVLTree<T, U>::iterator::operator!=(const iterator &other) const
{
	return pos != other.pos;
}

template <class T, class U>
__always_inline
	bool AVLTree<T, U>::iterator::operator==(const iterator &other) const
{
	return pos == other.pos;
}

template <class T, class U>
AVLTree<T, U>::AVLTree():
	root(nullptr), _size(0)
{}

template <class T, class U>
AVLTree<T, U>::~AVLTree()
{
	clear();
}

template <class T, class U>
__always_inline void AVLTree<T, U>::insert(const T &key, const U &value)
{
	U &ref = findValue(key);
	ref = value;
}

template <class T, class U>
__always_inline	typename AVLTree<T, U>::iterator AVLTree<T, U>::find(const T &key) const
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

template <class T, class U>
__always_inline	U &AVLTree<T, U>::findValue(const T &key)
{
	AVLNode **aentry;
	AVLNode *entry;
	AVLNode *newentry;
	AVLNode *parent = nullptr;
	AVLNode *sibling;
	AVLNode *grandParent;
	AVLNode *smallChild;
	AVLNode *largeChild;
	int diff;
	int smallH;
	int largeH;

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

template <class T, class U>
__always_inline	U AVLTree<T, U>::value(const T &key,
				       const U &defaultValue) const
{
	iterator iter = find(key);
	if (iter.pos == nullptr)
		return defaultValue;
	return iter.value();
}

template <class T, class U>
__always_inline	bool AVLTree<T, U>::contains(const T &key) const
{
	iterator iter = find(key);
	return iter.pos != nullptr;
}

template <class T, class U>
__always_inline	bool AVLTree<T, U>::isEmpty() const
{
	return _size == 0;
}

template <class T, class U>
__always_inline	int AVLTree<T, U>::size() const
{
	return _size;
}

template <class T, class U>
__always_inline	U &AVLTree<T, U>::operator[](const T &key)
{
	return findValue(key);
}

template <class T, class U>
AVLTree<T, U>::AVLNode::AVLNode():
	small(nullptr), large(nullptr)
{}

template <class T, class U>
__always_inline void AVLTree<T,U>::AVLNode::stealParent(
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

template <class T, class U>
__always_inline void AVLTree<T,U>::AVLNode::setHeightFromChildren()
{
	int lh;
	int rh;

	lh = (small != nullptr) ? (small->height) : -1;
	rh = (large != nullptr) ? (large->height) : -1;
	height = __AVLTREEMAX(lh, rh) + 1;
}

template <class T, class U>
__always_inline typename AVLTree<T, U>::iterator AVLTree<T, U>::begin() const
{
	iterator _begin;
	_begin.pos = root;
	if (_begin.pos == nullptr)
		return _begin;
	while (_begin.pos->small != nullptr)
		_begin.pos = _begin.pos->small;
	return _begin;
}

template <class T, class U>
__always_inline typename AVLTree<T, U>::iterator AVLTree<T, U>::end() const
{
	iterator _end;
	return _end;
}

template <class T, class U>
void AVLTree<T, U>::deleteNode(AVLNode *node) {
	if (node->small != nullptr)
		deleteNode(node->small);
	if (node->large != nullptr)
		deleteNode(node->large);
	delete node;
}

template <class T, class U>
void AVLTree<T, U>::clear()
{
	if (root != nullptr) {
		deleteNode(root);
		root = nullptr;
		_size = 0;
	}
}

#define __AVLTREE_H
#endif /* __AVLTREE_H */
