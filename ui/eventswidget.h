// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef EVENTSWIDGET_H
#define EVENTSWIDGET_H

#include <QDockWidget>
#include "misc/traceshark.h"
#include "vtl/time.h"

class TableView;
class EventsModel;
class TraceEvent;
namespace vtl {
	template<class T> class TList;
}

class EventsWidget : public QDockWidget
{
	Q_OBJECT
public:
	EventsWidget(QWidget *parent = 0);
	EventsWidget(vtl::TList<TraceEvent> *e, QWidget *parent = 0);
	virtual ~EventsWidget();
	void setEvents(vtl::TList<TraceEvent> *e);
	void setEvents(vtl::TList<const TraceEvent*> *e);
	void clear();
	void clearScrollTime();
	void beginResetModel();
	void endResetModel();
	void resizeColumnsToContents();
	void scrollTo(const vtl::Time &time);
	void scrollTo(int n);
	void scrollToSaved();
	vtl::Time getSavedScroll();
	const TraceEvent *getSelectedEvent();
public slots:
	void show();
signals:
	void timeSelected(vtl::Time time);
	void infoDoubleClicked(const TraceEvent &event);
	void eventSelected(const TraceEvent *event);
private slots:
	void handleClick(const QModelIndex &index);
	void handleDoubleClick(const QModelIndex &index);
	void handleSelectionChanged(const QItemSelection &selected,
				    const QItemSelection &deselected);
private:
	TableView *tableView;
	EventsModel *eventsModel;
	vtl::TList<TraceEvent> *events;
	vtl::TList<const TraceEvent*> *eventsPtrs;
	bool saveScrollTime;
	vtl::Time scrollTime;
	const TraceEvent *selectedEvent;
	int findBestMatch(const vtl::Time &time);
	int binarySearch(const vtl::Time &time, int start, int end);
	const TraceEvent* getEventAt(int index) const;
	unsigned int getSize() const;
};

#endif /* EVENTSWIDGET_H*/
