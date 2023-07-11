// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2019, 2021, 2023
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>

#include "vtl/avltree.h"
#include "vtl/error.h"

#include "ui/taskselectdialog.h"
#include "ui/taskmodel.h"
#include "ui/statsmodel.h"
#include "ui/statslimitedmodel.h"
#include "ui/tableview.h"
#include "misc/errors.h"
#include "misc/resources.h"
#include "misc/traceshark.h"

#define LBOX_INDEX_AND 0
#define LBOX_INDEX_OR  1

#define EBOX_INDEX_CSV   0
#define EBOX_INDEX_ASCII 1

TaskSelectDialog::TaskSelectDialog(QWidget *parent, const QString &title,
				   enum TaskSelectType type)
	: QDockWidget(title, parent), savedHeight(900)
{
	QWidget *widget = new QWidget(this);
	QVBoxLayout *mainLayout =  new QVBoxLayout(widget);
	setWidget(widget);
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	QHBoxLayout *filterLayout = new QHBoxLayout();
	QHBoxLayout *settingLayout = new QHBoxLayout();
	QHBoxLayout *exportLayout = nullptr;
	/* Do we have stats that can be exported ? */
	QPushButton *exportButton = nullptr;
	QPushButton *closeButton = nullptr;

	taskView = new TableView(this, TableView::TABLE_ROWSELECT);
	switch (type) {
	case TaskSelectStats:
		taskModel = new StatsModel(taskView);
		break;
	case TaskSelectStatsLimited:
		taskModel = new StatsLimitedModel(taskView);
		break;
	case TaskSelectRegular:
		taskModel = new TaskModel(taskView);
		break;
	default:
		vtl::errx(BSD_EX_SOFTWARE, "Unexpected type in %s:%d", __FILE__, __LINE__);
	}
	taskView->setModel(taskModel);

	mainLayout->addWidget(taskView);
	mainLayout->addLayout(buttonLayout);
	mainLayout->addLayout(filterLayout);
	mainLayout->addLayout(settingLayout);

	exportLayout = new QHBoxLayout();
	mainLayout->addLayout(exportLayout);

	QPushButton *colorButton = new QPushButton(QIcon(RESSRC_GPH_COLORTASK),
						   tr("Color"));
	QPushButton *addUnifiedButton =
		new QPushButton(tr("Add a unified graph"));
	QPushButton *addLegendButton =
		new QPushButton(tr("Add to legend"));
	buttonLayout->addStretch();
	buttonLayout->addWidget(colorButton);
	buttonLayout->addWidget(addUnifiedButton);
	buttonLayout->addWidget(addLegendButton);
	buttonLayout->addStretch();

	logicBox = new QComboBox();
	logicBox->addItem(QString(tr("AND")));
	logicBox->addItem(QString(tr("OR")));
	logicBox->setCurrentIndex(LBOX_INDEX_AND);

	QPushButton *addFilterButton =
		new QPushButton(tr("Create pid filter"));
	QPushButton *resetFilterButton =
		new QPushButton(tr("Reset pid filter"));

	filterLayout->addStretch();
	filterLayout->addWidget(logicBox);
	filterLayout->addWidget(addFilterButton);
	filterLayout->addWidget(resetFilterButton);
	filterLayout->addStretch();

	QLabel *boxlabel = new QLabel(
		tr("Include waking/wakeup/fork/switch from other PIDs"));
	includeBox = new QCheckBox();
	includeBox->setChecked(true);

	settingLayout->addStretch();
	settingLayout->addWidget(boxlabel);
	settingLayout->addWidget(includeBox);
	settingLayout->addStretch();

	closeButton = new QPushButton(tr("Close"));
	exportButton = new QPushButton(tr("Export"));
	QLabel *exportlabel = new QLabel(tr("Export format:"));
	exportBox = new QComboBox();
	exportBox->addItem(QString(tr("CSV")));
	exportBox->addItem(QString(tr("ASCII")));
	exportBox->setCurrentIndex(EBOX_INDEX_CSV);

	exportLayout->addWidget(closeButton);
	exportLayout->addWidget(exportlabel);
	exportLayout->addWidget(exportBox);
	exportLayout->addWidget(exportButton);
	exportLayout->addStretch();

	hide();

	indexMap = new QMap<int, int>();
	filterMap = new QMap<int, int>();
	colorList = new QList<int>();

	tsconnect(colorButton, clicked(), this, colorClicked());
	tsconnect(closeButton, clicked(), this, closeClicked());
	tsconnect(addUnifiedButton, clicked(), this, addUnifiedClicked());
	tsconnect(addLegendButton, clicked(), this, addLegendClicked());
	tsconnect(addFilterButton, clicked(), this, addFilterClicked());
	tsconnect(taskView, doubleClicked(const QModelIndex &),
		  this, handleDoubleClick(const QModelIndex &));
	sigconnect(resetFilterButton, clicked(), this, resetFilter());
	tsconnect(exportButton, clicked(), this, exportClicked());
}

