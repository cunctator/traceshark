/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui/cursorinfo.h"
#include "misc/traceshark.h"
#include <QIcon>
#include <QHBoxLayout>
#include <QPixmap>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <cmath>

#define RED_CURSOR_RESOURCE ":/traceshark/images/movered30x30.png"
#define BLUE_CURSOR_RESOURCE ":/traceshark/images/moveblue30x30.png"
#define RED_CURSOR_TOOLTIP "Move the red cursor to the specified time"
#define BLUE_CURSOR_TOOLTIP "Move the blue cursor to the specified time"

CursorInfo::CursorInfo(int nr, QWidget *parent):
	QWidget(parent), id(nr)
{
	QString text;
	QString qresource;
	QHBoxLayout *layout  = new QHBoxLayout(this);
	line = new QLineEdit(this);
	QPushButton *button;

	line->setReadOnly(false);
	line->setInputMask(QString("0000000.0000000"));

	switch (nr) {
	case TShark::RED_CURSOR:
		text = QString(tr(RED_CURSOR_TOOLTIP));
		qresource = QLatin1String(RED_CURSOR_RESOURCE);
		break;
	case TShark::BLUE_CURSOR:
		text = QString(tr(BLUE_CURSOR_TOOLTIP));
		qresource = QLatin1String(BLUE_CURSOR_RESOURCE);
		break;
	default:
		text = QString(tr("error in cursorinfo.cpp"));
		break;
	}

	layout->addWidget(line);

	QPixmap buttonPM(qresource);
	QIcon buttonIcon(buttonPM);

	button = new QPushButton(buttonIcon, tr(""), this);
	button->setToolTip(text);
	button->setIconSize(buttonPM.size());
	layout->addWidget(button);

	updateValue(0);
	tsconnect(button, clicked(), this, buttonClicked());
}

CursorInfo::~CursorInfo()
{
}

void CursorInfo::updateValue(double value)
{
	int precision = 7;
	double extra = 0;

	if (value >= 10)
		extra = floor (log(value) / log(10));

	precision += (int) extra;
	line->setText(QString::number((value), 'f', precision));
}

void CursorInfo::buttonClicked()
{
	if (line->hasAcceptableInput())
		emit valueChanged(line->text().toDouble(), id);
}
