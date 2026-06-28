// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2018, 2023, 2026
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

#include "misc/chunk.h"
#include "misc/errors.h"
#include "misc/qtcompat.h"
#include "misc/traceshark.h"
#include "ui/eventinfodialog.h"
#include "parser/traceevent.h"
#include "parser/tracefile.h"

#include <QByteArray>
#include <QFont>
#include <QString>
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
	QRect screenSize;
	int wscreen;
	int hscreen;
	int width = 1600;
	int height = 850;

	screenSize = QtCompat::availableGeometry();

	wscreen = screenSize.width();
	hscreen = screenSize.height();

	width = TSMIN(width, wscreen);
	height = TSMIN(height, hscreen);

	setGeometry(wscreen / 2 - width / 2, hscreen / 2 - height / 2,
		    width, height);
	setFixedWidth(width);
	setFixedHeight(height);
}

void EventInfoDialog::show(const TraceEvent &event, TraceFile &file,
			   tracetype_t ttype)
{
	QByteArray array;
	QString text;
	int ts_errno = 0;
	const Chunk *chunk;

	/*
	 * The post-event info may be a chain of chunks. For perf traces it is a
	 * single chunk holding the backtrace; for ftrace traces it may hold a
	 * kernel stack and a user stack captured from separate events.
	 */
	for (chunk = event.postEventInfo; chunk != nullptr; chunk = chunk->next) {
		if (chunk->len <= 0)
			continue;
		array += file.getChunkArray(chunk, &ts_errno);
		if (ts_errno != 0) {
			vtl::warn(ts_errno, "Could not retrieve event info");
			return;
		}
	}

	if (array.isEmpty())
		return;

	if (file.isIntact(&ts_errno)) {
		if (ttype == TRACE_TYPE_FTRACE) {
			/*
			 * Strip the ftrace kernel_stack/user_stack header lines
			 * so that only the stack frames are shown. The result is
			 * never longer than the input, so the input size is
			 * enough for the output.
			 */
			QByteArray cleaned;
			int len;
			cleaned.resize(array.size());
			len = TShark::stripStackTraceHeaders(array.constData(),
							     (int) array.size(),
							     cleaned.data());
			cleaned.resize(len);
			text = QString(cleaned);
		} else {
			text = QString(array);
		}
		textEdit->setPlainText(text);
		QDialog::show();
		return;
	}
	if (ts_errno == 0)
		ts_errno = - TS_ERROR_FILECHANGED;
	vtl::warn(ts_errno, "Could not retrieve event info");
}
