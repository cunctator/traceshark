// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "ui/tcheckbox.h"
#include "ui/tspinbox.h"
#include "vtl/error.h"

#include <QComboBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QVBoxLayout>

GraphEnableDialog::GraphEnableDialog(SettingStore *sstore,
				     QWidget *parent):
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

	checkBoxMap = new QMap<Setting::Index, TCheckBox*>;
	spinBoxMap = new QMap<Setting::Index, TSpinBox*>;

	for (idx = 0; idx < Setting::NR_SETTINGS; idx++) {
		idxn = (Setting::Index) idx;
		if (checkBoxMap->contains(idxn) || spinBoxMap->contains(idxn))
			continue;
		const QString &name = settingStore->getName(idxn);
		const QString &unit = settingStore->getUnit(idxn);
		TCheckBox *checkBox;
		TSpinBox *spinBox;
		bool enabled;
		int vint;
		int vmaxint, vminint;

		if (settingStore->isFlagSet(idxn,
					    Setting::FLAG_MUST_BE_CONSUMED))
			consumeList.append(idxn);

		const Setting::Value &value = settingStore->getValue(idxn);

		switch (value.type()) {
		case Setting::Value::TYPE_BOOL:
			enabled = value.boolv();
			checkBox = new TCheckBox(idx, enabled);
			tsconnect(checkBox, boxClicked(TCheckBox *, bool),
				  this, handleBoxClicked(TCheckBox *, bool));
			checkBox->setText(name);
			(*checkBoxMap)[idxn] = checkBox;
			layout->addWidget(checkBox, idx, 0, Qt::AlignLeft);
			break;
		case Setting::Value::TYPE_INT:
			vmaxint = settingStore->getMaxValue(idxn).intv();
			vminint = settingStore->getMinValue(idxn).intv();
			vint = value.intv();
			spinBox = new TSpinBox(idx, vminint, vmaxint);
			spinBox->setValue(vint);
			spinBox->setName(name);
			spinBox->setUnit(unit);
			tsconnect(spinBox, boxChanged(TSpinBox *, int),
				  this, handleSpinChanged(TSpinBox *, int));
			(*spinBoxMap)[idxn] = spinBox;
			layout->addWidget(spinBox, idx, 0, Qt::AlignLeft);
			break;
		default:
			vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
		}


		unsigned int nrdep = settingStore->getNrDependents(idxn);
		unsigned int d;
		for (d = 0; d < nrdep; d++) {
			const Setting::Dependency &dy =
				settingStore->getDependent(idxn, d);
			Setting::Index d_idx = (Setting::Index) dy.index();
			bool dep_ok = dy.check(value);

			if (checkBoxMap->contains(d_idx) ||
			    spinBoxMap->contains(d_idx))
				vtl::errx(BSD_EX_SOFTWARE,
					  "Seems like the order of the enum "
					  "Setting::Index is wrong in "
					  "ui/setting.h!\n");
			const QString &dname = settingStore->getName(d_idx);
			const QString &dunit = settingStore->getUnit(d_idx);
			const Setting::Value &defval =
				settingStore->getDisabledValue(d_idx);
			const Setting::Value &dvalue =
				settingStore->getValue(d_idx);
			switch (dvalue.type()) {
			case Setting::Value::TYPE_BOOL:
				enabled = dep_ok ?
					dvalue.boolv() : defval.boolv();
				checkBox = new TCheckBox(d_idx, enabled);
				checkBox->setChecked(enabled);
				checkBox->setEnabled(dep_ok);
				checkBox->setText(dname);
				(*checkBoxMap)[d_idx] = checkBox;
				layout->addWidget(checkBox, idx, 1 + d,
						  Qt::AlignJustify);
				break;
			case Setting::Value::TYPE_INT:
				vmaxint = settingStore->getMaxValue(d_idx)
					.intv();
				vminint = settingStore->getMinValue(d_idx)
					.intv();
				vint = dep_ok ? dvalue.intv() : defval.intv();
				spinBox = new TSpinBox(d_idx, vminint, vmaxint);
				spinBox->setValue(vint);
				spinBox->setEnabled(dep_ok);
				spinBox->setName(dname);
				spinBox->setUnit(dunit);
				(*spinBoxMap)[d_idx] = spinBox;
				layout->addWidget(spinBox, idx, 1 + d,
						  Qt::AlignJustify);
				break;
			default:
				vtl::errx(BSD_EX_SOFTWARE, "%s:%d",
					  __FILE__, __LINE__);
				break;
			}
		}
	}

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	mainLayout->addLayout(buttonLayout);
	cancelButton = new QPushButton(tr("Cancel"));
	okButton = new QPushButton(tr("OK"));
	applyButton = new QPushButton(tr("Apply"));
	saveButton = new QPushButton(tr("Apply && Save"));
	buttonLayout->addStretch();
	buttonLayout->addWidget(cancelButton);
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(applyButton);
	buttonLayout->addWidget(saveButton);
	buttonLayout->addStretch();
	tsconnect(cancelButton, clicked(), this, cancelClicked());
	tsconnect(okButton, clicked(), this, okClicked());
	tsconnect(applyButton, clicked(), this, applyClicked());
	tsconnect(saveButton, clicked(), this, saveClicked());
}

