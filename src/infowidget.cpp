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
#include "traceshark.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <cmath>

InfoWidget::InfoWidget(QWidget *parent):
	QDockWidget(parent)
{
	QWidget *widget = new QWidget;
	QHBoxLayout *mainLayout = new QHBoxLayout;
	widget->setLayout(mainLayout);
	setWidget(widget);

	cursorInfos[0] = new CursorInfo(0);
	cursorInfos[1] = new CursorInfo(1);
	mainLayout->addWidget(cursorInfos[0]);
	mainLayout->addWidget(cursorInfos[1]);

	QLabel *diffLabel = new QLabel(tr("difference:"));
	mainLayout->addWidget(diffLabel);

	diffLine = new QLineEdit();
	diffLine->setReadOnly(true);
	diffLine->setText(QString::number((double) 0, 'f', 7));
	diffLine->setMaxLength(18);
	mainLayout->addWidget(diffLine);

	QLabel *comboLabel = new QLabel(tr("Active cursor:"));
	mainLayout->addWidget(comboLabel);

	cursorComboBox = new QComboBox;
	cursorComboBox->addItem(QString(tr("Red")));
	cursorComboBox->addItem(QString(tr("Blue")));
	cursorComboBox->setCurrentIndex(0);
	mainLayout->addWidget(cursorComboBox);

	mainLayout->addStretch();
	sigconnect(cursorInfos[0], valueChanged(double, int), this,
		   valueChanged(double, int));
	sigconnect(cursorInfos[1], valueChanged(double, int), this,
		   valueChanged(double, int));
	tsconnect(cursorInfos[0], valueChanged(double, int), this,
		  updateChange(double, int));
	tsconnect(cursorInfos[0], valueChanged(double, int), this,
		  updateChange(double, int));
}

InfoWidget::~InfoWidget()
{
}

void InfoWidget::setTime(double time, int cursorIdx)
{
	if (cursorIdx == RED_CURSOR || cursorIdx == BLUE_CURSOR) {
		cursorInfos[cursorIdx]->updateValue(time);
		cursorValues[cursorIdx] = time;
		updateDifference();
	}
}


void InfoWidget::updateChange(double value, int nr)
{
	if (nr == RED_CURSOR || nr == BLUE_CURSOR) {
		cursorValues[nr] = value;
		updateDifference();
	}
}

void InfoWidget::updateDifference()
{
	int precision = 7;
	double extra = 0;
	double diff = fabs(cursorValues[RED_CURSOR]
			   - cursorValues[BLUE_CURSOR]);

	if (diff >= 10)
		extra = floor (log(diff) / log(10));

	precision += (int) extra;
	diffLine->setText(QString::number(diff, 'f', precision));
}

int InfoWidget::getCursorIdx()
{
	return cursorComboBox->currentIndex();
}
