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
#include <QLabel>
#include <QSpinBox>

#include "misc/traceshark.h"
#include "vtl/error.h"
#include "tspinbox.h"

#define MAX_SIZE_COMBOBOX (10)
#define TSPIN_MAX(A, B) (A >= B ? A : B)
#define TSPIN_MIN(A, B) (A <= B ? A : B)

TSpinBox::TSpinBox(int id_arg, int vmin, int vmax, QWidget *parent):
	QWidget(parent), id(id_arg), value_min(vmin), value_max(vmax)
{
	QHBoxLayout *layout = new QHBoxLayout();
	setLayout(layout);

	label = new QLabel();
	layout->addWidget(label);

	if (vmin >= vmax) {
		vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,	\
			  __LINE__);
	}

	bool is_spinbox = (vmax - vmin) > MAX_SIZE_COMBOBOX;
	type = is_spinbox ? TYPE_SPINBOX : TYPE_COMBOBOX;

	if (type == TYPE_SPINBOX) {
		sbox = new QSpinBox();
		sbox->setMinimum(vmin);
		sbox->setMaximum(vmax);
		layout->addWidget(sbox);
		tsconnect(sbox, valueChanged(int), this, sBoxChanged(int));
	} else { /* type == TYPE_COMBOBOX */
		cbox = new QComboBox();
		for (int i = vmin; i <= vmax; i++) {
			cbox->addItem(QString::number(i));
		}
		layout->addWidget(cbox);
		tsconnect(cbox, currentIndexChanged(int), this,
			  cBoxChanged(int));
	}
}

TSpinBox::~TSpinBox()
{}

void TSpinBox::sBoxChanged(int value)
{
	emit boxChanged(this, value);
}

void TSpinBox::cBoxChanged(int index)
{
	int value = index + value_min;
	emit boxChanged(this, value);
}

int TSpinBox::getId() const
{
	return id;
}

void TSpinBox::setText(const QString &str)
{
	label->setText(str);
}


void TSpinBox::setValue(int value)
{
	value = TSPIN_MIN(value, value_max);
	value = TSPIN_MAX(value, value_min);
	if (type == TYPE_SPINBOX) {
		sbox->setValue(value);
	} else { /* type == TYPE_COMBOBOX */
		cbox->setCurrentIndex(value - value_min);
	}
}

int TSpinBox::value() const
{
	if (type == TYPE_SPINBOX) {
		return sbox->value();
	} else { /* type == TYPE_COMBOBOX */
		return cbox->currentIndex() + value_min;
	}
}
