// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018, 2020, 2024
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef EVENTSMODEL_H
#define EVENTSMODEL_H

#include <QAbstractTableModel>
#include "vtl/compiler.h"

class TraceEvent;
namespace vtl {
	template<class T> class TList;
}

class EventsModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	typedef enum : int {
		COLUMN_TIME = 0,
		COLUMN_TASKNAME,
		COLUMN_PID,
		COLUMN_CPU,
		COLUMN_FLAGS,
		COLUMN_TYPE,
		COLUMN_INFO,
		NR_COLUMNS
	} column_t;
	EventsModel(QObject *parent = 0);
	EventsModel(vtl::TList<TraceEvent> *e, QObject *parent = 0);
	void setEvents(vtl::TList<TraceEvent> *e);
	void setEvents(vtl::TList<const TraceEvent*> *e);
	void clear();
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
	vtl_always_inline column_t int_to_column(int i) const;
	vtl_always_inline int column_to_int(column_t c) const;
private:
	bool has_flag_field;
	vtl::TList<TraceEvent> *events;
	vtl::TList<const TraceEvent*> *eventsPtrs;
	const TraceEvent* getEventAt(int index) const;
	int getSize() const;
	void checkFlagField(void);
};

vtl_always_inline EventsModel::column_t EventsModel::int_to_column(int i) const
{
	if (!has_flag_field && i >= ((int)(COLUMN_FLAGS)))
		i++;
	return (column_t) i;
}

vtl_always_inline int EventsModel::column_to_int(EventsModel::column_t c) const
{
	int i = (int) c;

	if (!has_flag_field && i >= ((int)(COLUMN_FLAGS) + 1))
		i--;
	return i;
}

#endif /* EVENTSMODEL_H */
