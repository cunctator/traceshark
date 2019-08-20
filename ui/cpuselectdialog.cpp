// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016, 2017, 2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QComboBox>
#include <QTableView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMap>

#include "ui/cpuselectdialog.h"
#include "ui/cpuselectmodel.h"
#include "ui/tableview.h"
#include "misc/traceshark.h"

#define CBOX_INDEX_AND 0
#define CBOX_INDEX_OR  1

CPUSelectDialog::CPUSelectDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowCloseButtonHint), savedHeight(900)
{
	QVBoxLayout *mainLayout =  new QVBoxLayout(this);
	QHBoxLayout *filterLayout = new QHBoxLayout();

	cpuView = new TableView(this, TableView::TABLE_ROWSELECT);
	cpuModel = new CPUSelectModel(cpuView);
	cpuView->setModel(cpuModel);

	mainLayout->addWidget(cpuView);
	mainLayout->addLayout(filterLayout);

	QPushButton *closeButton = new QPushButton(tr("Close"));

	logicBox = new QComboBox();
	logicBox->addItem(QString(tr("AND")));
	logicBox->addItem(QString(tr("OR")));
	logicBox->setCurrentIndex(CBOX_INDEX_AND);

	QPushButton *addFilterButton =
		new QPushButton(tr("Create CPU filter"));
	QPushButton *resetFilterButton =
		new QPushButton(tr("Reset CPU filter"));

	filterLayout->addStretch();
	filterLayout->addWidget(closeButton);
	filterLayout->addWidget(logicBox);
	filterLayout->addWidget(addFilterButton);
	filterLayout->addWidget(resetFilterButton);
	filterLayout->addStretch();

	tsconnect(closeButton, clicked(), this, closeClicked());
	tsconnect(addFilterButton, clicked(), this, addFilterClicked());
	sigconnect(resetFilterButton, clicked(), this, resetFilter());

	filterMap = new QMap<unsigned int, unsigned int>();
}

CPUSelectDialog::~CPUSelectDialog()
{
	delete filterMap;
}

void CPUSelectDialog::setNrCPUs(unsigned int cpus)
{
	cpuModel->setNrCPUs(cpus);
}

void CPUSelectDialog::beginResetModel()
{
	cpuModel->beginResetModel();
}

void CPUSelectDialog::endResetModel()
{
	cpuModel->endResetModel();
}

/*
 * Apparently it's a bad idea to do cpuView->resizeColumnsToContents() if we
 * are not visible.
 */
void CPUSelectDialog::resizeColumnsToContents()
{
	if (QDialog::isVisible())
		cpuView->resizeColumnsToContents();
}

void CPUSelectDialog::show()
{
	QSize size;
	QDialog::show();
	QDialog::activateWindow();
	cpuView->resizeColumnsToContents();
	size = QDialog::size();
	size.setHeight(savedHeight);
	QDialog::resize(size);
}

void CPUSelectDialog::closeClicked()
{
	QSize size = QDialog::size();
	savedHeight = size.height();
	QDialog::hide();
}


void CPUSelectDialog::addFilterClicked()
{
	const QList<QModelIndex> &indexList = cpuView->selectedIndexes();
	unsigned int cpu;
	bool ok;
	int i, s;
	bool orlogic;

	filterMap->clear();
	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);
		cpu = cpuModel->rowToCPU(index.row(), ok);
		if (ok)
			(*filterMap)[cpu] = cpu;
	}
	orlogic = logicBox->currentIndex() == CBOX_INDEX_OR;
	emit createFilter(*filterMap, orlogic);
}
