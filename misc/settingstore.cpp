// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2019-2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include <QObject>

#include "settingstore.h"
#include "misc/errors.h"
#include "misc/traceshark.h"
#include "misc/translate.h"

#define TRACESHARK_VERSION_KEY "TRACESHARK_FILE_VERSION"

const int SettingStore::this_version = 2;

SettingStore::SettingStore()
{
	QObject q;

	Setting::Dependency schedDep(Setting::SHOW_SCHED_GRAPHS, true);
	Setting::Dependency unlimitedDep(Setting::SHOW_MIGRATION_GRAPHS, true);
	Setting::Dependency openglDep(Setting::OPENGL_ENABLED, true);
	Setting::Dependency vertlatDep(Setting::VERTICAL_LATENCY, true);
	Setting::Dependency loadsizeDep(Setting::LOAD_WINDOW_SIZE_START, true);

	setName(Setting::SHOW_SCHED_GRAPHS, q.tr("Show scheduling graphs"));
	setKey(Setting::SHOW_SCHED_GRAPHS, QString("SHOW_SCHED_GRAPHS"));
	initBoolValue(Setting::SHOW_SCHED_GRAPHS, true);

	setName(Setting::HORIZONTAL_LATENCY,
		q.tr("Show horizontal latency graphs"));
	setKey(Setting::HORIZONTAL_LATENCY, QString("HORIZONTAL_LATENCY"));
	initBoolValue(Setting::HORIZONTAL_LATENCY, false);
	initDisabledBoolValue(Setting::HORIZONTAL_LATENCY, false);
	addDependency(Setting::HORIZONTAL_LATENCY, schedDep);

	setName(Setting::VERTICAL_LATENCY,
		q.tr("Show vertical latency graphs"));
	setKey(Setting::VERTICAL_LATENCY, QString("VERTICAL_LATENCY"));
	initBoolValue(Setting::VERTICAL_LATENCY, true);
	initDisabledBoolValue(Setting::VERTICAL_LATENCY, false);
	addDependency(Setting::VERTICAL_LATENCY, schedDep);

	setName(Setting::MAX_VRT_LATENCY,
		q.tr("Latency of a full vertical latency bar"));
	setUnit(Setting::MAX_VRT_LATENCY, q.tr("ms"));
	setKey(Setting::MAX_VRT_LATENCY,
	       QString("MAX_VRT_LATENCY"));
	initIntValue(Setting::MAX_VRT_LATENCY, DEFAULT_MAX_VRT_LATENCY);
	initMaxIntValue(Setting::MAX_VRT_LATENCY, MAX_MAX_VRT_LATENCY);
	initMinIntValue(Setting::MAX_VRT_LATENCY, MIN_MAX_VRT_LATENCY);
	initDisabledIntValue(Setting::MAX_VRT_LATENCY,
			     DEFAULT_MAX_VRT_LATENCY);
	addDependency(Setting::MAX_VRT_LATENCY, vertlatDep);

	setName(Setting::SHOW_CPUFREQ_GRAPHS,
		q.tr("Show CPU frequency graphs"));
	setKey(Setting::SHOW_CPUFREQ_GRAPHS, QString("SHOW_CPUFREQ_GRAPHS"));
	initBoolValue(Setting::SHOW_CPUFREQ_GRAPHS, true);

	setName(Setting::SHOW_CPUIDLE_GRAPHS, q.tr("Show CPU idle graphs"));
	setKey(Setting::SHOW_CPUIDLE_GRAPHS, QString("SHOW_CPUIDLE_GRAPHS"));
	initBoolValue(Setting::SHOW_CPUIDLE_GRAPHS, true);

	QString maxstr = QString::number(MAX_NR_MIGRATIONS / 1000);
	maxstr = maxstr + QString("k");
	setName(Setting::SHOW_MIGRATION_GRAPHS, q.tr("Show migrations if < ")
		+ maxstr);
	setKey(Setting::SHOW_MIGRATION_GRAPHS,
	       QString("SHOW_MIGRATION_GRAPHS"));
	initBoolValue(Setting::SHOW_MIGRATION_GRAPHS, true);

	setName(Setting::SHOW_MIGRATION_UNLIMITED,
		q.tr("Unlimited migrations"));
	setKey(Setting::SHOW_MIGRATION_UNLIMITED,
	       QString("SHOW_MIGRATION_UNLIMITED"));
	initBoolValue(Setting::SHOW_MIGRATION_UNLIMITED, false);
	initDisabledBoolValue(Setting::SHOW_MIGRATION_UNLIMITED, false);
	addDependency(Setting::SHOW_MIGRATION_UNLIMITED, unlimitedDep);

	bool opengl = has_opengl() && !Setting::isLowResScreen();
	int width = opengl ? DEFAULT_LINE_WIDTH_OPENGL : DEFAULT_LINE_WIDTH;

	setName(Setting::OPENGL_ENABLED, q.tr("Enable OpenGL"));
	setKey(Setting::OPENGL_ENABLED, QString("OPENGL_ENABLED"));
	initBoolValue(Setting::OPENGL_ENABLED, opengl);
	setFlag(Setting::OPENGL_ENABLED, Setting::FLAG_MUST_BE_CONSUMED);
	initDisabledBoolValue(Setting::OPENGL_ENABLED, false);
	if (!has_opengl())
		permanentlyDisable(Setting::OPENGL_ENABLED);

	setName(Setting::LINE_WIDTH, q.tr("Width of sched graphs:"));
	setUnit(Setting::LINE_WIDTH, q.tr("pixels"));
	setKey(Setting::LINE_WIDTH, QString("SCHED_GRAPH_LINE_WIDTH"));
	initIntValue(Setting::LINE_WIDTH, width);
	initMaxIntValue(Setting::LINE_WIDTH, MAX_LINE_WIDTH_OPENGL);
	initMinIntValue(Setting::LINE_WIDTH, MIN_LINE_WIDTH_OPENGL);
	initDisabledIntValue(Setting::LINE_WIDTH, DEFAULT_LINE_WIDTH);
	addDependency(Setting::LINE_WIDTH, openglDep);
	if (!has_opengl())
		permanentlyDisable(Setting::LINE_WIDTH);

	setName(Setting::IDLE_LINE_WIDTH, q.tr("Width of idle graphs"));
	setUnit(Setting::IDLE_LINE_WIDTH, q.tr("pixels"));
	setKey(Setting::IDLE_LINE_WIDTH, QString("IDLE_GRAPH_LINE_WIDTH"));
	initIntValue(Setting::IDLE_LINE_WIDTH, DEFAULT_IDLE_LINE_WIDTH);
	initMaxIntValue(Setting::IDLE_LINE_WIDTH, MAX_IDLE_LINE_WIDTH);
	initMinIntValue(Setting::IDLE_LINE_WIDTH, MIN_IDLE_LINE_WIDTH);
	initDisabledIntValue(Setting::IDLE_LINE_WIDTH, DEFAULT_IDLE_LINE_WIDTH);
	addDependency(Setting::IDLE_LINE_WIDTH, openglDep);

	setName(Setting::FREQ_LINE_WIDTH, q.tr("Width of frequency graphs"));
	setUnit(Setting::FREQ_LINE_WIDTH, q.tr("pixels"));
	setKey(Setting::FREQ_LINE_WIDTH, QString("FREQ_GRAPH_LINE_WIDTH"));
	initIntValue(Setting::FREQ_LINE_WIDTH, DEFAULT_FREQ_LINE_WIDTH);
	initMaxIntValue(Setting::FREQ_LINE_WIDTH, MAX_FREQ_LINE_WIDTH);
	initMinIntValue(Setting::FREQ_LINE_WIDTH, MIN_FREQ_LINE_WIDTH);
	initDisabledIntValue(Setting::FREQ_LINE_WIDTH, DEFAULT_FREQ_LINE_WIDTH);

	setName(Setting::MIGRATION_WIDTH, q.tr("Width of migration arrows"));
	setUnit(Setting::MIGRATION_WIDTH, q.tr("pixels"));
	setKey(Setting::MIGRATION_WIDTH, QString("MIGRATION_LINE_WIDTH"));
	initIntValue(Setting::MIGRATION_WIDTH, DEFAULT_MIGRATION_WIDTH);
	initMaxIntValue(Setting::MIGRATION_WIDTH, MAX_MIGRATION_WIDTH);
	initMinIntValue(Setting::MIGRATION_WIDTH, MIN_MIGRATION_WIDTH);
	initDisabledIntValue(Setting::MIGRATION_WIDTH, DEFAULT_MIGRATION_WIDTH);
	addDependency(Setting::MIGRATION_WIDTH, openglDep);

	setName(Setting::LOAD_WINDOW_SIZE_START,
		q.tr("Restore the size of the main window at startup"));
	setKey(Setting::LOAD_WINDOW_SIZE_START,
	       QString("LOAD_WINDOW_SIZE_START"));
	initBoolValue(Setting::LOAD_WINDOW_SIZE_START, false);

	setName(Setting::MAINWINDOW_WIDTH, q.tr("Width of the main window"));
	setUnit(Setting::MAINWINDOW_WIDTH, q.tr("pixels"));
	setKey(Setting::MAINWINDOW_WIDTH, QString("MAINWINDOW_WIDTH"));
	initIntValue(Setting::MAINWINDOW_WIDTH, DEFAULT_MAINWINDOW_WIDTH);
	initMaxIntValue(Setting::MAINWINDOW_WIDTH, MAX_MAINWINDOW_WIDTH);
	initMinIntValue(Setting::MAINWINDOW_WIDTH, MIN_MAINWINDOW_WIDTH);
	initDisabledIntValue(Setting::MAINWINDOW_WIDTH,
			     DEFAULT_MAINWINDOW_WIDTH);
	addDependency(Setting::MAINWINDOW_WIDTH, loadsizeDep);

	setName(Setting::MAINWINDOW_HEIGHT, q.tr("Height of the main window"));
	setUnit(Setting::MAINWINDOW_HEIGHT, q.tr("pixels"));
	setKey(Setting::MAINWINDOW_HEIGHT, QString("MAINWINDOW_HEIGHT"));
	initIntValue(Setting::MAINWINDOW_HEIGHT, DEFAULT_MAINWINDOW_HEIGHT);
	initMaxIntValue(Setting::MAINWINDOW_HEIGHT, MAX_MAINWINDOW_HEIGHT);
	initMinIntValue(Setting::MAINWINDOW_HEIGHT, MIN_MAINWINDOW_HEIGHT);
	initDisabledIntValue(Setting::MAINWINDOW_HEIGHT,
			     DEFAULT_MAINWINDOW_HEIGHT);
	addDependency(Setting::MAINWINDOW_HEIGHT, loadsizeDep);

	setName(Setting::SAVE_WINDOW_SIZE_EXIT,
		q.tr("Save the size of the main window at exit"));
	setKey(Setting::SAVE_WINDOW_SIZE_EXIT,
	       QString("SAVE_WINDOW_SIZE_EXIT"));
	initBoolValue(Setting::SAVE_WINDOW_SIZE_EXIT, false);

	setName(Setting::EVENT_PID_FLT_INCL_ON,
		q.tr("Event pid filtering: include relevant wakeup, fork and scheduling events from other PIDs"));
	setKey(Setting::EVENT_PID_FLT_INCL_ON,
	       QString("EVENT_PID_FLT_INCL_ON"));
	initBoolValue(Setting::EVENT_PID_FLT_INCL_ON, false);

	/*
	 * These are legacy settings that are needed for file compatibility in
	 * settingstore.cpp
	 */
	setKey(Setting::HORIZONTAL_WAKEUP, QString("HORIZONTAL_WAKEUP"));
	initBoolValue(Setting::HORIZONTAL_WAKEUP, false);

	setKey(Setting::VERTICAL_WAKEUP, QString("VERTICAL_WAKEUP"));
	initBoolValue(Setting::VERTICAL_WAKEUP, true);

	setKey(Setting::MAX_VRT_WAKEUP_LATENCY,
	       QString("MAX_VRT_WAKEUP_LATENCY"));
	initIntValue(Setting::MAX_VRT_WAKEUP_LATENCY, DEFAULT_MAX_VRT_LATENCY);
	initMaxIntValue(Setting::MAX_VRT_WAKEUP_LATENCY, MAX_MAX_VRT_LATENCY);
	initMinIntValue(Setting::MAX_VRT_WAKEUP_LATENCY, MIN_MAX_VRT_LATENCY);

	/*
	 * The values that we have initialized above with initIntValue() and
	 * initBoolValue() are not expected to break dependencies but let's
	 * check anyway.
	 */
	checkAllDependents();
}

