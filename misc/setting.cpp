// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <stdlib.h>

#include "misc/errors.h"
#include "misc/traceshark.h"
#include "ui/graphenabledialog.h"
#include "vtl/error.h"
#include "setting.h"
#include "translate.h"

#define TRACESHARK_VERSION_KEY "TRACESHARK_FILE_VERSION"

Setting::Setting(): enabled(true), nrDep(0), nrDependents(0)
{
	bzero(dependency, sizeof(dependency));
	bzero(dependent, sizeof(dependent));
}

void Setting::setName(enum SettingIndex idx, const QString &n)
{
	settings[idx].name = n;
}

void Setting::setEnabled(enum SettingIndex idx, bool e)
{
	settings[idx].enabled = e;
}

void Setting::clearDependencies(enum SettingIndex idx)
{
	settings[idx].nrDep = 0;
}

void Setting::addDependency(enum SettingIndex idx,
			    const SettingDependency &d)
{
	SettingDependency dy;
	unsigned int *nrDependents;
	unsigned int *nrDep;

	nrDep = &settings[idx].nrDep;

	if (*nrDep >=
	    sizeof(settings[idx].dependency) / sizeof(SettingDependency))
		return;
	settings[idx].dependency[*nrDep] = d;
	(*nrDep)++;

	dy.index = idx;
	dy.desiredValue = d.desiredValue;
	nrDependents = &settings[d.index].nrDependents;
	if (*nrDependents >=
	    sizeof(settings[d.index].dependent) / sizeof(SettingDependency))
		return;
	settings[d.index].dependent[*nrDependents] = dy;
	(*nrDependents)++;
}

unsigned int Setting::getNrDependencies(enum SettingIndex idx)
{
	return settings[idx].nrDep;
}

unsigned int Setting::getNrDependents(enum SettingIndex idx)
{
	return settings[idx].nrDependents;
}

const QString &Setting::getName(enum SettingIndex idx)
{
	return settings[idx].name;
}

bool Setting::isEnabled(enum SettingIndex idx)
{
	return settings[idx].enabled;
}

const SettingDependency &Setting::getDependency(enum SettingIndex idx,
						unsigned int nr)
{
	return settings[idx].dependency[nr];
}

const SettingDependency &Setting::getDependent(enum SettingIndex idx,
					       unsigned int nr)
{
	return settings[idx].dependent[nr];
}

Setting Setting::settings[NR_SETTINGS];

int Setting::line_width = 0;

bool Setting::opengl = false;

QMap<QString, enum Setting::SettingIndex> Setting::fileKeyMap;

const int Setting::this_version = 1;

void Setting::setLineWidth(int width)
{
	line_width = width;
}

int Setting::getLineWidth()
{
	return line_width;
}

bool Setting::isOpenGLEnabled()
{
	return opengl;
}

void Setting::setOpenGLEnabled(bool e)
{
	opengl = e;
}

void Setting::setKey(enum SettingIndex idx, const QString &key)
{
	fileKeyMap[key] = idx;
}

QString Setting::getFileName()
{
	char *homedir = getenv("HOME");
	QString name = QString(homedir);
	name += QString("/");
	name += QString(TS_SETTING_FILENAME);
	return name;
}

int Setting::saveSettings()
{
	QString name = getFileName();
	QFile file(name);
	QMap<QString, enum Setting::SettingIndex>::const_iterator iter;

	if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
		QFileDevice::FileError error = file.error();
		return -translate_FileError(error);
	}
	QTextStream stream(&file);
        stream << QString(TRACESHARK_VERSION_KEY) << " ";
	stream << QString::number(this_version) << "\n";

	for (iter = fileKeyMap.cbegin(); iter != fileKeyMap.cend(); iter++) {
		SettingIndex idx = iter.value();
		const QString &key = iter.key();
		if (idx >= 0 && idx < NR_SETTINGS) {
			const Setting &s = settings[idx];
			stream << key << " ";
			stream << boolToQString(s.enabled) << "\n";
		} else if (idx == OPENGL_ENABLED) {
			stream << key << " ";
			stream << boolToQString(opengl) << "\n";
		} else if (idx == LINE_WIDTH) {
			stream << key << " ";
			stream << QString::number(line_width) << "\n";
		}
	}
	stream.flush();
	QFileDevice::FileError err = file.error();
	if (err != QFileDevice::NoError)
		return -translate_FileError(err);
	return 0;
}

