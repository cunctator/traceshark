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

#include <QObject>

#include "misc/errors.h"
#include "misc/traceshark.h"
#include "ui/graphenabledialog.h"
#include "vtl/error.h"
#include "setting.h"
#include "translate.h"

#define TRACESHARK_VERSION_KEY "TRACESHARK_FILE_VERSION"

Setting::Value::Value() :
	type_(TYPE_INT)
{
	value.int_value = 0;
}

Setting::Value::Value(bool b) :
	type_(TYPE_BOOL)
{
	value.bool_value = b;
}

Setting::Value::Value(int i) :
	type_(TYPE_INT)
{
	value.int_value = i;
}


Setting::Setting(): flags(FLAG_NO_FLAG), nrDep(0), nrDependents(0)
{
	bzero(dependency, sizeof(dependency));
	bzero(dependent, sizeof(dependent));
}

void Setting::setupSettings()
{
	QObject q;

	Dependency schedDep(Setting::SHOW_SCHED_GRAPHS, true);
	Dependency unlimitedDep(Setting::SHOW_MIGRATION_GRAPHS, true);
	Dependency openglDep(Setting::OPENGL_ENABLED, true);

	setName(Setting::SHOW_SCHED_GRAPHS, q.tr("Show scheduling graphs"));
	setKey(Setting::SHOW_SCHED_GRAPHS, QString("SHOW_SCHED_GRAPHS"));
	initBoolValue(Setting::SHOW_SCHED_GRAPHS, true);

	setName(Setting::HORIZONTAL_WAKEUP, q.tr("Show horizontal wakeup"));
	setKey(Setting::HORIZONTAL_WAKEUP, QString("HORIZONTAL_WAKEUP"));
	initBoolValue(Setting::HORIZONTAL_WAKEUP, false);
	initDisabledBoolValue(Setting::HORIZONTAL_WAKEUP, false);
	addDependency(Setting::HORIZONTAL_WAKEUP, schedDep);

	setName(Setting::VERTICAL_WAKEUP, q.tr("Show vertical wakeup"));
	setKey(Setting::VERTICAL_WAKEUP, QString("VERTICAL_WAKEUP"));
	initBoolValue(Setting::VERTICAL_WAKEUP, true);
	initDisabledBoolValue(Setting::VERTICAL_WAKEUP, false);
	addDependency(Setting::VERTICAL_WAKEUP, schedDep);

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

	bool opengl = has_opengl() && !isLowResScreen();
	int width = opengl ? DEFAULT_LINE_WIDTH_OPENGL : DEFAULT_LINE_WIDTH;

	setName(Setting::OPENGL_ENABLED, q.tr("Enable OpenGL"));
	setKey(Setting::OPENGL_ENABLED, QString("OPENGL_ENABLED"));
	initBoolValue(Setting::OPENGL_ENABLED, opengl);
	setFlag(Setting::OPENGL_ENABLED, FLAG_MUST_BE_CONSUMED);

	setName(Setting::LINE_WIDTH, q.tr("Line width of sched graphs:"));
	setKey(Setting::LINE_WIDTH, QString("SCHED_GRAPH_LINE_WIDTH"));
	initIntValue(Setting::LINE_WIDTH, width);
	initMaxIntValue(Setting::LINE_WIDTH, MAX_LINE_WIDTH_OPENGL);
	initMinIntValue(Setting::LINE_WIDTH, MIN_LINE_WIDTH_OPENGL);
	initDisabledIntValue(Setting::LINE_WIDTH, DEFAULT_LINE_WIDTH);
	addDependency(Setting::LINE_WIDTH, openglDep);
}

bool Setting::isWideScreen()
{
	QRect geometry;

	geometry = QApplication::desktop()->availableGeometry();
	return geometry.width() > 1800;
}

bool Setting::isLowResScreen()
{
	QRect geometry;

	geometry = QApplication::desktop()->availableGeometry();
	/* This is a heuristic */
	return geometry.width() < 1700 && geometry.height() < 1220;
}

void Setting::setName(enum Index idx, const QString &n)
{
	settings[idx].name = n;
}

void Setting::setBoolValue(enum Index idx, bool v)
{
	assert_bool(settings[idx].value);
	settings[idx].value.value.bool_value = v;
}

void Setting::initBoolValue(enum Index idx, bool v)
{
	settings[idx].value.type_ = Value::TYPE_BOOL;
	settings[idx].value.value.bool_value = v;
}

void Setting::setIntValue(enum Index idx, int v)
{
	assert_int(settings[idx].value);
	settings[idx].value.value.int_value = v;
}

void Setting::initIntValue(enum Index idx, int v)
{
	settings[idx].value.type_ = Value::TYPE_INT;
	settings[idx].value.value.int_value = v;
}

void Setting::initMaxIntValue(enum Index idx, int v)
{
	settings[idx].max_value.type_ = Value::TYPE_INT;
	settings[idx].max_value.value.int_value = v;
}

void Setting::initMinIntValue(enum Index idx, int v)
{
	settings[idx].min_value.type_ = Value::TYPE_INT;
	settings[idx].min_value.value.int_value = v;
}

void Setting::initDisabledBoolValue(enum Index idx, bool v)
{
	settings[idx].disabled_value.type_ = Value::TYPE_BOOL;
	settings[idx].disabled_value.value.bool_value = v;
}

void Setting::initDisabledIntValue(enum Index idx, int v)
{
	settings[idx].disabled_value.type_ = Value::TYPE_INT;
	settings[idx].disabled_value.value.int_value = v;
}

const Setting::value_t &Setting::getValue(enum Index idx)
{
	return settings[idx].value;
}

const Setting::value_t &Setting::getDisabledValue(enum Index idx)
{
	return settings[idx].disabled_value;
}