TaskSelectDialog::~TaskSelectDialog()
{
	delete colorList;
	delete filterMap;
	delete indexMap;
}

void TaskSelectDialog::setTaskMap(vtl::AVLTree<int, TaskHandle> *map,
				  unsigned int nrcpus)
{
	taskModel->setTaskMap(map, nrcpus);
}

void TaskSelectDialog::beginResetModel()
{
	taskModel->beginResetModel();
}

void TaskSelectDialog::endResetModel()
{
	taskModel->endResetModel();
}

/*
 * Apparently it's a bad idea to do taskView->resizeColumnsToContents() if we
 * are not visible.
 */
void TaskSelectDialog::resizeColumnsToContents()
{
	if (QDockWidget::isVisible())
		taskView->resizeColumnsToContents();
}

int TaskSelectDialog::exportStats(bool csv, const QString &filename)
{
	return taskModel->exportStats(csv, filename);
}

void TaskSelectDialog::show()
{
	QDockWidget::show();
	taskView->resizeColumnsToContents();
}

void TaskSelectDialog::closeClicked()
{
	QDockWidget::hide();
	emit QDockWidgetNeedsRemoval(this);
}

void TaskSelectDialog::colorClicked()
{
	const QList<QModelIndex> &indexList = taskView->selectedIndexes();
	QList<QModelIndex>::const_iterator iter;
	bool ok = false;
	int pid = 0;

	colorList->clear();
	indexMap->clear();

	for (iter = indexList.cbegin(); iter != indexList.cend(); iter++) {
		const QModelIndex &index = *iter;

		pid = taskModel->rowToPid(index.row(), ok);
		if (!ok)
			continue;
		if (indexMap->find(pid) == indexMap->cend()) {
			(*indexMap)[pid] = pid;
			colorList->append(pid);
		}
	}

	if (colorList->size() > 0)
		emit colorChangeReq(colorList);
}

void TaskSelectDialog::addUnifiedClicked()
{
	const QList<QModelIndex> &indexList = taskView->selectedIndexes();
	int pid;
	bool ok;
	bool need = false;
	int i, s;

	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);

		pid = taskModel->rowToPid(index.row(), ok);
		if (ok && pid != 0) {
			emit addTaskGraph(pid);
			need = true;
		}
	}
	if (need)
		emit needReplot();
}

void TaskSelectDialog::addLegendClicked()
{
	const QList<QModelIndex> &indexList = taskView->selectedIndexes();
	int pid;
	bool ok;
	int i, s;
	bool need = false;

	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);

		pid = taskModel->rowToPid(index.row(), ok);
		if (ok && pid != 0) {
			need = true;
			emit addTaskToLegend(pid);
		}
	}
	if (need) {
		emit needReplot();
		emit needLegendCheck();
	}
}

void TaskSelectDialog::addFilterClicked()
{
	const QList<QModelIndex> &indexList = taskView->selectedIndexes();
	int pid;
	bool ok;
	int i, s;
	bool orlogic, inclusive;

	filterMap->clear();
	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);

		pid = taskModel->rowToPid(index.row(), ok);
		if (ok)
			(*filterMap)[pid] = pid;
	}
	orlogic = logicBox->currentIndex() == LBOX_INDEX_OR;
	inclusive = includeBox->isChecked();
	emit createFilter(*filterMap, orlogic, inclusive);
}

void TaskSelectDialog::exportClicked()
{
	bool csv = exportBox->currentIndex() == EBOX_INDEX_CSV;

	emit doExport(csv);
}

void TaskSelectDialog::handleDoubleClick(const QModelIndex &index)
{
	bool ok;
	int pid = taskModel->rowToPid(index.row(), ok);

	if (ok)
		emit taskDoubleClicked(pid);
}
