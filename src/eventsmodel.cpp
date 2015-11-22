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

#include "tlist.h"
#include <QVariant>
#include <QString>
#include "eventsmodel.h"
#include "traceevent.h"


EventsModel::EventsModel(QObject *parent):
	QAbstractTableModel(parent), events(NULL)
{}

EventsModel::EventsModel(TList<TraceEvent> *e, QObject *parent):
	QAbstractTableModel(parent), events(e)
{}

void EventsModel::setEvents(TList<TraceEvent> *e)
{
	events = e;
}

int EventsModel::rowCount(const QModelIndex & /*parent*/) const
{
	if (events != NULL)
		return events->size();
	else 
		return 0;
}

int EventsModel::columnCount(const QModelIndex & /* parent */) const
{
	return 6; /* Number from data() and headerData() */
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	
	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	} else if (role == Qt::DisplayRole) {
		int row = index.row();
		int column = index.column();

		if (events == NULL || row >= events->size())
			return QVariant();
		TraceEvent &event = (*events)[row];
		switch(column) {
		case 0:
			return QString::number(event.time, 'f', 6);
		case 1:
			return QString(event.taskName->ptr);
		case 2:
			return QString::number(event.pid);
		case 3:
			return QString("[") + QString::number(event.cpu) +
				QString("]");
		case 4:
			return QString(event.eventName->ptr);
		case 5:
			QString str;
			unsigned int i;
			for (i = 0; i < event.argc; i++) {
				str += QString(event.argv[i]->ptr);
				if (i < event.argc - 1)
					str += QString(tr(" "));
			}
			return str;
		}
	}
	return QVariant();
}

bool EventsModel::setData(const QModelIndex &/*index*/, const QVariant
			  &/*value*/, int /*role*/)
{
	return false;
}

QVariant EventsModel::headerData(int section,
                               Qt::Orientation orientation,
                               int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch(section) {
		case 0:
			return QString(tr("Time"));
		case 1:
			return QString(tr("Task"));
		case 2:
			return QString(tr("PID(TID)"));
		case 3:
			return QString(tr("CPU"));
		case 4:
			return QString(tr("Event"));
		case 5:
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
