// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016, 2017-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "ui/taskrangeallocator.h"

TaskRangeAllocator::TaskRangeAllocator(double decValue) :
	rangeList(nullptr), top(0), bottom(0), dec(decValue)
{}

TaskRangeAllocator::~TaskRangeAllocator()
{
	clearAll();
}

void TaskRangeAllocator::clearAll()
{
	TaskRange *r = rangeList;
	TaskRange *s;
	while (r != nullptr) {
		s = r;
		r = r->next;
		delete s;
	}
	bottom = top;
	rangeList = nullptr;
	pidMap.clear();
}

void TaskRangeAllocator::setStart(double topValue)
{
	top = topValue;
	bottom = topValue;
}


TaskRange *TaskRangeAllocator::getTaskRange(int pid, bool &isNew)
{
	TaskRange *r;
	TaskRange **tr;
	TaskRange *prev;
	double upper;
	QMap<int, TaskRange*>::iterator iter;

	isNew = false;

	iter = pidMap.find(pid);
	if (iter != pidMap.end())
		return iter.value();

	r = new TaskRange;
	isNew = true;
	if (r == nullptr)
		return r;

	r->pid = pid;
	pidMap[pid] = r;

	upper = top;
	tr = &rangeList;
	prev = nullptr;
	while (*tr != nullptr) {
		if (upper - (*tr)->upper >= dec) {
			/* Insert here */
			r->next = *tr;
			r->prev = prev;
			r->upper = upper;
			r->lower = upper - dec;
			(*tr)->prev = r;
			*tr = r;
			return r;
		}
		prev = *tr;
		upper = (*tr)->lower;
		tr = &((*tr)->next);
	}
	r->upper = bottom;
	bottom -= dec;
	r->lower = bottom;
	r->prev = prev;
	r->next = nullptr;
	*tr = r;
	return r;
}

void TaskRangeAllocator::putTaskRange(TaskRange *range)
{
	if (range->prev != nullptr) {
		range->prev->next = range->next;
	} else {
		/* prev == nullptr, thus must be the first in the list */
		rangeList = range->next;
	}
	if (range->next != nullptr) {
		range->next->prev = range->prev;
	} else {
		TaskRange *r = rangeList;
		double newbottom = top;
		while (r != nullptr) {
			newbottom = r->lower;
			r = r->next;
		}
		bottom = newbottom;
	}
	pidMap.remove(range->pid);
	delete range;
}

void TaskRangeAllocator::putTaskRange(int pid)
{
	QMap<int, TaskRange*>::iterator iter;
	iter = pidMap.find(pid);
	if (iter != pidMap.end())
		putTaskRange(iter.value());
}

double TaskRangeAllocator::getBottom()
{
	if (bottom != top)
		/*
		 * If there is at least on unified task graph, we will add an
		 * offset to prevent it from being on top of the axis.
		 */
		return bottom - taskSectionOffset;
	else
		/*
		 * In this case no offset is needed because the migration graph
		 * has an offset of it's own.
		 */
		return bottom;
}

bool TaskRangeAllocator::isEmpty()
{
	return rangeList == nullptr;
}

bool TaskRangeAllocator::contains(int pid)
{
	return pidMap.contains(pid);
}

TaskRangeAllocator::iterator TaskRangeAllocator::begin()
{
	TaskRangeAllocator::iterator r;
	r.ptr = rangeList;
	return r;
}

TaskRangeAllocator::iterator TaskRangeAllocator::end()
{
	TaskRangeAllocator::iterator r;
	r.ptr = nullptr;
	return r;
}

TaskRangeAllocator::iterator TaskRangeAllocator::iterator::operator++(int)
{
	TaskRangeAllocator::iterator r;
	ptr = ptr->next;
	r.ptr = ptr;
	return r;
}

TaskRangeAllocator::iterator TaskRangeAllocator::iterator::operator--(int)
{
	TaskRangeAllocator::iterator r;
	ptr = ptr->prev;
	r.ptr = ptr;
	return r;
}

bool TaskRangeAllocator::iterator::operator==(iterator i)
{
	return ptr == i.ptr;
}

bool TaskRangeAllocator::iterator::operator!=(iterator i)
{
	return ptr != i.ptr;
}

const TaskRange &TaskRangeAllocator::iterator::value()
{
	return  *ptr;
}

QList<int> TaskRangeAllocator::getPidList() const
{
	QList<int> rlist;
	TaskRange *r = rangeList;
	while (r != nullptr) {
		rlist.append(r->pid);
		r = r->next;
	}
	return rlist;
}
