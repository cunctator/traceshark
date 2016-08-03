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

#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractTableModel>
#include "misc/traceshark.h"

template<class T> class TList;
class Task;

QT_BEGIN_NAMESPACE
class QStringList;
template<class Key, class T> class QMap;
QT_END_NAMESPACE

class TaskModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	TaskModel(QObject *parent = 0);
	~TaskModel();
	void setTaskMap(QMap<unsigned int, Task> *map);
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value,
		     int role);
	QVariant headerData(int section, Qt::Orientation orientation,
			    int role) const;
	unsigned int rowToPid(int row, bool &ok) const;
	void beginResetModel();
	void endResetModel();
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	QMap<unsigned int, Task> *taskMap;
	TList<Task*> *taskList;
	QStringList *taskNameList;
};

#endif /* TASKMODEL_H */
