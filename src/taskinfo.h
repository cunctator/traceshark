/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
class QLineEdit;
QT_END_NAMESPACE

class TaskGraph;

class TaskInfo : public QWidget
{
	Q_OBJECT
public:
	TaskInfo(QWidget *parent = 0);
	virtual ~TaskInfo();

	void setTaskGraph(TaskGraph *graph);
	void removeTaskGraph();
	void checkGraphSelection();
	void clear();
	void pidRemoved(unsigned int pid);
private slots:
	void addClicked();
	void clearClicked();
private:
	QLineEdit *pidLine;
	QLineEdit *nameLine;
	TaskGraph *taskGraph;
	QMap <unsigned int, TaskGraph*> legendPidMap;
};

#endif /* TASKINFO_H */