GraphEnableDialog::~GraphEnableDialog()
{
	delete checkBoxMap;
	delete spinBoxMap;
}

void GraphEnableDialog::okClicked()
{
	hide();
	applyClicked();
}

void GraphEnableDialog::cancelClicked()
{
	hide();
	QMap<Setting::Index, TCheckBox*>::iterator iter;
	for(iter = checkBoxMap->begin(); iter != checkBoxMap->end(); iter++) {
		TCheckBox *tbox = iter.value();
		Setting::Index idxn = (Setting::Index)
			tbox->getId();
		const Setting::Value &value = settingStore->getValue(idxn);
		tbox->setChecked(value.boolv());
	}

	QMap<Setting::Index, TSpinBox*>::iterator siter;
	for(siter = spinBoxMap->begin(); siter != spinBoxMap->end(); siter++) {
		TSpinBox *sbox = siter.value();
		Setting::Index idxn = (Setting::Index)
			sbox->getId();
		const Setting::Value &value = settingStore->getValue(idxn);
		sbox->setValue(value.intv());
	}
}

void GraphEnableDialog::applyClicked()
{
	bool changed = false;
	QMap<Setting::Index, TCheckBox*>::iterator iter;

	for(iter = checkBoxMap->begin(); iter != checkBoxMap->end(); iter++) {
		TCheckBox *tbox = iter.value();
		Setting::Index idxn = (Setting::Index)
			tbox->getId();
		bool uivalue;
		uivalue = tbox->isChecked();
		const Setting::Value &setvalue = settingStore->getValue(idxn);
		if (uivalue != setvalue.boolv()) {
			changed = true;
			settingStore->setBoolValue(idxn, uivalue);
		}
	}

	QMap<Setting::Index, TSpinBox*>::iterator siter;
	for(siter = spinBoxMap->begin(); siter != spinBoxMap->end(); siter++)
	{
		TSpinBox *sbox = siter.value();
		Setting::Index idxn = (Setting::Index)
			sbox->getId();
		int uivalue;
		uivalue = sbox->value();
		const Setting::Value &setvalue = settingStore->getValue(idxn);
		if (uivalue != setvalue.intv()) {
			changed = true;
			settingStore->setIntValue(idxn, uivalue);
		}
	}

	if (changed)
		emit settingsChanged();
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

void GraphEnableDialog::handleBoxClicked(TCheckBox *checkBox, bool checked)
{
	Setting::Index id = (Setting::Index) checkBox->getId();
	unsigned int d;
	QMap<Setting::Index, TCheckBox*>::iterator iter;
	QMap<Setting::Index, TSpinBox*>::iterator siter;
	TCheckBox *dBox;
	TSpinBox *sBox;
	bool bval;
	int ival;
	bool consume = settingStore->isFlagSet(id, Setting::FLAG_MUST_BE_CONSUMED);

	Setting::Value newval(checked);
	const Setting::Value &savedval = settingStore->getValue(id);

	unsigned int nrdep = settingStore->getNrDependents(id);
	for (d = 0; d < nrdep; d++) {
		Setting::Dependency dep = settingStore->getDependent(id, d);
		Setting::Index dep_idx = (Setting::Index)
			dep.index();
		bool dep_ok = dep.check(newval);
		if (consume)
			dep_ok = dep_ok && (dep.check(savedval));
		iter = checkBoxMap->find(dep_idx);
		if (iter != checkBoxMap->end()) {
			dBox = iter.value();
			dBox->setDisabled(!dep_ok);
			if (!dep_ok)
				bval = settingStore->getDisabledValue(dep_idx)
					.boolv();
			else
				bval = settingStore->getValue(dep_idx).boolv();
			dBox->setChecked(bval);
			continue;
		}
		siter = spinBoxMap->find(dep_idx);
		if (siter != spinBoxMap->end()) {
			sBox = siter.value();
			sBox->setDisabled(!dep_ok);
			if (!dep_ok)
				ival = settingStore->getDisabledValue(dep_idx)
					.intv();
			else
				ival = settingStore->getValue(dep_idx).intv();
			sBox->setValue(ival);
		} else {
			vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,
				  __LINE__);
		}
	}
}

