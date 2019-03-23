// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TASK_H
#define TASK_H

#include <cstring>

#include <QString>

#include "analyzer/abstracttask.h"
#include "vtl/time.h"

class QCPGraph;
class TaskGraph;
class Task;

typedef enum : int {
	STATUS_ALIVE,
	STATUS_EXITCALLED,
	STATUS_FINAL
} exitstatus_t;

class TaskHandle {
public:
	TaskHandle():task(nullptr) {};
	Task *task;
	__always_inline Task &getTask();
};

class TaskName {
public:
	TaskName();
	const char *str;
	TaskName *prev;
	bool forkname;
};

class Task : public AbstractTask {
public:
	Task();
	~Task();
	void addName(const char *name);
	__always_inline void checkName(const char *name, bool forkname = false);
	void generateDisplayName();
	QString getLastName() const;

	TaskName     *taskName;
	exitstatus_t exitStatus;

	/* lastWakeUP is only used during extraction */
	vtl::Time    lastWakeUP;

	vtl::Time    lastSleepEntry;

	/*
	 * The unified task needs to save pointers to these graphs so that they
	 * can be deleted when the user requests the unified task to be 
	 * removed
	 */
	QCPGraph     *wakeUpGraph;
	QCPGraph     *preemptedGraph;
	QCPGraph     *runningGraph;
	QCPGraph     *uninterruptibleGraph;
	QString      *displayName;
private:
	__always_inline void appendName(const TaskName *name, bool isnewest);
};

__always_inline void Task::checkName(const char *name, bool forkname)
{
	if (taskName == nullptr || strcmp(taskName->str, name) != 0) {
		addName(name);
		taskName->forkname = forkname;
	}
}

__always_inline Task &TaskHandle::getTask()
{
	if (task == nullptr)
		task = new Task;
	return *task;
}
/*
 * If the name is not the newest name and is "forkname", then we will will
 * surrount it with {}. If the name is not the newest and not a forkname,
 * then we will surround it with ()
 */
__always_inline void Task::appendName(const TaskName *name, bool isnewest)
{
	bool curly  =  name->forkname && !isnewest;
	bool normal = !name->forkname && !isnewest;

	if (curly)
		displayName->append("{");
	if (normal)
		displayName->append("(");
	displayName->append(name->str);
	if (curly)
		displayName->append("}");
	if (normal)
		displayName->append(")");
}

#endif /* TASK_H */
