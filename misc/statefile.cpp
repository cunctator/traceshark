// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QColor>
#include <QFile>
#include <QTextStream>

#include "misc/translate.h"
#include "misc/errors.h"
#include "misc/statefile.h"
#include "misc/traceshark.h"

StateFile::StateFile()
{}

StateFile::~StateFile()
{}

const int StateFile::this_version = 1;

void StateFile::setTraceFile(const QString &name)
{
	/*
	 * Only allow the trace file name to be set once, unless we call
	 * clear() first.
	 */
	if (traceFile.isEmpty())
		traceFile = name;
}

void StateFile::setTaskColor(int pid, const QColor &color)
{
	colorMap[pid] = color;
}

bool StateFile::getTaskColor(int pid, QColor *color) const
{
	QMap<int, QColor>::const_iterator iter = colorMap.find(pid);

	if (iter == colorMap.cend())
		return false;

	*color = iter.value();
	return true;
}

void StateFile::checkStateFile()
{
	QChar qdot('.');
	int n;
	QString basename;

	if (!stateFile.isEmpty() || traceFile.isEmpty())
		return;

	n = traceFile.lastIndexOf(QString("."));

	if (n < 0) {
		basename = traceFile;
	} else if (n > 0) {
		basename = traceFile.left(n);
	}

	stateFile = basename + QString(".tssetting");
}


int StateFile::saveState()
{
	bool flush_err;

	if (colorMap.isEmpty())
		return 0;

	checkStateFile();

	if (stateFile.isEmpty())
		return -TS_ERROR_UNSPEC;;

	QFile file(stateFile);

	if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
		qfile_error_t error = file.error();
		if (error != qfile_error_class::NoError)
			return -translate_FileError(error);
		return -TS_ERROR_UNSPEC;
	}

	QTextStream stream(&file);

	stream << STATE_VERSION_KEY << " ";
	stream << QString::number(this_version) << "\n";
	stream << SECTION_BEGIN << " ";
	stream << SECTION_COLORS << "\n";

	QMap<int, QColor>::const_iterator iter;

	for (iter = colorMap.cbegin(); iter != colorMap.cend(); iter++) {
		int pid = iter.key();
		const QColor &color = iter.value();
		const uint32_t coluint = ((uint32_t) color.alpha()) |
			(((uint32_t) color.blue())  * 0x100) |
			(((uint32_t) color.green()) * 0x10000) |
			(((uint32_t) color.red())   * 0x1000000);

		stream << QString::number(pid)           << " "
		       << QString::number(coluint, 16) << "\n";
	}

	stream << SECTION_END << " ";
	stream << SECTION_COLORS << "\n";

	stream.flush();
	flush_err = !file.flush();
	qfile_error_t err = file.error();
	if (flush_err || err != qfile_error_class::NoError) {
		file.close();
		if (err ==  qfile_error_class::NoError)
			return -TS_ERROR_UNSPEC;
		return -translate_FileError(err);
	}
	file.close();
	return 0;
}

int StateFile::loadState()
{
	QString key;
	QString value;
	bool ok;
	int version;
	int rval = 0;

	checkStateFile();

	if (stateFile.isEmpty())
		return -TS_ERROR_UNSPEC;

	if (!colorMap.isEmpty())
		return -TS_ERROR_UNSPEC;

	QFile file(stateFile);

	/* It's completely OK if we have no state file for a trace */
	if (!file.exists())
		return 0;

	if (!file.open(QIODevice::ReadOnly)) {
		qfile_error_t error = file.error();
		if (error != qfile_error_class::NoError)
			return -translate_FileError(error);
		return -TS_ERROR_UNSPEC;
	}

	QTextStream stream(&file);

	if (stream.atEnd())
		return -TS_ERROR_EOF;


	rval = TShark::readKeyValuePair(stream, key, value);
	if (rval != 0)
		return rval;

	if (key != QString(STATE_VERSION_KEY))
		return -TS_ERROR_FILEFORMAT;

	version = value.toInt(&ok);
	if (!ok)
		return -TS_ERROR_FILEFORMAT;

	if (version > this_version)
		return -TS_ERROR_NEWFORMAT;

	if (stream.atEnd())
		return -TS_ERROR_EOF;

	rval = TShark::readKeyValuePair(stream, key, value);
	if (rval != 0)
		return rval;

	if (key == SECTION_BEGIN) {
		if (value == SECTION_COLORS) {
			rval = loadColorSection(stream);
			if (rval != 0)
				return rval;
		}
	}
	return 0;
}


int StateFile::loadColorSection(QTextStream &stream)
{
	QString key;
	QString value;
	QMap<int, QColor> map;
	int rval;
	bool ok;

	while (!stream.atEnd()) {
		rval = TShark::readKeyValuePair(stream, key, value);
		if (rval != 0)
			return rval;

		if (key == SECTION_END &&
		    value == SECTION_COLORS)
			break;

		int pid = key.toInt(&ok);
		if (!ok)
			return -TS_ERROR_FILEFORMAT;

		qulonglong coluint = value.toULongLong(&ok, 16);

		if (!ok)
			return -TS_ERROR_FILEFORMAT;

		unsigned red =   (coluint / 0x1000000) & 0xff;
		unsigned green = (coluint / 0x10000)   & 0xff;
		unsigned blue =  (coluint / 0x100)     & 0xff;
		unsigned alpha =  coluint              & 0xff;

		QColor color(red, green, blue, alpha);
		map[pid] = color;
	}

	colorMap = map;
	return 0;
}

void StateFile::clear()
{
	colorMap.clear();
	traceFile.clear();
	stateFile.clear();
}

const QString StateFile::SECTION_BEGIN("BEGIN_SECTION");

const QString StateFile::SECTION_END("END_SECTION");

const QString StateFile::SECTION_COLORS("COLORS");

const QString StateFile::STATE_VERSION_KEY("TRACESHARK_STATE_FILE_VERSION");
