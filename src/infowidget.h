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

#ifndef INFOWIDGET_H
#define INFOWIDGET_H

#include <QDockWidget>
#include "traceshark.h"

class CursorInfo;
class QLineEdit;
class QComboBox;

class InfoWidget : public QDockWidget
{
	Q_OBJECT
public:
	InfoWidget(QWidget *parent = 0);
	virtual ~InfoWidget();
	void setTime(double time, int cursorIdx);
	int getCursorIdx();
signals:
	void valueChanged(double value, int nr);
private slots:
	void updateChange(double value, int nr);
private:
	CursorInfo *cursorInfos[TShark::NR_CURSORS];
	QLineEdit *diffLine;
	QComboBox *cursorComboBox;
	double cursorValues[TShark::NR_CURSORS];
	void updateDifference();
};

#endif /* INFOWIDGET_H */
