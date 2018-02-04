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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
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
	sigconnect(taskInfo, findWakeup(int), this, findWakeup(int));
	sigconnect(taskInfo, addTaskGraph(int), this, addTaskGraph(int));
	sigconnect(taskInfo, removeTaskGraph(int), this, removeTaskGraph(int));
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
	int index =  cursorComboBox->currentIndex();

	if (index >= 0 && index < TShark::NR_CURSORS)
		return index;
	return 0;
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

void InfoWidget::pidRemoved(int pid)
{
	taskInfo->pidRemoved(pid);
}

void InfoWidget::clear()
{
	taskInfo->clear();
}

void InfoWidget::setTraceActionsEnabled(bool e)
{
	taskInfo->setTraceActionsEnabled(e);
	cursorInfos[0]->setTraceActionsEnabled(e);
	cursorInfos[1]->setTraceActionsEnabled(e);
}
