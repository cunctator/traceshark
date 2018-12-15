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

#include <cstdio>

#include <QApplication>
#include <QDateTime>
#include <QToolBar>

#include "ui/cursor.h"
#include "ui/eventinfodialog.h"
#include "ui/eventswidget.h"
#include "analyzer/traceanalyzer.h"
#include "ui/errordialog.h"
#include "ui/graphenabledialog.h"
#include "ui/infowidget.h"
#include "ui/licensedialog.h"
#include "ui/mainwindow.h"
#include "ui/migrationline.h"
#include "ui/taskgraph.h"
#include "ui/taskrangeallocator.h"
#include "ui/taskselectdialog.h"
#include "ui/tasktoolbar.h"
#include "ui/eventselectdialog.h"
#include "parser/traceevent.h"
#include "ui/traceplot.h"
#include "ui/yaxisticker.h"
#include "misc/errors.h"
#include "misc/resources.h"
#include "misc/traceshark.h"
#include "threads/workqueue.h"
#include "threads/workitem.h"
#include "qcustomplot/qcustomplot.h"
#include "vtl/compiler.h"
#include "vtl/error.h"


#define TOOLTIP_OPEN			\
"Open a new trace file"

#define TOOLTIP_CLOSE			\
"Close the currently open tracefile"

#define TOOLTIP_SAVESCREEN		\
"Take a screenshot of the current graph and save it to a file"

#define TOOLTIP_EXIT			\
"Exit traceshark"

#define TOOLTIP_SHOWTASKS		\
"Show a list of all tasks and it's possible to select one"

#define TOOLTIP_SHOWEVENTS		\
"Show a list of event types and it's possible to select which to filter on"

#define TOOLTIP_TIMEFILTER		\
"Filter on the time interval specified by the current position of the cursors"

#define TOOLTIP_GRAPHENABLE		\
"Select which types of graphs should be enabled"

#define TOOLTIP_RESETFILTERS		\
"Reset all filters"

#define TOOLTIP_EXPORTEVENTS		\
"Export the filtered events"

#define TOOLTIP_EXPORT_CPU		\
"Export cycles/cpu-cycles events"

#define TOOLTIP_GETSTATS		\
"Show the statistics dialog"

#define TOOLTIP_GETSTATS_TIMELIMITED	\
"Show the dialog with statistics that are time limited by the cursors"

#define TOOLTIP_FIND_SLEEP		\
"Find the next sched_switch event that puts the selected task to sleep"

#define FIND_WAKEUP_TOOLTIP		\
"Find the wakeup of the selected task that precedes the active cursor"

#define FIND_WAKING_TOOLTIP		\
"Find the waking event that precedes this wakeup event"

#define FIND_WAKING_DIRECT_TOOLTIP	\
"Find the waking event of the selected task that precedes the active cursor"

#define REMOVE_TASK_TOOLTIP		\
"Remove the unified graph for this task"

#define TASK_FILTER_TOOLTIP		\
"Filter on the selected task"

#define TASK_FILTER_TIMELIMIT_TOOLTIP	\
"Filter on the selected task and time limited by the cursors"

#define ADD_UNIFIED_TOOLTIP		\
"Add a unified graph for this task"

#define ADD_LEGEND_TOOLTIP		\
"Add this task to the legend"

#define CLEAR_LEGEND_TOOLTIP		\
"Remove all tasks from the legend"

#define ABOUT_QT_TOOLTIP		\
"Show info about Qt"

#define ABOUT_TSHARK_TOOLTIP		\
"Show info about Traceshark"

#define SHOW_QCP_TOOLTIP		\
"Show info about QCustomPlot"

#define SHOW_LICENSE_TOOLTIP		\
"Show the license of Traceshark"

MainWindow::MainWindow():
	tracePlot(nullptr), filterActive(false)
{
	analyzer = new TraceAnalyzer;

	infoWidget = new InfoWidget(this);
	infoWidget->setAllowedAreas(Qt::TopDockWidgetArea |
				    Qt::BottomDockWidgetArea);
	addDockWidget(Qt::TopDockWidgetArea, infoWidget);

	createActions();
	createToolBars();
	createMenus();
	createStatusBar();

	plotWidget = new QWidget(this);
	plotLayout = new QVBoxLayout(plotWidget);
	setCentralWidget(plotWidget);

	/* createTracePlot needs to have plotWidget created */
	createTracePlot();

	tsconnect(tracePlot, mouseWheel(QWheelEvent*), this, mouseWheel());
	tsconnect(tracePlot->xAxis, rangeChanged(QCPRange), tracePlot->xAxis2,
		  setRange(QCPRange));
	tsconnect(tracePlot, mousePress(QMouseEvent*), this, mousePress());
	tsconnect(tracePlot,selectionChangedByUser() , this,
		  selectionChanged());
	tsconnect(tracePlot, plottableClick(QCPAbstractPlottable *, int,
					    QMouseEvent *), this,
		  plottableClicked(QCPAbstractPlottable *, int, QMouseEvent *));
	tsconnect(tracePlot, legendDoubleClick(QCPLegend*,
					       QCPAbstractLegendItem*,
					       QMouseEvent*), this,
		  legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*));
	eventsWidget = new EventsWidget(this);
	eventsWidget->setAllowedAreas(Qt::TopDockWidgetArea |
				      Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, eventsWidget);

	cursors[TShark::RED_CURSOR] = nullptr;
	cursors[TShark::BLUE_CURSOR] = nullptr;

	setupSettings();

	errorDialog = new ErrorDialog();
	licenseDialog = new LicenseDialog();
	eventInfoDialog = new EventInfoDialog();
	taskSelectDialog =
		new TaskSelectDialog(nullptr, tr("Task Selector"),
				     TaskSelectDialog::TaskSelectRegular);
	statsDialog = new TaskSelectDialog(nullptr, tr("Global Statistics"),
					   TaskSelectDialog::TaskSelectStats);
	statsLimitedDialog =
		new TaskSelectDialog(nullptr, tr("Cursor Statistics"),
				     TaskSelectDialog::TaskSelectStatsLimited);

	taskSelectDialog->setAllowedAreas(Qt::LeftDockWidgetArea);
	statsDialog->setAllowedAreas(Qt::LeftDockWidgetArea);
	statsLimitedDialog->setAllowedAreas(Qt::RightDockWidgetArea);

	eventSelectDialog = new EventSelectDialog();
	graphEnableDialog = new GraphEnableDialog();

	vtl::set_error_handler(errorDialog);

	tsconnect(tracePlot, mouseDoubleClick(QMouseEvent*),
		  this, plotDoubleClicked(QMouseEvent*));
	tsconnect(infoWidget, valueChanged(vtl::Time, int),
		  this, infoValueChanged(vtl::Time, int));

	/* Events widget */
	tsconnect(eventsWidget, timeSelected(vtl::Time), this,
		  moveActiveCursor(vtl::Time));
	tsconnect(eventsWidget, infoDoubleClicked(const TraceEvent &),
		  this, showEventInfo(const TraceEvent &));
	tsconnect(eventsWidget, eventSelected(const TraceEvent *),
		  this, handleEventSelected(const TraceEvent *));

	/* task select dialog */
	tsconnect(taskSelectDialog, addTaskGraph(int), this, addTaskGraph(int));
	tsconnect(taskSelectDialog, addTaskToLegend(int), this,
		  addTaskToLegend(int));
	tsconnect(taskSelectDialog, createFilter(QMap<int, int> &, bool, bool),
		  this, createPidFilter(QMap<int, int> &, bool, bool));
	tsconnect(taskSelectDialog, resetFilter(), this, resetPidFilter());
	tsconnect(taskSelectDialog, QDockWidgetNeedsRemoval(QDockWidget*),
		  this, removeQDockWidget(QDockWidget*));
	tsconnect(taskSelectDialog, taskDoubleClicked(int),
		  this, taskTriggered(int));

	/* statistics Dialog */
	tsconnect(statsDialog, addTaskGraph(int), this, addTaskGraph(int));
	tsconnect(statsDialog, addTaskToLegend(int), this,
		  addTaskToLegend(int));
	tsconnect(statsDialog, createFilter(QMap<int, int> &, bool, bool),
		  this, createPidFilter(QMap<int, int> &, bool, bool));
	tsconnect(statsDialog, resetFilter(), this, resetPidFilter());
	tsconnect(statsDialog, QDockWidgetNeedsRemoval(QDockWidget*),
		  this, removeQDockWidget(QDockWidget*));
	tsconnect(statsDialog, taskDoubleClicked(int),
		  this, taskTriggered(int));

	/* Time limited statistics Dialog */
	tsconnect(statsLimitedDialog, addTaskGraph(int), this,
		  addTaskGraph(int));
	tsconnect(statsLimitedDialog, addTaskToLegend(int), this,
		  addTaskToLegend(int));
	tsconnect(statsLimitedDialog,
		  createFilter(QMap<int, int> &, bool, bool),
		  this, createPidFilter(QMap<int, int> &, bool, bool));
	tsconnect(statsLimitedDialog, resetFilter(), this, resetPidFilter());
	tsconnect(statsLimitedDialog, QDockWidgetNeedsRemoval(QDockWidget*),
		  this, removeQDockWidget(QDockWidget*));
	tsconnect(statsLimitedDialog, taskDoubleClicked(int),
		  this, taskTriggered(int));

	/* event select dialog */
	tsconnect(eventSelectDialog, createFilter(QMap<event_t, event_t> &,
						  bool),
		  this, createEventFilter(QMap<event_t, event_t> &, bool));
	tsconnect(eventSelectDialog, resetFilter(), this, resetEventFilter());

	/* graph enable dialog */
	tsconnect(graphEnableDialog, settingsChanged(),
		  this, consumeSettings());

	cursorPos[TShark::RED_CURSOR] = 0;
	cursorPos[TShark::BLUE_CURSOR] = 0;
}

