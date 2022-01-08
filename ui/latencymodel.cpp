// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2021, 2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstdio>
#include <cstdint>
#include "misc/traceshark.h"
#include "analyzer/traceanalyzer.h"
#include "ui/latencymodel.h"

LatencyModel::LatencyModel(enum Latency::Type type, QObject *parent):
	QAbstractTableModel(parent), latency_type(type), latencies(nullptr),
	analyzer(nullptr)
{}

LatencyModel::~LatencyModel()
{}

void LatencyModel::setAnalyzer(TraceAnalyzer *azr)
{
	beginResetModel();

	if (latency_type == Latency::TYPE_WAKEUP)
		latencies = &azr->wakeLatencies;
	else if (latency_type == Latency::TYPE_SCHED)
		latencies = &azr->schedLatencies;
	else
		latencies = nullptr;
	analyzer = azr;

	endResetModel();
}

void LatencyModel::clear()
{
	beginResetModel();

	latencies = nullptr;
	analyzer = nullptr;

	endResetModel();
}

int LatencyModel::rowCount(const QModelIndex & /* parent */) const
{
	return getSize();
}

int LatencyModel::columnCount(const QModelIndex & /* parent */) const
{
	return column_to_int(NR_COLUMNS);
}

QVariant LatencyModel::data(const QModelIndex &index, int role) const
{
	QString str;

	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	} else if (role == Qt::DisplayRole) {
		int row = index.row();
		column_t column = int_to_column(index.column());
		Task *task;

		const Latency *latency = rowToLatency(row);
		if (latency == nullptr)
			return QVariant();

		switch(column) {
		case COLUMN_PID:
			return QString::number(latency->pid);
		case COLUMN_TASKNAME:
			task = analyzer->findTask(latency->pid);
			return *task->displayName;
		case COLUMN_TIME:
			return latency->time.toQString();
		case COLUMN_DELAY:
			return latency->delay.toQString();
		case COLUMN_PLACE:
			return QString::number(latency->place);
		case COLUMN_PERCENT:
			return placeToPct(latency->place);
		default:
			break;
		}
	}
	return QVariant();
}

bool LatencyModel::setData(const QModelIndex & /* index */,
			   const QVariant & /* value */, int /* role */)
{
	return false;
}

QVariant LatencyModel::headerData(int section,
				  Qt::Orientation orientation,
				  int role) const
{
	column_t column = int_to_column(section);
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch(column) {
		case COLUMN_PID:
			return QString(tr("PID(TID)"));
		case COLUMN_TASKNAME:
			return QString(tr("Task"));
		case COLUMN_TIME:
			return QString(tr("Time"));
		case COLUMN_DELAY:
			return QString(tr("Delay"));
		case COLUMN_PLACE:
			return QString(tr("Place"));
		case COLUMN_PERCENT:
			return QString(tr("Pctile"));
		default:
			return QString(tr("Error in latencymodel.cpp"));
		}
	}
	return QVariant();
}

Qt::ItemFlags LatencyModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	return flags;
}

const Latency* LatencyModel::rowToLatency(int index) const
{
	if (latencies != nullptr && index >= 0 &&
	    index < latencies->size())
		return &latencies->at(index);
	return nullptr;
}

int LatencyModel::getSize() const
{
	if (latencies != nullptr)
		return latencies->size();
	return 0;
}


QString LatencyModel::placeToPct(unsigned int place) const
{
	uint64_t pct = 10000UL;
	const int size = getSize();
	const unsigned int usize = size > 1 ? size - 1 : 1;
	char buf[16];
	int bufsize = arraylen(buf);
	int r;

	pct *= usize - place;
	pct /= usize;

	r = snprintf(buf, bufsize - 1, "%u.%02u", (unsigned) (pct / 100),
		     (unsigned) (pct % 100));
	if (r < 0)
		return QString();

	buf[bufsize - 1] = '\0';
	return QString(buf);
}
