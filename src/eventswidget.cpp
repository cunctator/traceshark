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

#include <QTableView>
#include <cmath>
#include "eventsmodel.h"
#include "eventswidget.h"
#include "traceshark.h"

EventsWidget::EventsWidget(QWidget *parent):
	QDockWidget(parent), events(nullptr)
{
	tableView = new QTableView(this);
	eventsModel = new EventsModel(tableView);
	tableView->setModel(eventsModel);
	setWidget(tableView);
	tableView->horizontalHeader()->setStretchLastSection(true);
	tableView->resizeColumnsToContents();
	tableView->show();
	tsconnect(tableView, clicked(const QModelIndex &),
		  this, handleClick(const QModelIndex &));
	tsconnect(tableView, doubleClicked(const QModelIndex &),
		  this, handleDoubleClick(const QModelIndex &));
}

EventsWidget::EventsWidget(TList<TraceEvent> *e, QWidget *parent):
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
	tsconnect(tableView, clicked(const QModelIndex &),
		  this, handleClick(const QModelIndex &));
	tsconnect(tableView, doubleClicked(const QModelIndex &),
		  this, handleDoubleClick(const QModelIndex &));
}

EventsWidget::~EventsWidget()
{
}

void EventsWidget::setEvents(TList<TraceEvent> *e)
{
	eventsModel->setEvents(e);
	events = e;
}

void EventsWidget::beginResetModel()
{
	eventsModel->beginResetModel();
	events = nullptr;
}

void EventsWidget::endResetModel()
{
	eventsModel->endResetModel();
	tableView->resizeColumnsToContents();
}

void EventsWidget::scrollTo(double time)
{
	if (events != nullptr) {
		int n = findBestMatch(time);
		tableView->selectRow(n);
		tableView->resizeColumnsToContents();
	}
}

void EventsWidget::scrollTo(int n)
{
	if (n < 0 || events == nullptr)
		return;
	unsigned int index = (unsigned int) n;
	if (index < events->size()) {
		tableView->selectRow(index);
		tableView->resizeColumnsToContents();
	}
}

/* This function checks the value at, before and after the value found 
 * with binary search in order to determine the one with smallest difference
 */
int EventsWidget::findBestMatch(double time)
{
	int n = 0;
	int c, next, prev;
	int end;
	int cand[3];
	double diffs[3];
	double best;
	int bestN;
	int i;

	end = events->size() - 1;

	if (end < 0)
		return 0;

	c =  binarySearch(time, 0, end);

	cand[n] = c;
	diffs[n] = fabs(events->at(c).time - time);
	bestN = c;
	best = diffs[n];
	n++;

	next = c + 1;
	prev = c - 1;

	if (next <= end) {
		cand[n] = next;
		diffs[n] = fabs(events->at(next).time - time);
		n++;
	}

	if (prev >= 0) {
		cand[n] = prev;
		diffs[n] = fabs(events->at(prev).time - time);
		n++;
	}

	for (i = 0; i < n; i++) {
		if (diffs[i] < best) {
			best = diffs[i];
			bestN = cand[i];
		}
	}

	/* Basic sanity in case the beginning or end has multiple events
	 * with the same time */
	if (time > events->at(end).time)
		bestN = end;
	if (time < events->at(0).time)
		bestN = 0;

	return bestN;
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

void EventsWidget::handleClick(const QModelIndex &index)
{
	if (index.column() == 0) {
		double time = events->at(index.row()).time;
		emit timeSelected(time);
	}
}

void EventsWidget::handleDoubleClick(const QModelIndex &index)
{
	if (index.column() == 5) {
		const TraceEvent &event = events->at(index.row());
		emit infoDoubleClicked(event);
	}
}
