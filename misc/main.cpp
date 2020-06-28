// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016, 2018, 2020
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

#include <QApplication>
#include <QString>
#include <QtCore>
#include "misc/errors.h"
#include "misc/resources.h"
#include "ui/mainwindow.h"
#include "ui/tracesharkstyle.h"
#include "vtl/error.h"

#define QT4_WARNING \
"WARNING!!! WARNING!!! WARNING!!!\n" \
"WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!\n" \
"\n" \
"You are using a Qt version that is < 5.0.0. This may go well but is\n"  \
"not recommended unless you have a preference for Qt 4.\n" \
"\n" \
"WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!\n" \
"WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!! WARNING!!!"

static char *prgname;

static void parseOption(const char */*opt*/)
{}

static void parseArguments(QString *fileName, int argc, char* argv[])
{
	if (argc > 0) {
		prgname = *argv;
		argc--;
		argv++;
	}

	while (argc > 0) {
		if (**argv == '-')
			parseOption(*argv);
		else
			*fileName = QString(*argv);
		argc--;
		argv++;
	}
}


int main(int argc, char* argv[])
{
	/* must be called before QApplication is created */
#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5,1,0)
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

	QApplication app(argc, argv);
	MainWindow mainWindow;
	QPixmap pm(QLatin1String(RESSRC_GPH_SHARK));
	QIcon icon;
	QString appname = QLatin1String("Traceshark");
	QRect geometry;
	int width, height;
	QString fileName;

	vtl::set_strerror(ts_strerror);

	parseArguments(&fileName, argc, argv);
	/* Set graphicssystem to opengl if we have old enough Qt */
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#ifdef TRACESHARK_QT4_OPENGL
	QApplication::setGraphicsSystem("opengl");
#pragma message(QT4_WARNING)
#endif /* TRACESHARK_QT4_OPENGL */
#endif /* QT_VERSION < QT_VERSION_CHECK(5, 0, 0) */

	app.setStyle(new TraceSharkStyle);
	app.setApplicationName(appname);
	icon.addPixmap(pm);
	app.setWindowIcon(icon);
	
	mainWindow.setWindowIconText(appname);
	mainWindow.setWindowTitle(appname);

	mainWindow.show();

	geometry = QApplication::desktop()->availableGeometry();


	width = geometry.width() - geometry.width() / 32;
	height = geometry.height() - geometry.height() / 16;

	mainWindow.resize(width, height);
	if (!fileName.isEmpty())
		mainWindow.openFile(fileName);

	return app.exec();
}
