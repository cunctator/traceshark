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

#ifndef CURSORINFO_H
#define CURSORINFO_H

#include <QWidget>
class QLineEdit;

class CursorInfo : public QWidget
{
	Q_OBJECT
public:
	CursorInfo(int nr, QWidget *parent = 0);
	virtual ~CursorInfo();
public slots:
	void updateValue(double value);
signals:
	void valueChanged(double value);
private slots:
	void buttonClicked();
private:
	QLineEdit *line;
};

#endif /* CURSORINFO_H */
