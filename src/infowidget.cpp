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

#include "infowidget.h"
#include "cursorinfo.h"
#include <QHBoxLayout>
#include <QWidget>

InfoWidget::InfoWidget(QWidget *parent):
	QDockWidget(parent)
{
	QWidget *widget = new QWidget;
	QHBoxLayout *mainLayout = new QHBoxLayout;
	widget->setLayout(mainLayout);
	setWidget(widget);

	cursorInfos[0] = new CursorInfo(1);
	cursorInfos[1] = new CursorInfo(2);
	mainLayout->addWidget(cursorInfos[0]);
	mainLayout->addWidget(cursorInfos[1]);
	mainLayout->addStretch();
}

InfoWidget::~InfoWidget()
{
}