void SettingStore::setName(enum Setting::Index idx, const QString &n)
{
	settings[idx].name = n;
}

void SettingStore::setUnit(enum Setting::Index idx, const QString &u)
{
	settings[idx].unit = u;
}

void SettingStore::setBoolValue(enum Setting::Index idx, bool v)
{
	Setting::assert_bool(settings[idx].value);
	if (settings[idx].supported)
		settings[idx].value.value.bool_value = v;
}

void SettingStore::initBoolValue(enum Setting::Index idx, bool v)
{
	settings[idx].value.type_ = Setting::Value::TYPE_BOOL;
	if (settings[idx].supported)
		settings[idx].value.value.bool_value = v;
}

void SettingStore::setIntValue(enum Setting::Index idx, int v)
{
	Setting::assert_int(settings[idx].value);
	if (settings[idx].supported)
		settings[idx].value.value.int_value = v;
}

void SettingStore::setValue(enum Setting::Index idx, const Setting::Value &v)
{
	Setting::assert_same(settings[idx].value.type_, v.type_);
	if (settings[idx].supported)
		settings[idx].value = v;
}

void SettingStore::initIntValue(enum Setting::Index idx, int v)
{
	settings[idx].value.type_ = Setting::Value::TYPE_INT;
	if (settings[idx].supported)
		settings[idx].value.value.int_value = v;
}