void GraphEnableDialog::handleSpinChanged(TSpinBox */*spinBox*/, int /*value*/)
{
	/*
	 * We currently do not have any dependencies that would depend on a
	 * spinbox having a value. I don't bother write code that there is
	 * no test case for.
	 */
}

void GraphEnableDialog::show()
{
	QDialog::show();
}

void GraphEnableDialog::checkConsumption()
{
	Setting::Index idx;
	QList<Setting::Index>::const_iterator iter;
	for (iter = consumeList.begin(); iter != consumeList.end(); iter++) {
		idx = *iter;
		QMap<Setting::Index, TCheckBox*>::iterator iter;
		QMap<Setting::Index, TSpinBox*>::iterator siter;

		iter = checkBoxMap->find(idx);
		if (iter != checkBoxMap->end()) {
			checkCBoxConsumption(idx, *iter);
			continue;
		}
		siter = spinBoxMap->find(idx);
		if (siter != spinBoxMap->end()) {
			checkSBoxConsumption(idx, *siter);
		} else {
			vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d", __FILE__,
				  __LINE__);
		}
	}
}

void GraphEnableDialog::checkCBoxConsumption(Setting::Index idx,
					     TCheckBox *box)
{
	QMap<Setting::Index, TCheckBox*>::iterator iter;
	QMap<Setting::Index, TSpinBox*>::iterator siter;
	const Setting::Value &value = settingStore->getValue(idx);
	bool bval = settingStore->getValue(idx).boolv();
	if (bval != box->isChecked())
		box->setChecked(bval);
	TCheckBox *cBox;
	TSpinBox *sBox;
	int ival;

	unsigned int nrdep = settingStore->getNrDependents(idx);
	unsigned int d;

	for (d = 0; d < nrdep; d++) {
		Setting::Dependency dep = settingStore->getDependent(idx, d);
		Setting::Index dep_idx = (Setting::Index) dep.index();
		bool dep_ok = dep.check(value);
		iter = checkBoxMap->find(dep_idx);
		if (iter != checkBoxMap->end()) {
			cBox = iter.value();
			cBox->setEnabled(dep_ok);
			if (dep_ok) {
				bval =  settingStore->getValue(dep_idx).boolv();
			} else {
				bval = settingStore->getDisabledValue(dep_idx)
					.boolv();
				settingStore->setBoolValue(dep_idx, bval);
			}
			cBox->setChecked(bval);
			continue;
		}
		siter = spinBoxMap->find(dep_idx);
		if (siter != spinBoxMap->end()) {
			sBox = siter.value();
			sBox->setEnabled(dep_ok);
			if (dep_ok) {
				ival =  settingStore->getValue(dep_idx).intv();
			} else {
				ival = settingStore->getDisabledValue(dep_idx)
					.intv();
				settingStore->setIntValue(dep_idx, bval);
			}
			sBox->setValue(ival);
			continue;
		}
	}
}

void GraphEnableDialog::checkSBoxConsumption(Setting::Index /*idx*/,
					      TSpinBox */*box*/)
{
	/*
	 * We currently do not have any spinboxes that would have the
	 * Setting::FLAG_MUST_BE_CONSUMED flag set. I don't bother write code
	 * that there is no test case for.
	 */
}
