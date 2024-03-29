// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2019, 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef TASKSELECTDIALOG
#define TASKSELECTDIALOG

#include <QDialog>
#include <QString>

#include "analyzer/task.h"
#include "vtl/avltree.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
template <typename> class QList;
template <typename, typename> class QMap;
QT_END_NAMESPACE

class TaskModel;
class TableView;
class AbstractTaskModel;
class StatsModel;

class TaskSelectDialog : public QDockWidget {
	Q_OBJECT
public:
	enum TaskSelectType {
		TaskSelectStats = 0,
		TaskSelectStatsLimited,
		TaskSelectRegular
	};
	TaskSelectDialog(QWidget *parent, const QString &title,
			 enum TaskSelectType type);
	~TaskSelectDialog();
	void setTaskMap(vtl::AVLTree<int, TaskHandle> *map,
			unsigned int nrcpus);
	void beginResetModel();
	void endResetModel();
	void resizeColumnsToContents();
	int exportStats(bool csv, const QString &filename);
public slots:
	void show();
signals:
	void addTaskGraph(int pid);
	void needReplot(void);
	void needLegendCheck(void);
	void addTaskToLegend(int pid);
	void resetFilter(void);
	void doExport(bool csv);
	void createFilter(QMap<int, int> &map, bool orlogic, bool inclusive);
	void QDockWidgetNeedsRemoval(QDockWidget *widget);
	void taskDoubleClicked(int pid);
	void colorChangeReq(const QList<int> *list);
private slots:
	void colorClicked();
	void closeClicked();
	void addUnifiedClicked();
	void addLegendClicked();
	void addFilterClicked();
	void exportClicked();
	void handleDoubleClick(const QModelIndex &index);
private:
	TableView *taskView;
	AbstractTaskModel *taskModel;
	QComboBox *logicBox;
	QComboBox *exportBox;
	QCheckBox *includeBox;
	/* indexMap is used to avoid multiple calls per pid */
	QMap<int, int> *indexMap;
	QMap<int, int> *filterMap;
	QList<int> *colorList;
	int savedHeight;
};

#endif /* TASKSELECTDIALOG */
