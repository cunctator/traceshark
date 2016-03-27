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

#ifndef TASKRANGEALLOCATOR
#define TASKRANGEALLOCATOR

class TaskRange {
public:
	double upper;
	double lower;
	unsigned int pid;
	TaskRange *next;
	TaskRange *prev;
};

class TaskRangeAllocator {
public:
	TaskRangeAllocator(double decValue);
	~TaskRangeAllocator();
	void setStart(double topValue);
	TaskRange *getTaskRange(unsigned int pid, bool &isNew);
	void putTaskRange(unsigned int pid);
	void putTaskRange(TaskRange *range);
	void clearAll();
	double getBottom();
private:
	TaskRange *rangeList;
	double top;
	double bottom;
	double dec;
};

#endif /* TASKRANGEALLOCATOR */
