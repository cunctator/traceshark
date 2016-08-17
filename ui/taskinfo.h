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

#ifndef TASKINFO_H
#define TASKINFO_H

#include <QWidget>
#include <QMap>

QT_BEGIN_NAMESPACE
class QAction;
class QLineEdit;
class QToolBar;
QT_END_NAMESPACE

class TaskGraph;

class TaskInfo : public QWidget
{
	Q_OBJECT
public:
	TaskInfo(QWidget *parent = 0);
	virtual ~TaskInfo();

	void addTaskGraphToLegend(TaskGraph *graph);
	void setTaskGraph(TaskGraph *graph);
	void removeTaskGraph();
	void checkGraphSelection();
	void clear();
	void pidRemoved(unsigned int pid);
signals:
	void findWakeup(unsigned int pid);
	void addTaskGraph(unsigned int pid);
	void removeTaskGraph(unsigned int pid);
private slots:
	void addTaskGraphTriggered();
	void addToLegendTriggered();
	void clearTriggered();
	void findTriggered();
	void removeTaskGraphTriggered();
private:
	void createActions();
	void createToolBar();

	QAction *addTaskGraphAction;
	QAction *addToLegendAction;
	QAction *clearAction;
	QAction *findAction;
	QAction *removeTaskGraphAction;

	QToolBar *taskToolBar;

	QLineEdit *pidLine;
	QLineEdit *nameLine;
	TaskGraph *taskGraph;
	QMap <unsigned int, TaskGraph*> legendPidMap;
};

#endif /* TASKINFO_H */
