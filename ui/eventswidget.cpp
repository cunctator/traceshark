/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
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
#include "ui/eventsmodel.h"
#include "ui/eventswidget.h"
#include "misc/traceshark.h"

EventsWidget::EventsWidget(QWidget *parent):
	QDockWidget(tr("Events"), parent), events(nullptr)
{
	tableView = new QTableView(this);
	eventsModel = new EventsModel(tableView);
	tableView->setModel(eventsModel);
	setWidget(tableView);
	tableView->horizontalHeader()->setStretchLastSection(true);
	resizeColumnsToContents();
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
	resizeColumnsToContents();
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
	resizeColumnsToContents();
}

void EventsWidget::scrollTo(double time)
{
	if (events != nullptr) {
		int n = findBestMatch(time);
		tableView->selectRow(n);
		resizeColumnsToContents();
	}
}

void EventsWidget::scrollTo(int n)
{
	if (n < 0 || events == nullptr)
		return;
	unsigned int index = (unsigned int) n;
	if (index < events->size()) {
		tableView->selectRow(index);
		resizeColumnsToContents();
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
