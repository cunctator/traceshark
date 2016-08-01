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

#ifndef TASKSELECTDIALOG
#define TASKSELECTDIALOG

#include <QDialog>
#include <QString>
#include <QMap>

#include "analyzer/task.h"

QT_BEGIN_NAMESPACE
class QStringList;
QT_END_NAMESPACE

class TaskModel;
class TaskView;

class TaskSelectDialog : public QDialog {
	Q_OBJECT
public:
	TaskSelectDialog(QWidget *parent = 0);
	~TaskSelectDialog();
	void setTaskMap(QMap<unsigned int, Task> *map);
	void beginResetModel();
	void endResetModel();
	void resizeColumnsToContents();
	void show();
signals:
	void addTaskGraph(unsigned int pid);
	void addTaskToLegend(unsigned int pid);
private slots:
	void closeClicked();
	void addUnifiedClicked();
	void addLegendClicked();
private:
	TaskView *taskView;
	TaskModel *taskModel;
};

#endif /* TASKSELECTDIALOG */
