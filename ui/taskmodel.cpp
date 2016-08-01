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

#include "ui/taskmodel.h"
#include <QStringList>

TaskModel::TaskModel(QObject *parent):
	QAbstractTableModel(parent), taskMap(nullptr)
{
	taskNameList = new QStringList();
}

TaskModel::~TaskModel()
{
	delete taskNameList;
}

void TaskModel::setTaskMap(QMap<unsigned int, Task> *map)
{
	taskMap = map;
	nameMap.clear();
	taskNameList->clear();

	if (map == nullptr)
		return;

	DEFINE_TASKMAP_ITERATOR(iter) = taskMap->begin();
	while (iter != taskMap->end()) {
		Task &task = iter.value();
		QString name = task.getDisplayName();
		nameMap.insertMulti(name, task.pid);
		taskNameList->append(name);
		iter++;
	}
	taskNameList->sort();
}

int TaskModel::rowCount(const QModelIndex & /* index */) const
{
	if (taskMap != nullptr)
		return taskNameList->size();
	else
		return 0;
}

int TaskModel::columnCount(const QModelIndex & /* index */) const
{
	return 2; /* Number from data() and headerData() */
}

unsigned int TaskModel::rowToPid(int row, bool &ok) const
{
	QString name;
	QList<unsigned int> pidList;
	int s;
	int i;
	int j;
	int n = 0;

	ok = true;
	name = taskNameList->at(row);
	pidList = nameMap.values(name);
	s = pidList.size();
	n = 0;
	/* Simple swap sort of the list so we display tasks */
	for (i = 1; i < s; i++) {
		j = i - 1;
		if (pidList[j] > pidList[i])
			pidList.swap(i, j);
	}
	/* In the general case, this row points to the nth task
	 * with the same name. This determines n */
	for (i = row - 1; i >= 0; i--) {
		if (taskNameList->at(i) != name)
			break;
		n++;
	}
	if (n >= s) {
		ok = false;
		return 0;
	}
	return pidList[n];
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
		int size = taskNameList->size();
		unsigned int pid;

		if (row >= size || row < 0)
			return QVariant();

		switch(column) {
		case 0:
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