void SettingStore::initMaxIntValue(enum Setting::Index idx, int v)
{
	settings[idx].max_value.type_ = Setting::Value::TYPE_INT;
	settings[idx].max_value.value.int_value = v;
}

void SettingStore::initMinIntValue(enum Setting::Index idx, int v)
{
	settings[idx].min_value.type_ = Setting::Value::TYPE_INT;
	settings[idx].min_value.value.int_value = v;
}

void SettingStore::permanentlyDisable(enum Setting::Index idx)
{
	settings[idx].supported = false;
	settings[idx].value = settings[idx].disabled_value;
}

void SettingStore::initDisabledBoolValue(enum Setting::Index idx, bool v)
{
	settings[idx].disabled_value.type_ = Setting::Value::TYPE_BOOL;
	settings[idx].disabled_value.value.bool_value = v;
	if (!settings[idx].supported) {
		settings[idx].value.type_ = Setting::Value::TYPE_BOOL;
		settings[idx].value.value.bool_value = v;
	}
}

void SettingStore::initDisabledIntValue(enum Setting::Index idx, int v)
{
	settings[idx].disabled_value.type_ = Setting::Value::TYPE_INT;
	settings[idx].disabled_value.value.int_value = v;
	if (!settings[idx].supported) {
		settings[idx].value.type_ = Setting::Value::TYPE_INT;
		settings[idx].value.value.int_value = v;
	}
}

