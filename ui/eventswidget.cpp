// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QTableView>
#include <cmath>
#include "vtl/tlist.h"
#include "ui/eventsmodel.h"
#include "ui/eventswidget.h"
#include "ui/tableview.h"
#include "misc/traceshark.h"
#include "parser/traceevent.h"

EventsWidget::EventsWidget(QWidget *parent):
	QDockWidget(tr("Events"), parent), events(nullptr),
	eventsPtrs(nullptr), saveScrollTime(false), selectedEvent(nullptr)
{
	tableView = new TableView(this, TableView::TABLE_SINGLEROWSELECT);
	eventsModel = new EventsModel(tableView);
	tableView->setModel(eventsModel);
	setWidget(tableView);
	tableView->horizontalHeader()->setStretchLastSection(true);
	resizeColumnsToContents();
	tableView->show();
	tsconnect(tableView, doubleClicked(const QModelIndex &),
		  this, handleDoubleClick(const QModelIndex &));
	tsconnect(tableView, sigSelectionChanged(const QItemSelection &,
					      const QItemSelection &),
		  this, handleSelectionChanged(const QItemSelection &,
					       const QItemSelection &));
}

EventsWidget::EventsWidget(vtl::TList<TraceEvent> *e, QWidget *parent):
	QDockWidget(parent), eventsPtrs(nullptr), saveScrollTime(false),
	selectedEvent(nullptr)
{
	tableView = new TableView(this, TableView::TABLE_SINGLEROWSELECT);
	eventsModel = new EventsModel(e, tableView);
	events = e;
	tableView->setModel(eventsModel);
	setWidget(tableView);
	tableView->horizontalHeader()->setStretchLastSection(true);
	resizeColumnsToContents();
	tableView->show();
	tsconnect(tableView, doubleClicked(const QModelIndex &),
		  this, handleDoubleClick(const QModelIndex &));
	tsconnect(tableView, sigSelectionChanged(const QItemSelection &,
					      const QItemSelection &),
		  this, handleSelectionChanged(const QItemSelection &,
					       const QItemSelection &));
}

EventsWidget::~EventsWidget()
{
}

void EventsWidget::setEvents(vtl::TList<TraceEvent> *e)
{
	eventsModel->setEvents(e);
	events = e;
	eventsPtrs = nullptr;
}

void EventsWidget::setEvents(vtl::TList<const TraceEvent*> *e)
{
	eventsModel->setEvents(e);
	events = nullptr;
	eventsPtrs = e;
}

void EventsWidget::clear()
{
	eventsModel->clear();
	events = nullptr;
	eventsPtrs = nullptr;
}

void EventsWidget::clearScrollTime()
{
	saveScrollTime = false;
}

void EventsWidget::beginResetModel()
{
	eventsModel->beginResetModel();
	events = nullptr;
	eventsPtrs = nullptr;
}

void EventsWidget::endResetModel()
{
	eventsModel->endResetModel();
	resizeColumnsToContents();
}

void EventsWidget::scrollTo(const vtl::Time &time)
{
	if (events != nullptr || eventsPtrs != nullptr) {
		int n = findBestMatch(time);
		tableView->selectRow(n);
		resizeColumnsToContents();
		scrollTime = time;
		saveScrollTime = true;
	}
}

void EventsWidget::scrollTo(int n)
{
	if (n < 0 || (events == nullptr && eventsPtrs == nullptr))
		return;
	unsigned int index = (unsigned int) n;
	if (index < getSize()) {
		tableView->selectRow(index);
		resizeColumnsToContents();
		scrollTime = getEventAt(index)->time;
		saveScrollTime = true;
	}
}

void EventsWidget::scrollToSaved()
{
	if (saveScrollTime)
		scrollTo(scrollTime);
}


/* This function checks the value at, before and after the value found 
 * with binary search in order to determine the one with smallest difference
 */
int EventsWidget::findBestMatch(const vtl::Time &time)
{
	int n = 0;
	int c, next, prev;
	int end;
	int cand[3];
	vtl::Time diffs[3];
	vtl::Time best;
	int bestN;
	int i;

	end = getSize() - 1;

	if (end < 0)
		return 0;

	c =  binarySearch(time, 0, end);

	cand[n] = c;
	diffs[n] = (getEventAt(c)->time - time).fabs();
	bestN = c;
	best = diffs[n];
	n++;

	next = c + 1;
	prev = c - 1;

	if (next <= end) {
		cand[n] = next;
		diffs[n] = (getEventAt(next)->time - time).fabs();
		n++;
	}

	if (prev >= 0) {
		cand[n] = prev;
		diffs[n] = (getEventAt(prev)->time - time).fabs();
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
	if (time > getEventAt(end)->time)
		bestN = end;
	if (time < getEventAt(0)->time)
		bestN = 0;

	return bestN;
}

int EventsWidget::binarySearch(const vtl::Time &time, int start, int end)
{
	int pivot = (end + start) / 2;
	if (pivot == start)
		return pivot;
	if (time < getEventAt(pivot)->time)
		return binarySearch(time, start, pivot);
	else
		return binarySearch(time, pivot, end);
}

void EventsWidget::handleDoubleClick(const QModelIndex &index)
{
	const TraceEvent &event = *getEventAt(index.row());
	EventsModel::column_t col = (EventsModel::column_t) index.column();
	emit eventDoubleClicked(col, event);
}

void EventsWidget::handleSelectionChanged(const QItemSelection &/*selected*/,
					  const QItemSelection &/*deselected*/)
{
	const TraceEvent *event = getSelectedEvent();

	if (event != selectedEvent) {
		if (event != nullptr) {
			scrollTime = event->time;
			saveScrollTime = true;
		}
		selectedEvent = event;
		emit eventSelected(event);
	}
}

const TraceEvent *EventsWidget::getSelectedEvent()
{
	int s, i, row;
	const QModelIndexList list = tableView->selectedIndexes();
	const TraceEvent *event = nullptr;

	s = list.size();
	if (s < 1)
		goto out;

	row = list[0].row();

	for (i = 1; i < s; i++) {
		const QModelIndex &idx = list[i];
		/* If more than one row is selected, don't bother */
		if (idx.row() != row)
			goto out;
	}

	if (events != nullptr) {
		event = &events->at(row);
	} else if (eventsPtrs != nullptr) {
		event = eventsPtrs->at(row);
	}

out:
	return event;
}

/* Apparently it's a bad idea to do tableView->resizeColumnsToContents() if we
 * are not visible */
void EventsWidget::resizeColumnsToContents()
{
	if (QDockWidget::isVisible())
		tableView->resizeColumnsToContents();
}

void EventsWidget::show()
{
	QDockWidget::show();
	tableView->resizeColumnsToContents();
}

const TraceEvent* EventsWidget::getEventAt(int index) const
{
	if (events != nullptr)
		return &events->at(index);
	if (eventsPtrs != nullptr)
		return eventsPtrs->at(index);
	return nullptr;
}

unsigned int EventsWidget::getSize() const
{
	if (events != nullptr)
		return events->size();
	if (eventsPtrs != nullptr)
		return eventsPtrs->size();
	return 0;
}

vtl::Time EventsWidget::getSavedScroll()
{
	if (saveScrollTime)
		return scrollTime;
	return 0;
}
