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

#include <QVector>
#include <QVariant>
#include <QString>
#include "eventsmodel.h"
#include "traceevent.h"


EventsModel::EventsModel():
	events(NULL)
{}

EventsModel::EventsModel(QVector<TraceEvent> *e):
	events(e)
{}

void EventsModel::setEvents(QVector<TraceEvent> *e)
{
	beginResetModel();
	events = e;
	endResetModel();
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
	return 5;
}

QVariant EventsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();
	
	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignRight | Qt::AlignVCenter);
	} else if (role == Qt::DisplayRole) {
		QString str;
		int row = index.row();
		int column = index.column();

		if (events == NULL || row >= events->size())
			return QVariant();
		TraceEvent &event = (*events)[row];
		switch(column) {
		case 0:
			return QString(event.taskName->ptr);
		case 1:
			str = QString::number(event.cpu);
			return str;
		case 2:
			str = QString::number(event.time, 'f', 6);
			return str;
		case 3:
			return QString(event.eventName->ptr);
		case 4:
			unsigned int i;
			for (i = 0; i < event.argc; i++)
				str += QString(event.argv[i]->ptr);
			return str;
		}
	}
	return QVariant();
}

QVariant EventsModel::headerData(int section,
                               Qt::Orientation /* orientation */,
                               int role) const
{
	if (role == Qt::DisplayRole) {
		switch(section) {
		case 0:
			return QString(tr("Task"));
		case 1:
			return QString(tr("CPU"));
		case 2:
			return QString(tr("Time"));
		case 3:
			return QString(tr("Event"));
		case 4:
			return QString(tr("Info"));
		default:
			return QString(tr("Error in eventsmodel.cpp"));	
		}
	}
	return QVariant();
}

void EventsModel::beginResetModel()
{
	QAbstractTableModel::beginResetModel();
}

void EventsModel::endResetModel()
{
	QAbstractTableModel::endResetModel();
}