void MainWindow::createTracePlot()
{
	QString mainLayerName = QString("main");
	QString cursorLayerName = QString("cursor");
	QCPLayer *mainLayer;
	yaxisTicker = new YAxisTicker();
	QSharedPointer<QCPAxisTicker> ticker((QCPAxisTicker*) (yaxisTicker));

	tracePlot = new TracePlot(plotWidget);
#ifdef QCUSTOMPLOT_USE_OPENGL
	tracePlot->setOpenGl(true, 16);
#endif /* QCUSTOMPLOT_USE_OPENGL */

	tracePlot->yAxis->setTicker(ticker);
	taskRangeAllocator = new TaskRangeAllocator(schedHeight
						    + schedSpacing);
	taskRangeAllocator->setStart(bugWorkAroundOffset);

	mainLayer = tracePlot->layer(mainLayerName);

	tracePlot->addLayer(cursorLayerName, mainLayer, QCustomPlot::limAbove);
	cursorLayer = tracePlot->layer(cursorLayerName);

	tracePlot->setCurrentLayer(mainLayerName);

	tracePlot->setAutoAddPlottableToLegend(false);
	tracePlot->hide();
	plotLayout->addWidget(tracePlot);

	tracePlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
				   QCP::iSelectAxes | QCP::iSelectLegend |
				   QCP::iSelectPlottables);

	analyzer->setQCustomPlot(tracePlot);
}

MainWindow::~MainWindow()
{
	int i;

	closeTrace();
	delete analyzer;
	delete tracePlot;
	delete taskRangeAllocator;
	delete licenseDialog;
	delete eventInfoDialog;
	delete taskSelectDialog;
	delete statsDialog;
	delete statsLimitedDialog;
	delete eventSelectDialog;
	delete graphEnableDialog;

	for (i = 0; i < STATUS_NR; i++)
		delete statusStrings[i];
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	/* Here is a great place to save settings, if we ever want to do it */
	taskSelectDialog->hide();
	eventSelectDialog->hide();
	statsDialog->hide();
	statsLimitedDialog->hide();
	event->accept();
	/* event->ignore() could be used to refuse to close the window */
}

void MainWindow::openTrace()
{
	QString name = QFileDialog::getOpenFileName(this);
	if (!name.isEmpty()) {
		openFile(name);
	}
}

void MainWindow::openFile(const QString &name)
{
	int ts_errno;

	if (analyzer->isOpen())
		closeTrace();
	ts_errno = loadTraceFile(name);

	if (ts_errno != 0) {
		vtl::warn(ts_errno, "Failed to open trace file %s",
			  name.toLocal8Bit().data());
		return;
	}

	if (analyzer->isOpen()) {
		quint64 start, process, layout, rescale, showt, eventsw;
		quint64 scursor, tshow;

		clearPlot();
		start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		processTrace();
		process = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		computeLayout();
		layout = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		eventsWidget->beginResetModel();
		eventsWidget->setEvents(analyzer->events);
		eventsWidget->endResetModel();

		taskSelectDialog->beginResetModel();
		taskSelectDialog->setTaskMap(&analyzer->taskMap,
					     analyzer->getNrCPUs());
		taskSelectDialog->endResetModel();

		eventSelectDialog->beginResetModel();
		eventSelectDialog->setStringTree(TraceEvent::getStringTree());
		eventSelectDialog->endResetModel();

		eventsw = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		setupCursors();
		scursor = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		rescaleTrace();
		rescale = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		computeStats();
		statsDialog->beginResetModel();
		statsDialog->setTaskMap(&analyzer->taskMap,
					analyzer->getNrCPUs());
		statsDialog->endResetModel();

		statsLimitedDialog->beginResetModel();
		statsLimitedDialog->setTaskMap(&analyzer->taskMap,
					       analyzer->getNrCPUs());
		statsLimitedDialog->endResetModel();

		showTrace();
		showt = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		tracePlot->show();
		tshow = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

		setStatus(STATUS_FILE, &name);

		printf("processTrace() took %.6lf s\n"
		       "computeLayout() took %.6lf s\n"
		       "updating EventsWidget took %.6lf s\n"
		       "setupCursors() took %.6lf s\n"
		       "rescaleTrace() took %.6lf s\n"
		       "showTrace() took %.6lf s\n"
		       "tracePlot->show() took %.6lf s\n",
		       (double) (process - start) / 1000,
		       (double) (layout - process) / 1000,
		       (double) (eventsw - layout) / 1000,
		       (double) (scursor - eventsw) / 1000,
		       (double) (rescale - scursor) / 1000,
		       (double) (showt - rescale) / 1000,
		       (double) (tshow - showt) / 1000);
		fflush(stdout);
		tracePlot->legend->setVisible(true);
		setTraceActionsEnabled(true);
	} else
		setStatus(STATUS_ERROR);
}

void MainWindow::processTrace()
{
	analyzer->processTrace();
}

void MainWindow::computeLayout()
{
	unsigned int cpu;
	unsigned int nrCPUs;
	unsigned int offset;
	QString label;
	double inc, o, p;
	double start, end;
	QColor color;

	start = analyzer->getStartTime().toDouble();
	end = analyzer->getEndTime().toDouble();

	bottom = bugWorkAroundOffset;
	offset = bottom;
	nrCPUs = analyzer->getNrCPUs();
	ticks.resize(0);
	tickLabels.resize(0);

	if (Setting::isEnabled(Setting::SHOW_MIGRATION_GRAPHS)) {
		offset += migrateSectionOffset;

		analyzer->setMigrationOffset(offset);
		inc = nrCPUs * 315 + 67.5;
		analyzer->setMigrationScale(inc);

		/* add labels and lines here for the migration graph */
		color = QColor(135, 206, 250); /* Light sky blue */
		label = QString("fork/exit");
		ticks.append(offset);
		new MigrationLine(start, end, offset, color, tracePlot);
		tickLabels.append(label);
		o = offset;
		p = inc / nrCPUs ;
		for (cpu = 0; cpu < nrCPUs; cpu++) {
			o += p;
			label = QString("cpu") + QString::number(cpu);
			ticks.append(o);
			tickLabels.append(label);
			new MigrationLine(start, end, o, color, tracePlot);
		}

		offset += inc;
		offset += p;
	}

	if (Setting::isEnabled(Setting::SHOW_SCHED_GRAPHS)) {
		offset += schedSectionOffset;

		/* Set the offset and scale of the scheduling graphs */
		for (cpu = 0; cpu < nrCPUs; cpu++) {
			analyzer->setSchedOffset(cpu, offset);
			analyzer->setSchedScale(cpu, schedHeight);
			label = QString("cpu") + QString::number(cpu);
			ticks.append(offset);
			tickLabels.append(label);
			offset += schedHeight + schedSpacing;
		}
	}

	if (Setting::isEnabled(Setting::SHOW_CPUFREQ_GRAPHS) ||
	    Setting::isEnabled(Setting::SHOW_CPUIDLE_GRAPHS)) {
		offset += cpuSectionOffset;

		for (cpu = 0; cpu < nrCPUs; cpu++) {
			analyzer->setCpuFreqOffset(cpu, offset);
			analyzer->setCpuIdleOffset(cpu, offset);
			analyzer->setCpuFreqScale(cpu, cpuHeight);
			analyzer->setCpuIdleScale(cpu, cpuHeight);
			label = QString("cpu") + QString::number(cpu);
			ticks.append(offset);
			tickLabels.append(label);
			offset += cpuHeight + cpuSpacing;
		}
	}

	top = offset;
}

void MainWindow::rescaleTrace()
{
	analyzer->doScale();
}

void MainWindow::computeStats()
{
	analyzer->doStats();
}

void MainWindow::clearPlot()
{
	cursors[TShark::RED_CURSOR] = nullptr;
	cursors[TShark::BLUE_CURSOR] = nullptr;
	tracePlot->clearItems();
	tracePlot->clearPlottables();
	tracePlot->hide();
	TaskGraph::clearMap();
	taskRangeAllocator->clearAll();
	infoWidget->setTime(0, TShark::RED_CURSOR);
	infoWidget->setTime(0, TShark::BLUE_CURSOR);
}

void MainWindow::showTrace()
{
	unsigned int cpu;
	double start, end;
	int precision = 7;
	double extra = 0;
	QColor color;

	start = analyzer->getStartTime().toDouble();
	end = analyzer->getEndTime().toDouble();

	if (end >= 10)
		extra = floor (log(end) / log(10));

	precision += (int) extra;

	tracePlot->yAxis->setRange(QCPRange(bottom, top));
	tracePlot->xAxis->setRange(QCPRange(start, end));
	tracePlot->xAxis->setNumberPrecision(precision);
	tracePlot->yAxis->setTicks(false);
	yaxisTicker->setTickVector(ticks);
	yaxisTicker->setTickVectorLabels(tickLabels);
	tracePlot->yAxis->setTicks(true);


	if (!Setting::isEnabled(Setting::SHOW_CPUFREQ_GRAPHS) &&
	    !Setting::isEnabled(Setting::SHOW_CPUIDLE_GRAPHS))
		goto skipIdleFreqGraphs;

	/* Show CPU frequency and idle graphs */
	for (cpu = 0; cpu <= analyzer->getMaxCPU(); cpu++) {
		QPen pen = QPen();
		QPen penF = QPen();

		QCPGraph *graph;
		QString name;
		QCPScatterStyle style;

		if (Setting::isEnabled(Setting::SHOW_CPUIDLE_GRAPHS)) {
			graph = tracePlot->addGraph(tracePlot->xAxis,
						    tracePlot->yAxis);
			graph->setSelectable(QCP::stNone);
			name = QString(tr("cpuidle")) + QString::number(cpu);
			style = QCPScatterStyle(QCPScatterStyle::ssCircle, 5);
			pen.setColor(Qt::red);
			style.setPen(pen);
			graph->setScatterStyle(style);
			pen.setColor(Qt::green);
			graph->setPen(pen);
			graph->setName(name);
			graph->setAdaptiveSampling(true);
			graph->setLineStyle(QCPGraph::lsStepLeft);
			graph->setData(analyzer->cpuIdle[cpu].timev,
				       analyzer->cpuIdle[cpu].scaledData);
		}

		if (Setting::isEnabled(Setting::SHOW_CPUFREQ_GRAPHS)) {
			graph = tracePlot->addGraph(tracePlot->xAxis,
						    tracePlot->yAxis);
			graph->setSelectable(QCP::stNone);
			name = QString(tr("cpufreq")) + QString::number(cpu);
			penF.setColor(Qt::blue);
			penF.setWidth(2);
			graph->setPen(penF);
			graph->setName(name);
			graph->setAdaptiveSampling(true);
			graph->setLineStyle(QCPGraph::lsStepLeft);
			graph->setData(analyzer->cpuFreq[cpu].timev,
				       analyzer->cpuFreq[cpu].scaledData);
		}
	}

skipIdleFreqGraphs:

	/* Show scheduling graphs */
	for (cpu = 0; cpu <= analyzer->getMaxCPU(); cpu++) {
		DEFINE_CPUTASKMAP_ITERATOR(iter) = analyzer->
			cpuTaskMaps[cpu].begin();
		while(iter != analyzer->cpuTaskMaps[cpu].end()) {
			CPUTask &task = iter.value();
			iter++;

			addSchedGraph(task);
			if(Setting::isEnabled(Setting::SHOW_SCHED_GRAPHS)) {
				addHorizontalWakeupGraph(task);
				addWakeupGraph(task);
				addPreemptedGraph(task);
				addStillRunningGraph(task);
			}
		}
	}

	tracePlot->replot();
}

