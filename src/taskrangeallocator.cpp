/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "taskrangeallocator.h"

TaskRangeAllocator::TaskRangeAllocator(double decValue) :
	rangeList(nullptr), dec(decValue)
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
}

void TaskRangeAllocator::setStart(double topValue)
{
	top = topValue;
	bottom = topValue;
}


TaskRange *TaskRangeAllocator::getTaskRange(unsigned int pid, bool &isNew)
{
	TaskRange *r;
	TaskRange **tr;
	TaskRange *prev;
	double upper;

	isNew = false;

	r = rangeList;
	while (r != nullptr) {
		if (r->pid == pid)
			break;
		r = r->next;
	}

	if (r != nullptr)
		return r;

	r = new TaskRange;
	isNew = true;
	if (r == nullptr)
		return r;

	r->pid = pid;

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
	delete range;
}

void TaskRangeAllocator::putTaskRange(unsigned int pid)
{
	TaskRange *r = rangeList;

	while (r != nullptr) {
		if (r->pid == pid)
			break;
		r = r->next;
	}

	if (r != nullptr)
		putTaskRange(r);
}

double TaskRangeAllocator::getBottom()
{
	if (bottom != top)
		/* If there is at least on unified task graph, we will add an
		 * offset to prevent it from being on top of the axis */
		return bottom - taskSectionOffset;
	else
		/* In this case no offset is needed because the migration graph
		 * has an offset of it's own */
		return bottom;
}
