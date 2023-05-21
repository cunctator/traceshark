// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2018, 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QFile>

#include "vtl/avltree.h"
#include "vtl/heapsort.h"
#include "vtl/tlist.h"

#include "misc/translate.h"
#include "ui/statsmodel.h"
#include "analyzer/task.h"

StatsModel::StatsModel(QObject *parent):
	AbstractTaskModel(parent)
{}

void StatsModel::setTaskMap(vtl::AVLTree<int, TaskHandle> *map,
			    unsigned int nrcpus)
{
	vtl::Time delta = getDeltaTime();

	taskList->clear();

	if (map == nullptr)
		return;

	vtl::Time &idle_time = getRelevantTime(idleTask);
	unsigned &idle_pct = getRelevantPct(idleTask);

	idle_time = delta * nrcpus;

	DEFINE_TASKMAP_ITERATOR(iter) = map->begin();
	while (iter != map->end()) {
		Task *task = iter.value().task;

		vtl::Time &stat_time = getRelevantTime(task);
		if (!checkZeroTime() || !stat_time.isZero()) {
			taskList->append(task);
			idle_time -= stat_time;
		}
		iter++;
	}

	idle_pct = (unsigned)
		(10000 * (idle_time.toDouble() / delta.toDouble() +
			  0.00005));

	/* Add a fake idle task for event filtering purposes */
	taskList->append(idleTask);
	sortTaskList();
}

int StatsModel::rowCount(const QModelIndex & /* index */) const
{
	return taskList->size();
}

int StatsModel::columnCount(const QModelIndex & /* index */) const
{
	return 4; /* Number from data() and headerData() */
}

void StatsModel::rowToPct(QString &str, int row, bool &ok) const
{
	char buf[10];
	unsigned int p;
	unsigned int div;
	unsigned int rest;
	int i, n;
	bool nonzero;

	if (row < 0) {
		ok = false;
		return;
	}
	if (row >= taskList->size()) {
		ok = false;
		return;
	}

	ok = true;
	const Task *task = taskList->at(row);
	unsigned pct = getRelevantPctConst(task);

	/* We assume no system has more than 9999 CPUs */
	if (pct > 99990000) {
		ok = false;
		return;
	}

	div = 10000000;
	n = 0;
	rest = pct;
	nonzero = false;
	for (i = 0; i <= 7; i++) {
		if (i == 6) {
			buf[n] = '.';
			n++;
		}
		p = rest / div;
		if (i >= 5 || p != 0 || nonzero) {
			nonzero = true;
			buf[n] = '0' + p;
			n++;
		} else if (i >= 3) {
			buf[n] = ' ';
			n++;
		}
		rest = rest % div;
		div = div / 10;
	}
	buf[n] = '\0';
	str = QString(&buf[0]);
}

void StatsModel::rowToTime(QString &str, int row, bool &ok) const
{
	if (row < 0) {
		ok = false;
		return;
	}
	if (row >= taskList->size()) {
		ok = false;
		return;
	}

	ok = true;
	const Task *task = taskList->at(row);
	str = getRelevantTimeConst(task).toQString();
}


QVariant StatsModel::data(const QModelIndex &index, int role) const
{
	bool ok;
	bool ghost;

	if (!index.isValid())
		return QVariant();

	if (role == Qt::TextAlignmentRole)
		return int(Qt::AlignLeft | Qt::AlignVCenter);

	if (role == Qt::DisplayRole) {
		int row = index.row();
		int column = index.column();
		int pid;
		QString str;

		switch(column) {
		case 0:
			str = rowToName(row, ok);
			if (ok)
				return str;
			break;
		case 1:
			pid = rowToPid(row, ok);
			if (ok)
				return QString::number(pid);
			break;
		case 2:
			rowToPct(str, row, ok);
			if (ok)
				return str;
			break;
		case 3:
			rowToTime(str, row, ok);
			if (ok)
				return str;
			break;
		default:
			break;
		}
	}

	if (role == Qt::BackgroundRole) {
		int row = index.row();

		ghost = rowToGhostStatus(row, ok);
		if (ok && ghost)
			return QBrush(Qt::cyan);
		else
			return QVariant();
	}

	if (role == Qt::ForegroundRole) {
		int row = index.row();

		ghost = rowToGhostStatus(row, ok);
		if (ok && ghost)
			return QBrush(Qt::red);
		else
			return QVariant();
	}

	return QVariant();
}

