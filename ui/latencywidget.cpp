// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2021, 2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>

#include "misc/traceshark.h"
#include "ui/latencymodel.h"
#include "ui/latencywidget.h"
#include "ui/tableview.h"

LatencyWidget::LatencyWidget(const QString &title, enum Latency::Type type,
			     QWidget *parent)
	: QDockWidget(title, parent)
{
	QWidget *widget = new QWidget(this);
	QVBoxLayout *mainLayout =  new QVBoxLayout(widget);
	setWidget(widget);
	QHBoxLayout *buttonLayout = new QHBoxLayout();

	latencyView =  new TableView(this, TableView::TABLE_SINGLEROWSELECT);
	latencyModel = new LatencyModel(type);

	latencyView->setModel(latencyModel);

	mainLayout->addWidget(latencyView);
	mainLayout->addLayout(buttonLayout);

	formatBox = new QComboBox();

	/*
	 * These must be in the same order as the items in
	 * TraceAnalyzer:exportformat_t
	 */
	formatBox->addItem(QString(tr("ASCII")));
	formatBox->addItem(QString(tr("CSV")));
	formatBox->setCurrentIndex(0);

	QPushButton *exportButton = new QPushButton(tr("Export"));
	QPushButton *closeButton = new QPushButton(tr("Close"));

	buttonLayout->addStretch();
	buttonLayout->addWidget(formatBox);
	buttonLayout->addWidget(exportButton);
	buttonLayout->addWidget(closeButton);
	buttonLayout->addStretch();

	hide();

	tsconnect(closeButton, clicked(), this, closeClicked());
	tsconnect(exportButton, clicked(), this, exportClicked());
	tsconnect(latencyView, doubleClicked(const QModelIndex &),
		  this, handleDoubleClick(const QModelIndex &));
}


LatencyWidget::~LatencyWidget()
{}

void LatencyWidget::setAnalyzer(TraceAnalyzer *azr)
{
	latencyModel->setAnalyzer(azr);
}

void LatencyWidget::clear()
{
	latencyModel->clear();
}

/*
 * Apparently it's a bad idea to do taskView->resizeColumnsToContents() if we
 * are not visible.
 */
void LatencyWidget::resizeColumnsToContents()
{
	if (QDockWidget::isVisible())
		latencyView->resizeColumnsToContents();
}

void LatencyWidget::show()
{
	QDockWidget::show();
	latencyView->resizeColumnsToContents();
}

void LatencyWidget::closeClicked()
{
	QDockWidget::hide();
	emit QDockWidgetNeedsRemoval(this);
}

void LatencyWidget::exportClicked()
{
	emit exportRequested(formatBox->currentIndex());
}

void LatencyWidget::handleDoubleClick(const QModelIndex &index)
{
	const Latency *latency = latencyModel->rowToLatency(index.row());

	if (latency != nullptr)
		emit latencyDoubleClicked(latency);
}
