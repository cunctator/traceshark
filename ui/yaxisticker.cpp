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

#include "ui/yaxisticker.h"

YAxisTicker::YAxisTicker() : QCPAxisTicker() {}

void YAxisTicker::setTickVector(QVector<double> &ticks)
{
	storedTicks = ticks;
}

void YAxisTicker::setTickVectorLabels(QVector<QString> &labels)
{
	storedTickLabels = labels;
}

QVector<double> YAxisTicker::createTickVector(double /* tickStep */,
					      const QCPRange &range)
{
	QVector<double> rval;
	int s = storedTicks.size();
	int i, start, end;
	double lower = range.lower;
	double upper = range.upper;

	if (s <= 0)
		return rval;

	if (lower >= upper)
		return rval;
	
	if (lower > storedTicks.last() || upper < storedTicks.first())
		return rval;
	
	for (i = 0; i < s; i++) {
		if (storedTicks.at(i) > lower)
			break;
	}
	start = i;
	if (start > 0)
		start--;

	for (i = s - 1; i >= 0; i--) {
		if (storedTicks.at(i) < upper)
			break;
	}
	end = i;
	if (end < s - 1)
		end++;

	for (i = start; i <= end; i++)
		rval.append(storedTicks.at(i));

	return rval;
}

QVector<QString> YAxisTicker::createLabelVector(const QVector<double> &ticks,
						const QLocale & /* locale */,
						QChar /* formatChar */,
						int /* precision */)
{
	QVector<QString> rval;
	int s = storedTicks.size();
	int t = storedTickLabels.size();
	int ts = ticks.size();
	int i, j, offset;


	if (ts <= 0 || t <= 0 || ts <= 0)
		return rval;

	for (i = 0; i < s; i++) {
		if (storedTicks.at(i) == ticks.at(0))
			break;
	}

	if (i >= s)
		return rval;

	offset = i;

	for (i = 0; i < ts; i++) {
		j = i + offset;
		if (j >= t)
			break;
		rval.append(storedTickLabels.at(j));
	}
	return rval;
}

int YAxisTicker::getSubTickCount(double /* tickStep */)
{
	return 0;
}
