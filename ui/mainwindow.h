// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileDialog>
#include <QMainWindow>
#include <QMap>
#include <QVector>
#include <QString>
#include "analyzer/traceanalyzer.h"
#include "misc/setting.h"
#include "misc/traceshark.h"
#include "parser/traceevent.h"
#include "threads/workitem.h"
#include "ui/eventsmodel.h"

#ifdef CONFIG_SYSTEM_QCUSTOMPLOT
	/*
	 * Most distros probably build QCustomPlot without OpenGL suppport so we
	 * provide the user with a reasonable guess.
	 */
#define qcp_warn_failed_opengl_enable()					\
	vtl::warnx(							\
"QCustomPlot failed to enable OpenGL. Perhaps the QCustomPlot library\n"\
"on your system was compiled without QCUSTOMPLOT_USE_OPENGL.\n")
#else
	/*
	 * In this case we are using the internal QCustomPlot so we have no idea
	 * what the problem is.
	 */
#define qcp_warn_failed_opengl_enable() \
	vtl::warnx("QCustomPlot failed to enable OpenGL.\n")
#endif

#define qcp_warn_failed_opengl_disable() \
	vtl::warnx("QCustomPlot failed to disable OpenGL.\n")

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QPlainTextEdit;
class QMessageBox;
class QMouseEvent;
class QScrollBar;
class QToolBar;
class QVBoxLayhout;
QT_END_NAMESPACE

class TraceAnalyzer;
class EventsWidget;
class InfoWidget;
class Cursor;
class CPUTask;
class ErrorDialog;
class GraphEnableDialog;
class LatencyWidget;
class LicenseDialog;
class EventInfoDialog;
class QCPAbstractPlottable;
class QCPGraph;
class QCPLayer;
class QCPLegend;
class QCustomPlot;
class QCPAbstractLegendItem;
class RegexDialog;
class RegexFilter;
class SettingStore;
class TaskToolBar;
class TracePlot;
class TraceEvent;
class TaskRangeAllocator;
class TaskSelectDialog;
class EventSelectDialog;
class CPUSelectDialog;
class YAxisTicker;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	virtual ~MainWindow();
	void openFile(const QString &name);
	void resizeEvent(QResizeEvent *event);
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
	void yAxisSelectionChange(const QCPAxis::SelectableParts &parts);
	void configureScrollBar();
	void scrollBarChanged(int value);
	void yAxisChanged(QCPRange range);
	void plotDoubleClicked(QMouseEvent *event);
	void infoValueChanged(vtl::Time value, int nr);
	void moveActiveCursor(vtl::Time time);
	void moveCursor(vtl::Time time, int cursorIdx);
	void handleEventDoubleClicked(EventsModel::column_t col,
				      const TraceEvent &event);
	void taskTriggered(int pid);
	void showLatency(const Latency *latency);
	void handleEventSelected(const TraceEvent *event);
	void handleEventChanged(bool selected);
	void selectionChanged();
	void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem
			       *abstractItem);
	void legendEmptyChanged(bool empty);
	void addTaskGraph(int pid);
	void doReplot();
	void doLegendCheck();
	void addTaskToLegend(int pid);
	void removeTaskGraph(int pid);
	void cursorZoom();
	void defaultZoom();
	void fullZoom();
	void verticalZoom();
	void showSchedLatencyWidget();
	void showWakeupLatencyWidget();
	void showLatencyWidget(LatencyWidget *lwidget,
			       Qt::DockWidgetArea area);
	void showTaskSelector();
	void filterOnCPUs();
	void showArgFilter();
	void showEventFilter();
	void showGraphEnable();
	void showWakeupOrWaking(int pid, event_t wakevent);
	void showWaking(const TraceEvent *event);
	void createPidFilter(QMap<int, int> &map,
			     bool orlogic, bool inclusive);
	void createCPUFilter(QMap<unsigned, unsigned> &map, bool orlogic);
	void createEventFilter(QMap<event_t, event_t> &map, bool orlogic);
	void createRegexFilter(RegexFilter &regexFilter, bool orlogic);
	void resetPidFilter();
	void resetCPUFilter();
	void resetEventFilter();
	void resetRegexFilter();
	void resetFilters();
	void timeFilter();
	void exportEvents(TraceAnalyzer::exporttype_t export_type);
	void exportEventsTriggered();
	void exportCPUTriggered();
	void exportSchedLatencies(int format);
	void exportWakeupLatencies(int format);
	void consumeSettings();
	void consumeFilterSettings();
	void consumeSizeChange();
	void transmitSize();
	void showStats();
	void showStatsTimeLimited();
	void removeQDockWidget(QDockWidget *widget);
	void taskFilter();

	void addTaskGraphTriggered();
	void addToLegendTriggered();
	void clearLegendTriggered();
	void findSleepTriggered();
	void findWakeupTriggered();
	void findWakingTriggered();
	void findWakingDirectTriggered();
	void removeTaskGraphTriggered();
	void clearTaskGraphsTriggered();
	void taskFilterTriggered();
	void taskFilterLimitedTriggered();
	void showBackTraceTriggered();
	void eventCPUTriggered();
	void eventTypeTriggered();
	void eventPIDTriggered();
	void eventMoveBlueTriggered();
	void eventMoveRedTriggered();

