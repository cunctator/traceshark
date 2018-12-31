// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "vtl/avltree.h"
#include "vtl/heapsort.h"
#include "vtl/tlist.h"

#include "ui/taskmodel.h"
#include "misc/traceshark.h"
#include "analyzer/task.h"

static const char swappername[] = "swapper";

TaskModel::TaskModel(QObject *parent):
	AbstractTaskModel(parent)
{
	taskList = new vtl::TList<const Task*>;
	errorStr = new QString(tr("Error in taskmodel.cpp"));
	idleTask = new Task;
	idleTask->pid = 0;
	idleTask->checkName(swappername, false);
	idleTask->generateDisplayName();
}

TaskModel::~TaskModel()
{
	delete taskList;
	delete errorStr;
	delete idleTask;
}

void TaskModel::setTaskMap(vtl::AVLTree<int, TaskHandle> *map,
			   unsigned int /*nrcpus*/)
{
	taskList->clear();

	if (map == nullptr)
		return;

	DEFINE_TASKMAP_ITERATOR(iter) = map->begin();
	while (iter != map->end()) {
		Task *task = iter.value().task;
		taskList->append(task);
		iter++;
	}

	/* Add a fake idle task for event filtering purposes */
	taskList->append(idleTask);

	vtl::heapsort<vtl::TList, const Task*>(
		*taskList, [] (const Task *&a, const Task *&b) -> int {
			const QString &as = *a->displayName;
			const QString &bs = *b->displayName;
			int cmp1 = as.compare(bs);
			if (cmp1 != 0)
				return cmp1;
			long cmp2 = (long) a->pid - (long) b->pid;
			return (int) cmp2;
		});
}

int TaskModel::rowCount(const QModelIndex & /* index */) const
{
	return taskList->size();
}

int TaskModel::columnCount(const QModelIndex & /* index */) const
{
	return 2; /* Number from data() and headerData() */
}

int TaskModel::rowToPid(int row, bool &ok) const
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

const QString &TaskModel::rowToName(int row, bool &ok) const
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

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
	bool ok;

	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole)
		return int(Qt::AlignLeft | Qt::AlignVCenter);

	if (role == Qt::DisplayRole) {
		int row = index.row();
		int column = index.column();
		int pid;
		QString name;

		switch(column) {
		case 0:
			name = rowToName(row, ok);
			if (ok)
				return name;
			break;
		case 1:
			pid = rowToPid(row, ok);
			if (ok)
				return QString::number(pid);
			break;
		default:
			break;
		}
	}
	return QVariant();
}

bool TaskModel::setData(const QModelIndex &/*index*/, const QVariant
			&/*value*/, int /*role*/)
{
	return false;
}

QVariant TaskModel::headerData(int section,
				 Qt::Orientation orientation,
				 int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch(section) {
		case 0:
			return QString(tr("Name"));
		case 1:
			return QString(tr("PID(TID)"));
		default:
			return *errorStr;
		}
	}
	return QVariant();
}

Qt::ItemFlags TaskModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	return flags;
}

void TaskModel::beginResetModel()
{
	QAbstractTableModel::beginResetModel();
}

void TaskModel::endResetModel()
{
	QAbstractTableModel::endResetModel();
}
