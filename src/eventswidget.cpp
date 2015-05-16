/*
 * Traceshark - a visualizer for visualizing ftrace traces
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

#include <QTableView>
#include "eventsmodel.h"
#include "eventswidget.h"

EventsWidget::EventsWidget(QWidget *parent):
	QDockWidget(parent), events(NULL)
{
	tableView = new QTableView(this);
	eventsModel = new EventsModel(tableView);
	tableView->setModel(eventsModel);
	setWidget(tableView);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->resizeColumnsToContents();
	tableView->show();
}

EventsWidget::EventsWidget(QList<TraceEvent> *e, QWidget *parent):
	QDockWidget(parent)
{
	tableView = new QTableView(this);
	eventsModel = new EventsModel(e, tableView);
	events = e;
	tableView->setModel(eventsModel);
	setWidget(tableView);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->resizeColumnsToContents();
	tableView->show();
}

EventsWidget::~EventsWidget()
{
}

void EventsWidget::setEvents(QList<TraceEvent> *e)
{
	eventsModel->setEvents(e);
	events = e;
}

void EventsWidget::beginResetModel()
{
	eventsModel->beginResetModel();
	events = NULL;
}

void EventsWidget::endResetModel()
{
	eventsModel->endResetModel();
	tableView->resizeColumnsToContents();
}

void EventsWidget::scrollTo(double time)
{
	if (events != NULL) {
		int n = binarySearch(time, 0, events->size() - 1);
		tableView->selectRow(n);
		tableView->resizeColumnsToContents();
	}
}

int EventsWidget::binarySearch(double time, int start, int end)
{
	int pivot = (end + start) / 2;
	if (pivot == start)
		return pivot;
	if (time < events->at(pivot).time)
		return binarySearch(time, start, pivot);
	else
		return binarySearch(time, pivot, end);
}