const Setting::Value &SettingStore::getValue(enum Setting::Index idx) const
{
	return settings[idx].value;
}

const Setting::Value &SettingStore::getDisabledValue(enum Setting::Index idx)
	const
{
	return settings[idx].disabled_value;
}

const Setting::Value &SettingStore::getMaxValue(enum Setting::Index idx) const
{
	return settings[idx].max_value;
}

const Setting::Value &SettingStore::getMinValue(enum Setting::Index idx) const
{
	return settings[idx].min_value;
}

void SettingStore::setFlag(enum Setting::Index idx, enum Setting::Flag f)
{
	unsigned int flags = settings[idx].flags;
	flags |= (unsigned int) f;
	settings[idx].flags = (enum Setting::Flag) flags;
}

void SettingStore::clearFlag(enum Setting::Index idx, enum Setting::Flag f)
{
	unsigned int flags = settings[idx].flags;
        flags &= ~ (unsigned int) f;
	settings[idx].flags = (enum Setting::Flag) flags;
}

bool SettingStore::isFlagSet(enum Setting::Index idx , enum Setting::Flag f)
	const
{
	return (settings[idx].flags & f) != 0;
}

void SettingStore::addDependency(enum Setting::Index idx,
				 const Setting::Dependency &d)
{
	Setting::Dependency dy;
	unsigned int *nrDependents;
	unsigned int *nrDep;

	nrDep = &settings[idx].nrDep;

	if (*nrDep >= arraylen(settings[idx].dependency))
		return;
	settings[idx].dependency[*nrDep] = d;
	(*nrDep)++;

	Setting::assert_same(d.desired_value.type_,
			     settings[d.index_].value.type_);

	dy.index_ = idx;
	dy.desired_value = d.desired_value;
	dy.type_ = d.type_;
	dy.low_value = d.low_value;
	dy.high_value = d.high_value;
	nrDependents = &settings[d.index_].nrDependents;
	if (*nrDependents >= arraylen(settings[d.index_].dependent))
		return;
	settings[d.index_].dependent[*nrDependents] = dy;
	(*nrDependents)++;
}

