// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "vtl/tlist.h"

#include "abstracttaskmodel.h"
#include "analyzer/task.h"

static const char swappername[] = "swapper";

AbstractTaskModel::AbstractTaskModel(QObject *parent) :
	QAbstractTableModel(parent)
{
	taskList = new vtl::TList<const Task*>;
	errorStr = new QString(tr("Error in a task mdoel"));
	idleTask = new Task;
	idleTask->pid = 0;
	idleTask->checkName(swappername, false);
	idleTask->generateDisplayName();
}

AbstractTaskModel::~AbstractTaskModel()
{
	delete taskList;
	delete errorStr;
	delete idleTask;
}

int AbstractTaskModel::rowToPid(int row, bool &ok) const
{
	if (row < 0) {
		ok = false;
		return 0;
	}
	if (row >= taskList->size()) {
		ok = false;
		return 0;
	}

	ok = true;
	const Task *task = taskList->at(row);
	return task->pid;
}

const QString &AbstractTaskModel::rowToName(int row, bool &ok) const
{
	if (row < 0) {
		ok = false;
		return *errorStr;
	}
	if (row >= taskList->size()) {
		ok = false;
		return *errorStr;
	}

	ok = true;
	const Task *task = taskList->at(row);

	return *task->displayName;
}

bool AbstractTaskModel::rowToGhostStatus(int row, bool &ok) const
{
	if (row < 0) {
		ok = false;
		return false;
	}
	if (row >= taskList->size()) {
		ok = false;
		return false;
	}

	ok = true;
	const Task *task = taskList->at(row);

	return task->isGhostAlias;
}
