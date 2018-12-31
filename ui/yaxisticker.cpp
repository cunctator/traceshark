// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
