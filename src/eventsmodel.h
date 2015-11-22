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

#ifndef EVENTSMODEL_H
#define EVENTSMODEL_H

#include <QAbstractTableModel>
#include "tlist.h"
#include "traceevent.h"

//class TraceEvent;
//class QVector<TraceEvent>

class EventsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	EventsModel(QObject *parent = 0);
	EventsModel(TList<TraceEvent> *e, QObject *parent = 0);
	void setEvents(TList<TraceEvent> *e);
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value,
		     int role);
	QVariant headerData(int section, Qt::Orientation orientation,
			    int role) const;
	void beginResetModel();
	void endResetModel();
	Qt::ItemFlags flags(const QModelIndex &index) const;
private:
	TList<TraceEvent> *events;
};

#endif /* EVENTSMODEL_H */