void MainWindow::setupCursors()
{
	double start, end, red, blue;

	start = analyzer->getStartTime().toDouble();
	end = analyzer->getEndTime().toDouble();

	cursors[TShark::RED_CURSOR] = new Cursor(tracePlot,
						 TShark::RED_CURSOR);
	cursors[TShark::BLUE_CURSOR] = new Cursor(tracePlot,
						  TShark::BLUE_CURSOR);

	cursors[TShark::RED_CURSOR]->setLayer(cursorLayer);
	cursors[TShark::BLUE_CURSOR]->setLayer(cursorLayer);

	red = (start + end) / 2;
	vtl::Time redtime = vtl::Time::fromDouble(red);
	redtime.setPrecision(analyzer->getTimePrecision());
	cursors[TShark::RED_CURSOR]->setPosition(redtime);
	cursorPos[TShark::RED_CURSOR] = red;
	infoWidget->setTime(redtime, TShark::RED_CURSOR);
	blue = (start + end) / 2 + (end - start) / 10;
	vtl::Time bluetime = vtl::Time::fromDouble(blue);
	bluetime.setPrecision(analyzer->getTimePrecision());
	cursors[TShark::BLUE_CURSOR]->setPosition(bluetime);
	cursorPos[TShark::BLUE_CURSOR] = blue;
	infoWidget->setTime(bluetime, TShark::BLUE_CURSOR);
	checkStatsTimeLimited();

	scrollTo(redtime);
}

void MainWindow::setupSettings()
{
	SettingDependency schedDep;
	schedDep.index = Setting::SHOW_SCHED_GRAPHS;
	schedDep.desiredValue = true;

	Setting::setName(Setting::HORIZONTAL_WAKEUP,
			 tr("Show horizontal wakeup"));
	Setting::setEnabled(Setting::HORIZONTAL_WAKEUP, false);
	Setting::addDependency(Setting::HORIZONTAL_WAKEUP, schedDep);

	Setting::setName(Setting::VERTICAL_WAKEUP,
			 tr("Show vertical wakeup"));
	Setting::setEnabled(Setting::VERTICAL_WAKEUP, true);
	Setting::addDependency(Setting::VERTICAL_WAKEUP, schedDep);

	Setting::setName(Setting::SHOW_SCHED_GRAPHS,
			 tr("Show scheduling graphs"));
	Setting::setEnabled(Setting::SHOW_SCHED_GRAPHS, true);

	Setting::setName(Setting::SHOW_CPUFREQ_GRAPHS,
			 tr("Show CPU frequency graphs"));
	Setting::setEnabled(Setting::SHOW_CPUFREQ_GRAPHS, true);

	Setting::setName(Setting::SHOW_CPUIDLE_GRAPHS,
			 tr("Show CPU idle graphs"));
	Setting::setEnabled(Setting::SHOW_CPUIDLE_GRAPHS, true);

	Setting::setName(Setting::SHOW_MIGRATION_GRAPHS,
			 tr("Show migration graphs"));
	Setting::setEnabled(Setting::SHOW_MIGRATION_GRAPHS, true);
}

void MainWindow::addSchedGraph(CPUTask &cpuTask)
{
	/* Add scheduling graph */
	TaskGraph *graph = new TaskGraph(tracePlot);
	QColor color = analyzer->getTaskColor(cpuTask.pid);
	Task *task = analyzer->findTask(cpuTask.pid);
	QPen pen = QPen();

	pen.setColor(color);
	graph->setPen(pen);
	graph->setTask(task);
	if (Setting::isEnabled(Setting::SHOW_SCHED_GRAPHS))
		graph->setData(cpuTask.schedTimev, cpuTask.scaledSchedData);
	/*
	 * Save a pointer to the graph object in the task. The destructor of
	 * AbstractClass will delete this when it is destroyed.
	 */
	cpuTask.graph = graph;
}

void MainWindow::addHorizontalWakeupGraph(CPUTask &task)
{
	if (!Setting::isEnabled(Setting::HORIZONTAL_WAKEUP))
		return;

	/* Add wakeup graph on top of scheduling */
	QCPGraph *graph = tracePlot->addGraph(tracePlot->xAxis,
					      tracePlot->yAxis);
	QCPScatterStyle style = QCPScatterStyle(QCPScatterStyle::ssDot);
	QColor color = analyzer->getTaskColor(task.pid);
	QPen pen = QPen();
	QCPErrorBars *errorBars = new QCPErrorBars(tracePlot->xAxis,
						   tracePlot->yAxis);
	errorBars->setAntialiased(false);
	pen.setColor(color);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task.wakeTimev, task.wakeHeight);
	errorBars->setData(task.wakeDelay, task.wakeZero);
	errorBars->setErrorType(QCPErrorBars::etKeyError);
	errorBars->setPen(pen);
	errorBars->setWhiskerWidth(4);
	errorBars->setDataPlottable(graph);
	/* errorBars->setSymbolGap(0); */
}

void MainWindow::addWakeupGraph(CPUTask &task)
{
	if (!Setting::isEnabled(Setting::VERTICAL_WAKEUP))
		return;

	/* Add wakeup graph on top of scheduling */
	QCPGraph *graph = tracePlot->addGraph(tracePlot->xAxis,
					      tracePlot->yAxis);
	QCPScatterStyle style = QCPScatterStyle(QCPScatterStyle::ssDot);
	QColor color = analyzer->getTaskColor(task.pid);
	QPen pen = QPen();
	QCPErrorBars *errorBars = new QCPErrorBars(tracePlot->xAxis,
						   tracePlot->yAxis);
	errorBars->setAntialiased(false);

	pen.setColor(color);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task.wakeTimev, task.wakeHeight);
	errorBars->setData(task.wakeZero, task.verticalDelay);
	errorBars->setErrorType(QCPErrorBars::etValueError);
	errorBars->setPen(pen);
	errorBars->setWhiskerWidth(4);
	errorBars->setDataPlottable(graph);
}

void MainWindow::addPreemptedGraph(CPUTask &task)
{
	/* Add still running graph on top of the other two...*/
	if (task.runningTimev.size() == 0)
		return;
	QCPGraph *graph = tracePlot->addGraph(tracePlot->xAxis,
					      tracePlot->yAxis);
	QString name = QString(tr("was preempted"));
	graph->setName(name);
	QCPScatterStyle style = QCPScatterStyle(QCPScatterStyle::ssCircle, 5);
	QPen pen = QPen();

	pen.setColor(Qt::red);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task.preemptedTimev, task.scaledPreemptedData);
}

void MainWindow::addStillRunningGraph(CPUTask &task)
{
	/* Add still running graph on top of the other two...*/
	if (task.runningTimev.size() == 0)
		return;
	QCPGraph *graph = tracePlot->addGraph(tracePlot->xAxis,
					      tracePlot->yAxis);
	QString name = QString(tr("is runnable"));
	graph->setName(name);
	QCPScatterStyle style = QCPScatterStyle(QCPScatterStyle::ssCircle, 5);
	QPen pen = QPen();

	pen.setColor(Qt::blue);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task.runningTimev, task.scaledRunningData);
}

/*
 * These are actions that should be enabled whenever we have a trace open
 */
void MainWindow::setTraceActionsEnabled(bool e)
{
	infoWidget->setTraceActionsEnabled(e);

	saveAction->setEnabled(e);
	closeAction->setEnabled(e);
	exportCPUAction->setEnabled(e);
	showTasksAction->setEnabled(e);
	showEventsAction->setEnabled(e);
	timeFilterAction->setEnabled(e);
	showStatsAction->setEnabled(e);
	showStatsTimeLimitedAction->setEnabled(e);
	clearLegendAction->setEnabled(e);
}

/*
 * These are actions that should be enabled whenever a task is selected
 */
void MainWindow::setTaskActionsEnabled(bool e)
{
	findWakeupAction->setEnabled(e);
	findWakingDirectAction->setEnabled(e);
	findSleepAction->setEnabled(e);
	addToLegendAction->setEnabled(e);
	addTaskGraphAction->setEnabled(e);
	removeTaskGraphAction->setEnabled(e);
	taskFilterAction->setEnabled(e);
	taskFilterLimitedAction->setEnabled(e);
}

void MainWindow::setWakeupActionsEnabled(bool e)
{
	findWakingAction->setEnabled(e);
}

void MainWindow::closeTrace()
{
	resetFilters();

	eventsWidget->beginResetModel();
	eventsWidget->clear();
	eventsWidget->endResetModel();
	eventsWidget->clearScrollTime();

	taskSelectDialog->beginResetModel();
	taskSelectDialog->setTaskMap(nullptr, 0);
	taskSelectDialog->endResetModel();

	statsDialog->beginResetModel();
	statsDialog->setTaskMap(nullptr, 0);
	statsDialog->endResetModel();

	statsLimitedDialog->beginResetModel();
	statsLimitedDialog->setTaskMap(nullptr, 0);
	statsLimitedDialog->endResetModel();

	eventSelectDialog->beginResetModel();
	eventSelectDialog->setStringTree(nullptr);
	eventSelectDialog->endResetModel();

	clearPlot();
	if(analyzer->isOpen())
		analyzer->close();
	taskToolBar->clear();
	setTraceActionsEnabled(false);
	setTaskActionsEnabled(false);
	setWakeupActionsEnabled(false);
	setStatus(STATUS_NOFILE);
}

