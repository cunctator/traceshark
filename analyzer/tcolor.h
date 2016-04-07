/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
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

#ifndef TCOLOR_H
#define TCOLOR_H

#include <cstdint>

class QColor;

class TColor {
public:
	TColor(int red, int green, int blue);
	TColor();
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	QColor toQColor();
	__always_inline unsigned int SqDistance(TColor color);
	static TColor fromQColor(const QColor &qColor);
	static TColor getRandomColor();
};

__always_inline unsigned int TColor::SqDistance(TColor color)
{
	int r, g, b;

	r = color.red;
	g = color.green;
	b = color.blue;

	r -= red;
	g -= green;
	b -= blue;

	r *= r;
	g *= g;
	b *= b;

	return r + g + b;
}

#endif /* TCOLOR_H */
