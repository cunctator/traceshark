/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "ui/infowidget.h"
#include "ui/cursorinfo.h"
#include "ui/taskinfo.h"
#include "misc/traceshark.h"
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
	QHBoxLayout *mainLayout = new QHBoxLayout(widget);
	setWidget(widget);

	cursorInfos[0] = new CursorInfo(0, widget);
	cursorInfos[1] = new CursorInfo(1, widget);
	mainLayout->addWidget(cursorInfos[0]);
	mainLayout->addWidget(cursorInfos[1]);

	QLabel *diffLabel = new QLabel(tr("difference:"), widget);
	mainLayout->addWidget(diffLabel);

	diffLine = new QLineEdit(widget);
	diffLine->setReadOnly(true);
	diffLine->setText(QString::number((double) 0, 'f', 7));
	diffLine->setMaxLength(18);
	mainLayout->addWidget(diffLine);

	QLabel *comboLabel = new QLabel(tr("Active cursor:"), widget);
	mainLayout->addWidget(comboLabel);

	cursorComboBox = new QComboBox(widget);
	cursorComboBox->addItem(QString(tr("Red")));
	cursorComboBox->addItem(QString(tr("Blue")));
	cursorComboBox->setCurrentIndex(0);
	mainLayout->addWidget(cursorComboBox);

	taskInfo = new TaskInfo(widget);
	mainLayout->addWidget(taskInfo);

	for (i = 0; i < TShark::NR_CURSORS; i++)
		cursorValues[i] = 0;

	mainLayout->addStretch();
	sigconnect(cursorInfos[0], valueChanged(double, int), this,
		   valueChanged(double, int));
	sigconnect(cursorInfos[1], valueChanged(double, int), this,
		   valueChanged(double, int));
	sigconnect(taskInfo, findWakeup(unsigned int), this,
		   findWakeup(unsigned int));
	sigconnect(taskInfo, addTaskGraph(unsigned int), this,
		   addTaskGraph(unsigned int));
	sigconnect(taskInfo, removeTaskGraph(unsigned int), this,
		   removeTaskGraph(unsigned int));
	tsconnect(cursorInfos[0], valueChanged(double, int), this,
		  updateChange(double, int));
	tsconnect(cursorInfos[1], valueChanged(double, int), this,
		  updateChange(double, int));
}

InfoWidget::~InfoWidget()
{
}

void InfoWidget::addTaskGraphToLegend(TaskGraph *graph)
{
	taskInfo->addTaskGraphToLegend(graph);
}

void InfoWidget::setTime(double time, int cursorIdx)
{
	if (cursorIdx == TShark::RED_CURSOR ||
	    cursorIdx == TShark::BLUE_CURSOR) {
		cursorInfos[cursorIdx]->updateValue(time);
		cursorValues[cursorIdx] = time;
		updateDifference();
	}
}


void InfoWidget::updateChange(double value, int nr)
{
	if (nr == TShark::RED_CURSOR || nr == TShark::BLUE_CURSOR) {
		cursorValues[nr] = value;
		updateDifference();
	}
}

void InfoWidget::updateDifference()
{
	int precision = 7;
	double extra = 0;
	double diff = fabs(cursorValues[TShark::RED_CURSOR]
			   - cursorValues[TShark::BLUE_CURSOR]);

	if (diff >= 10)
		extra = floor (log(diff) / log(10));

	precision += (int) extra;
	diffLine->setText(QString::number(diff, 'f', precision));
}

int InfoWidget::getCursorIdx()
{
	return cursorComboBox->currentIndex();
}

void InfoWidget::setTaskGraph(TaskGraph *graph)
{
	taskInfo->setTaskGraph(graph);
}

void InfoWidget::removeTaskGraph()
{
	taskInfo->removeTaskGraph();
}

void InfoWidget::checkGraphSelection()
{
	taskInfo->checkGraphSelection();
}

void InfoWidget::pidRemoved(unsigned int pid)
{
	taskInfo->pidRemoved(pid);
}

void InfoWidget::clear()
{
	taskInfo->clear();
}
