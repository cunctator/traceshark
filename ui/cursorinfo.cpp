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

#include <QHBoxLayout>
#include <QIcon>
#include <QToolBar>
#include <QAction>
#include <QLineEdit>
#include <QString>
#include <QWidget>
#include <cmath>
#include <cstdio>

#include "ui/cursorinfo.h"
#include "misc/resources.h"
#include "misc/traceshark.h"

#define RED_CURSOR_TOOLTIP "Move the red cursor to the specified time"
#define BLUE_CURSOR_TOOLTIP "Move the blue cursor to the specified time"

CursorInfo::CursorInfo(int nr, QWidget *parent):
	QWidget(parent), id(nr)
{
	QString text;
	QString qresource;
	QHBoxLayout *layout  = new QHBoxLayout(this);
	line = new QLineEdit(this);

	line->setReadOnly(false);
	line->setInputMask(QString("0000000.000000"));

	switch (nr) {
	case TShark::RED_CURSOR:
		text = QString(tr(RED_CURSOR_TOOLTIP));
		qresource = QLatin1String(RESSRC_PNG_MOVE_RED);
		break;
	case TShark::BLUE_CURSOR:
		text = QString(tr(BLUE_CURSOR_TOOLTIP));
		qresource = QLatin1String(RESSRC_PNG_MOVE_BLUE);
		break;
	default:
		text = QString(tr("error in cursorinfo.cpp"));
		break;
	}

	layout->addWidget(line);

	moveCursorAction = new QAction(tr("Move"), this);
	moveCursorAction->setIcon(QIcon(qresource));
	/* Todo: come up with a shortcut below */
	/* moveCursorAction->setShortcuts(I_dont_know); */
	moveCursorAction->setToolTip(text);
	setTraceActionsEnabled(false);

	moveToolBar = new QToolBar(tr("Move Toolbar"), this);
	layout->addWidget(moveToolBar);
	moveToolBar->addAction(moveCursorAction);

	updateValue(VTL_TIME_ZERO);
	tsconnect(moveCursorAction, triggered(), this, moveTriggered());
}

CursorInfo::~CursorInfo()
{
}

void CursorInfo::updateValue(vtl::Time value)
{
	unsigned int p = value.getPrecision();
	QString mask = createInputMask(p);
	line->setInputMask(mask);
	line->setText(value.toQString());
}

void CursorInfo::moveTriggered()
{
	if (line->hasAcceptableInput()) {
		bool ok;
		vtl::Time t = vtl::Time::fromSpacedString(
			line->text().toLocal8Bit().data(), ok);
		if (ok)
			emit valueChanged(t, id);
	}
}

void CursorInfo::setTraceActionsEnabled(bool e)
{
	if (e == false)
		line->clear();
	line->setEnabled(e);
	moveCursorAction->setEnabled(e);
}

QString CursorInfo::createInputMask(unsigned int precision)
{
	char buf[40];
	int s = sprintf(buf, "0000000.");
	char *c = &buf[s];
	unsigned int i;

	for (i = 0; i < precision; i++) {
		*c = '0';
		c++;
	}

	*c = '\0';

	QString qstr(buf);
	return qstr;
}
