// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2018-2020, 2022
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

#include <QColor>
#include <QPen>

#include "analyzer/abstracttask.h"
#include "ui/cursor.h"
#include "vtl/time.h"

Cursor::Cursor(QCustomPlot *parent, TShark::CursorIdx idx):
	QCPItemLine(parent), position(0), time(0, 6), cursorIdx(idx)
{
	QColor color;

	switch (idx) {
	case TShark::BLUE_CURSOR:
		color = Qt::blue;
		break;
	case TShark::RED_CURSOR:
		color = Qt::red;
		break;
	default:
		color = Qt::magenta;
		break;
	}

	QPen pen;
	pen.setStyle(Qt::DashLine);
	pen.setWidth(2);
	pen.setColor(color);
	setPen(pen);
	setPosition(0);
}

void Cursor::setPosition(double pos)
{
	time = vtl::Time::fromDouble(pos);
	/*
	 * Fixme: Make some functions so that the precision can be propagated
	 * from the TraceAnalyzer class, via MainWindow.
	 */
	time.setPrecision(6);
	advertiseTime(time);
	setPosition_(pos);
}

void Cursor::setPosition(const vtl::Time &t)
{
	time = t;
	advertiseTime(time);
	setPosition_(time.toDouble());
}

void Cursor::setPosition_(double pos)
{
	start->setCoords(pos, -10000000000000000);
	end->setCoords(pos, +10000000000000000);
	parentPlot()->replot();
	position = pos;
}

double Cursor::getPosition()
{
	return position;
}

const vtl::Time &Cursor::getTime()
{
	return time;
}

void Cursor::advertiseTime(const vtl::Time &time)
{
	AbstractTask::setCursorTime(cursorIdx, time);
}
