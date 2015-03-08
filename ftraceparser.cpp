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

#include <QtGlobal>
#include <QString>
#include <QTextStream>
#include "ftraceparser.h"
#include "tracefile.h"

bool FtraceParser::open(const QString &fileName)
{
	unsigned long long nr = 0;
	bool ok = false;
	TraceFile file(fileName.toLocal8Bit().data(), ok, 1024*1024);

	if (!ok)
		return ok;

	//vector.resize(0);
	vector.reserve(80000000);

	while(!file.atEnd()) {
		TraceLine line;
		quint32 n = file.ReadLine(&line);
		vector.append(line);
		nr += n;
	}
	QTextStream(stdout) << nr << "\n";
	return true;
}

FtraceParser::FtraceParser()
{
}