void MainWindow::saveScreenshot()
{
	QFileDialog dialog(this);
	QStringList fileNameList;
	QString fileName;
	QString pngSuffix = QString(".png");
	QString bmpSuffix = QString(".bmp");
	QString jpgSuffix = QString(".jpg");
	QString pdfSuffix = QString(".pdf");
	QString pdfCreator = QString("traceshark ");
	QString pdfTitle;

	pdfCreator += QString(TRACESHARK_VERSION_STRING);

	if (!analyzer->isOpen())
		return;

	switch (analyzer->getTraceType()) {
	case TRACE_TYPE_FTRACE:
		pdfTitle = tr("Ftrace rendered by ");
		break;
	case TRACE_TYPE_PERF:
		pdfTitle = tr("Perf events rendered by ");
		break;
	default:
		pdfTitle = tr("Unknown garbage rendered by ");
		break;
	}

	pdfTitle += pdfCreator;

	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setNameFilter(tr("Images (*.png *.bmp *.jpg *.pdf)"));
	dialog.setViewMode(QFileDialog::Detail);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix(QString("png"));

	if (dialog.exec())
		fileNameList = dialog.selectedFiles();

	if (fileNameList.size() != 1)
		return;

	fileName = fileNameList.at(0);

	if (fileName.endsWith(pngSuffix, Qt::CaseInsensitive)) {
		tracePlot->savePng(fileName);
	} else if (fileName.endsWith(bmpSuffix, Qt::CaseInsensitive)) {
		tracePlot->saveBmp(fileName);
	} else if (fileName.endsWith(jpgSuffix, Qt::CaseInsensitive)) {
		tracePlot->saveJpg(fileName);
	} else if (fileName.endsWith(pdfSuffix, Qt::CaseInsensitive)) {
		tracePlot->savePdf(fileName, 0, 0,  QCP::epAllowCosmetic,
				   pdfCreator, pdfTitle);
	}
}

void MainWindow::about()
{
	QString textAboutCaption;
	QString textAbout;

	textAboutCaption = QMessageBox::tr(
	       "<h1>About Traceshark</h1>"
	       "<p>This is version %1.</p>"
	       "<p>Built with " VTL_COMPILER " at " __DATE__ " " __TIME__
	       "</p>"
		).arg(QLatin1String(TRACESHARK_VERSION_STRING));
	textAbout = QMessageBox::tr(
	       "<p>Copyright &copy; 2014-2018 Viktor Rosendahl"
	       "<p>This program comes with ABSOLUTELY NO WARRANTY; details below."
	       "<p>This is free software, and you are welcome to redistribute it"
	       " under certain conditions; select \"License\" under the \"Help\""
	       " menu for details."

	       "<h2>15. Disclaimer of Warranty.</h2>"
	       "<p>THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT "
	       "PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN "
	       "WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES PROVIDE "
	       "THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER "
	       "EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE "
	       "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A "
	       "PARTICULAR PURPOSE. THE ENTIRE RISK AS TO THE QUALITY AND "
	       "PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE PROGRAM "
	       "PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY "
	       "SERVICING, REPAIR OR CORRECTION."

	       "<h2>16. Limitation of Liability.</h2>"
	       "<p>IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED "
	       "TO IN WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY "
	       "WHO MODIFIES AND/OR CONVEYS THE PROGRAM AS PERMITTED ABOVE, "
	       "BE LIABLE TO YOU FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, "
	       "INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR "
	       "INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED TO "
	       "LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES "
	       "SUSTAINED BY YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM "
	       "TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH HOLDER OR "
	       "OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH "
	       "DAMAGES."

	       "<h2>17. Interpretation of Sections 15 and 16.</h2>"
	       "<p>If the disclaimer of warranty and limitation of "
	       "liability provided above cannot be given local legal effect "
	       "according to their terms, reviewing courts shall apply local "
	       "law that most closely approximates an absolute waiver of all "
	       "civil liability in connection with the Program, unless a "
	       "warranty or assumption of liability accompanies a copy of the "
	       "Program in return for a fee.");
	QMessageBox *msgBox = new QMessageBox(this);
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setWindowTitle(tr("About Traceshark"));
	msgBox->setText(textAboutCaption);
	msgBox->setInformativeText(textAbout);

	QPixmap pm(QLatin1String(RESSRC_PNG_SHARK));
	if (!pm.isNull())
		msgBox->setIconPixmap(pm);
	msgBox->show();
}

void MainWindow::aboutQCustomPlot()
{
	QString textAboutCaption;
	QString textAbout;

	textAboutCaption = QMessageBox::tr(
	       "<h1>About QCustomPlot</h1>"
	       "<p>This program contains a modified version of QCustomPlot %1.</p>"
		).arg(QLatin1String(QCUSTOMPLOT_VERSION_STR));
	textAbout = QMessageBox::tr(
	       "<p>Copyright &copy; 2011-2018 Emanuel Eichhammer"
	       "<p>QCustomPlot is licensed under GNU General Public License as "
	       "published by the Free Software Foundation, either version 3 of "
	       " the License, or (at your option) any later version.</p>"
	       "<p>See <a href=\"%1/\">%1</a> for more information about QCustomPlot.</p>"
	       "<p>This program comes with ABSOLUTELY NO WARRANTY; select \"License\" under the \"Help\""
	       " menu for details."
	       "<p>This is free software, and you are welcome to redistribute it"
	       " under certain conditions; see the license for details.").arg(QLatin1String("http://qcustomplot.com"));
	QMessageBox *msgBox = new QMessageBox(this);
	msgBox->setAttribute(Qt::WA_DeleteOnClose);
	msgBox->setWindowTitle(tr("About QCustomPlot"));
	msgBox->setText(textAboutCaption);
	msgBox->setInformativeText(textAbout);

	QPixmap pm(QLatin1String(RESSRC_PNG_QCP_LOGO));
	if (!pm.isNull())
		msgBox->setIconPixmap(pm);
	msgBox->show();
}

void MainWindow::license()
{
	licenseDialog->show();
}

void MainWindow::mouseWheel()
{
	bool xSelected = tracePlot->yAxis->selectedParts().
		testFlag(QCPAxis::spAxis);
	bool ySelected = tracePlot->yAxis->selectedParts().
		testFlag(QCPAxis::spAxis);

	/* This is not possible but would be cool */
	if (xSelected && ySelected)
		tracePlot->axisRect()->setRangeZoom(Qt::Vertical |
						    Qt::Horizontal);
	else if (ySelected)
		tracePlot->axisRect()->setRangeZoom(Qt::Vertical);
	else
		tracePlot->axisRect()->setRangeZoom(Qt::Horizontal);
}

void MainWindow::mousePress()
{
	bool xSelected = tracePlot->yAxis->selectedParts().
		testFlag(QCPAxis::spAxis);
	bool ySelected = tracePlot->yAxis->selectedParts().
		testFlag(QCPAxis::spAxis);

	/* This is not possible but would be cool */
	if (xSelected && ySelected)
		tracePlot->axisRect()->setRangeDrag(Qt::Vertical |
						    Qt::Horizontal);
	else if (ySelected)
		tracePlot->axisRect()->setRangeDrag(Qt::Vertical);
	else
		tracePlot->axisRect()->setRangeDrag(Qt::Horizontal);
}

void MainWindow::plotDoubleClicked(QMouseEvent *event)
{
	int cursorIdx;
	QVariant details;
	QCPLayerable *clickedLayerable;
	QCPLegend *legend;
	QCPAbstractLegendItem *legendItem;

	/* Let's filter out double clicks on the legend or its items */
	clickedLayerable = tracePlot->getLayerableAt(event->pos(), false,
						     &details);
	if (clickedLayerable != nullptr) {
		legend = qobject_cast<QCPLegend*>(clickedLayerable);
		if (legend != nullptr)
			return;
		legendItem = qobject_cast<QCPAbstractLegendItem*>
			(clickedLayerable);
		if (legendItem != nullptr)
			return;
	}

	cursorIdx = infoWidget->getCursorIdx();
	if (cursorIdx != TShark::RED_CURSOR && cursorIdx != TShark::BLUE_CURSOR)
		return;

	Cursor *cursor = cursors[cursorIdx];
	if (cursor != nullptr) {
		double pixel = (double) event->x();
		double coord = tracePlot->xAxis->pixelToCoord(pixel);
		vtl::Time time = vtl::Time::fromDouble(coord);
		time.setPrecision(analyzer->getTimePrecision());
		cursorPos[cursorIdx] = coord;
		cursor->setPosition(time);
		checkStatsTimeLimited();
		eventsWidget->scrollTo(time);
		infoWidget->setTime(time, cursorIdx);
	}
}

void MainWindow::infoValueChanged(vtl::Time value, int nr)
{
	Cursor *cursor;
	double dblValue = value.toDouble();
	if (nr == TShark::RED_CURSOR || nr == TShark::BLUE_CURSOR) {
		cursor = cursors[nr];
		if (cursor != nullptr) {
			cursor->setPosition(value);
			checkStatsTimeLimited();
		}
		eventsWidget->scrollTo(value);
		cursorPos[nr] = dblValue;
	}
}

void MainWindow::moveActiveCursor(vtl::Time time)
{
	int cursorIdx;
	double dblTime = time.toDouble();

	cursorIdx = infoWidget->getCursorIdx();
	if (cursorIdx != TShark::RED_CURSOR && cursorIdx != TShark::BLUE_CURSOR)
		return;

	Cursor *cursor = cursors[cursorIdx];
	if (cursor != nullptr) {
		cursor->setPosition(time);
		checkStatsTimeLimited();
		infoWidget->setTime(time, cursorIdx);
		cursorPos[cursorIdx] = dblTime;
	}
}

void MainWindow::showEventInfo(const TraceEvent &event)
{
	eventInfoDialog->show(event);
}

void MainWindow::taskTriggered(int pid)
{
	Task *task;
	unsigned int cpu;
	CPUTask *cpuTask;
	CPUTask *maxTask;
	int maxSize;
	QCPGraph *qcpGraph;
	QCPDataRange range;
	QCPDataSelection selection;
	int end;
	TaskGraph *taskGraph;
	bool enableActions = false;

	tracePlot->deselectAll();
	taskToolBar->removeTaskGraph();
	if (pid == 0)
		goto out;

	task = analyzer->findTask(pid);
	if (task == nullptr)
		goto out;

	if (task->graph != nullptr) {
		qcpGraph = task->graph->getQCPGraph();
		if (qcpGraph == nullptr)
			goto do_cpugraph;
		range.setBegin(0);
		end = task->schedTimev.size() - 1;
		if (end < 0)
			goto do_cpugraph;
		range.setEnd(end);
		selection.clear();
		selection.addDataRange(range);
		qcpGraph->setSelection(selection);
		taskToolBar->setTaskGraph(task->graph);
		enableActions = true;
		goto out;
	}

do_cpugraph:
	maxTask = nullptr;
	maxSize = 0;
	for (cpu = 0; cpu < analyzer->getNrCPUs(); cpu++) {
		cpuTask = analyzer->findCPUTask(pid, cpu);
		if (cpuTask != nullptr) {
			if (cpuTask->schedTimev.size() > maxSize) {
				maxSize = cpuTask->schedTimev.size();
				maxTask = cpuTask;
			}
		}
	}

	if (maxTask->graph != nullptr) {
		qcpGraph = maxTask->graph->getQCPGraph();
		if (qcpGraph == nullptr)
			goto out;
		range.setBegin(0);
		end = maxTask->schedTimev.size() - 1;
		if (end < 0)
			goto out;
		range.setEnd(end);
		selection.clear();
		selection.addDataRange(range);
		qcpGraph->setSelection(selection);
		taskGraph = TaskGraph::fromQCPGraph(qcpGraph);
		/* Should not happen but check anyway */
		if (taskGraph == nullptr)
			goto out;
		taskToolBar->setTaskGraph(taskGraph);
		enableActions = true;
	}

out:
	setTaskActionsEnabled(enableActions);
	tracePlot->replot();
}

