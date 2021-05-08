// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2019, 2021  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef GRAPHENABLEDIALOG_H
#define GRAPHENABLEDIALOG_H

#include "misc/setting.h"
#include "vtl/error.h"
#include <QDialog>

QT_BEGIN_NAMESPACE
class QComboBox;
class QMainWindow;
class QPushButton;
class QTextEdit;
template <typename T, typename U> class QMap;
QT_END_NAMESPACE

class ValueBox;

class GraphEnableDialog : public QDialog {
	Q_OBJECT

public:
	GraphEnableDialog(SettingStore *sstore, QWidget *parent = 0);
	~GraphEnableDialog();
	void checkConsumption();
	void setMainWindowSize(int wt, int ht);
signals:
	void settingsChanged();
	void filterSettingsChanged();
	void sizeChanged();
	void sizeRequest();
private:
	QMap<Setting::Index, ValueBox*> *valueBoxMap;
	QList<Setting::Index> consumeList;
	int savedHeight;
	SettingStore *settingStore;
	QPushButton *getSizeButton;
	void valueBoxConsumption(Setting::Index idx, ValueBox *box);
	void handleDependencies(Setting::Index idx,
				const Setting::Value &value,
				bool need_consume, bool store);
	void handleSpecialCases(Setting::Index idx,
				const Setting::Value &value);
	void refreshUIValues();
	void setUIValue(Setting::Index id, const Setting::Value &value);
	Setting::Value getUIValue(Setting::Index id, bool &ok);
public slots:
	void show();
private slots:
	void okClicked();
	void cancelClicked();
	void applyClicked();
	void saveClicked();
	void handleBoxChanged(ValueBox *valueBox, Setting::Value value);
};

#endif /* GRAPHENABLEDIALOG_H */
