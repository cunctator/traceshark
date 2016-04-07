/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "misc/traceshark.h"
#include "ui/licensedialog.h"

#include <QFile>
#include <QString>
#include <QTextStream>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>


LicenseDialog::LicenseDialog(QWidget *parent)
	:QDialog(parent, Qt::WindowCloseButtonHint)
{
	QFile file(QLatin1String(":/traceshark/LICENSE"));

	if (!file.open(QIODevice::ReadOnly))
		qDebug() << "Warning, could not read license!\n";

	QTextStream textStream(&file);
	QString text = textStream.readAll();

	QTextEdit *textEdit = new QTextEdit();
	textEdit->setAcceptRichText(false);
	textEdit->setPlainText(text);
	textEdit->setReadOnly(true);
	textEdit->setLineWrapMode(QTextEdit::NoWrap);

	QVBoxLayout *vlayout = new QVBoxLayout;
	setLayout(vlayout);
	vlayout->addWidget(textEdit);

	QHBoxLayout *hlayout = new QHBoxLayout;
	vlayout->addLayout(hlayout);
	hlayout->addStretch();

	QPushButton *button = new QPushButton(tr("OK"));
	hlayout->addWidget(button);

	hlayout->addStretch();
	setModal(true);
	updateSize();

	hide();
	tsconnect(button, clicked(), this, hide());
}

void LicenseDialog::updateSize()
{
	QTextStream qout(stdout);
	QSize screenSize;
	int wscreen;
	int hscreen;
	int width = 640;
	int height = 700;

	screenSize = QApplication::desktop()->availableGeometry(QCursor::pos())
		.size();

	wscreen = screenSize.width();
	hscreen = screenSize.height();

	width = TSMIN(width, wscreen);
	height = TSMIN(height, hscreen);

	setGeometry(wscreen / 2 - width / 2, hscreen / 2 - height / 2,
		    width, height);
	setFixedWidth(width);
	setFixedHeight(height);
}
