// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstring>

#include <QString>

#include "vtl/tlist.h"
#include "vtl/heapsort.h"

#include "ui/eventselectmodel.h"
#include "analyzer/task.h"
#include "mm/stringtree.h"

EventSelectModel::EventSelectModel(QObject *parent):
	QAbstractTableModel(parent), stringTree(nullptr)
{
	eventList = new vtl::TList<event_t>;
	errorStr = new QString(tr("Error in eventselectmodel.cpp"));
}

EventSelectModel::~EventSelectModel()
{
	delete eventList;
	delete errorStr;
}

void EventSelectModel::setStringTree(const StringTree<> *stree)
{
	event_t event;
	int i, maxevent;
	eventList->clear();

	stringTree = stree;

	if (stree == nullptr)
		return;

	maxevent = (int) stree->getMaxEvent();
	for (i = 0; i <= maxevent; i++) {
		event = (event_t) i;
		eventList->append(event);
	}

	vtl::heapsort<vtl::TList, event_t>(
		*eventList, [stree] (event_t a, event_t b) -> int {
			const TString *as = stree->stringLookup(a);
			const TString *bs = stree->stringLookup(b);
			return strcmp(as->ptr, bs->ptr);
		});
}

int EventSelectModel::rowCount(const QModelIndex & /* index */) const
{
	return eventList->size();
}

int EventSelectModel::columnCount(const QModelIndex & /* index */) const
{
	return 1; /* Number from data() and headerData() */
}

event_t EventSelectModel::rowToEvent(int row, bool &ok) const
{
	if (row < 0) {
		ok = false;
		return (event_t)0;
	}
	if (row >= eventList->size()) {
		ok = false;
		return (event_t)0;
	}

	ok = true;
	return  eventList->at(row);
}

QString EventSelectModel::rowToName(int row, bool &ok) const
{
	QString name;
	const TString *str;
	event_t event;

	if (row < 0)
		goto error;
	if (row >= eventList->size())
		goto error;

	event = eventList->at(row);
	str = stringTree->stringLookup(event);
	if (str == nullptr || str->ptr == nullptr)
		goto error;

	name = QString(str->ptr);
	ok = true;
	return name;
error:
	ok = false;
	return *errorStr;
}

QVariant EventSelectModel::data(const QModelIndex &index, int role) const
{
	bool ok;

	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole)
		return int(Qt::AlignLeft | Qt::AlignVCenter);

	if (role == Qt::DisplayRole) {
		int row = index.row();
		int column = index.column();
		QString name;

		if (column == 0) {
			name = rowToName(row, ok);
			if (ok)
				return name;
		}
	}
	return QVariant();
}

bool EventSelectModel::setData(const QModelIndex &/*index*/, const QVariant
			&/*value*/, int /*role*/)
{
	return false;
}

QVariant EventSelectModel::headerData(int section,
				      Qt::Orientation orientation,
				      int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if (section == 0)
			return QString(tr("Event Name"));
		return *errorStr;
	}
	return QVariant();
}

Qt::ItemFlags EventSelectModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	return flags;
}

void EventSelectModel::beginResetModel()
{
	QAbstractTableModel::beginResetModel();
}

void EventSelectModel::endResetModel()
{
	QAbstractTableModel::endResetModel();
}
