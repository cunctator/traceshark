// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "misc/setting.h"
#include "misc/traceshark.h"
#include "ui/graphenabledialog.h"
#include "ui/tcheckbox.h"
#include "vtl/error.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QVBoxLayout>

GraphEnableDialog::GraphEnableDialog(QWidget *parent):
	QDialog(parent, Qt::WindowCloseButtonHint), savedHeight(900)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	QGridLayout *layout = new QGridLayout();
	mainLayout->addLayout(layout);
	int idx;
	Setting::SettingIndex idxn;
	QPushButton *okButton;
	QPushButton *cancelButton;

	checkBoxMap = new QMap<Setting::SettingIndex, TCheckBox*>;
	for (idx = 0; idx < Setting::NR_SETTINGS; idx++) {
		idxn = (Setting::SettingIndex) idx;
		if (checkBoxMap->contains(idxn))
			continue;
		const QString &name = Setting::getName(idxn);
		bool enabled = Setting::isEnabled(idxn);
		TCheckBox *checkBox = new TCheckBox(idx, enabled);
		tsconnect(checkBox, boxClicked(TCheckBox *, bool),
			  this, handleBoxClicked(TCheckBox *, bool));
		checkBox->setText(name);
		layout->addWidget(checkBox, idx, 0, Qt::AlignLeft);
		(*checkBoxMap)[idxn] = checkBox;

		unsigned int nrdep = Setting::getNrDependents(idxn);
		unsigned int d;
		for (d = 0; d < nrdep; d++) {
			Setting::SettingIndex d_idx = (Setting::SettingIndex)
				Setting::getDependent(idxn, d).index;
			if (checkBoxMap->contains(d_idx))
				vtl::errx(BSD_EX_SOFTWARE,
				"Seems like the order of the enum "
				"Setting::SettingIndex is wrong in "
				"ui/setting.h is wrong!\n");

			const QString &dname = Setting::getName(d_idx);
			enabled = Setting::isEnabled(d_idx);
			checkBox = new TCheckBox(d_idx, enabled);
			checkBox->setChecked(enabled);
			checkBox->setText(dname);
			layout->addWidget(checkBox, idx, 1 + d,
					  Qt::AlignJustify);
			(*checkBoxMap)[d_idx] = checkBox;
		}
	}

	QHBoxLayout *comboLayout =  new QHBoxLayout();
	layout->addLayout(comboLayout, idx, 0, Qt::AlignLeft);
	comboBox = new QComboBox();
	comboBox->addItem(tr("1"));
	comboBox->addItem(tr("2"));
	comboBox->addItem(tr("3"));
	comboBox->addItem(tr("4"));
	comboBox->setCurrentIndex(Setting::getLineWidth() - 1);
	QLabel *comboLabel = new QLabel(tr("Line width of sched graphs:"));
	comboLayout->addWidget(comboLabel);
	comboLayout->addWidget(comboBox);

#ifndef QCUSTOMPLOT_USE_OPENGL
	/*
	 * We only let the user increase the line width of the scheduling
	 * graphs if we have opengl enabled.
	 */
	comboBox->setEnabled(false);
	QLabel *disabledLabel = new QLabel(tr("(Enable OpenGL to change line width)"));
	comboLayout->addWidget(disabledLabel);
#endif

	comboLayout->addStretch();

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonLayout);
	cancelButton = new QPushButton(tr("Cancel"));
	okButton = new QPushButton(tr("OK"));
	buttonLayout->addStretch();
	buttonLayout->addWidget(cancelButton);
	buttonLayout->addWidget(okButton);
	buttonLayout->addStretch();
	tsconnect(cancelButton, clicked(), this, cancelClicked());
	tsconnect(okButton, clicked(), this, okClicked());
}

GraphEnableDialog::~GraphEnableDialog()
{
	delete checkBoxMap;
}

void GraphEnableDialog::okClicked()
{
	bool changed = false;
	int new_width;
	QMap<Setting::SettingIndex, TCheckBox*>::iterator iter;

	hide();

	for(iter = checkBoxMap->begin(); iter != checkBoxMap->end(); iter++) {
		TCheckBox *tbox = iter.value();
		Setting::SettingIndex idxn = (Setting::SettingIndex)
			tbox->getId();
		bool checked = tbox->isChecked();
		bool enabled = Setting::isEnabled(idxn);
		if (checked != enabled) {
			changed = true;
			Setting::setEnabled(idxn, checked);
		}
	}

	new_width = comboBox->currentIndex() + 1;
	if (new_width != Setting::getLineWidth()) {
		changed = true;
		Setting::setLineWidth(new_width);
	}

	if (changed)
		emit settingsChanged();
}

void GraphEnableDialog::cancelClicked()
{
	hide();
	QMap<Setting::SettingIndex, TCheckBox*>::iterator iter;
	for(iter = checkBoxMap->begin(); iter != checkBoxMap->end(); iter++) {
		TCheckBox *tbox = iter.value();
		Setting::SettingIndex idxn = (Setting::SettingIndex)
			tbox->getId();
		bool enabled = Setting::isEnabled(idxn);
		tbox->setChecked(enabled);
	}
	comboBox->setCurrentIndex(Setting::getLineWidth() - 1);
}

void GraphEnableDialog::handleBoxClicked(TCheckBox *checkBox, bool checked)
{
	Setting::SettingIndex id = (Setting::SettingIndex) checkBox->getId();
	unsigned int d;
	QMap<Setting::SettingIndex, TCheckBox*>::iterator iter;
	TCheckBox *dBox;

	unsigned int nrdep = Setting::getNrDependents(id);
	for (d = 0; d < nrdep; d++) {
		SettingDependency dep = Setting::getDependent(id, d);
		Setting::SettingIndex dep_idx = (Setting::SettingIndex)
			dep.index;
		iter = checkBoxMap->find(dep_idx);
		if (iter == checkBoxMap->end())
			vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,
				  __LINE__);
		dBox = iter.value();
		dBox->setDisabled(checked != dep.desiredValue);
	}
}
