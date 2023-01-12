// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2017, 2020, 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef VTL_HEAPSORT_H
#define VTL_HEAPSORT_H

#include "vtl/compiler.h"

namespace vtl {

vtl_always_inline long heap_iParent_(long i)
{
	return (i - 1) / 2;
}

vtl_always_inline long heap_iLeftChild_(long i)
{
	return 2 * i + 1;
}

vtl_always_inline long heap_iRightChild_(long i)
{
	return 2 * i + 2;
}

template<template <typename> class C, typename T, typename TCompFunc>
vtl_always_inline void heap_siftdown_(C<T> &container,
				      long start,
				      long end,
				      TCompFunc compFunc)
{
	long root, child, rchild, swap;

	root = start;
	while (heap_iLeftChild_(root) <= end) {
		child = heap_iLeftChild_(root);
		swap = root;
		if (compFunc(container[swap], container[child]) < 0)
			swap = child;
		rchild = child + 1;
		if (rchild <= end && compFunc(container[swap],
					      container[rchild]) < 0)
			swap = rchild;
		if (swap == root)
			return;
		else {
			container.swapItemsAt(root, swap);
			root = swap;
		}
	}
}

template<template <typename> class C, typename T, typename TCompFunc>
vtl_always_inline void heap_heapify_(C<T> &container,
				     TCompFunc compFunc)
{
	long count = container.size();
	long start;

	start = heap_iParent_(count - 1);

	while(start >= 0) {
		heap_siftdown_(container, start, count - 1, compFunc);
		start--;
	}
}

template<template <typename> class C, typename T, typename TCompFunc>
	void heapsort(C<T> &container,
		      TCompFunc compFunc)
{
	long count = container.size();
	long end;

	if (count < 2)
		return;

	heap_heapify_(container, compFunc);

	end = count - 1;
	while (end > 0) {
		container.swapItemsAt(0, end);
		end--;
		heap_siftdown_(container, 0, end, compFunc);
	}
}
 
}

#endif /* VTL_HEAPSORT_H */
