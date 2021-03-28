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

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>

#include "vtl/error.h"
#include "misc/traceshark.h"
#include "analyzer/regexfilter.h"
#include "regexwidget.h"

RegexWidget::RegexWidget(QWidget *parent, enum Type type):
	QWidget(parent), myType(REGEX_LATER)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	QLabel *notLabel = new QLabel(tr("NOT:"));
	QLabel *posTLabel = new QLabel(tr("Pos type:"));
	QLabel *posLabel = new QLabel(tr("Pos:"));
	QLabel *label = new QLabel(tr("Regex:"));
	QLabel *extLabel = new QLabel(tr("Ext. Regex"));
	QLabel *caseLabel = new QLabel(tr("Match Case"));
	QPushButton *addButton = new QPushButton(tr("Add Regex"));
	int i;

	removeButton = new QPushButton(tr("Remove"));
	logicBox = new QComboBox();

	for (i = 0; i < TShark::LOGIC_NR; i++)
		logicBox->addItem(QString(TShark::logic_names[i]));

	logicBox->setCurrentIndex((int)TShark::LOGIC_AND);

	notBox = new QCheckBox();
	posBox = new QComboBox();

	for (i = 0; i < Regex::POS_NR; i++)
		posBox->addItem(QString(Regex::posNames[i]));
	setType(type);

	posBox->setCurrentIndex((int)Regex::POS_NONE);

	posSpinBox = new QSpinBox();
	posSpinBox->setRange(0, 0);
	posSpinBox->setEnabled(false);

	extendedBox = new QCheckBox();
	extendedBox->setChecked(false);

	caseBox = new QCheckBox();
	caseBox->setChecked(true);

	regexLine = new QLineEdit();

	layout->addWidget(logicBox);
	layout->addWidget(notLabel);
	layout->addWidget(notBox);
	layout->addWidget(posTLabel);
	layout->addWidget(posBox);
	layout->addWidget(posLabel);
	layout->addWidget(posSpinBox);
	layout->addWidget(extLabel);
	layout->addWidget(extendedBox);
	layout->addWidget(caseLabel);
	layout->addWidget(caseBox);
	layout->addWidget(label);
	layout->addWidget(regexLine);
	layout->addWidget(addButton);
	layout->addWidget(removeButton);
	layout->addStretch();

	tsconnect(addButton, clicked(), this, addClicked());
	tsconnect(removeButton, clicked(), this, removeClicked());
	tsconnect(posBox, currentIndexChanged(int), this, posBoxChanged(int));
}

RegexWidget::~RegexWidget()
{
}

const Regex *RegexWidget::regex()
{
	int idx = logicBox->currentIndex();
	int pidx =  posBox->currentIndex();

	if (idx < 0 || idx >= (int) TShark::LOGIC_NR) {
		vtl::errx(BSD_EX_SOFTWARE, "Internal error at %s:%d\n",
			  __FILE__, __LINE__);
	}

	if (pidx < 0 || pidx >= (int) Regex::POS_NR) {
		vtl::errx(BSD_EX_SOFTWARE, "Internal error at %s:%d\n",
			  __FILE__, __LINE__);
	}

	regex_m.logic = (enum TShark::Logic) idx;
	regex_m.inverted = notBox->isChecked();
	regex_m.isExtended = extendedBox->isChecked();
	regex_m.caseSensitive = caseBox->isChecked();
	regex_m.posType = (enum Regex::PosType) pidx;
	if (regex_m.posType == Regex::POS_NONE)
		regex_m.pos = 0;
	else
		regex_m.pos = posSpinBox->value();

	regex_m.text = regexLine->text();
	regex_m.regex_valid = false;
	return &regex_m;
}

void RegexWidget::setRemoveEnabled(bool e)
{
	if (e != removeButton->isEnabled())
		removeButton->setEnabled(e);
}

void RegexWidget::removeClicked()
{
	emit remove(this);
}

void RegexWidget::addClicked()
{
	emit addAfter(this);
}

void RegexWidget::posBoxChanged(int index)
{
	enum Regex::PosType type = (enum Regex::PosType) index;

	switch (type) {
	case Regex::POS_NONE:
		posSpinBox->setEnabled(false);
		break;
	case Regex::POS_ABSOLUTE:
		posSpinBox->setRange(0, 99);
		posSpinBox->setEnabled(true);
		break;
	case Regex::POS_RELATIVE:
		posSpinBox->setRange(-99, 99);
		posSpinBox->setEnabled(true);
		break;
	default:
		vtl::errx(BSD_EX_SOFTWARE, "Unexpected valued in %s()",
			__FUNCTION__);
	}
}

void RegexWidget::setType(enum Type type)
{
	bool was_rel;
	const int ridx = (int) Regex::POS_RELATIVE;

	if (type == myType)
		return;

	if (type == REGEX_FIRST) {
		was_rel = posBox->currentIndex() == ridx;
		posBox->removeItem(ridx);
		if (was_rel)
			posBox->setCurrentIndex(Regex::POS_NONE);
	} else if (type == REGEX_LATER)
		posBox->insertItem(ridx, QString(Regex::posNames[ridx]));
}
