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

#ifndef EVENTSWIDGET_H
#define EVENTSWIDGET_H

#include <QDockWidget>
#include <QList>
#include "traceevent.h"
#include "traceshark.h"

class QTableView;
class EventsModel;

class EventsWidget : public QDockWidget
{
	Q_OBJECT
public:
	EventsWidget(QWidget *parent = 0);
	EventsWidget(QList<TraceEvent> *e, QWidget *parent = 0);
	virtual ~EventsWidget();
	void setEvents(QList<TraceEvent> *e);
	void beginResetModel();
	void endResetModel();
	void scrollTo(double time);
signals:
	void timeSelected(double time);
private slots:
	void handleClick(const QModelIndex &index);
private:
	QTableView *tableView;
	EventsModel *eventsModel;
	QList<TraceEvent> *events;
	int findBestMatch(double time);
	int binarySearch(double time, int start, int end);
};

#endif /* EVENTSWIDGET_H*/
