// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef MISC_QTAPI_H
#define MISC_QTAPI_H

#include <QApplication>

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)

#include <QDesktopWidget>

#else /* QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) */

#include <QScreen>

#endif

#include <QFileDialog>
#include <QMouseEvent>
#include <QString>

#include "vtl/compiler.h"

#define QT4_WARNING \
"WARNING!!! WARNING!!! WARNING!!!\n" \
"WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!\n" \
"\n" \
"You are using a Qt version that is < 5.0.0. This may go well but is\n"  \
"not recommended unless you have a strong preference for Qt 4.\n" \
"\n" \
"WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!\n" \
"WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!"

namespace QtCompat {

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)

	vtl_always_inline double
	getPosXFromMouseEvent(const QMouseEvent *event) {
		return (double) event->position().x();
	}

#else /* QT_VERSION < QT_VERSION_CHECK(6,0,0)  */

	vtl_always_inline double
	getPosXFromMouseEvent(const QMouseEvent *event) {
		return (double) event->x();
	}

#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)

	vtl_always_inline QRect availableGeometry() {
		return QApplication::desktop()->availableGeometry();
	}

#else /* QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) */

	vtl_always_inline QRect availableGeometry() {
		return QApplication::primaryScreen()->availableGeometry();
	}

#endif

	vtl_always_inline void enableHighDpi() {

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0) && \
	QT_VERSION < QT_VERSION_CHECK(6,0,0)
		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5,1,0) && \
	QT_VERSION < QT_VERSION_CHECK(6,0,0)
		QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
	}

	vtl_always_inline void Qt4_enableOpenGL() {
		/* Set graphicssystem to opengl if we have old enough Qt */
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#ifdef TRACESHARK_QT4_OPENGL
		QApplication::setGraphicsSystem("opengl");
#pragma message(QT4_WARNING)
#endif /* TRACESHARK_QT4_OPENGL */
#endif /* QT_VERSION < QT_VERSION_CHECK(5, 0, 0) */
	}

	extern const QFileDialog::Options ts_foptions;

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)

	extern const QString::SplitBehavior SkipEmptyParts;

#else /* QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)  */

	extern const Qt::SplitBehavior SkipEmptyParts;

#endif
}
#endif /* MISC_QTAPI_H */