void MainWindow::handleEventSelected(const TraceEvent *event)
{
	if (event == nullptr) {
		handleWakeUpChanged(false);
		return;
	}

	if (event->type == SCHED_WAKEUP || event->type == SCHED_WAKEUP_NEW) {
		handleWakeUpChanged(true);
	} else {
		handleWakeUpChanged(false);
	}
}

void MainWindow::handleWakeUpChanged(bool selected)
{
	setWakeupActionsEnabled(selected);
}

void MainWindow::createActions()
{
	openAction = new QAction(tr("&Open..."), this);
	openAction->setIcon(QIcon(RESSRC_PNG_OPEN));
	openAction->setShortcuts(QKeySequence::Open);
	openAction->setToolTip(tr(TOOLTIP_OPEN));
	tsconnect(openAction, triggered(), this, openTrace());

	closeAction = new QAction(tr("&Close"), this);
	closeAction->setIcon(QIcon(RESSRC_PNG_CLOSE));
	closeAction->setShortcuts(QKeySequence::Close);
	closeAction->setToolTip(tr(TOOLTIP_CLOSE));
	tsconnect(closeAction, triggered(), this, closeTrace());

	saveAction = new QAction(tr("&Save screenshot as..."), this);
	saveAction->setIcon(QIcon(RESSRC_PNG_SCREENSHOT));
	saveAction->setShortcuts(QKeySequence::SaveAs);
	saveAction->setToolTip(tr(TOOLTIP_SAVESCREEN));
	tsconnect(saveAction, triggered(), this, saveScreenshot());

	showTasksAction = new QAction(tr("Show task list..."), this);
	showTasksAction->setIcon(QIcon(RESSRC_PNG_TASKSELECT));
	showTasksAction->setToolTip(tr(TOOLTIP_SHOWTASKS));
	tsconnect(showTasksAction, triggered(), this, showTaskSelector());

	showEventsAction = new QAction(tr("Filter on event type..."), this);
	showEventsAction->setIcon(QIcon(RESSRC_PNG_EVENTFILTER));
	showEventsAction->setToolTip(tr(TOOLTIP_SHOWEVENTS));
	tsconnect(showEventsAction, triggered(), this, showEventFilter());

	timeFilterAction = new QAction(tr("Filter on time"), this);
	timeFilterAction->setIcon(QIcon(RESSRC_PNG_TIMEFILTER));
	timeFilterAction->setToolTip(tr(TOOLTIP_TIMEFILTER));
	tsconnect(timeFilterAction, triggered(), this, timeFilter());

	graphEnableAction = new QAction(tr("Select graphs..."), this);
	graphEnableAction->setIcon(QIcon(RESSRC_PNG_GRAPHENABLE));
	graphEnableAction->setToolTip(tr(TOOLTIP_GRAPHENABLE));
	tsconnect(graphEnableAction, triggered(), this, showGraphEnable());

	resetFiltersAction = new QAction(tr("Reset all filters"), this);
	resetFiltersAction->setIcon(QIcon(RESSRC_PNG_RESETFILTERS));
	resetFiltersAction->setToolTip(tr(TOOLTIP_RESETFILTERS));
	resetFiltersAction->setEnabled(false);
	tsconnect(resetFiltersAction, triggered(), this, resetFilters());

	exportEventsAction = new QAction(tr("Export events to a file..."),
					 this);
	exportEventsAction->setIcon(QIcon(RESSRC_PNG_EXPORTEVENTS));
	exportEventsAction->setToolTip(tr(TOOLTIP_EXPORTEVENTS));
	exportEventsAction->setEnabled(false);
	tsconnect(exportEventsAction, triggered(), this,
		  exportEventsTriggered());

	exportCPUAction = new QAction(
		tr("Export cpu-cycles events to a file..."), this);
	exportCPUAction->setIcon(QIcon(RESSRC_PNG_EXPORTCPUEVENTS));
	exportCPUAction->setToolTip(tr(TOOLTIP_EXPORT_CPU));
	exportCPUAction->setEnabled(false);
	tsconnect(exportCPUAction, triggered(), this,
		  exportCPUTriggered());

	showStatsAction = new QAction(tr("Show stats..."), this);
	showStatsAction->setIcon(QIcon(RESSRC_PNG_GETSTATS));
	showStatsAction->setToolTip(TOOLTIP_GETSTATS);
	tsconnect(showStatsAction, triggered(), this, showStats());

	showStatsTimeLimitedAction = new QAction(
		tr("Show stats cursor time..."), this);
	showStatsTimeLimitedAction->setIcon(
		QIcon(RESSRC_PNG_GETSTATS_TIMELIMIT));
	showStatsTimeLimitedAction->setToolTip(TOOLTIP_GETSTATS_TIMELIMITED);
	tsconnect(showStatsTimeLimitedAction, triggered(), this,
		  showStatsTimeLimited());

	exitAction = new QAction(tr("E&xit"), this);
	exitAction->setShortcuts(QKeySequence::Quit);
	exitAction->setToolTip(tr(TOOLTIP_EXIT));
	tsconnect(exitAction, triggered(), this, close());

	aboutQtAction = new QAction(tr("About &Qt"), this);
	aboutQtAction->setIcon(QIcon(RESSRC_PNG_QT_LOGO));
	aboutQtAction->setToolTip(tr(ABOUT_QT_TOOLTIP));
	tsconnect(aboutQtAction, triggered(), qApp, aboutQt());

	aboutAction = new QAction(tr("&About Traceshark"), this);
	aboutAction->setIcon(QIcon(RESSRC_PNG_SHARK));
	aboutAction->setToolTip(tr(ABOUT_TSHARK_TOOLTIP));
	tsconnect(aboutAction, triggered(), this, about());

	aboutQCPAction = new QAction(tr("About QCustom&Plot"), this);
	aboutQCPAction->setIcon(QIcon(RESSRC_PNG_QCP_LOGO));
	aboutAction->setToolTip(tr(SHOW_QCP_TOOLTIP));
	tsconnect(aboutQCPAction, triggered(), this, aboutQCustomPlot());

	licenseAction = new QAction(tr("&License"), this);
	licenseAction->setToolTip(tr(SHOW_LICENSE_TOOLTIP));
	tsconnect(licenseAction, triggered(), this, license());

	addTaskGraphAction = new QAction(tr("Add task graph"), this);
	addTaskGraphAction->setIcon(QIcon(RESSRC_PNG_ADD_TASK));
	addTaskGraphAction->setToolTip(tr(ADD_UNIFIED_TOOLTIP));
	tsconnect(addTaskGraphAction, triggered(), this,
		  addTaskGraphTriggered());

	addToLegendAction = new QAction(tr("Add task to the legend"), this);
	addToLegendAction->setIcon(QIcon(RESSRC_PNG_ADD_TO_LEGEND));
	addToLegendAction->setToolTip(tr(ADD_LEGEND_TOOLTIP));
	tsconnect(addToLegendAction, triggered(), this,addToLegendTriggered());

	clearLegendAction = new QAction(tr("Clear the legend"), this);
	clearLegendAction->setIcon(QIcon(RESSRC_PNG_CLEAR_LEGEND));
	clearLegendAction->setToolTip(tr(CLEAR_LEGEND_TOOLTIP));
	tsconnect(clearLegendAction, triggered(), this, clearLegendTriggered());

	findWakeupAction = new QAction(tr("Find wakeup"), this);
	findWakeupAction->setIcon(QIcon(RESSRC_PNG_FIND_WAKEUP));
	findWakeupAction->setToolTip(tr(FIND_WAKEUP_TOOLTIP));
	tsconnect(findWakeupAction, triggered(), this, findWakeupTriggered());

	findWakingAction = new QAction(tr("Find waking"), this);
	findWakingAction->setIcon(QIcon(RESSRC_PNG_FIND_WAKING));
	findWakingAction->setToolTip(tr(FIND_WAKING_TOOLTIP));
	tsconnect(findWakingAction, triggered(), this, findWakingTriggered());

	findWakingDirectAction = new QAction(tr("Find waking direct"), this);
	findWakingDirectAction->setIcon(QIcon(RESSRC_PNG_FIND_WAKING_DIRECT));
	findWakingDirectAction->setToolTip(tr(FIND_WAKING_DIRECT_TOOLTIP));
	tsconnect(findWakingDirectAction, triggered(), this,
		  findWakingDirectTriggered());

	findSleepAction = new QAction(tr("Find sched_switch sleep event"),
				      this);
	findSleepAction->setIcon(QIcon(RESSRC_PNG_FIND_SLEEP));
	findSleepAction->setToolTip(tr(TOOLTIP_FIND_SLEEP));
	tsconnect(findSleepAction, triggered(), this, findSleepTriggered());

	removeTaskGraphAction = new QAction(tr("Remove task graph"), this);
	removeTaskGraphAction->setIcon(QIcon(RESSRC_PNG_REMOVE_TASK));
	removeTaskGraphAction->setToolTip(tr(REMOVE_TASK_TOOLTIP));
	tsconnect(removeTaskGraphAction, triggered(), this,
		  removeTaskGraphTriggered());

	taskFilterAction = new QAction(tr("Filter on selected task"), this);
	taskFilterAction->setIcon(QIcon(RESSRC_PNG_FILTERCURRENT));
	taskFilterAction->setToolTip(tr(TASK_FILTER_TOOLTIP));
	tsconnect(taskFilterAction, triggered(), this,
		  taskFilterTriggered());

	taskFilterLimitedAction = new QAction(tr("Filter on selected task"),
					      this);
	taskFilterLimitedAction->setIcon(QIcon(RESSRC_PNG_FILTERCURRENT_LIMIT));
	taskFilterLimitedAction->setToolTip(tr(TASK_FILTER_TIMELIMIT_TOOLTIP));
	tsconnect(taskFilterLimitedAction, triggered(), this,
		  taskFilterLimitedTriggered());

	setTraceActionsEnabled(false);
	setTaskActionsEnabled(false);
	setWakeupActionsEnabled(false);
}

