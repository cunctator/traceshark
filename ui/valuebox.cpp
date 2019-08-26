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

#include <QComboBox>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>

#include "misc/setting.h"
#include "misc/settingstore.h"
#include "misc/traceshark.h"
#include "vtl/error.h"
#include "valuebox.h"

#define MAX_SIZE_COMBOBOX (10)

ValueBox::ValueBox(Setting::Index id_arg, SettingStore *sstore,
		   QWidget *parent):
	QWidget(parent), settingStore(sstore), id(id_arg), type(TYPE_ERROR)
{
	const QString &name = settingStore->getName(id);
	const Setting::Value &value = settingStore->getValue(id);
	Setting::Value::type_t vtype = value.type();
	supported = settingStore->isSupported(id);

	if (vtype == Setting::Value::TYPE_INT) {
		QHBoxLayout *layout = new QHBoxLayout();
		setLayout(layout);
		const QString &unit = settingStore->getUnit(id);
		QLabel *label = new QLabel();
		label->setText(name);
		layout->addWidget(label);
		value_min = settingStore->getMinValue(id).value.int_value;
		value_max = settingStore->getMaxValue(id).value.int_value;
		createIntBox(layout);
		QLabel *ulabel = new QLabel();
		ulabel->setText(unit);
		layout->addWidget(ulabel);
	} else if (vtype == Setting::Value::TYPE_BOOL) {
		/*
		 * QStackedLayout is maybe a strange choice here but it does
		 * what I want here with a single widget added to it.
		 * QHBoxLayout tend to reserve too much space.
		 */
		QStackedLayout *layout = new QStackedLayout();
		setLayout(layout);
		createBoolBox(layout);
		checkBox->setText(name);
	} else
		vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,
			  __LINE__);

	setValue(value);
	QWidget::setEnabled(supported);
}

void ValueBox::createIntBox(QLayout *layout)
{
	if (value_min >= value_max) {
		vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,	\
			  __LINE__);
	}

	bool is_spinbox = (value_max - value_min) > MAX_SIZE_COMBOBOX;
	type = is_spinbox ? TYPE_SPINBOX : TYPE_COMBOBOX;
	if (type == TYPE_SPINBOX) {
		spinBox = new QSpinBox();
		spinBox->setMinimum(value_min);
		spinBox->setMaximum(value_max);
		layout->addWidget(spinBox);
		tsconnect(spinBox, valueChanged(int), this,
			  spinBoxChanged(int));
	} else { /* type == TYPE_COMBOBOX */
		comboBox = new QComboBox();
		for (int i = value_min; i <= value_max; i++) {
			comboBox->addItem(QString::number(i));
		}
		layout->addWidget(comboBox);
		tsconnect(comboBox, currentIndexChanged(int), this,
			  comboBoxChanged(int));
	}
}

void ValueBox::createBoolBox(QLayout *layout)
{
	type = TYPE_CHECKBOX;
	checkBox = new QCheckBox();
	layout->addWidget(checkBox);
	tsconnect(checkBox, stateChanged(int), this, checkBoxChanged(int));
}

ValueBox::~ValueBox()
{}

void ValueBox::spinBoxChanged(int val)
{
	Setting::Value value;
	value.type_ = Setting::Value::TYPE_INT;
	value.value.int_value = val;
	emit boxChanged(this, value);
}

void ValueBox::comboBoxChanged(int index)
{
	Setting::Value value;
	value.type_ = Setting::Value::TYPE_INT;
	value.value.int_value = index + value_min;
	emit boxChanged(this, value);
}

void ValueBox::checkBoxChanged(int /*s*/)
{
	Setting::Value value;
	value.type_ = Setting::Value::TYPE_BOOL;
	value.value.bool_value = checkBox->isChecked();
	emit boxChanged(this, value);
}

void ValueBox::setValue(const Setting::Value &v)
{
	switch (type) {
	case TYPE_CHECKBOX:
		Setting::assert_bool(v);
		checkBox->setChecked(v.value.bool_value);
		break;
	case TYPE_COMBOBOX:
		Setting::assert_int(v);
		comboBox->setCurrentIndex(v.value.bool_value - value_min);
		break;
	case TYPE_SPINBOX:
		Setting::assert_int(v);
		spinBox->setValue(v.value.int_value);
		break;
	default:
		vtl::errx(BSD_EX_SOFTWARE,"Unexpected internal state %s:%d",
			  __FILE__, __LINE__);
		break;
	}
}

Setting::Value ValueBox::value() const
{
	Setting::Value value;
	switch (type) {
	case TYPE_CHECKBOX:
		value.type_ = Setting::Value::TYPE_BOOL;
		value.value.bool_value = checkBox->isChecked();
		break;
	case TYPE_COMBOBOX:
		value.type_ = Setting::Value::TYPE_INT;
		value.value.int_value = comboBox->currentIndex() + value_min;
		break;
	case TYPE_SPINBOX:
		value.type_ = Setting::Value::TYPE_INT;
		value.value.int_value = spinBox->value();
		break;
	default:
		vtl::errx(BSD_EX_SOFTWARE, "Unexpected internal state %s:%d",
			  __FILE__, __LINE__);
		break;
	}
	return value;
}

void ValueBox::setEnabled(bool en)
{
	if (en == QWidget::isEnabled() || !supported)
		return;

	if (en) {
		setValue(enabledValue);
	} else {
		enabledValue = value();
		setValue(settingStore->getDisabledValue(id));
	}
	QWidget::setEnabled(en);
}

void ValueBox::setDisabled(bool dis)
{
	ValueBox::setEnabled(!dis);
}