void SettingStore::updateDependents(enum Setting::Index idx)
{
	unsigned i;
	const Setting &setting = settings[idx];

	for (i = 0; i < setting.nrDependents; i++) {
		const Setting::Dependency &d = setting.dependent[i];
		if (d.desired_value != setting.value) {
			settings[d.index_].value =
				settings[d.index_].disabled_value;
		}
	}
}

void SettingStore::checkAllDependents()
{
	int i;
	enum Setting::Index idx;

	for (i = 0; i < Setting::NR_SETTINGS; i++) {
		idx = (enum Setting::Index) i;
		updateDependents(idx);
	}
}

unsigned int SettingStore::getNrDependencies(enum Setting::Index idx) const
{
	return settings[idx].nrDep;
}

unsigned int SettingStore::getNrDependents(enum Setting::Index idx) const
{
	return settings[idx].nrDependents;
}

const QString &SettingStore::getName(enum Setting::Index idx) const
{
	return settings[idx].name;
}

const QString &SettingStore::getUnit(enum Setting::Index idx) const
{
	return settings[idx].unit;
}

const Setting::Dependency &SettingStore::getDependency(enum Setting::Index idx,
						       unsigned int nr) const
{
	return settings[idx].dependency[nr];
}

const Setting::Dependency &SettingStore::getDependent(enum Setting::Index idx,
						      unsigned int nr) const
{
	return settings[idx].dependent[nr];
}

void SettingStore::setKey(enum Setting::Index idx, const QString &key)
{
	fileKeyMap[key] = idx;
}

const QString &SettingStore::getFileName()
{
	static bool need_init = true;
	static QString name;
	if (need_init) {
		char *homedir = getenv("HOME");
		name = QString(homedir);
		name += QString("/");
		name += QString(TS_SETTING_FILENAME);
		need_init = false;
	}
	return name;
}

