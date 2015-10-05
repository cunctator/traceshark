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

#include "taskinfo.h"
#include "traceshark.h"
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QString>

TaskInfo::TaskInfo(QWidget *parent):
	QWidget(parent)
{
	QHBoxLayout *layout  = new QHBoxLayout(this);
	QLabel *colonLabel = new QLabel(tr(":"));
	QPushButton *button = new QPushButton(tr("Add to legend"), this);

	nameLine = new QLineEdit(this);
	pidLine = new QLineEdit(this);


	nameLine->setReadOnly(true);
	pidLine->setReadOnly(true);

	layout->addWidget(nameLine);
	layout->addWidget(colonLabel);
	layout->addWidget(pidLine);
	layout->addWidget(button);

	hide();
	tsconnect(button, clicked(), this, addClicked());
}

TaskInfo::~TaskInfo()
{
}


void TaskInfo::setInfo(unsigned int pid, const char *name)
{
	QString nameStr = QLatin1String(name);
	QString pidStr = QString::number(pid);
	nameLine->setText(nameStr);
	pidLine->setText(pidStr);
	currentPid = pid;
	show();
}

void TaskInfo::removeInfo()
{
	nameLine->setText(tr(""));
	pidLine->setText(tr(""));
	hide();
}

void TaskInfo::addClicked()
{
	emit addTask(currentPid);
}