void MainWindow::createToolBars()
{
	bool widescreen = isWideScreen();

	fileToolBar = new QToolBar(tr("&File"));
	addToolBar(Qt::LeftToolBarArea, fileToolBar);
	fileToolBar->addAction(openAction);
	fileToolBar->addAction(closeAction);
	fileToolBar->addAction(saveAction);
	fileToolBar->addAction(exportEventsAction);
	fileToolBar->addAction(exportCPUAction);

	viewToolBar = new QToolBar(tr("&View"));
	addToolBar(Qt::LeftToolBarArea, viewToolBar);
	viewToolBar->addAction(showTasksAction);
	viewToolBar->addAction(showEventsAction);
	viewToolBar->addAction(timeFilterAction);
	viewToolBar->addAction(resetFiltersAction);
	viewToolBar->addAction(graphEnableAction);
	viewToolBar->addAction(showStatsAction);
	viewToolBar->addAction(showStatsTimeLimitedAction);

	taskToolBar = new TaskToolBar(tr("Task"));
	if (widescreen) {
		infoWidget->addToolBar(taskToolBar);
	} else {
		addToolBar(Qt::TopToolBarArea, taskToolBar);
		infoWidget->addStretch();
	}

	taskToolBar->addAction(addToLegendAction);
	taskToolBar->addAction(clearLegendAction);
	taskToolBar->addAction(findWakeupAction);
	taskToolBar->addAction(findWakingAction);
	taskToolBar->addAction(findWakingDirectAction);
	taskToolBar->addAction(findSleepAction);
	taskToolBar->addAction(addTaskGraphAction);
	taskToolBar->addAction(removeTaskGraphAction);
	taskToolBar->addAction(taskFilterAction);
	taskToolBar->addAction(taskFilterLimitedAction);
	taskToolBar->addStretch();
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAction);
	fileMenu->addAction(closeAction);
	fileMenu->addAction(saveAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exportEventsAction);
	fileMenu->addAction(exportCPUAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(showTasksAction);
	viewMenu->addAction(showEventsAction);
	viewMenu->addAction(timeFilterAction);
	viewMenu->addAction(resetFiltersAction);
	viewMenu->addAction(graphEnableAction);
	viewMenu->addAction(showStatsAction);
	viewMenu->addAction(showStatsTimeLimitedAction);

	taskMenu = menuBar()->addMenu(tr("&Task"));
	taskMenu->addAction(addToLegendAction);
	taskMenu->addAction(clearLegendAction);
	taskMenu->addAction(findWakeupAction);
	taskMenu->addAction(findWakingAction);
	taskMenu->addAction(findWakingDirectAction);
	taskMenu->addAction(findSleepAction);
	taskMenu->addAction(addTaskGraphAction);
	taskMenu->addAction(removeTaskGraphAction);
	taskMenu->addAction(taskFilterAction);
	taskMenu->addAction(taskFilterLimitedAction);

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutAction);
	helpMenu->addAction(aboutQCPAction);
	helpMenu->addAction(aboutQtAction);
	helpMenu->addAction(licenseAction);
}

void MainWindow::createStatusBar()
{
	statusLabel = new QLabel(" W999 ");
	statusLabel->setAlignment(Qt::AlignHCenter);
	statusLabel->setMinimumSize(statusLabel->sizeHint());
	statusBar()->addWidget(statusLabel);

	statusStrings[STATUS_NOFILE] = new QString(tr("No file loaded"));
	statusStrings[STATUS_FILE] = new QString(tr("Loaded file "));
	statusStrings[STATUS_ERROR] = new QString(tr("An error has occured"));

	setStatus(STATUS_NOFILE);
}

void MainWindow::setStatus(status_t status, const QString *fileName)
{
	QString string;
	if (fileName != nullptr)
		string = *statusStrings[status] + *fileName;
	else
		string = *statusStrings[status];

	statusLabel->setText(string);
}

int MainWindow::loadTraceFile(const QString &fileName)
{
	qint64 start, stop;
        int rval;

	printf("opening %s\n", fileName.toLocal8Bit().data());
	
	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	rval = analyzer->open(fileName);
	stop = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	stop = stop - start;

	printf("Loading took %.6lf s\n", (double) stop / 1000);
	return rval;
}

void MainWindow::plottableClicked(QCPAbstractPlottable *plottable,
				  int /* dataIndex */,
				  QMouseEvent * /* event */)
{
	QCPGraph *qcpGraph;
	TaskGraph *graph;

	qcpGraph = qobject_cast<QCPGraph *>(plottable);
	if (qcpGraph == nullptr)
		return;

	graph = TaskGraph::fromQCPGraph(qcpGraph);
	if (graph == nullptr)
		return;

	if (qcpGraph->selected()) {
		setTaskActionsEnabled(true);
		taskToolBar->setTaskGraph(graph);
	} else {
		setTaskActionsEnabled(false);
		taskToolBar->removeTaskGraph();
	}
}

void MainWindow::selectionChanged()
{
	if (taskToolBar->checkGraphSelection())
		setTaskActionsEnabled(false);
}

void MainWindow::legendDoubleClick(QCPLegend * /* legend */,
				   QCPAbstractLegendItem *abstractItem)
{
	QCPPlottableLegendItem *plottableItem;
	QCPAbstractPlottable *plottable;
	QCPGraph *legendGraph;

	plottableItem = qobject_cast<QCPPlottableLegendItem*>(abstractItem);
	if (plottableItem == nullptr)
		return;
	plottable = plottableItem->plottable();
	legendGraph = qobject_cast<QCPGraph*>(plottable);
	if (legendGraph == nullptr)
		return;

	handleLegendGraphDoubleClick(legendGraph);
}

void MainWindow::handleLegendGraphDoubleClick(QCPGraph *graph)
{
	TaskGraph *tgraph = TaskGraph::fromQCPGraph(graph);
	const Task *task;

	if (tgraph == nullptr)
		return;
	tgraph->removeFromLegend();
	task = tgraph->getTask();
	if (task == nullptr)
		return;
	/*
	 * Inform the TaskToolBar class that the pid has been removed. This is
	 * needed because TaskToolBar keeps track of this for the purpose of
	 * preventing the same pid being added twice from different legend 
	 * graphs, there might be "identical" legend graphs when the same pid
	 * has migrated between CPUs
	 */
	taskToolBar->pidRemoved(task->pid);
}

void MainWindow::addTaskToLegend(int pid)
{
	CPUTask *cpuTask = nullptr;
	unsigned int cpu;

	/*
	 * Let's find a per CPU taskGraph, because they are always created,
	 * the unified graphs only exist for those that have been chosen to be
	 * displayed by the user
	 */
	for (cpu = 0; cpu < analyzer->getNrCPUs(); cpu++) {
		cpuTask = analyzer->findCPUTask(pid, cpu);
		if (cpuTask != nullptr)
			break;
	}

	if (cpuTask == nullptr)
		return;

	taskToolBar->addTaskGraphToLegend(cpuTask->graph);
}

void MainWindow::setEventsWidgetEvents()
{
	if (analyzer->isFiltered())
		eventsWidget->setEvents(&analyzer->filteredEvents);
	else
		eventsWidget->setEvents(analyzer->events);
}

void MainWindow::scrollTo(const vtl::Time &time)
{
	vtl::Time start, end;
	start = analyzer->getStartTime();
	end = analyzer->getEndTime();

	/*
	 * Fixme:
	 * For some reason the EventsWidget doesn't want to make its first
	 * scroll to somewhere in the middle of the trace. As a work around
	 * we first scroll to the beginning and to the end, and then to
	 * where we want.
	 */
	eventsWidget->scrollTo(start);
	eventsWidget->scrollTo(end);
	eventsWidget->scrollTo(time);
}

void MainWindow::updateResetFiltersEnabled(void)
{
	if (analyzer->isFiltered()) {
		resetFiltersAction->setEnabled(true);
		exportEventsAction->setEnabled(true);
	} else {
		resetFiltersAction->setEnabled(false);
		exportEventsAction->setEnabled(false);
	}
}

void MainWindow::timeFilter(void)
{
	double min, max;
	vtl::Time saved = eventsWidget->getSavedScroll();

	min = TSMIN(cursorPos[TShark::RED_CURSOR],
		    cursorPos[TShark::BLUE_CURSOR]);
	max = TSMAX(cursorPos[TShark::RED_CURSOR],
		    cursorPos[TShark::BLUE_CURSOR]);

	vtl::Time tmin = vtl::Time::fromDouble(min);
	vtl::Time tmax = vtl::Time::fromDouble(max);

	eventsWidget->beginResetModel();
	analyzer->createTimeFilter(tmin, tmax, false);
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}

void MainWindow::createPidFilter(QMap<int, int> &map,
				 bool orlogic, bool inclusive)
{
	vtl::Time saved = eventsWidget->getSavedScroll();

	eventsWidget->beginResetModel();
	analyzer->createPidFilter(map, orlogic, inclusive);
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}

void MainWindow::createEventFilter(QMap<event_t, event_t> &map, bool orlogic)
{
	vtl::Time saved = eventsWidget->getSavedScroll();

	eventsWidget->beginResetModel();
	analyzer->createEventFilter(map, orlogic);
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}


void MainWindow::resetPidFilter()
{
	vtl::Time saved;

	if (!analyzer->filterActive(FilterState::FILTER_PID))
		return;

	saved = eventsWidget->getSavedScroll();
	eventsWidget->beginResetModel();
	analyzer->disableFilter(FilterState::FILTER_PID);
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}

void MainWindow::resetEventFilter()
{
	vtl::Time saved;

	if (!analyzer->filterActive(FilterState::FILTER_EVENT))
		return;

	saved = eventsWidget->getSavedScroll();
	eventsWidget->beginResetModel();
	analyzer->disableFilter(FilterState::FILTER_EVENT);
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}

