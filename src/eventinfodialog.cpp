/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "traceshark.h"
#include "eventinfodialog.h"
#include "traceevent.h"

#include <QByteArray>
#include <QFont>
#include <QString>
#include <QTextStream>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>


EventInfoDialog::EventInfoDialog(QWidget *parent)
	:QDialog(parent, Qt::WindowCloseButtonHint)
{
	textEdit = new QPlainTextEdit();
	textEdit->setReadOnly(true);

	QVBoxLayout *vlayout = new QVBoxLayout;
	setLayout(vlayout);
	vlayout->addWidget(textEdit);

	QHBoxLayout *hlayout = new QHBoxLayout;
	vlayout->addLayout(hlayout);
	hlayout->addStretch();

	QPushButton *button = new QPushButton(tr("OK"));
	hlayout->addWidget(button);

	QFont font("Monospace");
	font.setStyleHint(QFont::TypeWriter);
	textEdit->setFont(font);
	
	hlayout->addStretch();
	setModal(false);
	updateSize();

	hide();
	tsconnect(button, clicked(), this, hide());
}

void EventInfoDialog::updateSize()
{
	QTextStream qout(stdout);
	QSize screenSize;
	int wscreen;
	int hscreen;
	int width = 1024;
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

void EventInfoDialog::show(const TraceEvent &event)
{
	QByteArray array(event.postEventInfo->ptr, event.postEventInfo->len);
	QString text(array);
	textEdit->setPlainText(text);
	QDialog::show();
}
