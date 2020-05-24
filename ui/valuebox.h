 // SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2019, 2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef _TS_UI_VALUEBOX_H
#define _TS_UI_VALUEBOX_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QComboBox;
class QLayout;
class QSpinBox;
class QLabel;
QT_END_NAMESPACE

#include "misc/setting.h"
#include "vtl/compiler.h"

class SettingStore;

class ValueBox : public QWidget {
	Q_OBJECT
public:
	ValueBox(Setting::Index id_arg = Setting::SHOW_SCHED_GRAPHS,
		 SettingStore *sstore = nullptr,
		 QWidget *parent = 0);
	virtual ~ValueBox();
	vtl_always_inline int getId() const;
	void setDefaultValue();
	Setting::Value value() const;
	void setValue(const Setting::Value &v);
	void setEnabled(bool en);
	void setDisabled(bool dis);
signals:
	void boxChanged(ValueBox *, Setting::Value value);
private:

	typedef enum Type : int {
		TYPE_ERROR = 0,
		TYPE_CHECKBOX,
		TYPE_COMBOBOX,
		TYPE_SPINBOX,
	} type_t;

	void createIntBox(QLayout *layout);
	void createBoolBox(QLayout *layout);
	SettingStore *settingStore;
	Setting::Index id;
	int value_min;
	int value_max;
	type_t type;
	QCheckBox *checkBox;
	QComboBox *comboBox;
	QSpinBox *spinBox;
	Setting::Value enabledValue;
	bool supported;
private slots:
	void checkBoxChanged(int s);
	void comboBoxChanged(int value);
	void spinBoxChanged(int value);
};

vtl_always_inline int ValueBox::getId() const
{
	return id;
}

#endif /* _TS_UI_TSPINBOX_H */