void MainWindow::resetFilters()
{
	vtl::Time saved;

	if (!analyzer->isFiltered())
		return;

	const TraceEvent *event = eventsWidget->getSelectedEvent();

	if (event != nullptr) {
		saved = event->time;
	} else {
		saved = eventsWidget->getSavedScroll();
	}

	eventsWidget->beginResetModel();
	analyzer->disableAllFilters();
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}

void MainWindow::exportEvents(TraceAnalyzer::exporttype_t export_type)
{
	QFileDialog dialog(this);
	QStringList fileNameList;
	QString fileName;
	int ts_errno;

	dialog.setFileMode(QFileDialog::AnyFile);
	dialog.setNameFilter(tr("ASCII Text (*.asc *.txt)"));
	dialog.setViewMode(QFileDialog::Detail);
	dialog.setAcceptMode(QFileDialog::AcceptSave);
	dialog.setDefaultSuffix(QString("asc"));

	if (!dialog.exec())
		return;

	fileNameList = dialog.selectedFiles();

	if (fileNameList.size() != 1) {
		vtl::warnx("You can only select one filename, not %d",
			   fileNameList.size());
		return;
	}

	fileName = fileNameList.at(0);

	if (!analyzer->exportTraceFile(fileName.toLocal8Bit().data(),
				       &ts_errno, export_type)) {
		vtl::warn(ts_errno, "Failed to export trace to %s",
			  fileName.toLocal8Bit().data());
	}
}

void MainWindow::exportCPUTriggered()
{
	exportEvents(TraceAnalyzer::EXPORT_TYPE_CPU_CYCLES);
}

void MainWindow::exportEventsTriggered()
{
	exportEvents(TraceAnalyzer::EXPORT_TYPE_ALL);
}

void MainWindow::consumeSettings()
{
	unsigned int cpu;

	if (!analyzer->isOpen())
		return;

	clearPlot();
	taskToolBar->clear();

	for (cpu = 0; cpu <= analyzer->getMaxCPU(); cpu++) {
		DEFINE_CPUTASKMAP_ITERATOR(iter);
		for (iter = analyzer->cpuTaskMaps[cpu].begin();
		     iter != analyzer->cpuTaskMaps[cpu].end();
		     iter++) {
			CPUTask &task = iter.value();
			delete task.graph;
			task.graph = nullptr;
		}
	}

	DEFINE_TASKMAP_ITERATOR(iter);
	for (iter = analyzer->taskMap.begin();
	     iter != analyzer->taskMap.end();
	     iter++) {
		Task *task = iter.value().task;
		if (task->graph != nullptr) {
			/*
			 * This implies that the task had a task graph added.
			 * We delete the TaskGraph object and set the pointers
			 * to nullptr. The actual QCPGraph objects is already
			 * deleted by the clearPlot() function above.
			 */
			delete task->graph;
			task->graph = nullptr;
			task->wakeUpGraph = nullptr;
			task->runningGraph = nullptr;
			task->preemptedGraph = nullptr;
		}
	}

	computeLayout();
	setupCursors();
	rescaleTrace();
	showTrace();
	tracePlot->show();
}

void MainWindow::addTaskGraph(int pid)
{
	/* Add a unified scheduling graph for pid */
	bool isNew;
	TaskRange *taskRange;
	TaskGraph *taskGraph;
	unsigned int cpu;
	CPUTask *cpuTask = nullptr;

	taskRange = taskRangeAllocator->getTaskRange(pid, isNew);

	if (!isNew || taskRange == nullptr)
		return;

	Task *task = analyzer->findTask(pid);
	QColor color = analyzer->getTaskColor(pid);

	if (task == nullptr) {
		taskRangeAllocator->putTaskRange(taskRange);
		return;
	}

	for (cpu = 0; cpu < analyzer->getNrCPUs(); cpu++) {
		cpuTask = analyzer->findCPUTask(pid, cpu);
		if (cpuTask != nullptr)
			break;
	}
	if (cpuTask == nullptr || cpuTask->graph == nullptr) {
		taskRangeAllocator->putTaskRange(taskRange);
		return;
	}

	bottom = taskRangeAllocator->getBottom();

	taskGraph = new TaskGraph(tracePlot);
	taskGraph->setTaskGraphForLegend(cpuTask->graph);
	QPen pen = QPen();

	pen.setColor(color);
	taskGraph->setPen(pen);
	taskGraph->setTask(task);

	task->offset = taskRange->lower;
	task->scale = schedHeight;
	task->doScale();
	task->doScaleWakeup();
	task->doScaleRunning();
	task->doScalePreempted();

	taskGraph->setData(task->schedTimev, task->scaledSchedData);
	task->graph = taskGraph;

	/* Add the horizontal wakeup graph as well */
	QCPGraph *graph = tracePlot->addGraph(tracePlot->xAxis,
					      tracePlot->yAxis);
	QCPErrorBars *errorBars = new QCPErrorBars(tracePlot->xAxis,
						   tracePlot->yAxis);
	errorBars->setAntialiased(false);
	QCPScatterStyle style = QCPScatterStyle(QCPScatterStyle::ssDot);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task->wakeTimev, task->wakeHeight);
	errorBars->setData(task->wakeDelay, task->wakeZero);
	errorBars->setErrorType(QCPErrorBars::etKeyError);
	errorBars->setPen(pen);
	errorBars->setWhiskerWidth(4);
	errorBars->setDataPlottable(graph);
	task->wakeUpGraph = graph;

	/* Add the still running graph on top of the other two... */
	QString name = QString(tr("is runnable"));
	QCPScatterStyle rstyle = QCPScatterStyle(QCPScatterStyle::ssCircle, 5);
	if (task->runningTimev.size() == 0) {
		task->runningGraph = nullptr;
		goto out;
	}
	graph = tracePlot->addGraph(tracePlot->xAxis, tracePlot->yAxis);
	graph->setName(name);

	pen.setColor(Qt::blue);
	rstyle.setPen(pen);
	graph->setScatterStyle(rstyle);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task->runningTimev, task->scaledRunningData);
	task->runningGraph = graph;

	/* ...and then the preempted graph */
	name = QString(tr("was preempted"));
	rstyle = QCPScatterStyle(QCPScatterStyle::ssCircle, 5);
	if (task->preemptedTimev.size() == 0) {
		task->preemptedGraph = nullptr;
		goto out;
	}
	graph = tracePlot->addGraph(tracePlot->xAxis, tracePlot->yAxis);
	graph->setName(name);

	pen.setColor(Qt::red);
	rstyle.setPen(pen);
	graph->setScatterStyle(rstyle);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(task->preemptedTimev, task->scaledPreemptedData);
	task->preemptedGraph = graph;

out:
	/*
	 * We only modify the lower part of the range to show the newly
	 * added unified task graph.
	 */
	QCPRange range = tracePlot->yAxis->range();
	tracePlot->yAxis->setRange(QCPRange(bottom, range.upper));

	tracePlot->replot();
}

void MainWindow::removeTaskGraph(int pid)
{
	Task *task = analyzer->findTask(pid);
	QCPGraph *qcpGraph;

	if (task == nullptr)
		return;

	if (task->graph != nullptr) {
		qcpGraph = task->graph->getQCPGraph();
		if (qcpGraph != nullptr && qcpGraph->selected() &&
		    taskToolBar->getPid() == task->pid)
			taskToolBar->removeTaskGraph();
		task->graph->destroy();
		task->graph = nullptr;
	}

	if (task->wakeUpGraph != nullptr) {
		tracePlot->removeGraph(task->wakeUpGraph);
		task->wakeUpGraph = nullptr;
	}

	if (task->runningGraph != nullptr) {
		tracePlot->removeGraph(task->runningGraph);
		task->runningGraph = nullptr;
	}

	if (task->preemptedGraph != nullptr) {
		tracePlot->removeGraph(task->preemptedGraph);
		task->preemptedGraph = nullptr;
	}

	taskRangeAllocator->putTaskRange(pid);
	bottom = taskRangeAllocator->getBottom();

	QCPRange range = tracePlot->yAxis->range();
	tracePlot->yAxis->setRange(QCPRange(bottom, range.upper));

	tracePlot->replot();
}

void MainWindow::showTaskSelector()
{
	taskSelectDialog->show();
	if (dockWidgetArea(taskSelectDialog) == Qt::NoDockWidgetArea)
		addDockWidget(Qt::LeftDockWidgetArea, taskSelectDialog);

	if (dockWidgetArea(statsDialog) == Qt::LeftDockWidgetArea)
		tabifyDockWidget(statsDialog, taskSelectDialog);
}

void MainWindow::showEventFilter()
{
	eventSelectDialog->show();
}

void MainWindow::showGraphEnable()
{
	graphEnableDialog->show();
}

void MainWindow::showStats()
{
	statsDialog->show();
	if (dockWidgetArea(statsDialog) == Qt::NoDockWidgetArea)
		addDockWidget(Qt::LeftDockWidgetArea, statsDialog);

	if (dockWidgetArea(taskSelectDialog) == Qt::LeftDockWidgetArea)
		tabifyDockWidget(taskSelectDialog, statsDialog);
}

void MainWindow::showStatsTimeLimited()
{
	statsLimitedDialog->beginResetModel();
	analyzer->doLimitedStats();
	statsLimitedDialog->setTaskMap(&analyzer->taskMap,
				       analyzer->getNrCPUs());
	statsLimitedDialog->endResetModel();
	statsLimitedDialog->show();
	if (dockWidgetArea(statsLimitedDialog) == Qt::NoDockWidgetArea)
		addDockWidget(Qt::RightDockWidgetArea, statsLimitedDialog);
}

void MainWindow::removeQDockWidget(QDockWidget *widget)
{
	if (dockWidgetArea(widget) != Qt::NoDockWidgetArea)
		removeDockWidget(widget);
}

