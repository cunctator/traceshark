// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2019, 2021  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "misc/settingstore.h"
#include "misc/traceshark.h"
#include "ui/graphenabledialog.h"
#include "ui/valuebox.h"
#include "vtl/error.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QVBoxLayout>

GraphEnableDialog::GraphEnableDialog(SettingStore *sstore, QWidget *parent):
	QDialog(parent, Qt::WindowCloseButtonHint), savedHeight(900),
	settingStore(sstore)
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	QGridLayout *layout = new QGridLayout();
	mainLayout->addLayout(layout);
	int idx;
	Setting::Index idxn;
	QPushButton *okButton;
	QPushButton *cancelButton;
	QPushButton *applyButton;
	QPushButton *saveButton;

	valueBoxMap = new QMap<Setting::Index, ValueBox*>;

	for (idx = 0; idx < Setting::NR_SETTINGS; idx++) {
		idxn = (Setting::Index) idx;
		if (valueBoxMap->contains(idxn))
			continue;
		ValueBox *valueBox;

		if (settingStore->isFlagSet(idxn,
					    Setting::FLAG_MUST_BE_CONSUMED))
			consumeList.append(idxn);

		valueBox = new ValueBox(idxn, settingStore);
		const Setting::Value &value = valueBox->value();
		tsconnect(valueBox, boxChanged(ValueBox *, Setting::Value),
			  this, handleBoxChanged(ValueBox *, Setting::Value));
		(*valueBoxMap)[idxn] = valueBox;
		layout->addWidget(valueBox, idx, 0, Qt::AlignLeft);

		unsigned int nrdep = settingStore->getNrDependents(idxn);
		unsigned int d;
		for (d = 0; d < nrdep; d++) {
			const Setting::Dependency &dy =
				settingStore->getDependent(idxn, d);
			Setting::Index d_idx = (Setting::Index) dy.index();
			bool dep_ok = dy.check(value);
			const Setting::Value &defval =
				settingStore->getDisabledValue(d_idx);

			if (valueBoxMap->contains(d_idx))
				vtl::errx(BSD_EX_SOFTWARE,
					  "Seems like the order of the enum "
					  "Setting::Index is wrong in "
					  "ui/setting.h!\n");

			valueBox = new ValueBox(d_idx, settingStore);
			if (!dep_ok)
				valueBox->setValue(defval);
			valueBox->setEnabled(dep_ok);
			tsconnect(valueBox, boxChanged(ValueBox *,
						       Setting::Value),
				  this, handleBoxChanged(ValueBox *,
							 Setting::Value));
			(*valueBoxMap)[d_idx] = valueBox;
			layout->addWidget(valueBox, idx, 1 + d,
					  Qt::AlignJustify);
		}
	}

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonLayout);
	cancelButton = new QPushButton(tr("Cancel"));
	getSizeButton = new QPushButton(tr("Use current main window size"));
	okButton = new QPushButton(tr("OK"));
	applyButton = new QPushButton(tr("Apply"));
	saveButton = new QPushButton(tr("Apply && Save"));

	if (!settingStore->getValue(Setting::LOAD_WINDOW_SIZE_START).boolv())
		getSizeButton->setEnabled(false);

	buttonLayout->addStretch();
	buttonLayout->addWidget(cancelButton);
	buttonLayout->addWidget(getSizeButton);
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(applyButton);
	buttonLayout->addWidget(saveButton);
	buttonLayout->addStretch();
	tsconnect(cancelButton, clicked(), this, cancelClicked());
	sigconnect(getSizeButton, clicked(), this, sizeRequest());
	tsconnect(okButton, clicked(), this, okClicked());
	tsconnect(applyButton, clicked(), this, applyClicked());
	tsconnect(saveButton, clicked(), this, saveClicked());
}

GraphEnableDialog::~GraphEnableDialog()
{
	delete valueBoxMap;
}

void GraphEnableDialog::okClicked()
{
	hide();
	applyClicked();
}

void GraphEnableDialog::cancelClicked()
{
	hide();
}

void GraphEnableDialog::setMainWindowSize(int wt, int ht)
{
	Setting::Value wt_value(wt);
	Setting::Value ht_value(ht);

	setUIValue(Setting::MAINWINDOW_WIDTH, wt_value);
	setUIValue(Setting::MAINWINDOW_HEIGHT, ht_value);
}

Setting::Value GraphEnableDialog::getUIValue(Setting::Index id, bool &ok)
{
	QMap<Setting::Index, ValueBox*>::iterator iter;
	ValueBox *vbox;

	iter = valueBoxMap->find(id);
	if (iter == valueBoxMap->end()) {
		ok = false;
		return Setting::Value(0);
	}
	vbox = *iter;
	ok = true;
	return vbox->value();
}

void GraphEnableDialog::refreshUIValues()
{
	QMap<Setting::Index, ValueBox*>::iterator iter;
	for(iter = valueBoxMap->begin(); iter != valueBoxMap->end(); iter++) {
		ValueBox *vbox = iter.value();
		Setting::Index idxn = (Setting::Index) vbox->getId();
		const Setting::Value &value = settingStore->getValue(idxn);
		vbox->setValue(value);
	}
}

