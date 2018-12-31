// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "ui/infowidget.h"
#include "ui/cursorinfo.h"
#include "misc/traceshark.h"
#include "vtl/time.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <cmath>

InfoWidget::InfoWidget(QWidget *parent):
	QDockWidget(tr("Info"), parent)
{
	unsigned int i;
	QWidget *widget = new QWidget(this);
	mainLayout = new QHBoxLayout(widget);
	setWidget(widget);

	cursorInfos[0] = new CursorInfo(0, widget);
	cursorInfos[1] = new CursorInfo(1, widget);
	mainLayout->addWidget(cursorInfos[0]);
	mainLayout->addWidget(cursorInfos[1]);

	QLabel *diffLabel = new QLabel(tr("difference:"), widget);
	mainLayout->addWidget(diffLabel);

	diffLine = new QLineEdit(widget);
	diffLine->setReadOnly(true);
	diffLine->setText(VTL_TIME_ZERO.toQString());
	diffLine->setMaxLength(18);
	mainLayout->addWidget(diffLine);

	QLabel *comboLabel = new QLabel(tr("Active cursor:"), widget);
	mainLayout->addWidget(comboLabel);

	cursorComboBox = new QComboBox(widget);
	cursorComboBox->addItem(QString(tr("Red")));
	cursorComboBox->addItem(QString(tr("Blue")));
	cursorComboBox->setCurrentIndex(0);
	mainLayout->addWidget(cursorComboBox);

	for (i = 0; i < TShark::NR_CURSORS; i++)
		cursorValues[i] = 0;

	sigconnect(cursorInfos[0], valueChanged(vtl::Time, int), this,
		   valueChanged(vtl::Time, int));
	sigconnect(cursorInfos[1], valueChanged(vtl::Time, int), this,
		   valueChanged(vtl::Time, int));
	tsconnect(cursorInfos[0], valueChanged(vtl::Time, int), this,
		  updateChange(vtl::Time, int));
	tsconnect(cursorInfos[1], valueChanged(vtl::Time, int), this,
		  updateChange(vtl::Time, int));
}

InfoWidget::~InfoWidget()
{
}

void InfoWidget::setTime(const vtl::Time &time, int cursorIdx)
{
	if (cursorIdx == TShark::RED_CURSOR ||
	    cursorIdx == TShark::BLUE_CURSOR) {
		cursorInfos[cursorIdx]->updateValue(time);
		cursorValues[cursorIdx] = time;
		updateDifference();
	}
}

void InfoWidget::updateChange(const vtl::Time &value, int nr)
{
	if (nr == TShark::RED_CURSOR || nr == TShark::BLUE_CURSOR) {
		cursorValues[nr] = value;
		updateDifference();
	}
}

void InfoWidget::updateDifference()
{
	vtl::Time diff = (cursorValues[TShark::RED_CURSOR]
			  - cursorValues[TShark::BLUE_CURSOR]).fabs();
	diffLine->setText(diff.toQString());
}

int InfoWidget::getCursorIdx()
{
	int index =  cursorComboBox->currentIndex();

	if (index >= 0 && index < TShark::NR_CURSORS)
		return index;
	return 0;
}

void InfoWidget::setTraceActionsEnabled(bool e)
{
	cursorInfos[0]->setTraceActionsEnabled(e);
	cursorInfos[1]->setTraceActionsEnabled(e);
}

void InfoWidget::addToolBar(QToolBar *toolbar)
{
	mainLayout->addWidget(toolbar);
	mainLayout->addStretch();
}

void InfoWidget::addStretch()
{
	mainLayout->addStretch();
}
