/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QHBoxLayout>
#include <QMainWindow>
#include <QVector>
#include <QString>
#include "misc/setting.h"
#include "misc/traceshark.h"
#include "parser/traceevent.h"
#include "threads/workitem.h"

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QMouseEvent;
class QToolBar;
template<class T, class U> class QMap;
QT_END_NAMESPACE

class TraceAnalyzer;
class EventsWidget;
class InfoWidget;
class Cursor;
class CPUTask;
class ErrorDialog;
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
class EventSelectDialog;
class YAxisTicker;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();
	void openFile(const QString &name);
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
	void infoValueChanged(vtl::Time value, int nr);
	void moveActiveCursor(vtl::Time time);
	void showEventInfo(const TraceEvent &event);
	void selectionChanged();
	void plottableClicked(QCPAbstractPlottable *plottable, int dataIndex,
			      QMouseEvent *event);
	void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem
			       *abstractItem);
	void addTaskGraph(int pid);
	void addTaskToLegend(int pid);
	void removeTaskGraph(int pid);
	void showTaskSelector();
	void showEventFilter();
	void showWakeup(int pid);
	void createPidFilter(QMap<int, int> &map,
			     bool orlogic, bool inclusive);
	void createEventFilter(QMap<event_t, event_t> &map, bool orlogic);
	void resetPidFilter();
	void resetEventFilter();
	void resetFilters();
	void timeFilter();
	void exportEvents();
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
	void updateResetFiltersEnabled();

	void addSchedGraph(CPUTask &task);
	void addHorizontalWakeupGraph(CPUTask &task);
	void addWakeupGraph(CPUTask &task);
	void addPreemptedGraph(CPUTask &task);
	void addStillRunningGraph(CPUTask &task);

	void setTraceActionsEnabled(bool e);
	void setEventsWidgetEvents();
	void scrollTo(const vtl::Time &time);

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

	void setStatus(status_t status, const QString *fileName = nullptr);
	int loadTraceFile(const QString &);

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
	QAction *showEventsAction;
	QAction *timeFilterAction;
	QAction *resetFiltersAction;
	QAction *exportEventsAction;
	QAction *aboutAction;
	QAction *licenseAction;
	QAction *aboutQtAction;
	QAction *aboutQCPAction;

	TraceAnalyzer *analyzer;

	ErrorDialog *errorDialog;
	LicenseDialog *licenseDialog;
	EventInfoDialog *eventInfoDialog;
	TaskSelectDialog *taskSelectDialog;
	EventSelectDialog *eventSelectDialog;

	const double bugWorkAroundOffset = 100;
	const double schedSectionOffset = 100;
	const double schedSpacing = 250;
	const double schedHeight = 950;
	const double cpuSectionOffset = 100;
	const double cpuSpacing = 100;
	const double cpuHeight = 800;
	/*
	 * const double migrateHeight doesn't exist. The value used is the
	 * dynamically calculated inc variable in MainWindow::computeLayout()
	 */

	const double migrateSectionOffset = 250;

	double bottom;
	double top;
	QVector<double> ticks;
	QVector<QString> tickLabels;
	Cursor *cursors[TShark::NR_CURSORS];
	Setting settings[Setting::MAX_SETTINGS];
	bool filterActive;
	double cursorPos[TShark::NR_CURSORS];
};

#endif /* MAINWINDOW_H */
