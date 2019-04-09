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

#ifndef _TS_SETTINGSTORE_H
#define _TS_SETTINGSTORE_H

#include <QString>
#include "setting.h"

#define TS_SETTING_FILENAME ".traceshark"

class SettingStore
{
public:
	SettingStore();
	void setBoolValue(enum Setting::Index idx, bool v);
	void setIntValue(enum Setting::Index idx, int v);
	void setValue(enum Setting::Index idx, const Setting::Value &v);
	const Setting::Value &getValue(enum Setting::Index idx) const;
	const Setting::Value &getDisabledValue(enum Setting::Index idx) const;
	const Setting::Value &getMinValue(enum Setting::Index idx) const;
	const Setting::Value &getMaxValue(enum Setting::Index idx) const;
	bool isFlagSet(enum Setting::Index idx , enum Setting::Flag f) const;
	__always_inline bool isSupported(enum Setting::Index idx) const;
	unsigned int getNrDependencies(enum Setting::Index idx) const;
	unsigned int getNrDependents(enum Setting::Index idx) const;
	const QString &getName(enum Setting::Index idx) const;
	const QString &getUnit(enum Setting::Index idx) const;
	const Setting::Dependency &getDependency(enum Setting::Index idx,
						 unsigned int nr) const;
	const Setting::Dependency &getDependent(enum Setting::Index idx,
						unsigned int nr) const;
	int loadSettings();
	int saveSettings() const;
	static const QString &getFileName();
private:
	void setFlag(enum Setting::Index idx, enum Setting::Flag f);
	void clearFlag(enum Setting::Index idx, enum Setting::Flag f);
	void initBoolValue(enum Setting::Index idx, bool v);
	void initIntValue(enum Setting::Index idx, int v);
	void initDisabledBoolValue(enum Setting::Index idx, bool v);
	void initDisabledIntValue(enum Setting::Index idx, int v);
	void initMaxIntValue(enum Setting::Index idx, int v);
	void initMinIntValue(enum Setting::Index idx, int v);
	void permanentlyDisable(enum Setting::Index idx);
	void setName(enum Setting::Index idx, const QString &n);
	void setUnit(enum Setting::Index idx, const QString &u);
	void setKey(enum Setting::Index idx, const QString &key);
	void addDependency(enum Setting::Index idx,
			   const Setting::Dependency &d);
	int readKeyValuePair(QTextStream &stream,
			     QString &key,
			     QString &value);
	int handleOlderVersion(int oldver, int newver);
	static const QString &boolToQString(bool b);
	static bool boolFromValue(bool *ok, const QString &value);
	Setting settings[Setting::NR_SETTINGS];
	QMap<QString, enum Setting::Index> fileKeyMap;
	static const int this_version;
};

__always_inline bool SettingStore::isSupported(enum Setting::Index idx) const
{
	return settings[idx].supported;
}

#endif /* _TS_SETTINGSTORE_H */
