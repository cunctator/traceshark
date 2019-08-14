// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "ui/cpuselectmodel.h"

CPUSelectModel::CPUSelectModel(QObject *parent):
	QAbstractTableModel(parent), nrCPUs(0)
{
	errorStr = new QString(tr("Error in cpuselectmodel.cpp"));
}

CPUSelectModel::~CPUSelectModel()
{
	delete errorStr;
}

void CPUSelectModel::setNrCPUs(unsigned int cpus)
{
	nrCPUs = (int) cpus;
}

int CPUSelectModel::rowCount(const QModelIndex & /* index */) const
{
	return nrCPUs;
}

int CPUSelectModel::columnCount(const QModelIndex & /* index */) const
{
	return 1; /* Number from data() and headerData() */
}

unsigned int CPUSelectModel::rowToCPU(int row, bool &ok) const
{
	ok = true;
	if (row >= 0 && row < nrCPUs)
		return (unsigned int) row;
	ok = false;
	return 0;
}


QString CPUSelectModel::rowToName(int row, bool &ok) const
{
	QString name;

	name = QString(tr("CPU")) + QString::number(rowToCPU(row, ok));
	if (!ok) {
		name = *errorStr;
	}
	return name;
}

QVariant CPUSelectModel::data(const QModelIndex &index, int role) const
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

bool CPUSelectModel::setData(const QModelIndex &/*index*/, const QVariant
			&/*value*/, int /*role*/)
{
	return false;
}

QVariant CPUSelectModel::headerData(int section,
				      Qt::Orientation orientation,
				      int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		if (section == 0)
			return QString(tr("CPU"));
		return *errorStr;
	}
	return QVariant();
}

Qt::ItemFlags CPUSelectModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	return flags;
}

void CPUSelectModel::beginResetModel()
{
	QAbstractTableModel::beginResetModel();
}

void CPUSelectModel::endResetModel()
{
	QAbstractTableModel::endResetModel();
}
