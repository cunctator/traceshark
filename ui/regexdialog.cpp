// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2020, 2021  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "misc/traceshark.h"
#include "ui/regexdialog.h"
#include "ui/regexwidget.h"
#include "ui/valuebox.h"
#include "vtl/error.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#define CBOX_INDEX_AND 0
#define CBOX_INDEX_OR  1

RegexDialog::RegexDialog(QWidget *parent):
	QDialog(parent, Qt::WindowCloseButtonHint), savedHeight(900)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	layout = new QVBoxLayout();
	QHBoxLayout *buttonLayout = new QHBoxLayout();

	mainLayout->addLayout(layout);
	mainLayout->addLayout(buttonLayout);

	logicBox = new QComboBox();
	logicBox->addItem(QString(tr("AND")));
	logicBox->addItem(QString(tr("OR")));
	logicBox->setCurrentIndex(CBOX_INDEX_AND);

	QPushButton *addFilterButton =
		new QPushButton(tr("Create regex filter"));
	QPushButton *resetButton =
		new QPushButton(tr("Reset regex filter"));
	QPushButton *closeButton = new QPushButton(tr("Close"));
	QPushButton *okButton = new QPushButton(tr("OK"));

	buttonLayout->addStretch();
	buttonLayout->addWidget(closeButton);
	buttonLayout->addWidget(logicBox);
	buttonLayout->addWidget(addFilterButton);
	buttonLayout->addWidget(resetButton);
	buttonLayout->addWidget(okButton);
	buttonLayout->addStretch();

	tsconnect(closeButton, clicked(), this, closeClicked());
	tsconnect(resetButton, clicked(), this, resetClicked());
	tsconnect(addFilterButton, clicked(), this, addFilterClicked());
	tsconnect(okButton, clicked(), this, okClicked());

	addRegexWidget();
}

RegexDialog::~RegexDialog()
{
}

void RegexDialog::addRegexWidget(RegexWidget *after)
{
	RegexWidget *widget;
	QLinkedList<RegexWidget *>::iterator pos = findAfter(after);
	enum RegexWidget::Type type = RegexWidget::REGEX_LATER;

	if (after == nullptr)
		type = RegexWidget::REGEX_FIRST;

	widget = new RegexWidget(0, type);

	/*
	 * If this is the first and only RegexWidget, then we should disable the
	 * remove button.
	 */
	if (after == nullptr)
		widget->setRemoveEnabled(false);

	layout->addWidget(widget);
	regexWidgets.insert(pos, widget);
	tsconnect(widget, remove(RegexWidget *),
		  this, removeRegexWidget(RegexWidget *));
	tsconnect(widget, addAfter(RegexWidget *),
		  this, addRegexWidgetAfter(RegexWidget *));
}

QLinkedList<RegexWidget *>::iterator RegexDialog::find(RegexWidget *widget)
{
	QLinkedList<RegexWidget *>::iterator iter;

	for (iter = regexWidgets.begin(); iter != regexWidgets.end(); iter++)
		if (*iter == widget)
			break;
	return iter;
}

QLinkedList<RegexWidget *>::iterator RegexDialog::findAfter(RegexWidget *widget)
{
	QLinkedList<RegexWidget *>::iterator iter = find(widget);

	if (iter != regexWidgets.end())
		iter++;

	return iter;
}

void RegexDialog::okClicked()
{
	hide();
	addFilterClicked();
}

void RegexDialog::closeClicked()
{
	QLinkedList<RegexWidget *>::iterator iter;
	QLinkedList<RegexWidget *>::iterator next;
	RegexWidget *rwidget;

	hide();

	for (iter = regexWidgets.begin();
	     iter != regexWidgets.end();
	     iter = regexWidgets.erase(iter)) {
		rwidget = *iter;
		rwidget->deleteLater();
	}
	addRegexWidget();
}

void RegexDialog::resetClicked()
{
	emit resetFilter();
}

void RegexDialog::removeRegexWidget(RegexWidget *widget)
{
	regexWidgets.removeOne(widget);
	widget->deleteLater();

	/* Make sure that the first widget knows that it's first */
	if (!regexWidgets.isEmpty())
		regexWidgets.first()->setType(RegexWidget::REGEX_FIRST);

	/*
	 * If we only have one widget left, then we should disabled its remove
	 * button.
	 */
	if (regexWidgets.size() == 1)
		regexWidgets.first()->setRemoveEnabled(false);
}

void RegexDialog::addRegexWidgetAfter(RegexWidget *widget)
{
	addRegexWidget(widget);
	/*
	 * Make sure that the widget that added a new widget has its remove
	 * button enabled.
	 */
	widget->setRemoveEnabled(true);
}

void RegexDialog::addFilterClicked()
{
	QLinkedList<RegexWidget *>::iterator iter;
	RegexWidget *rwidget;
	Regex regex;

	filter.regvec.resize(0);
	filter.valid = false;

	for (iter = regexWidgets.begin(); iter != regexWidgets.end(); iter++) {
		rwidget = *iter;
		regex = *(rwidget->regex());
		filter.regvec.append(regex);
	}

	emit createFilter(filter,
			  logicBox->currentIndex() ==  CBOX_INDEX_OR);
}

void RegexDialog::show()
{
	QDialog::show();
}
