// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2018, 2019
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstdio>
#include <cstdlib>

#include "misc/errors.h"
#include "misc/resources.h"
#include "misc/traceshark.h"
#include "threads/tthread.h"
#include "ui/errordialog.h"

#include <QFile>
#include <QString>
#include <QTextEdit>
#include <QThread>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

extern "C" {
#include <string.h>
}

#define NEEDS_TO_CLOSE_MSG \
"\n\nUnfortunately, the application has encountered a serious internal error\n"\
"and needs to close."

ErrorDialog::ErrorDialog(QWidget *parent)
	:QDialog(parent, Qt::WindowCloseButtonHint)
{
	/* One extra character for null termination */
	buf = new char[bufSize + 1];

	textEdit = new QTextEdit();
	textEdit->setAcceptRichText(false);
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
	tsconnect(this, messageInThread(QString, bool, int), this,
		  handleMessageFromThread(QString, bool, int));
}

ErrorDialog::~ErrorDialog()
{
	delete[] buf;
}

void ErrorDialog::setText(const QString &text)
{
	textEdit->setPlainText(text);
	updateSize();
	exec();
}

void ErrorDialog::setErrno(int d_errno)
{
	QString text(strerror(d_errno));
	textEdit->setPlainText(text);
	updateSize();
	exec();
}

void ErrorDialog::killThreads()
{
	QList<QThread*>::iterator iter;
	QThread *current = QThread::currentThread();
	QList<QThread*> list;
	TThread::listThreads(list);
	int s = list.size();
	int i;

	/* Terminate all other threads */
	for (i = 0; i < s; i++) {
		QThread *thr = list[i];
		if (thr != current) {
			thr->terminate();
			thr->wait();
		}
	}

	/*
	 * Then ourselves. After this, only the main thread and some Qt
	 * internal threads should remain.
	 */
	current->terminate();
	current->wait();
}

void ErrorDialog::setTextThreadSafe(const QString &text, bool doExit,
				    int ecode)
{
	if (isMainThread()) {
		handleMessageFromThread(text, doExit, ecode);
	} else {
		emit messageInThread(text, doExit, ecode);
		if (doExit) {
			killThreads();
		}
	}
}

bool ErrorDialog::isMainThread()
{
	return QApplication::instance()->thread() == QThread::currentThread();
}

void ErrorDialog::handleMessageFromThread(QString text, bool doExit, int ecode)
{
	setText(text);
	if (doExit)
		exit(ecode);
}

void ErrorDialog::_warn(int vtl_errno, const char *fmt, va_list ap,
			bool doExit, int ecode)
{
	const char *emsg = nullptr;
	int r;

	if (vtl_errno < 0)
		emsg = ts_strerror(-vtl_errno);
	else if (vtl_errno > 0)
		emsg = strerror(vtl_errno);
	else
		emsg = "unknown error";

	QString qemsg = QString(emsg);

	r = vsnprintf(buf, bufSize, fmt, ap);
	if (r < 0 || r >= (int) bufSize)
		buf[bufSize] = '\0';

	QString qmsg(buf);

	QString wholeMessage = qmsg + QString(tr(":\n")) + qemsg;
	if (doExit)
		wholeMessage = wholeMessage + tr(NEEDS_TO_CLOSE_MSG);
	setTextThreadSafe(wholeMessage, doExit, ecode);
}

void ErrorDialog::_warnX(const char *fmt, va_list ap, bool doExit, int ecode)
{
	int r = vsnprintf(buf, bufSize, fmt, ap);
	if (r < 0 || r >= (int) bufSize)
		buf[bufSize] = '\0';

	QString qmsg(buf);
	if (doExit)
		qmsg = qmsg + tr(NEEDS_TO_CLOSE_MSG);
	setTextThreadSafe(qmsg, doExit, ecode);
}

void ErrorDialog::warn(int vtl_errno, const char *fmt, va_list ap)
{
	_warn(vtl_errno, fmt, ap, false, 0);
}

void ErrorDialog::warnX(const char *fmt, va_list ap)
{
	_warnX(fmt, ap, false, 0);
}

void ErrorDialog::error(int ecode, int vtl_errno, const char *fmt, va_list ap)
{
	_warn(vtl_errno, fmt, ap, true, ecode);
}

void ErrorDialog::errorX(int ecode, const char *fmt, va_list ap)
{
	_warnX(fmt, ap, true, ecode);
}

void ErrorDialog::updateSize()
{
	QSize screenSize;
	int wscreen;
	int hscreen;
	int width = 640;
	int height = 350;

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
