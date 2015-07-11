/*
 * Traceshark - a visualizer for visualizing ftrace traces
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

#ifndef MIGRATIONARROW_H
#define MIGRATIONARROW_H

#include <QColor>
#include "qcustomplot/qcustomplot.h"

class MigrationArrow : public QCPItemLine
{
public:
	MigrationArrow(double start, double end, double time, QColor &color,
		       QCustomPlot *parent);
private:
	QCPItemPosition *pos1;
	QCPItemPosition *pos2;
	QCPLineEnding head;
};

#endif /* MIGRATIONARROW_H */
