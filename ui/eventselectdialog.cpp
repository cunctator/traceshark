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

#include "ui/eventselectdialog.h"
#include "ui/eventselectmodel.h"
#include "ui/tableview.h"
#include "misc/traceshark.h"

#define CBOX_INDEX_AND 0
#define CBOX_INDEX_OR  1

EventSelectDialog::EventSelectDialog(QWidget *parent)
	: QDialog(parent, Qt::WindowCloseButtonHint), savedHeight(900)
{
	QVBoxLayout *mainLayout =  new QVBoxLayout(this);
	QHBoxLayout *filterLayout = new QHBoxLayout();

	eventView = new TableView(this, TableView::TABLE_ROWSELECT);
	eventModel = new EventSelectModel(eventView);
	eventView->setModel(eventModel);

	mainLayout->addWidget(eventView);
	mainLayout->addLayout(filterLayout);

	QPushButton *closeButton = new QPushButton(tr("Close"));

	logicBox = new QComboBox();
	logicBox->addItem(QString(tr("AND")));
	logicBox->addItem(QString(tr("OR")));
	logicBox->setCurrentIndex(CBOX_INDEX_AND);

	QPushButton *addFilterButton =
		new QPushButton(tr("Create event filter"));
	QPushButton *resetFilterButton =
		new QPushButton(tr("Reset event filter"));

	filterLayout->addStretch();
	filterLayout->addWidget(closeButton);
	filterLayout->addWidget(logicBox);
	filterLayout->addWidget(addFilterButton);
	filterLayout->addWidget(resetFilterButton);
	filterLayout->addStretch();

	tsconnect(closeButton, clicked(), this, closeClicked());
	tsconnect(addFilterButton, clicked(), this, addFilterClicked());
	sigconnect(resetFilterButton, clicked(), this, resetFilter());

	filterMap = new QMap<event_t, event_t>();
}

EventSelectDialog::~EventSelectDialog()
{
	delete filterMap;
}

void EventSelectDialog::setStringTree(const StringTree<> *stree)
{
	eventModel->setStringTree(stree);
}

void EventSelectDialog::beginResetModel()
{
	eventModel->beginResetModel();
}

void EventSelectDialog::endResetModel()
{
	eventModel->endResetModel();
}

/*
 * Apparently it's a bad idea to do eventView->resizeColumnsToContents() if we
 * are not visible.
 */
void EventSelectDialog::resizeColumnsToContents()
{
	if (QDialog::isVisible())
		eventView->resizeColumnsToContents();
}

void EventSelectDialog::show()
{
	QSize size;
	QDialog::show();
	QDialog::activateWindow();
	eventView->resizeColumnsToContents();
	size = QDialog::size();
	size.setHeight(savedHeight);
	QDialog::resize(size);
}

void EventSelectDialog::closeClicked()
{
	QSize size = QDialog::size();
	savedHeight = size.height();
	QDialog::hide();
}


void EventSelectDialog::addFilterClicked()
{
	const QList<QModelIndex> &indexList = eventView->selectedIndexes();
	event_t event;
	bool ok;
	int i, s;
	bool orlogic;

	filterMap->clear();
	s = indexList.size();
	for (i = 0; i < s; i++) {
		const QModelIndex &index = indexList.at(i);
		event = eventModel->rowToEvent(index.row(), ok);
		if (ok)
			(*filterMap)[event] = event;
	}
	orlogic = logicBox->currentIndex() == CBOX_INDEX_OR;
	emit createFilter(*filterMap, orlogic);
}
