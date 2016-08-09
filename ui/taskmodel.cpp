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

#include <QMap>
#include <QStringList>

#include "ui/taskmodel.h"
#include "misc/tlist.h"
#include "analyzer/task.h"

TaskModel::TaskModel(QObject *parent):
	QAbstractTableModel(parent), taskMap(nullptr)
{
	taskList = new TList<Task*>;
	taskNameList = new QStringList;
}

TaskModel::~TaskModel()
{
	delete taskList;
	delete taskNameList;
}

void TaskModel::setTaskMap(QMap<unsigned int, Task> *map)
{
	QMap<QString, unsigned int> *nameMap = new QMap<QString, unsigned int>;
	int i, j, s;
	int row;

	taskList->clear();
	taskNameList->clear();
	taskMap = map;

	if (map == nullptr) {
		delete nameMap;
		return;
	}

	DEFINE_TASKMAP_ITERATOR(iter) = taskMap->begin();
	while (iter != taskMap->end()) {
		Task &task = iter.value();
		QString name = task.getDisplayName();
		nameMap->insertMulti(name, task.pid);
		taskNameList->append(name);
		iter++;
	}
	taskNameList->sort();
	s = taskNameList->size();

	row = 0;
	while (row < s) {
		QString name = taskNameList->at(row);
		QList<unsigned int> pidList = nameMap->values(name);
		int sl = pidList.size();
		bool done;
		unsigned int pid;

		/* Pathetic bubble sort to display tasks with the same names in
		 * pid order */
		int start = 0;
		int end = sl - 1;
		do {
			done = true;
			for (i = start + 1; i <= end; i++) {
				j = i - 1;
				if (pidList[j] > pidList[i]) {
					pidList.swap(i, j);
					done = false;
				}
			}
			end--;
			if (done || start >= end)
				break;
			done = true;
			for (i = end; i > start; i--) {
				j = i - 1;
				if (pidList[j] > pidList[i]) {
					pidList.swap(i, j);
					done = false;
				}
			}
			start++;
		} while(!done && start < end);

		for (i = 0; i < sl; i++) {
			pid = pidList[i];
			Task &task = (*taskMap)[pid];
			Task *&taskptr = taskList->increase();
			taskptr = &task;
		}
		row += sl;
	}

	delete nameMap;
}

int TaskModel::rowCount(const QModelIndex & /* index */) const
{
	if (taskMap != nullptr)
		return taskList->size();
	else
		return 0;
}

int TaskModel::columnCount(const QModelIndex & /* index */) const
{
	return 2; /* Number from data() and headerData() */
}

unsigned int TaskModel::rowToPid(int row, bool &ok) const
{
	unsigned int urow;

	if (row < 0) {
		ok = false;
		return 0;
	}
	urow = (unsigned int) row;
	if (urow >= taskList->size()) {
		ok = false;
		return 0;
	}

	ok = true;
	const Task *task = taskList->at(row);
	return task->pid;
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
		int size;
		unsigned int pid;

		switch(column) {
		case 0:
			size = taskNameList->size();
			if (row >= size || row < 0)
				return QVariant();
			return taskNameList->at(row);
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
			return QString(tr("Error in taskmodel.cpp"));
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
