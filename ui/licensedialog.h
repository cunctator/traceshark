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

#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE

class LicenseDialog : public QDialog {
	Q_OBJECT

public:
	LicenseDialog(QWidget *parent = 0);
private:
	// QTextEdit *textEdit;
	void updateSize();
};

#endif /* LICENSEDIALOG_H */
