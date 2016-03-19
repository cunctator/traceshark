/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TASK_H
#define TASK_H

#include <QString>

class TaskName {
public:
	TaskName();
	char *str;
	TaskName *prev;
};

class Task {
public:
	Task();
	~Task();
	void addName(char *name);
	__always_inline void checkName(char *name);
	QString getDisplayName();
	QString getLastName();
	unsigned int pid; /* is really tid as all other pids here */
	TaskName *taskName;
	bool isNew;
	double lastWakeUP;
};


__always_inline void Task::checkName(char *name)
{
	if (taskName == nullptr || strcmp(taskName->str, name) != 0)
		addName(name);
}

#endif /* TASK_H */
