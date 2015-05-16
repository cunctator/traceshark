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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVector>
#include <QString>
#include "threads/workitem.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QCustomPlot;
QT_END_NAMESPACE

class FtraceParser;
class WorkQueue;
class EventsWidget;
class InfoWidget;
class Cursor;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();
protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void openTrace();
	void closeTrace();
	void about();
	void license();
	void mouseWheel();
	void mousePress();
	void plotDoubleClicked(QMouseEvent *event);

private:
	void processTrace();
	void computeLayout();
	void rescaleTrace();
	void clearPlot();
	void showTrace();
	void setupCursors();

	QCustomPlot *customPlot;
	QWidget *plotWidget;
	QHBoxLayout *plotLayout;
	EventsWidget *eventsWidget;
	InfoWidget *infoWidget;
	QString traceFile;

	void createActions();
	void createToolBars();
	void createMenus();
	void loadTraceFile(QString &);

	QMenu *fileMenu;
	QMenu *viewMenu;
	QMenu *helpMenu;

	QAction *openAction;
	QAction *closeAction;
	QAction *exitAction;
	QAction *aboutAction;
	QAction *licenseAction;
	QAction *aboutQtAction;

	FtraceParser *parser;
	WorkItem<FtraceParser> *schedItem;
	WorkItem<FtraceParser> *migItem;
	WorkItem<FtraceParser> *freqItem;
	WorkQueue *workQueue;

	const double graphSpacing = 100;
	const double schedSectionSpace = 100;
	const double schedSpacing = 100;
	const double schedHeight = 1000;
	const double cpuSectionOffset = 100;
	const double cpuSpacing = 100;
	const double cpuHeight = 1000;
	const double cpuIdleSkew = 0;
	const double migrateHeight = 5000;
	const double migratSectionOffset = 100;

	double bottom;
	double top;
	QVector<double> ticks;
	QVector<QString> tickLabels;
	Cursor *cursors[2];
	int cursorIdx;
};

#endif /* MAINWINDOW_H */