int SettingStore::saveSettings() const
{
	QString name = getFileName();
	QFile file(name);
	QMap<QString, enum Setting::Index>::const_iterator iter;

	if (!file.open(QIODevice::Truncate | QIODevice::WriteOnly)) {
		qfile_error_t error = file.error();
		if (error != qfile_error_class::NoError)
			return -translate_FileError(error);
		return -TS_ERROR_UNSPEC;
	}
	QTextStream stream(&file);
        stream << QString(TRACESHARK_VERSION_KEY) << " ";
	stream << QString::number(this_version) << "\n";

	for (iter = fileKeyMap.begin(); iter != fileKeyMap.end(); iter++) {
		Setting::Index idx = iter.value();
		const QString &key = iter.key();
		const Setting &s = settings[idx];
		stream << key << " ";
		switch (s.value.type()) {
		case Setting::Value::TYPE_BOOL:
			stream << boolToQString(s.value.value.bool_value)
			       << "\n";
			break;
		case Setting::Value::TYPE_INT:
			stream << QString::number(s.value.value.int_value)
			       << "\n";
			break;
		default:
			stream << QString("ERROR_YOU_SHOULD_NEVER_SEE_THIS\n");
			break;
		};
	}
	stream.flush();
	qfile_error_t err = file.error();
	if (err != qfile_error_class::NoError)
		return -translate_FileError(err);
	return 0;
}

int SettingStore::loadSettings()
{
	QString key;
	QString value;
	bool ok;
	int version;
	QString name = getFileName();
	QFile file(name);
	int rval = 0;
	int ival;
	bool bval;

	if (!file.exists())
		return 0;
	if (!file.open(QIODevice::ReadOnly)) {
		qfile_error_t error = file.error();
		if (error != qfile_error_class::NoError)
			return -translate_FileError(error);
		return -TS_ERROR_UNSPEC;
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
		enum Setting::Index idx;
		QMap<QString, enum Setting::Index>::const_iterator iter;
		iter = fileKeyMap.find(key);
		if (iter == fileKeyMap.end())
			continue;
		idx = iter.value();
		Setting &setting = settings[idx];
		switch (setting.value.type()) {
		case Setting::Value::TYPE_BOOL:
			bval = boolFromValue(&ok, value);
			if (ok && setting.supported)
				setting.value.value.bool_value = bval;
			break;
		case Setting::Value::TYPE_INT:
			ival = value.toInt(&ok);
			if (ok && setting.supported &&
			    ival >= setting.min_value.value.int_value &&
			    ival <= setting.max_value.value.int_value)
				setting.value.value.int_value = ival;
			break;
		default:
			break;
		}
	}
	if (version < this_version)
		rval = handleOlderVersion(version, this_version);
	/*
	 * Let's check that we have not loaded values that break dependencies.
	 * This could happend if the user had manually edited the .traceshark
	 * file.
	 */
	checkAllDependents();
	return rval;
}

int SettingStore::readKeyValuePair(QTextStream &stream,
				   QString &key,
				   QString &value)
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

int SettingStore::handleOlderVersion(int oldver, int /*newver*/)
{
	/* This should not happen because currently this_version is 2 */
	if (oldver >= 2)
		return -TS_ERROR_INTERNAL;

	if (oldver <= 1) {
		/*
		 * These keys have changed name, so we transfer the value of the
		 * old setting into the new key.
		 */
		setValue(Setting::HORIZONTAL_LATENCY,
			 getValue(Setting::HORIZONTAL_WAKEUP));
		setValue(Setting::VERTICAL_LATENCY,
			 getValue(Setting::VERTICAL_WAKEUP));
		setValue(Setting::MAX_VRT_LATENCY,
			 getValue(Setting::MAX_VRT_WAKEUP_LATENCY));
	}
	return 0;
}

const QString &SettingStore::boolToQString(bool b)
{
	static const QString true_str("true");
	static const QString false_str("false");
	if (b)
		return true_str;
	else
		return false_str;
}


bool SettingStore::boolFromValue(bool *ok, const QString &value)
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
