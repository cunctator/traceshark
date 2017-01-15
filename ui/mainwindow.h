/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
#include "misc/setting.h"
#include "threads/workitem.h"
#include "misc/traceshark.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QMouseEvent;
class QToolBar;
QT_END_NAMESPACE

class TraceAnalyzer;
class EventsWidget;
class InfoWidget;
class Cursor;
class CPUTask;
class LicenseDialog;
class EventInfoDialog;
class QCPAbstractPlottable;
class QCPGraph;
class QCPLayer;
class QCPLegend;
class QCustomPlot;
class QCPAbstractLegendItem;
class TracePlot;
class TraceEvent;
class TaskRangeAllocator;
class TaskSelectDialog;
class YAxisTicker;

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
	void saveScreenshot();
	void about();
	void aboutQCustomPlot();
	void license();
	void mouseWheel();
	void mousePress();
	void plotDoubleClicked(QMouseEvent *event);
	void infoValueChanged(double value, int nr);
	void moveActiveCursor(double time);
	void showEventInfo(const TraceEvent &event);
	void selectionChanged();
	void plottableClicked(QCPAbstractPlottable *plottable, int dataIndex,
			      QMouseEvent *event);
	void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem
			       *abstractItem);
	void addTaskGraph(unsigned int pid);
	void addTaskToLegend(unsigned int pid);
	void removeTaskGraph(unsigned int pid);
	void showTaskSelector();
	void showWakeup(unsigned int pid);
private:
	typedef enum {
		STATUS_NOFILE = 0,
		STATUS_FILE,
		STATUS_ERROR,
		STATUS_NR
	} status_t;

	void processTrace();
	void computeLayout();
	void rescaleTrace();
	void clearPlot();
	void showTrace();
	void setupCursors();
	void setupSettings();

	void addSchedGraph(CPUTask &task);
	void addHorizontalWakeupGraph(CPUTask &task);
	void addWakeupGraph(CPUTask &task);
	void addPreemptedGraph(CPUTask &task);
	void addStillRunningGraph(CPUTask &task);

	void setTraceActionsEnabled(bool e);

	TracePlot *tracePlot;
	YAxisTicker *yaxisTicker;
	TaskRangeAllocator *taskRangeAllocator;
	QCPLayer *cursorLayer;
	QWidget *plotWidget;
	QVBoxLayout *plotLayout;
	EventsWidget *eventsWidget;
	InfoWidget *infoWidget;
	QString traceFile;

	void createActions();
	void createToolBars();
	void createMenus();
	void createTracePlot();
	void createStatusBar();

	void setStatus(status_t status, QString *fileName = nullptr);
	void loadTraceFile(QString &);

	QMenu *fileMenu;
	QMenu *viewMenu;
	QMenu *helpMenu;

	QToolBar *fileToolBar;
	QToolBar *viewToolBar;

	QLabel *statusLabel;
	QString *statusStrings[STATUS_NR];

	QAction *openAction;
	QAction *closeAction;
	QAction *saveAction;
	QAction *exitAction;
	QAction *showTasksAction;
	QAction *aboutAction;
	QAction *licenseAction;
	QAction *aboutQtAction;
	QAction *aboutQCPAction;

	TraceAnalyzer *analyzer;

	LicenseDialog *licenseDialog;
	EventInfoDialog *eventInfoDialog;
	TaskSelectDialog *taskSelectDialog;

	const double bugWorkAroundOffset = 100;
	const double schedSectionOffset = 100;
	const double schedSpacing = 250;
	const double schedHeight = 950;
	const double cpuSectionOffset = 100;
	const double cpuSpacing = 100;
	const double cpuHeight = 800;
	/* const double migrateHeight doesn't exist. The value used is the
	 * dynamically calculated inc variable in MainWindow::computeLayout()*/
	const double migrateSectionOffset = 250;

	double bottom;
	double top;
	QVector<double> ticks;
	QVector<QString> tickLabels;
	Cursor *cursors[TShark::NR_CURSORS];
	Setting settings[Setting::MAX_SETTINGS];
};

#endif /* MAINWINDOW_H */