const Setting::value_t &Setting::getMaxValue(enum Index idx)
{
	return settings[idx].max_value;
}

const Setting::value_t &Setting::getMinValue(enum Index idx)
{
	return settings[idx].min_value;
}

void Setting::setFlag(enum Index idx, enum Flag f)
{
	unsigned int flags = settings[idx].flags;
	flags |= (unsigned int) f;
	settings[idx].flags = (enum Flag) flags;
}

void Setting::clearFlag(enum Index idx, enum Flag f)
{
	unsigned int flags = settings[idx].flags;
        flags &= ~ (unsigned int) f;
	settings[idx].flags = (enum Flag) flags;
}

bool Setting::isFlagSet(enum Index idx , enum Flag f)
{
	return (settings[idx].flags & f) != 0;
}

void Setting::addDependency(enum Index idx,
			    const Dependency &d)
{
	Dependency dy;
	unsigned int *nrDependents;
	unsigned int *nrDep;

	nrDep = &settings[idx].nrDep;

	if (*nrDep >=
	    sizeof(settings[idx].dependency) / sizeof(Dependency))
		return;
	settings[idx].dependency[*nrDep] = d;
	(*nrDep)++;

	assert_same(d.desired_value.type_, settings[d.index_].value.type_);

	dy.index_ = idx;
	dy.desired_value = d.desired_value;
	dy.type_ = d.type_;
	dy.low_value = d.low_value;
	dy.high_value = d.high_value;
	nrDependents = &settings[d.index_].nrDependents;
	if (*nrDependents >=
	    sizeof(settings[d.index_].dependent) / sizeof(Dependency))
		return;
	settings[d.index_].dependent[*nrDependents] = dy;
	(*nrDependents)++;
}

unsigned int Setting::getNrDependencies(enum Index idx)
{
	return settings[idx].nrDep;
}

unsigned int Setting::getNrDependents(enum Index idx)
{
	return settings[idx].nrDependents;
}

const QString &Setting::getName(enum Index idx)
{
	return settings[idx].name;
}

const Setting::Dependency &Setting::getDependency(enum Index idx,
						  unsigned int nr)
{
	return settings[idx].dependency[nr];
}

const Setting::Dependency &Setting::getDependent(enum Index idx,
						 unsigned int nr)
{
	return settings[idx].dependent[nr];
}

Setting Setting::settings[NR_SETTINGS];

QMap<QString, enum Setting::Index> Setting::fileKeyMap;

const int Setting::this_version = 1;

void Setting::setKey(enum Index idx, const QString &key)
{
	fileKeyMap[key] = idx;
}

const QString &Setting::getFileName()
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

int Setting::saveSettings()
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
		Index idx = iter.value();
		const QString &key = iter.key();
		const Setting &s = settings[idx];
		stream << key << " ";
		switch (s.value.type()) {
		case Value::TYPE_BOOL:
			stream << boolToQString(s.value.value.bool_value)
			       << "\n";
			break;
		case Value::TYPE_INT:
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

int Setting::loadSettings()
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
		enum Index idx;
		QMap<QString, enum Index>::const_iterator iter;
		iter = fileKeyMap.find(key);
		if (iter == fileKeyMap.end())
			continue;
		idx = iter.value();
		Setting &setting = settings[idx];
		switch (setting.value.type()) {
		case Value::TYPE_BOOL:
			bval = boolFromValue(&ok, value);
			if (ok)
				setting.value.value.bool_value = bval;
			break;
		case Value::TYPE_INT:
			ival = value.toInt(&ok);
			if (ok &&
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

Setting::Dependency::Dependency()
{}

Setting::Dependency::Dependency(Setting::settingindex_t i, bool desired_val) :
	type_(DESIRED_VALUE), index_(i)
{
	desired_value.type_ = Value::TYPE_BOOL;
	desired_value.value.bool_value = desired_val;
}

Setting::Dependency::Dependency(Setting::settingindex_t i, int desired_val) :
	type_(DESIRED_VALUE), index_(i)
{
	desired_value.type_ = Value::TYPE_INT;
	desired_value.value.int_value = desired_val;
}

Setting::Dependency::Dependency(Setting::settingindex_t i, int low, int high) :
	type_(DESIRED_INTERVAL), index_(i)
{
	low_value.type_ = Value::TYPE_INT;
	low_value.value.int_value = low;
	high_value.type_ = Value::TYPE_INT;
	high_value.value.int_value = high;
}

bool Setting::Dependency::getDesiredBool() const
{
	assert_bool(desired_value);
	assert_desired();
	return desired_value.value.bool_value;
}

int Setting::Dependency::getDesiredInt() const
{
	assert_int(desired_value);
	assert_desired();
	return desired_value.value.int_value;
}

bool Setting::Dependency::check(const value_t &val) const
{
	if (type_ == DESIRED_VALUE) {
		return (val == desired_value);
	} else { /* type == DESIRED_INTERVAL */
		return (val <= high_value &&
			val >= low_value);
	}
}

const char *Setting::getValueTypeStr(Value::type_t type)
{
	static const char *boolstr = "bool";
	static const char *intstr = "int";
	static const char *unknownstr = "unknown";

	if (type == Value::TYPE_BOOL)
		return boolstr;
	if (type == Value::TYPE_INT)
		return intstr;
	return unknownstr;
}

void Setting::error_type(Value::type_t expected, Value::type_t was)
{
	const char *expstr = getValueTypeStr(expected);
	const char *wasstr = getValueTypeStr(was);

	vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d. Expected type %s but the type was %s\n", __FILE__, __LINE__, expstr, wasstr);
}

void Setting::Dependency::error_dep_type() const
{
	vtl::errx(BSD_EX_SOFTWARE,  "Error at %s:%d. Wrong dependency type\n");
}