void GraphEnableDialog::setUIValue(Setting::Index id,
				   const Setting::Value &value)
{
	QMap<Setting::Index, ValueBox*>::iterator iter;
	ValueBox *vbox;

	iter = valueBoxMap->find(id);
	if (iter == valueBoxMap->end())
		return;

	vbox = *iter;
	vbox->setValue(value);
}

void GraphEnableDialog::applyClicked()
{
	bool changed = false;
	bool filter_changed = false;
	QMap<Setting::Index, ValueBox*>::iterator iter;

	for(iter = valueBoxMap->begin(); iter != valueBoxMap->end(); iter++) {
		ValueBox *vbox = iter.value();
		Setting::Index idxn = (Setting::Index) vbox->getId();
		Setting::Value uivalue = vbox->value();
		const Setting::Value &setvalue = settingStore->getValue(idxn);
		if (uivalue != setvalue) {
			if (!Setting::isSizeSetting(idxn) &&
			    !Setting::isFilterSetting(idxn))
				changed = true;
			if (Setting::isFilterSetting(idxn))
				filter_changed = true;
			settingStore->setValue(idxn, uivalue);
		}
	}

	if (filter_changed)
		emit filterSettingsChanged();
	if (changed)
		emit settingsChanged();
	/*
	 * It's too difficult to figure out whether the size has really changed,
	 * so we do it unconditionally. The slot in the MainWindow class that is
	 * connected to this signal has to check it instead.
	 */
	emit sizeChanged();
}

void GraphEnableDialog::saveClicked()
{
	int ts_errno;

	okClicked();
	ts_errno = settingStore->saveSettings();
	if (ts_errno != 0)
		vtl::warn(ts_errno, "Failed to save settings to %s",
			  TS_SETTING_FILENAME);
	else {
		QMessageBox msgBox;
		QString msg = tr("The settings have been saved to:\n");
		msg += SettingStore::getFileName();
		msgBox.setText(msg);
		msgBox.exec();
	}
}

void GraphEnableDialog::handleBoxChanged(ValueBox *valueBox,
					 Setting::Value value)
{
	Setting::Index idx = (Setting::Index) valueBox->getId();

	const Setting::flag_t consumeflag = Setting::FLAG_MUST_BE_CONSUMED;
	bool need_consume = settingStore->isFlagSet(idx, consumeflag);

	handleDependencies(idx, value, need_consume, false);
	handleSpecialCases(idx, value);
}

void GraphEnableDialog::handleDependencies(Setting::Index idx,
					   const Setting::Value &value,
					   bool need_consume,
					   bool store)
{
	ValueBox *vbox;
	Setting::Value dval;
	unsigned int d;
	const Setting::Value &savedval = settingStore->getValue(idx);
	const unsigned int nrdep = settingStore->getNrDependents(idx);
	QMap<Setting::Index, ValueBox*>::iterator iter;

	for (d = 0; d < nrdep; d++) {
		Setting::Dependency dep = settingStore->getDependent(idx, d);
		Setting::Index dep_idx = (Setting::Index) dep.index();
		bool dep_ok = dep.check(value);
		if (need_consume)
			dep_ok = dep_ok && (dep.check(savedval));
		iter = valueBoxMap->find(dep_idx);
		if (iter != valueBoxMap->end()) {
			vbox = iter.value();
			bool is_enabled = vbox->isEnabled();
			if (is_enabled && dep_ok)
				continue;
			vbox->setEnabled(dep_ok);
			if (dep_ok) {
				dval = settingStore->getValue(dep_idx);
			} else {
				dval = settingStore->getDisabledValue(dep_idx);
				if (store)
					settingStore->setValue(dep_idx, dval);
			}
			vbox->setValue(dval);
		} else {
			vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,
				  __LINE__);
		}
	}
}

void GraphEnableDialog::handleSpecialCases(Setting::Index idx,
					   const Setting::Value &value)
{
	bool en;

	if (idx == Setting::LOAD_WINDOW_SIZE_START) {
		en = value.boolv();
		getSizeButton->setEnabled(en);
	}
}

void GraphEnableDialog::show()
{
	refreshUIValues();
	QDialog::show();
}

void GraphEnableDialog::checkConsumption()
{
	Setting::Index idx;
	QList<Setting::Index>::const_iterator iter;
	for (iter = consumeList.begin(); iter != consumeList.end(); iter++) {
		idx = *iter;
		QMap<Setting::Index, ValueBox*>::iterator viter;

		viter = valueBoxMap->find(idx);
		if (viter != valueBoxMap->end()) {
			valueBoxConsumption(idx, *viter);
			continue;
		}
		vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,
			  __LINE__);
	}
}

void GraphEnableDialog::valueBoxConsumption(Setting::Index idx,
					    ValueBox *box)
{
	QMap<Setting::Index, ValueBox*>::iterator iter;
	Setting::Value dval;
	const Setting::Value &value = settingStore->getValue(idx);
	if (value != box->value())
		box->setValue(value);

	handleDependencies(idx, value, false, true);
}