private:
	typedef enum : int {
		STATUS_NOFILE = 0,
		STATUS_FILE,
		STATUS_ERROR,
		STATUS_NR
	} status_t;

	typedef enum : int {
		PR_CPUGRAPH_ONLY,
		PR_TRY_TASKGRAPH
	} preference_t;

	/* Helper functions for the constructor */
	void createAboutBox();
	void createAboutQCustomPlot();
	void createActions();
	void createToolBars();
	void createMenus();
	void createTracePlot();
	void createScrollBar();
	void createStatusBar();
	void createDialogs();
	void createEventCPUFilter(const TraceEvent &event);
	void createEventPIDFilter(const TraceEvent &event);
	void createEventTypeFilter(const TraceEvent &event);
	void plotConnections();
	void widgetConnections();
	void dialogConnections();
	void loadSettings();

	/* Functions for opening and processing a trace*/
	void processTrace();
	void computeLayout();
	void computeStats();
	void rescaleTrace();
	void clearPlot();
	void showTrace();
	double adjustScatterSize(double defsize, int linewidth);
	double maxZoomVSize();
	double autoZoomVSize();
	int loadTraceFile(const QString &);
	void setStatus(status_t status, const QString *fileName = nullptr);

	/* The rest of the functions */
	void setupCursors();
	void setupCursors(const double &red, const double &blue);
	void setupCursors(const vtl::Time &redtime, const vtl::Time &bluetime);
	void setupCursors_(vtl::Time redtime, const double &red,
			   vtl::Time bluetime, const double &blue);
	void updateResetFiltersEnabled();
	void addSchedGraph(CPUTask &task, unsigned int cpu);
	void addHorizontalWakeupGraph(CPUTask &task);
	void addWakeupGraph(CPUTask &task);
	void addPreemptedGraph(CPUTask &task);
	void addStillRunningGraph(CPUTask &task);
	void addUninterruptibleGraph(CPUTask &task);
	void addGenericAccessoryGraph(const QString &name,
				      const QVector<double> &timev,
				      const QVector<double> &scaledData,
				      QCPScatterStyle::ScatterShape sshape,
				      double size,
				      const QColor &color);
	void addAccessoryTaskGraph(QCPGraph **graphPtr, const QString &name,
				   const QVector<double> &timev,
				   const QVector<double> &scaledData,
				   QCPScatterStyle::ScatterShape sshape,
				   double size, const QColor &color);
	void addStillRunningTaskGraph(Task *task);
	void addPreemptedTaskGraph(Task *task);
	void addUninterruptibleTaskGraph(Task *task);
	void resetFilter(FilterState::filter_t filter);
	void setTraceActionsEnabled(bool e);
	void setLegendActionsEnabled(bool e);
	void setCloseActionsEnabled(bool e);
	void setTaskActionsEnabled(bool e);
	void setWakeupActionsEnabled(bool e);
	void setAddTaskGraphActionEnabled(bool e);
	void setTaskGraphRemovalActionEnabled(bool e);
	void setTaskGraphClearActionEnabled(bool e);
	void setAddToLegendActionEnabled(bool e);
	void setEventActionsEnabled(bool e);
	void setEventsWidgetEvents();
	void scrollTo(const vtl::Time &time);
	void exportLatencies(TraceAnalyzer::exportformat_t format,
			     TraceAnalyzer::latencytype_t type);
	void handleLegendGraphDoubleClick(QCPGraph *legendGraph);
	void handleWakeUpChanged(bool selected);
	void checkStatsTimeLimited();
	bool selectQCPGraph(QCPGraph *graph);
	void selectTaskByPid(int pid, const unsigned int *preferred_cpu,
			     preference_t preference);
	bool isOpenGLEnabled();
	void setupOpenGL();
	void updateTaskGraphActions();
	void updateAddToLegendAction();
	TaskGraph *selectedGraph();

	TracePlot *tracePlot;
	QScrollBar *scrollBar;
	bool scrollBarUpdate;
	YAxisTicker *yaxisTicker;
	TaskRangeAllocator *taskRangeAllocator;
	QCPLayer *cursorLayer;
	QWidget *plotWidget;
	QHBoxLayout *plotLayout;
	EventsWidget *eventsWidget;
	InfoWidget *infoWidget;
	QString traceFile;

	QMenu *fileMenu;
	QMenu *viewMenu;
	QMenu *helpMenu;
	QMenu *taskMenu;
	QMenu *eventMenu;

	QToolBar *fileToolBar;
	QToolBar *viewToolBar;
	TaskToolBar *taskToolBar;

	QLabel *statusLabel;
	QString *statusStrings[STATUS_NR];

	QAction *openAction;
	QAction *closeAction;
	QAction *saveAction;
	QAction *exitAction;
	QAction *cursorZoomAction;
	QAction *defaultZoomAction;
	QAction *fullZoomAction;
	QAction *verticalZoomAction;
	QAction *showTasksAction;
	QAction *showSchedLatencyAction;
	QAction *showWakeupLatencyAction;
	QAction *filterCPUsAction;
	QAction *showEventsAction;
	QAction *showArgFilterAction;
	QAction *timeFilterAction;
	QAction *graphEnableAction;
	QAction *resetFiltersAction;
	QAction *exportEventsAction;
	QAction *exportCPUAction;
	QAction *showStatsAction;
	QAction *showStatsTimeLimitedAction;

	QAction *backTraceAction;
	QAction *eventCPUAction;
	QAction *eventTypeAction;
	QAction *eventPIDAction;
	QAction *moveBlueAction;
	QAction *moveRedAction;

	QAction *aboutAction;
	QAction *licenseAction;
	QAction *aboutQtAction;
	QAction *aboutQCPAction;

	QAction *addTaskGraphAction;
	QAction *addToLegendAction;
	QAction *clearLegendAction;
	QAction *findSleepAction;
	QAction *findWakeupAction;
	QAction *findWakingAction;
	QAction *findWakingDirectAction;
	QAction *removeTaskGraphAction;
	QAction *clearTaskGraphsAction;
	QAction *taskFilterAction;
	QAction *taskFilterLimitedAction;

	TraceAnalyzer *analyzer;

	ErrorDialog *errorDialog;
	LicenseDialog *licenseDialog;
	EventInfoDialog *eventInfoDialog;
	TaskSelectDialog *taskSelectDialog;
	TaskSelectDialog *statsDialog;
	TaskSelectDialog *statsLimitedDialog;
	LatencyWidget *schedLatencyWidget;
	LatencyWidget *wakeupLatencyWidget;
	EventSelectDialog *eventSelectDialog;
	CPUSelectDialog *cpuSelectDialog;
	GraphEnableDialog *graphEnableDialog;
	RegexDialog *regexDialog;

	static const double bugWorkAroundOffset;
	static const double schedSectionOffset;
	static const double schedSpacing;
	static const double schedHeight;
	static const double cpuSectionOffset;
	static const double cpuSpacing;
	static const double cpuHeight;
	static const double pixelZoomFactor;
	static const double refDpiY;
	/*
	 * const double migrateHeight doesn't exist. The value used is the
	 * dynamically calculated inc variable in MainWindow::computeLayout()
	 */

	static const double migrateSectionOffset;

	static const QString RUNNING_NAME;
	static const QString PREEMPTED_NAME;
	static const QString UNINT_NAME;

	static const double RUNNING_SIZE;
	static const double PREEMPTED_SIZE;
	static const double UNINT_SIZE;
	static const double CPUIDLE_SIZE;

	static const QCPScatterStyle::ScatterShape RUNNING_SHAPE;
	static const QCPScatterStyle::ScatterShape PREEMPTED_SHAPE;
	static const QCPScatterStyle::ScatterShape UNINT_SHAPE;
	static const QCPScatterStyle::ScatterShape CPUIDLE_SHAPE;

	static const QColor RUNNING_COLOR;
	static const QColor PREEMPTED_COLOR;
	static const QColor UNINT_COLOR;

	double bottom;
	double top;
	double startTime;
	double endTime;
	QVector<double> ticks;
	QVector<QString> tickLabels;
	Cursor *cursors[TShark::NR_CURSORS];
	SettingStore *settingStore;
	bool filterActive;
	double cursorPos[TShark::NR_CURSORS];
	QMap<unsigned, unsigned> eventCPUMap;
	QMap<int, int> eventPIDMap;
	QMap<event_t, event_t> eventTypeMap;
	QMessageBox *aboutBox;
	QMessageBox *aboutQCPBox;
	QFileDialog::Options foptions;
};

#endif /* MAINWINDOW_H */
