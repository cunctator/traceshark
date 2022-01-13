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

#ifndef _LATENCYMODEL_H
#define _LATENCYMODEL_H

#include <QAbstractTableModel>
#include "analyzer/latency.h"

namespace vtl {
	template<class T> class TList;
}

class Latency;
class TraceAnalyzer;

class LatencyModel : public QAbstractTableModel
{
	Q_OBJECT
public:
	typedef enum : int {
		COLUMN_PID = 0,
		COLUMN_TASKNAME,
		COLUMN_TIME,
		COLUMN_DELAY,
		COLUMN_PLACE,
		COLUMN_PERCENT,
		NR_COLUMNS
	} column_t;
	LatencyModel(enum Latency::Type type, QObject *parent = 0);
	~LatencyModel();
	void setAnalyzer(TraceAnalyzer *azr);
	void clear();
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;
	bool setData(const QModelIndex &index, const QVariant &value,
		     int role);
	QVariant headerData(int section, Qt::Orientation orientation,
			    int role) const;
	int rowToPid(int row, bool &ok) const;
	const QString &rowToName(int row, bool &ok) const;
	const Latency *rowToLatency(int row) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	static vtl_always_inline column_t int_to_column(int i);
	static vtl_always_inline int column_to_int(column_t c);
	vtl_always_inline enum Latency::Type getLatencyType() const;
private:
	int getSize() const;
	QString placeToPct(unsigned int place) const;
	enum Latency::Type latency_type;
	vtl::TList<Latency> *latencies;
	TraceAnalyzer *analyzer;
};

vtl_always_inline LatencyModel::column_t LatencyModel::int_to_column(int i)
{
	return (column_t) i;
}

vtl_always_inline int LatencyModel::column_to_int(LatencyModel::column_t c)
{
	return (int) c;
}

vtl_always_inline enum Latency::Type LatencyModel::getLatencyType() const
{
	return latency_type;
}

#endif /* _LATENCYMODEL_H */