int Setting::loadSettings()
{
	QString key;
	QString value;
	bool ok;
	int version;
	QString name = getFileName();
	QFile file(name);
	int rval = 0;

	if (!file.exists())
		return 0;
	if (!file.open(QIODevice::ReadOnly)) {
		QFileDevice::FileError error = file.error();
		return -translate_FileError(error);
	}
	QTextStream stream(&file);
	if (!stream.atEnd()) {
		rval = readKeyValuePair(stream, key, value);
		if (rval != 0)
			return rval;
		if (key != QString(TRACESHARK_VERSION_KEY))
			return -TS_ERROR_FILEFORMAT;
		version = value.toInt(&ok);
		if (!ok)
			return -TS_ERROR_FILEFORMAT;
		if (version > this_version)
			return -TS_ERROR_NEWFORMAT;
	} else {
		return -TS_ERROR_EOF;
	}
	while (!stream.atEnd()) {
		rval = readKeyValuePair(stream, key, value);
		if (rval != 0)
			return rval;
		enum SettingIndex idx;
		QMap<QString, enum SettingIndex>::const_iterator iter;
		iter = fileKeyMap.find(key);
		if (iter == fileKeyMap.cend())
		    continue;
		idx = iter.value();
		if (isIrregularIndex(idx)) {
			handleIrregularIndex(idx, value);
		} else if (isRegularIndex(idx)) {
			handleRegularIndex(idx, value);
		} else
			return -TS_ERROR_INTERNAL;
	}
	if (version < this_version)
		rval = handleOlderVersion(version, this_version);
	return rval;
}

int Setting::readKeyValuePair(QTextStream &stream, QString &key, QString &value)
{
	QString line;
	QStringList lineList;

	line = stream.readLine();
	do {
		lineList = line.split(' ', QString::SkipEmptyParts);
	} while(lineList.size() == 0 && !stream.atEnd());
	if (lineList.size() != 2)
		return -TS_ERROR_FILEFORMAT;
	key = lineList[0];
	value = lineList[1];
	return 0;
}

void Setting::setOpenGLEnabledKey(const QString &key)
{
	setKey(OPENGL_ENABLED, key);
}

void Setting::setLineWidthKey(const QString &key)
{
	setKey(LINE_WIDTH, key);
}

bool Setting::isIrregularIndex(enum SettingIndex idx)
{
	return idx > NR_SETTINGS && idx < END_SETTINGS;
}

bool Setting::isRegularIndex(enum SettingIndex idx)
{
	return idx >= 0 && idx < NR_SETTINGS;
}

bool Setting::boolFromValue(bool *ok, const QString &value)
{
	if (value == QString("true") || value == QString("TRUE")) {
		*ok = true;
		return true;
	}
	if (value == QString("false") || value == QString("FALSE")) {
		*ok = true;
		return false;
	}
	*ok = false;
	return false;
}

void Setting::handleRegularIndex(enum SettingIndex idx,
				 const QString &value)
{
	bool ok;
	bool enabled = boolFromValue(&ok, value);
	if (ok)
		settings[idx].enabled = enabled;
}

void Setting::handleIrregularIndex(enum SettingIndex idx,
				   const QString &value)
{
	bool enabled, ok;
	int width;
	switch(idx) {
	case OPENGL_ENABLED:
		enabled = boolFromValue(&ok, value);
		if (ok)
			opengl = enabled;
		break;
	case LINE_WIDTH:
		width = value.toInt(&ok);
		if (ok && width >= 1 && width <= MAX_LINE_WIDTH_OPENGL)
			line_width = width;
		break;
	default:
		break;
	}
}

int Setting::handleOlderVersion(int /*oldver*/, int /*newver*/)
{
	/*
	 * At present we are at version 1. There has never been an older
	 * version.
	 */
	return -TS_ERROR_INTERNAL;
}

const QString &Setting::boolToQString(bool b)
{
	static const QString true_str("true");
	static const QString false_str("false");
	if (b)
		return true_str;
	else
		return false_str;
}
