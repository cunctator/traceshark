/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2014-2015  Viktor Rosendahl
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

#include <QTime>
#include <QString>
#include <QFile>
#include <QtGlobal>
#include <QTextStream>
#include <QVector>
#include <cstdlib>
#include <new>
#include <ftraceparser.h>

int main(int argc, char* argv[]) {
	QString valstr;
	QString fileName;
	FtraceParser parser;

	if (argc < 2) {
		QTextStream(stdout) << "Usage:" << argv[0] <<
			" <filename>\n";
		return 0;
	}
	
	fileName = QString::fromLocal8Bit(argv[1]);

	QTextStream(stdout) << "opening " << fileName << "\n";
	
	if (!parser.open(fileName)) {
		QTextStream(stdout) << "failed to open " << fileName << "\n";
		return 0;
	}

	return 0;
}
