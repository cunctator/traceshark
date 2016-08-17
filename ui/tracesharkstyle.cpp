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

#include "ui/tracesharkstyle.h"

#define TRACESHARK_TOOLBARICONSSIZE 30

int TraceSharkStyle::pixelMetric(PixelMetric metric,
				 const QStyleOption *option,
				 const QWidget *widget) const
{
	if (metric == QStyle::PM_ToolBarIconSize)
		return TRACESHARK_TOOLBARICONSSIZE;
	else return QProxyStyle::pixelMetric(metric, option, widget);
}