void MainWindow::showWakeupOrWaking(int pid, event_t wakevent)
{
	int activeIdx = infoWidget->getCursorIdx();
	int inactiveIdx;
	int wakeUpIndex;
	int schedIndex;

	if (activeIdx != TShark::RED_CURSOR &&
	    activeIdx != TShark::BLUE_CURSOR) {
		return;
	}

	inactiveIdx = TShark::RED_CURSOR;
	if (activeIdx == inactiveIdx)
		inactiveIdx = TShark::BLUE_CURSOR;

	Cursor *activeCursor = cursors[activeIdx];
	Cursor *inactiveCursor = cursors[inactiveIdx];

	if (activeCursor == nullptr || inactiveCursor == nullptr)
		return;

	/*
	 * The time of the active cursor is taken to be the time that the
	 * user is interested in, i.e. finding the previous wake up event
	 * relative to
	 */
	double zerotime = activeCursor->getPosition();
	const TraceEvent *schedevent =
		analyzer->findPreviousSchedEvent(
			vtl::Time::fromDouble(zerotime), pid, &schedIndex);
	if (schedevent == nullptr)
		return;

	const TraceEvent *wakeupevent = analyzer->
		findPreviousWakEvent(schedIndex, pid, wakevent, &wakeUpIndex);
	if (wakeupevent == nullptr)
		return;
	/*
	 * This is what we do, we move the *active* cursor to the wakeup
	 * event, move the *inactive* cursor to the scheduling event and then
	 * finally scroll the events widget to the same time and highlight
	 * the task that was doing the wakeup. This way we can push the button
	 * again to see who woke up the task that was doing the wakeup
	 */
	activeCursor->setPosition(wakeupevent->time);
	inactiveCursor->setPosition(schedevent->time);
	checkStatsTimeLimited();
	infoWidget->setTime(wakeupevent->time, activeIdx);
	infoWidget->setTime(schedevent->time, inactiveIdx);
	cursorPos[activeIdx] = wakeupevent->time.toDouble();
	cursorPos[inactiveIdx] = schedevent->time.toDouble();

	if (!analyzer->isFiltered()) {
		eventsWidget->scrollTo(wakeUpIndex);
	} else {
		/*
		 * If a filter is enabled we need to try to find the index in
		 * analyzer->filteredEvents
		 */
		int filterIndex;
		if (analyzer->findFilteredEvent(wakeUpIndex, &filterIndex)
		    != nullptr)
			eventsWidget->scrollTo(filterIndex);
	}

	unsigned int wcpu = wakeupevent->cpu;
	int wpid = wakeupevent->pid;

	/* Deselect the selected task */
	tracePlot->deselectAll();

	/*
	 * If the wakeup task was run with pid 0 = swapper, then just remove
	 * the task from the task toolbar and disable the task actions.
	 */
	if (wpid == 0) {
		taskToolBar->removeTaskGraph();
		setTaskActionsEnabled(false);
		tracePlot->replot();
		return;
	}

	CPUTask *cpuTask = analyzer->findCPUTask(wpid, wcpu);

	/*
	 * If we can't find what we expected, we return, the advanced user
	 * could notice that something fishy is going on by the fact that
	 * no task is selected after this user interaction, white there still
	 * is a task in the task toolbar.
	 */
	if (cpuTask == nullptr || cpuTask->graph == nullptr) {
		tracePlot->replot();
		return;
	}
	QCPGraph *qcpGraph = cpuTask->graph->getQCPGraph();
	if (qcpGraph == nullptr) {
		tracePlot->replot();
		return;
	}

	/* Finally, mark the potential wakeup task as selected */
	int count = qcpGraph->dataCount();
	if (count > 0)
		count--;
	QCPDataRange wholeRange(0,  count);
	QCPDataSelection wholeSelection(wholeRange);
	qcpGraph->setSelection(wholeSelection);
	tracePlot->replot();

	/* Finally update the TaskToolBar to reflect the change in selection */
	TaskGraph *graph = TaskGraph::fromQCPGraph(qcpGraph);
	if (graph == nullptr) {
		taskToolBar->removeTaskGraph();
		setTaskActionsEnabled(false);
		return;
	}
	taskToolBar->setTaskGraph(graph);
	setTaskActionsEnabled(true);
}

void MainWindow::showWaking(const TraceEvent *wakeupevent)
{
	int activeIdx = infoWidget->getCursorIdx();
	int wakingIndex;

	if (activeIdx != TShark::RED_CURSOR &&
	    activeIdx != TShark::BLUE_CURSOR) {
		return;
	}

	Cursor *activeCursor = cursors[activeIdx];

	if (activeCursor == nullptr)
		return;

	const TraceEvent *wakingevent =
		analyzer->findWakingEvent(wakeupevent, &wakingIndex);
	if (wakingevent == nullptr)
		return;

	activeCursor->setPosition(wakingevent->time);
	infoWidget->setTime(wakingevent->time, activeIdx);
	checkStatsTimeLimited();
	cursorPos[activeIdx] = wakingevent->time.toDouble();

	if (!analyzer->isFiltered()) {
		eventsWidget->scrollTo(wakingIndex);
	} else {
		/*
		 * If a filter is enabled we need to try to find the index in
		 * analyzer->filteredEvents
		 */
		int filterIndex;
		if (analyzer->findFilteredEvent(wakingIndex, &filterIndex)
		    != nullptr)
			eventsWidget->scrollTo(filterIndex);
	}

	unsigned int wcpu = wakingevent->cpu;
	int wpid = wakingevent->pid;

	/*
	 * If the waking task was run with pid 0 = swapper, then just remove
	 * the task from the task toolbar and disable the task actions.
	 */
	if (wpid == 0) {
		taskToolBar->removeTaskGraph();
		setTaskActionsEnabled(false);
		tracePlot->replot();
		return;
	}

	CPUTask *cpuTask = analyzer->findCPUTask(wpid, wcpu);

	/*
	 * If we can't find what we expected, we return, the advanced user
	 * could notice that something fishy is going on by the fact that
	 * no task is selected after this user interaction, white there still
	 * is a task in the task toolbar.
	 */
	if (cpuTask == nullptr || cpuTask->graph == nullptr) {
		tracePlot->replot();
		return;
	}
	QCPGraph *qcpGraph = cpuTask->graph->getQCPGraph();
	if (qcpGraph == nullptr) {
		tracePlot->replot();
		return;
	}

	/* Finally, mark the potential wakeup task as selected */
	int count = qcpGraph->dataCount();
	if (count > 0)
		count--;
	QCPDataRange wholeRange(0,  count);
	QCPDataSelection wholeSelection(wholeRange);
	qcpGraph->setSelection(wholeSelection);
	tracePlot->replot();

	/* Finally update the TaskToolBar to reflect the change in selection */
	TaskGraph *graph = TaskGraph::fromQCPGraph(qcpGraph);
	if (graph == nullptr) {
		taskToolBar->removeTaskGraph();
		setTaskActionsEnabled(false);
		return;
	}
	taskToolBar->setTaskGraph(graph);
	setTaskActionsEnabled(true);
}

void MainWindow::checkStatsTimeLimited()
{
	if (statsLimitedDialog->isVisible()) {
		statsLimitedDialog->beginResetModel();
		analyzer->doLimitedStats();
		statsLimitedDialog->setTaskMap(&analyzer->taskMap,
					       analyzer->getNrCPUs());
		statsLimitedDialog->endResetModel();
	}
}

/* Add a task graph for the currently selected task */
void MainWindow::addTaskGraphTriggered()
{
	addTaskGraph(taskToolBar->getPid());
}

/* Adds the currently selected task to the legend */
void MainWindow::addToLegendTriggered()
{
	taskToolBar->addCurrentTaskToLegend();
}

/* Clears the legend of all tasks */
void MainWindow::clearLegendTriggered()
{
	taskToolBar->clearLegend();
}

/* Finds the preceding wakeup of the currently selected task */
void MainWindow::findWakeupTriggered()
{
	showWakeupOrWaking(taskToolBar->getPid(), SCHED_WAKEUP);
}

/* Finds the preceding waking of the currently selected wakeup event */
void MainWindow::findWakingTriggered()
{
	const TraceEvent *event = eventsWidget->getSelectedEvent();
	if (event != nullptr &&
	    (event->type == SCHED_WAKEUP || event->type == SCHED_WAKEUP_NEW))
		showWaking(event);
}

/* Finds the preceding waking of the currently selected task */
void MainWindow::findWakingDirectTriggered()
{
	showWakeupOrWaking(taskToolBar->getPid(), SCHED_WAKING);
}

/* Finds the next sched_switch event that puts the task to sleep */
void MainWindow::findSleepTriggered()
{
	int activeIdx = infoWidget->getCursorIdx();
	int pid = taskToolBar->getPid();
	int schedIndex;

	if (pid == 0)
		return;
	if (activeIdx != TShark::RED_CURSOR &&
	    activeIdx != TShark::BLUE_CURSOR) {
		return;
	}

	Cursor *activeCursor = cursors[activeIdx];

	if (activeCursor == nullptr)
		return;

	/*
	 * The time of the active cursor is taken to be the time that the
	 * user is interested in, i.e. finding the subsequent sched_swith event
	 * relative to
	 */
	double zerotime = activeCursor->getPosition();
	const TraceEvent *schedevent = analyzer->findNextSchedSleepEvent(
		vtl::Time::fromDouble(zerotime), pid, &schedIndex);

	if (schedevent == nullptr)
		return;

	activeCursor->setPosition(schedevent->time);
	checkStatsTimeLimited();
	infoWidget->setTime(schedevent->time, activeIdx);
	cursorPos[activeIdx] = schedevent->time.toDouble();

	if (!analyzer->isFiltered()) {
		eventsWidget->scrollTo(schedIndex);
	} else {
		/*
		 * If a filter is enabled we need to try to find the index in
		 * analyzer->filteredEvents
		 */
		int filterIndex;
		if (analyzer->findFilteredEvent(schedIndex, &filterIndex)
		    != nullptr)
			eventsWidget->scrollTo(filterIndex);
	}
}

/* Removes the task graph of the currently selected task */
void MainWindow::removeTaskGraphTriggered()
{
	removeTaskGraph(taskToolBar->getPid());
}

/* Filter on the currently selected task */
void MainWindow::taskFilter()
{
	vtl::Time saved = eventsWidget->getSavedScroll();
	int pid = taskToolBar->getPid();

	if (pid == 0)
		return;

	QMap<int, int> map;
	map[pid] = pid;

	eventsWidget->beginResetModel();
	analyzer->createPidFilter(map, false, true);
	setEventsWidgetEvents();
	eventsWidget->endResetModel();
	scrollTo(saved);
	updateResetFiltersEnabled();
}

/* Filter on the currently selected task */
void MainWindow::taskFilterTriggered()
{
	taskFilter();
}

/* Filter on the currently selected task */
void MainWindow::taskFilterLimitedTriggered()
{
	timeFilter();
	taskFilter();
}

bool MainWindow::isWideScreen()
{
	QRect geometry;

	geometry = QApplication::desktop()->availableGeometry();
	return geometry.width() > 1800;
}
