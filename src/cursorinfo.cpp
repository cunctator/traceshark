/*
 * Traceshark - a visualizer for visualizing ftrace traces
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

#include "cursorinfo.h"
#include "traceshark.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <cmath>

CursorInfo::CursorInfo(int nr, QWidget *parent):
	QWidget(parent), id(nr)
{
	QString text;
	QHBoxLayout *layout  = new QHBoxLayout;
	line = new QLineEdit();
	QPushButton *button;

	line->setReadOnly(false);
	line->setInputMask(QString("00000000000.0000000"));

	switch (nr) {
	case 0:
		text = QString(tr("Move Red"));
		break;
	case 1:
		text = QString(tr("Move Blue"));
		break;
	default:
		text = QString(tr("error in cursorinfo.cpp"));
		break;
	}

	setLayout(layout);
	layout->addWidget(line);

	button = new QPushButton(text);
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
