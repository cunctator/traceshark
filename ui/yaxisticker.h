/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef YAXISTICKER_H
#define YAXISTICKER_H

#include <QVector>
#include <QString>
#include "qcustomplot/qcustomplot.h"

class YAxisTicker : public QCPAxisTicker {
public:
	YAxisTicker();
	void setTickVector(QVector<double> &ticks);
	void setTickVectorLabels(QVector<QString> &labels);
protected:
	QVector<double> createTickVector(double tickStep,
					 const QCPRange &range);
	QVector<QString> createLabelVector(const QVector<double> &ticks,
					   const QLocale &locale,
					   QChar formatChar,
					   int precision);
	int getSubTickCount(double tickStep);
private:
	QVector<double> storedTicks;
	QVector<QString> storedTickLabels;
};

#endif /* YAXISTICKER_H */
