// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "vtl/compiler.h"
#include "vtl/time.h"

class QCPErrorBars;
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
	vtl_always_inline Task &getTask();
};

class TaskName {
public:
	TaskName();
	const char *str;
	TaskName *prev;
	bool forkname;
};

typedef enum RunStatus : int {
	RUN_STATUS_INVALID = 0,
	RUN_STATUS_WAKEUP,
	RUN_STATUS_SCHED
} runstatus_t;

class Task : public AbstractTask {
public:
	Task();
	~Task();
	void addName(const char *name);
	vtl_always_inline void checkName(const char *name, bool forkname = false);
	vtl_always_inline void generateDisplayName();
	vtl_always_inline void generateGhostName(const Task *realtask);
	void generateNameFromTask(const Task *rtask);
	QString getLastName() const;

	TaskName     *taskName;
	exitstatus_t exitStatus;

	/* lastRunnable is only used during extraction */
	vtl::Time    lastRunnable;
	int          lastRunnable_idx;
	runstatus_t  lastRunnable_status;

	vtl::Time    lastSleepEntry;

	/*
	 * The unified task needs to save pointers to these graphs so that they
	 * can be deleted when the user requests the unified task to be 
	 * removed
	 */
	QCPGraph     *delayGraph;
	QCPGraph     *preemptedGraph;
	QCPGraph     *runningGraph;
	QCPGraph     *uninterruptibleGraph;
	QString      *displayName;

	/*
	 * These contain the information if this task is a ghost alias and for
	 * what pid. A ghost alias is a pid that appear as the pid of a
	 * sched_switch event but is not the oldpid. The ghost alias pid may
	 * also appear as the pid of waking and wakeup events and this is the
	 * primary reason why we want to record this information.
	 */
	bool isGhostAlias;
	int isGhostAliasForPID;

	/* Currently, the thinking is that there is a one to one mapping between
	 * ghost alias tasks and real tasks. If we discover that this is not the
	 * case, that a task is an alias for sever tasks, then this information
	 * is recorded in oneToManyError.
	 */
	bool oneToManyError;
private:
	vtl_always_inline void appendName(const TaskName *name, bool isnewest);
};

vtl_always_inline void Task::checkName(const char *name, bool forkname)
{
	if (taskName == nullptr || strcmp(taskName->str, name) != 0) {
		addName(name);
		taskName->forkname = forkname;
	}
}

vtl_always_inline Task &TaskHandle::getTask()
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
vtl_always_inline void Task::appendName(const TaskName *name, bool isnewest)
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

vtl_always_inline void Task::generateGhostName(const Task *realtask)
{
	generateNameFromTask(realtask);
}

vtl_always_inline void Task::generateDisplayName()
{
	generateNameFromTask(this);
}

#endif /* TASK_H */
