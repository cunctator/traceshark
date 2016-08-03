/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "ui/taskselectdialog.h"
#include "ui/taskmodel.h"
#include "ui/taskview.h"
#include "misc/traceshark.h"

TaskSelectDialog::TaskSelectDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowCloseButtonHint), savedHeight(900)
{
	QVBoxLayout *mainLayout =  new QVBoxLayout(this);
	QHBoxLayout *buttonLayout = new QHBoxLayout();

	taskView = new TaskView(this);
	taskModel = new TaskModel(taskView);
	taskView->setModel(taskModel);

	mainLayout->addWidget(taskView);
	mainLayout->addLayout(buttonLayout);

	QPushButton *closeButton = new QPushButton(tr("Close"));
	QPushButton *addUnifiedButton =
		new QPushButton(tr("Add a unified graph"));
	QPushButton *addLegendButton =
		new QPushButton(tr("Add to legend"));
	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);
	buttonLayout->addWidget(addUnifiedButton);
	buttonLayout->addWidget(addLegendButton);
	buttonLayout->addStretch();

	tsconnect(closeButton, clicked(), this, closeClicked());
	tsconnect(addUnifiedButton, clicked(), this, addUnifiedClicked());
	tsconnect(addLegendButton, clicked(), this, addLegendClicked());
}

TaskSelectDialog::~TaskSelectDialog()
{}

void TaskSelectDialog::setTaskMap(QMap<unsigned int, Task> *map)
{
	taskModel->setTaskMap(map);
}

void TaskSelectDialog::beginResetModel()
{
	taskModel->beginResetModel();
}

void TaskSelectDialog::endResetModel()
{
	taskModel->endResetModel();
}

/* Apparently it's a bad idea to do taskView->resizeColumnsToContents() if we
 * are not visible */
void TaskSelectDialog::resizeColumnsToContents()
{
	if (QDialog::isVisible())
		taskView->resizeColumnsToContents();
}

void TaskSelectDialog::show()
{
	QSize size;
	QDialog::show();
	QDialog::activateWindow();
	taskView->resizeColumnsToContents();
	size = QDialog::size();
	size.setHeight(savedHeight);
	QDialog::resize(size);
}

void TaskSelectDialog::closeClicked()
{
	QSize size = QDialog::size();
	savedHeight = size.height();
	QDialog::hide();
}

void TaskSelectDialog::addUnifiedClicked()
{
	const QList<QModelIndex> &indexList = taskView->selectedIndexes();
	unsigned int pid;
	bool ok;
	int i, s;

	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);

		pid = taskModel->rowToPid(index.row(), ok);
		if (ok)
			emit addTaskGraph(pid);
	}
}

void TaskSelectDialog::addLegendClicked()
{
	const QList<QModelIndex> &indexList = taskView->selectedIndexes();
	unsigned int pid;
	bool ok;
	int i, s;

	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);

		pid = taskModel->rowToPid(index.row(), ok);
		if (ok)
			emit addTaskToLegend(pid);
	}
}