bool StatsModel::setData(const QModelIndex &/*index*/, const QVariant
			&/*value*/, int /*role*/)
{
	return false;
}

QVariant StatsModel::headerData(int section,
				 Qt::Orientation orientation,
				 int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
		switch(section) {
		case 0:
			return QString(tr("Name"));
		case 1:
			return QString(tr("PID(TID)"));
		case 2:
			return QString(tr("CPU(%)"));
		case 3:
			return QString(tr("CPU(s)"));
		default:
			return *errorStr;
		}
	}
	return QVariant();
}

Qt::ItemFlags StatsModel::flags(const QModelIndex &index) const
{
	Qt::ItemFlags flags = QAbstractItemModel::flags(index);
	return flags;
}

void StatsModel::beginResetModel()
{
	QAbstractTableModel::beginResetModel();
}

void StatsModel::endResetModel()
{
	QAbstractTableModel::endResetModel();
}

int StatsModel::exportStats(bool csv, const QString &filename)
{
	/* Todo add export code here */
	qfile_error_t err;
	int row;
	QString sep = csv ? QString(",") : QString("\t");
	QString str;
	bool ok;
	bool is_ghost;
	const int s = taskList->size();
	QTextStream stream;
	QString gstatus;

	if (filename.isEmpty())
		return 0;

	QFile file(filename);

	if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly))
		goto error_file;

	stream.setDevice(&file);

	stream << "TASK_NAME" << sep << "PID(TID)" << sep << "CPU_TIME_PCT(%)"
	       << sep << "CPU_TIME(s)" << sep << "GHOST_STATUS" << "\n";

	for (row = 0; row < s; row++) {
		str = rowToName(row, ok);
		if (ok)
			stream << str << sep;

		str = QString::number(rowToPid(row, ok));
		if (ok)
			stream << str << sep;

		rowToPct(str, row, ok);
		if (ok)
			stream << str << sep;

		rowToTime(str, row, ok);
		if (ok)
			stream << str;

		is_ghost = rowToGhostStatus(row, ok);

		if (ok) {
			gstatus = is_ghost ?
				QLatin1String("GHOST") :
				QLatin1String("REAL");
		} else {
			gstatus = QLatin1String("UNKNOWN");
		}
		stream << sep << gstatus;
		stream << "\n";
	}

	stream.flush();
	if (!file.flush())
		goto error_file;
	file.close();

	err = file.error();
	if (err == qfile_error_class::NoError)
		return 0;
	goto error_translate;

error_file:
	err = file.error();
	if (err == qfile_error_class::NoError)
		return -TS_ERROR_UNSPEC;
error_translate:
	return -translate_FileError(err);
}

vtl::Time &StatsModel::getRelevantTime(Task *task)
{
	return task->accTime;
}

const vtl::Time &StatsModel::getRelevantTimeConst(const Task *task) const
{
	return task->accTime;
}

unsigned &StatsModel::getRelevantPct(Task *task)
{
	return task->accPct;
}

const unsigned &StatsModel::getRelevantPctConst(const Task *task) const
{
	return task->accPct;
}

vtl::Time StatsModel::getDeltaTime() const
{
	return AbstractTask::endTime - AbstractTask::startTime;
}

bool StatsModel::checkZeroTime() const
{
	return false;
}

void StatsModel::sortTaskList()
{
	vtl::heapsort<vtl::TList, const Task*>(
		*taskList, [] (const Task *&a, const Task *&b) -> int {
			const QString &as = *a->displayName;
			const QString &bs = *b->displayName;

			if (a->accTime < b->accTime)
				return 1;
			if (a->accTime > b->accTime)
				return -1;

			int cmp1 = as.compare(bs);
			if (cmp1 != 0)
				return cmp1;
			long cmp2 = (long) a->pid - (long) b->pid;
			return (int) cmp2;
		});
}
