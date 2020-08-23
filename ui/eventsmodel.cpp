// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018, 2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QVariant>
#include <QString>
#include "ui/eventsmodel.h"
#include "parser/traceevent.h"
#include "misc/traceshark.h"
#include "vtl/tlist.h"


EventsModel::EventsModel(QObject *parent):
	QAbstractTableModel(parent), events(nullptr), eventsPtrs(nullptr)
{}

EventsModel::EventsModel(vtl::TList<TraceEvent> *e, QObject *parent):
	QAbstractTableModel(parent), events(e), eventsPtrs(nullptr)
{}

void EventsModel::setEvents(vtl::TList<TraceEvent> *e)
{
	events = e;
	eventsPtrs = nullptr;
}

void EventsModel::setEvents(vtl::TList<const TraceEvent*> *e)
{
	events = nullptr;
	eventsPtrs = e;
}

void EventsModel::clear()
{
	events = nullptr;
	eventsPtrs = nullptr;
}

int EventsModel::rowCount(const QModelIndex & /* parent */) const
{
	return getSize();
}

int EventsModel::columnCount(const QModelIndex & /* parent */) const
{
	return column_to_int(NR_COLUMNS);
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
	QString str;
	int i;

	if (!index.isValid())
		return QVariant();
	
	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	} else if (role == Qt::DisplayRole) {
		int row = index.row();
		column_t column = int_to_column(index.column());
		int size;

		if (events == nullptr && eventsPtrs == nullptr)
			return QVariant();
		size = getSize();
		if ( row >= size || row < 0)
			return QVariant();

		const TraceEvent &event = *getEventAt(row);
		switch(column) {
		case COLUMN_TIME:
			return event.time.toQString();
		case COLUMN_TASKNAME:
			return QString(event.taskName->ptr);
		case COLUMN_PID:
			return QString::number(event.pid);
		case COLUMN_CPU:
			return QString("[") + QString::number(event.cpu) +
				QString("]");
		case COLUMN_TYPE:
			return QString(event.getEventName()->ptr);
		case COLUMN_INFO:
			/*
			 * If there was an integer before the event name, then
			 * we will display that as if it had been the first 
			 * argument of the event
			 */
			if (event.intArg != 0) {
				str += QString::number(event.intArg);
				if (event.argc > 0)
					str += QString(tr(" "));
			}
			for (i = 0; i < event.argc; i++) {
				str += QString(event.argv[i]->ptr);
				if (i < event.argc - 1)
					str += QString(tr(" "));
			}
			return str;
		default:
			break;
		}
	}
	return QVariant();
}

bool EventsModel::setData(const QModelIndex & /* index */,
			  const QVariant & /* value */, int /* role */)
{
	return false;
}

QVariant EventsModel::headerData(int section,
				 Qt::Orientation orientation,
				 int role) const
{
	column_t column = int_to_column(section);
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch(column) {
		case COLUMN_TIME:
			return QString(tr("Time"));
		case COLUMN_TASKNAME:
			return QString(tr("Task"));
		case COLUMN_PID:
			return QString(tr("PID(TID)"));
		case COLUMN_CPU:
			return QString(tr("CPU"));
		case COLUMN_TYPE:
			return QString(tr("Event type"));
		case COLUMN_INFO:
			return QString(tr("Info"));
		default:
			return QString(tr("Error in eventsmodel.cpp"));	
		}
	}
	return QVariant();
}

Qt::ItemFlags EventsModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	return flags;
}

void EventsModel::beginResetModel()
{
	QAbstractTableModel::beginResetModel();
}

void EventsModel::endResetModel()
{
	QAbstractTableModel::endResetModel();
}

const TraceEvent* EventsModel::getEventAt(int index) const
{
	if (events != nullptr)
		return &events->at(index);
	if (eventsPtrs != nullptr)
		return eventsPtrs->at(index);
	return nullptr;
}

int EventsModel::getSize() const
{
	if (events != nullptr)
		return events->size();
	if (eventsPtrs != nullptr)
		return eventsPtrs->size();
	return 0;
}
