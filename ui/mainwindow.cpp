// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstdio>

#include <QApplication>
#include <QDateTime>
#include <QList>
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
#include "ui/cpuselectdialog.h"
#include "parser/traceevent.h"
#include "ui/traceplot.h"
#include "ui/yaxisticker.h"
#include "misc/errors.h"
#include "misc/resources.h"
#include "misc/setting.h"
#include "misc/settingstore.h"
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

#define CURSOR_ZOOM_TOOLTIP	        \
"Zoom to the time interval defined by the cursors"

#define DEFAULT_ZOOM_TOOLTIP	        \
"Zoom to the default zoom level"

#define TOOLTIP_EXIT			\
"Exit traceshark"

#define TOOLTIP_SHOWTASKS		\
"Show a list of all tasks and it's possible to select one"

#define TOOLTIP_CPUFILTER		\
"Select a subset of CPUs to filter on"

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

#define CLEAR_TASK_TOOLTIP		\
"Remove all the unified task graphs"

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

const double MainWindow::bugWorkAroundOffset = 100;
const double MainWindow::schedSectionOffset = 100;
const double MainWindow::schedSpacing = 250;
const double MainWindow::schedHeight = 950;
const double MainWindow::cpuSectionOffset = 100;
const double MainWindow::cpuSpacing = 100;
const double MainWindow::cpuHeight = 800;
/*
 * const double migrateHeight doesn't exist. The value used is the
 * dynamically calculated inc variable in MainWindow::computeLayout()
 */

const double MainWindow::migrateSectionOffset = 250;

const QString MainWindow::RUNNING_NAME = tr("is runnable");
const QString MainWindow::PREEMPTED_NAME = tr("was preempted");
const QString MainWindow::UNINT_NAME = tr("uninterruptible");

const double MainWindow::RUNNING_SIZE = 8;
const double MainWindow::PREEMPTED_SIZE = 8;
const double MainWindow::UNINT_SIZE = 12;

const QCPScatterStyle::ScatterShape MainWindow::RUNNING_SHAPE =
	QCPScatterStyle::ssTriangle;
const QCPScatterStyle::ScatterShape MainWindow::PREEMPTED_SHAPE =
	QCPScatterStyle::ssTriangle;
const QCPScatterStyle::ScatterShape MainWindow::UNINT_SHAPE =
	QCPScatterStyle::ssPlus;

const QColor MainWindow::RUNNING_COLOR = Qt::blue;
const QColor MainWindow::PREEMPTED_COLOR = Qt::red;
const QColor MainWindow::UNINT_COLOR = QColor(205, 0, 205);

MainWindow::MainWindow():
	tracePlot(nullptr), graphEnableDialog(nullptr), filterActive(false)
{
	settingStore = new SettingStore();
	loadSettings();

	analyzer = new TraceAnalyzer(settingStore);

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
	plotConnections();

	eventsWidget = new EventsWidget(this);
	eventsWidget->setAllowedAreas(Qt::TopDockWidgetArea |
				      Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, eventsWidget);

	cursors[TShark::RED_CURSOR] = nullptr;
	cursors[TShark::BLUE_CURSOR] = nullptr;
	cursorPos[TShark::RED_CURSOR] = 0;
	cursorPos[TShark::BLUE_CURSOR] = 0;

	createDialogs();
	widgetConnections();
	dialogConnections();
}

void MainWindow::createTracePlot()
{
	QString mainLayerName = QString("main");
	QString cursorLayerName = QString("cursor");
	QCPLayer *mainLayer;
	yaxisTicker = new YAxisTicker();
	QSharedPointer<QCPAxisTicker> ticker((QCPAxisTicker*) (yaxisTicker));

	tracePlot = new TracePlot(plotWidget);
	setupOpenGL();

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
	delete cpuSelectDialog;
	delete graphEnableDialog;

	vtl::set_error_handler(nullptr);
	delete errorDialog;

	for (i = 0; i < STATUS_NR; i++)
		delete statusStrings[i];
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	/* Here is a great place to save settings, if we ever want to do it */
	taskSelectDialog->hide();
	eventSelectDialog->hide();
	cpuSelectDialog->hide();
	statsDialog->hide();
	statsLimitedDialog->hide();
	event->accept();
	/* event->ignore() could be used to refuse to close the window */
}

void MainWindow::openTrace()
{
	QString name;
	QString caption = tr("Open a trace file");
	QFileDialog::Options options = QFileDialog::DontUseNativeDialog |
		QFileDialog::DontUseSheet;

	name = QFileDialog::getOpenFileName(this, caption, QString(),
					    tr("ASCII (*.asc *.txt)"), nullptr,
					    options);
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
		setupOpenGL();

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

		cpuSelectDialog->beginResetModel();
		cpuSelectDialog->setNrCPUs(analyzer->getNrCPUs());
		cpuSelectDialog->endResetModel();

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
		setCloseActionsEnabled(true);
		if (analyzer->events->size() <= 0)
			vtl::warnx("You have opened an empty trace!");
		else
			setTraceActionsEnabled(true);
	} else {
		setStatus(STATUS_ERROR);
		vtl::warnx("Unknown error when opening trace!");
	}
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

	if (analyzer->enableMigrations()) {
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

	if (settingStore->getValue(Setting::SHOW_SCHED_GRAPHS).boolv()) {
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

	if (settingStore->getValue(Setting::SHOW_CPUFREQ_GRAPHS).boolv() ||
	    settingStore->getValue(Setting::SHOW_CPUIDLE_GRAPHS).boolv()) {
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
	int maxwakeup;
	const Setting::Value &maxvalue =
		settingStore->getValue(Setting::MAX_VRT_WAKEUP_LATENCY);;

	maxwakeup = maxvalue.intv();
	CPUTask::setVerticalWakeupMAX(maxwakeup);
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


	if (!settingStore->getValue(Setting::SHOW_CPUFREQ_GRAPHS).boolv() &&
	    !settingStore->getValue(Setting::SHOW_CPUIDLE_GRAPHS).boolv())
		goto skipIdleFreqGraphs;

	/* Show CPU frequency and idle graphs */
	for (cpu = 0; cpu <= analyzer->getMaxCPU(); cpu++) {
		QPen pen = QPen();
		QPen penF = QPen();

		QCPGraph *graph;
		QString name;
		QCPScatterStyle style;

		if (settingStore->getValue(Setting::SHOW_CPUIDLE_GRAPHS)
		    .boolv()) {
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

		if (settingStore->getValue(Setting::SHOW_CPUFREQ_GRAPHS)
		    .boolv()) {
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

			addSchedGraph(task, cpu);
			if (settingStore->getValue(Setting::SHOW_SCHED_GRAPHS)
			    .boolv())
			{
				addHorizontalWakeupGraph(task);
				addWakeupGraph(task);
				addPreemptedGraph(task);
				addStillRunningGraph(task);
				addUninterruptibleGraph(task);
			}
		}
	}

	tracePlot->replot();
}

void MainWindow::loadSettings()
{
	int ts_errno;

	ts_errno = settingStore->loadSettings();
	if (ts_errno != 0)
		vtl::warn(ts_errno, "Failed to load settings from %s",
			  TS_SETTING_FILENAME);
}

void MainWindow::setupCursors()
{
	double start, end, red, blue;

	start = analyzer->getStartTime().toDouble();
	end = analyzer->getEndTime().toDouble();

	red = (start + end) / 2;
	blue = (start + end) / 2 + (end - start) / 10;

	setupCursors(red, blue);
}

void MainWindow::setupCursors(const double &red, const double &blue)
{
	vtl::Time redtime = vtl::Time::fromDouble(red);
	redtime.setPrecision(analyzer->getTimePrecision());
	vtl::Time bluetime = vtl::Time::fromDouble(blue);
	bluetime.setPrecision(analyzer->getTimePrecision());

	_setupCursors(redtime, red, bluetime, blue);
}

void MainWindow::setupCursors(const vtl::Time &redtime,
			      const vtl::Time &bluetime)
{
	double red = redtime.toDouble();
	double blue = bluetime.toDouble();

	_setupCursors(redtime, red, bluetime, blue);
}

void MainWindow::_setupCursors(vtl::Time redtime, const double &red,
			       vtl::Time bluetime, const double &blue)
{
	cursors[TShark::RED_CURSOR] = new Cursor(tracePlot,
						 TShark::RED_CURSOR);
	cursors[TShark::BLUE_CURSOR] = new Cursor(tracePlot,
						  TShark::BLUE_CURSOR);

	cursors[TShark::RED_CURSOR]->setLayer(cursorLayer);
	cursors[TShark::BLUE_CURSOR]->setLayer(cursorLayer);

	cursors[TShark::RED_CURSOR]->setPosition(redtime);
	cursorPos[TShark::RED_CURSOR] = red;
	infoWidget->setTime(redtime, TShark::RED_CURSOR);

	cursors[TShark::BLUE_CURSOR]->setPosition(bluetime);
	cursorPos[TShark::BLUE_CURSOR] = blue;
	infoWidget->setTime(bluetime, TShark::BLUE_CURSOR);
	checkStatsTimeLimited();

	scrollTo(redtime);
}

void MainWindow::addSchedGraph(CPUTask &cpuTask, unsigned int cpu)
{
	/* Add scheduling graph */
	TaskGraph *graph = new TaskGraph(tracePlot, cpu,
					 TaskGraph::GRAPH_CPUGRAPH);
	QColor color = analyzer->getTaskColor(cpuTask.pid);
	Task *task = analyzer->findTask(cpuTask.pid);
	QPen pen = QPen();

	pen.setColor(color);
	pen.setWidth(settingStore->getValue(Setting::LINE_WIDTH).intv());
	graph->setPen(pen);
	graph->setTask(task);
	if (settingStore->getValue(Setting::SHOW_SCHED_GRAPHS).boolv())
		graph->setData(cpuTask.schedTimev, cpuTask.scaledSchedData);
	/*
	 * Save a pointer to the graph object in the task. The destructor of
	 * AbstractClass will delete this when it is destroyed.
	 */
	cpuTask.graph = graph;
}

void MainWindow::addHorizontalWakeupGraph(CPUTask &task)
{
	if (!settingStore->getValue(Setting::HORIZONTAL_WAKEUP).boolv())
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
	pen.setWidth(settingStore->getValue(Setting::LINE_WIDTH).intv());
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
	if (!settingStore->getValue(Setting::VERTICAL_WAKEUP).boolv())
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
	pen.setWidth(settingStore->getValue(Setting::LINE_WIDTH).intv());
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

void MainWindow::addGenericAccessoryGraph(const QString &name,
					  const QVector<double> &timev,
					  const QVector<double> &scaledData,
					  QCPScatterStyle::ScatterShape sshape,
					  double size,
					  const QColor &color)
{
	/* Add still running graph on top of the other two...*/
	if (timev.size() == 0)
		return;
	QCPGraph *graph = tracePlot->addGraph(tracePlot->xAxis,
					      tracePlot->yAxis);
	graph->setName(name);
	QCPScatterStyle style = QCPScatterStyle(sshape, size);
	QPen pen = QPen();

	pen.setColor(color);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(timev, scaledData);
}

void MainWindow::addPreemptedGraph(CPUTask &task)
{
	addGenericAccessoryGraph(PREEMPTED_NAME, task.preemptedTimev,
				 task.scaledPreemptedData,
				 PREEMPTED_SHAPE, PREEMPTED_SIZE,
				 PREEMPTED_COLOR);
}

void MainWindow::addStillRunningGraph(CPUTask &task)
{
	addGenericAccessoryGraph(RUNNING_NAME, task.runningTimev,
				 task.scaledRunningData,
				 RUNNING_SHAPE, RUNNING_SIZE,
				 RUNNING_COLOR);
}

void MainWindow::addUninterruptibleGraph(CPUTask &task)
{
	addGenericAccessoryGraph(UNINT_NAME,
				 task.uninterruptibleTimev,
				 task.scaledUninterruptibleData,
				 UNINT_SHAPE, UNINT_SIZE,
				 UNINT_COLOR);
}

/*
 * These are actions that should be enabled whenever we have a non-empty
 * trace open
 */
void MainWindow::setTraceActionsEnabled(bool e)
{
	infoWidget->setTraceActionsEnabled(e);

	saveAction->setEnabled(e);
	exportEventsAction->setEnabled(e);
	exportCPUAction->setEnabled(e);
	cursorZoomAction->setEnabled(e);
	defaultZoomAction->setEnabled(e);
	showTasksAction->setEnabled(e);
	filterCPUsAction->setEnabled(e);
	showEventsAction->setEnabled(e);
	timeFilterAction->setEnabled(e);
	showStatsAction->setEnabled(e);
	showStatsTimeLimitedAction->setEnabled(e);
}

void MainWindow::setLegendActionsEnabled(bool e)
{
	clearLegendAction->setEnabled(e);
}

/*
 * These are action that should be enabled whenever we have a trace open
 */
void MainWindow::setCloseActionsEnabled(bool e)
{
	closeAction->setEnabled(e);
}

/*
 * These are actions that should be enabled whenever a task is selected
 */
void MainWindow::setTaskActionsEnabled(bool e)
{
	findWakeupAction->setEnabled(e);
	findWakingDirectAction->setEnabled(e);
	findSleepAction->setEnabled(e);
	taskFilterAction->setEnabled(e);
	taskFilterLimitedAction->setEnabled(e);
}

void MainWindow::setAddToLegendActionEnabled(bool e)
{
	addToLegendAction->setEnabled(e);
}

void MainWindow::setWakeupActionsEnabled(bool e)
{
	findWakingAction->setEnabled(e);
}

void MainWindow::setAddTaskGraphActionEnabled(bool e)
{
	addTaskGraphAction->setEnabled(e);
}

void MainWindow::setTaskGraphRemovalActionEnabled(bool e)
{
	removeTaskGraphAction->setEnabled(e);
}

void MainWindow::setTaskGraphClearActionEnabled(bool e)
{
	clearTaskGraphsAction->setEnabled(e);
}

void MainWindow::closeTrace()
{
	int ts_errno = 0;
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

	cpuSelectDialog->beginResetModel();
	cpuSelectDialog->setNrCPUs(0);
	cpuSelectDialog->endResetModel();

	clearPlot();
	if(analyzer->isOpen()) {
		analyzer->close(&ts_errno);
	}
	taskToolBar->clear();
	setTraceActionsEnabled(false);
	setLegendActionsEnabled(false);
	setCloseActionsEnabled(false);
	setTaskActionsEnabled(false);
	setWakeupActionsEnabled(false);
	setAddTaskGraphActionEnabled(false);
	setTaskGraphRemovalActionEnabled(false);
	setTaskGraphClearActionEnabled(false);
	setAddToLegendActionEnabled(false);
	setStatus(STATUS_NOFILE);
	if (ts_errno != 0)
		vtl::warn(ts_errno, "Failed to close() trace file");
}

void MainWindow::saveScreenshot()
{
	QStringList fileNameList;
	QString fileName;
	QString pngSuffix = QString(".png");
	QString bmpSuffix = QString(".bmp");
	QString jpgSuffix = QString(".jpg");
	QString pdfSuffix = QString(".pdf");
	QString pdfCreator = QString("traceshark ");
	QString pdfTitle;
	QString diagcapt;
	QFileDialog::Options options;

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

	diagcapt = tr("Save screenshot to image");
	options = QFileDialog::DontUseNativeDialog | QFileDialog::DontUseSheet;
	fileName = QFileDialog::getSaveFileName(this, diagcapt, QString(),
						tr("Images (*.png *.bmp *.jpg *.pdf)"),
						nullptr, options);
	if (fileName.isEmpty())
		return;

	if (fileName.endsWith(pngSuffix, Qt::CaseInsensitive)) {
		tracePlot->savePng(fileName);
	} else if (fileName.endsWith(bmpSuffix, Qt::CaseInsensitive)) {
		tracePlot->saveBmp(fileName);
	} else if (fileName.endsWith(jpgSuffix, Qt::CaseInsensitive)) {
		tracePlot->saveJpg(fileName);
	} else if (fileName.endsWith(pdfSuffix, Qt::CaseInsensitive)) {
		tracePlot->savePdf(fileName, 0, 0,  QCP::epAllowCosmetic,
				   pdfCreator, pdfTitle);
	} else {
		tracePlot->savePng(fileName + pngSuffix);
	}
}

void MainWindow::cursorZoom()
{
	double min, max;

	/* Give up if both cursors are exactly on the same location */
	if (cursorPos[TShark::RED_CURSOR] == cursorPos[TShark::BLUE_CURSOR])
		return;

	min = TSMIN(cursorPos[TShark::RED_CURSOR],
		    cursorPos[TShark::BLUE_CURSOR]);
	max = TSMAX(cursorPos[TShark::RED_CURSOR],
		    cursorPos[TShark::BLUE_CURSOR]);

	tracePlot->xAxis->setRange(QCPRange(min, max));
	tracePlot->replot();
}

void MainWindow::defaultZoom()
{
	double start, end;

	start = analyzer->getStartTime().toDouble();
	end = analyzer->getEndTime().toDouble();

	tracePlot->yAxis->setRange(QCPRange(bottom, top));
	tracePlot->xAxis->setRange(QCPRange(start, end));
	tracePlot->replot();
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
	       "<p>Copyright &copy; 2014-2019 Viktor Rosendahl"
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
	if (licenseDialog->isVisible())
		licenseDialog->hide();
	else
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
	eventInfoDialog->show(event, *analyzer->getTraceFile());
}

void MainWindow::taskTriggered(int pid)
{
	selectTaskByPid(pid, nullptr, PR_TRY_TASKGRAPH);
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

	filterCPUsAction = new QAction(tr("Filter on CPUs..."), this);
	filterCPUsAction->setIcon(QIcon(RESSRC_PNG_CPUFILTER));
	filterCPUsAction->setToolTip(tr(TOOLTIP_CPUFILTER));
	tsconnect(filterCPUsAction, triggered(), this, filterOnCPUs());

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

	cursorZoomAction = new QAction(tr("Cursor zoom"), this);
	cursorZoomAction->setIcon(QIcon(RESSRC_PNG_CURSOR_ZOOM));
	cursorZoomAction->setToolTip(tr(CURSOR_ZOOM_TOOLTIP));
	tsconnect(cursorZoomAction, triggered(), this, cursorZoom());

	defaultZoomAction = new QAction(tr("Default zoom"), this);
	defaultZoomAction->setIcon(QIcon(RESSRC_PNG_DEFAULT_ZOOM));
	defaultZoomAction->setToolTip(tr(DEFAULT_ZOOM_TOOLTIP));
	tsconnect(defaultZoomAction, triggered(), this,
		  defaultZoom());

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
	tsconnect(addToLegendAction, triggered(), this, addToLegendTriggered());

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

	clearTaskGraphsAction = new QAction(tr("Remove task graph"), this);
	clearTaskGraphsAction->setIcon(QIcon(RESSRC_PNG_CLEAR_TASK));
	clearTaskGraphsAction->setToolTip(tr(CLEAR_TASK_TOOLTIP));
	tsconnect(clearTaskGraphsAction, triggered(), this,
		  clearTaskGraphsTriggered());

	taskFilterAction = new QAction(tr("Filter on selected task"), this);
	taskFilterAction->setIcon(QIcon(RESSRC_PNG_FILTERCURRENT));
	taskFilterAction->setToolTip(tr(TASK_FILTER_TOOLTIP));
	tsconnect(taskFilterAction, triggered(), this,
		  taskFilterTriggered());

	taskFilterLimitedAction =
		new QAction(tr("Filter on selected task (time limited)"), this);
	taskFilterLimitedAction->setIcon(QIcon(RESSRC_PNG_FILTERCURRENT_LIMIT));
	taskFilterLimitedAction->setToolTip(tr(TASK_FILTER_TIMELIMIT_TOOLTIP));
	tsconnect(taskFilterLimitedAction, triggered(), this,
		  taskFilterLimitedTriggered());

	setTraceActionsEnabled(false);
	setLegendActionsEnabled(false);
	setCloseActionsEnabled(false);
	setTaskActionsEnabled(false);
	setWakeupActionsEnabled(false);
	setAddTaskGraphActionEnabled(false);
	setTaskGraphRemovalActionEnabled(false);
	setTaskGraphClearActionEnabled(false);
	setAddToLegendActionEnabled(false);
}

void MainWindow::createToolBars()
{
	bool widescreen = Setting::isWideScreen();

	fileToolBar = new QToolBar(tr("&File"));
	addToolBar(Qt::LeftToolBarArea, fileToolBar);
	fileToolBar->addAction(openAction);
	fileToolBar->addAction(closeAction);
	fileToolBar->addAction(saveAction);
	fileToolBar->addAction(exportEventsAction);
	fileToolBar->addAction(exportCPUAction);

	viewToolBar = new QToolBar(tr("&View"));
	addToolBar(Qt::LeftToolBarArea, viewToolBar);
	viewToolBar->addAction(cursorZoomAction);
	viewToolBar->addAction(defaultZoomAction);
	viewToolBar->addAction(showTasksAction);
	viewToolBar->addAction(filterCPUsAction);
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
	taskToolBar->addAction(clearTaskGraphsAction);
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
	viewMenu->addAction(cursorZoomAction);
	viewMenu->addAction(defaultZoomAction);
	viewMenu->addAction(showTasksAction);
	viewMenu->addAction(filterCPUsAction);
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
	taskMenu->addAction(clearTaskGraphsAction);
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

void MainWindow::createDialogs()
{
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
	cpuSelectDialog = new CPUSelectDialog();
	graphEnableDialog = new GraphEnableDialog(settingStore, nullptr);

	vtl::set_error_handler(errorDialog);
}

void MainWindow::plotConnections()
{
	tsconnect(tracePlot, mouseWheel(QWheelEvent*), this, mouseWheel());
	tsconnect(tracePlot->xAxis, rangeChanged(QCPRange), tracePlot->xAxis2,
		  setRange(QCPRange));
	tsconnect(tracePlot, mousePress(QMouseEvent*), this, mousePress());
	tsconnect(tracePlot, selectionChangedByUser() , this,
		  selectionChanged());
	tsconnect(tracePlot, legendDoubleClick(QCPLegend*,
					       QCPAbstractLegendItem*,
					       QMouseEvent*), this,
		  legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*));
	tsconnect(tracePlot, mouseDoubleClick(QMouseEvent*),
		  this, plotDoubleClicked(QMouseEvent*));
}

void MainWindow::widgetConnections()
{
	tsconnect(infoWidget, valueChanged(vtl::Time, int),
		  this, infoValueChanged(vtl::Time, int));

	/* Events widget */
	tsconnect(eventsWidget, timeSelected(vtl::Time), this,
		  moveActiveCursor(vtl::Time));
	tsconnect(eventsWidget, infoDoubleClicked(const TraceEvent &),
		  this, showEventInfo(const TraceEvent &));
	tsconnect(eventsWidget, eventSelected(const TraceEvent *),
		  this, handleEventSelected(const TraceEvent *));

	/* TaskToolBar widget */
	tsconnect(taskToolBar, LegendEmptyChanged(bool), this,
		  legendEmptyChanged(bool));
}

void MainWindow::dialogConnections()
{
	/* task select dialog */
	tsconnect(taskSelectDialog, addTaskGraph(int), this, addTaskGraph(int));
	tsconnect(taskSelectDialog, needReplot(), this, doReplot());
	tsconnect(taskSelectDialog, needLegendCheck(), this, doLegendCheck());
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
	tsconnect(statsDialog, needReplot(), this, doReplot());
	tsconnect(statsDialog, needLegendCheck(), this, doLegendCheck());
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
	tsconnect(statsLimitedDialog, needReplot(), this, doReplot());
	tsconnect(statsLimitedDialog, addTaskToLegend(int), this,
		  addTaskToLegend(int));
	tsconnect(statsLimitedDialog, needLegendCheck(), this, doLegendCheck());
	tsconnect(statsLimitedDialog,
		  createFilter(QMap<int, int> &, bool, bool),
		  this, createPidFilter(QMap<int, int> &, bool, bool));
	tsconnect(statsLimitedDialog, resetFilter(), this, resetPidFilter());
	tsconnect(statsLimitedDialog, QDockWidgetNeedsRemoval(QDockWidget*),
		  this, removeQDockWidget(QDockWidget*));
	tsconnect(statsLimitedDialog, taskDoubleClicked(int),
		  this, taskTriggered(int));

	/* the CPU filter dialog */
	tsconnect(cpuSelectDialog, createFilter(QMap<unsigned, unsigned> &,
						bool),
		  this, createCPUFilter(QMap<unsigned, unsigned> &, bool));
	tsconnect(cpuSelectDialog, resetFilter(), this, resetCPUFilter());

	/* event select dialog */
	tsconnect(eventSelectDialog, createFilter(QMap<event_t, event_t> &,
						  bool),
		  this, createEventFilter(QMap<event_t, event_t> &, bool));
	tsconnect(eventSelectDialog, resetFilter(), this, resetEventFilter());

	/* graph enable dialog */
	tsconnect(graphEnableDialog, settingsChanged(),
		  this, consumeSettings());
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

void MainWindow::selectionChanged()
{
	TaskGraph *graph = selectedGraph();
	if (graph == nullptr) {
		setTaskActionsEnabled(false);
		taskToolBar->removeTaskGraph();
		setTaskGraphRemovalActionEnabled(false);
		setAddTaskGraphActionEnabled(false);
		setAddToLegendActionEnabled(false);
		return;
	}

	setTaskActionsEnabled(true);
	taskToolBar->setTaskGraph(graph);
	updateTaskGraphActions();
	updateAddToLegendAction();
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

void MainWindow::legendEmptyChanged(bool empty)
{
	setLegendActionsEnabled(!empty);
}

void MainWindow::handleLegendGraphDoubleClick(QCPGraph *graph)
{
	TaskGraph *tgraph = TaskGraph::fromQCPGraph(graph);
	const Task *task;

	if (tgraph == nullptr)
		return;
	tgraph->removeFromLegend();
	task = tgraph->getTask();
	/*
	 * Inform the TaskToolBar class that the pid has been removed. This is
	 * needed because TaskToolBar keeps track of this for the purpose of
	 * preventing the same pid being added twice from different legend 
	 * graphs, there might be "identical" legend graphs when the same pid
	 * has migrated between CPUs
	 */
	if (task != nullptr)
		taskToolBar->pidRemoved(task->pid);
	updateAddToLegendAction();
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
	} else {
		resetFiltersAction->setEnabled(false);
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

void MainWindow::createCPUFilter(QMap<unsigned, unsigned> &map, bool orlogic)
{
	vtl::Time saved = eventsWidget->getSavedScroll();

	eventsWidget->beginResetModel();
	analyzer->createCPUFilter(map, orlogic);
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

void MainWindow::resetCPUFilter()
{
	vtl::Time saved;

	if (!analyzer->filterActive(FilterState::FILTER_CPU))
		return;

	saved = eventsWidget->getSavedScroll();
	eventsWidget->beginResetModel();
	analyzer->disableFilter(FilterState::FILTER_CPU);
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
	QStringList fileNameList;
	QString fileName;
	int ts_errno;
	QString caption;
	QFileDialog::Options options;

	if (analyzer->events->size() <= 0) {
		vtl::warnx("The trace is empty. There is nothing to export");
		return;
	}

	if (analyzer->getTraceType() != TRACE_TYPE_PERF) {
		vtl::warnx("The trace type is not perf. Only perf traces can be exported");
		return;
	}

	switch (export_type) {
	case TraceAnalyzer::EXPORT_TYPE_CPU_CYCLES:
		caption = tr("Export CPU cycles events");
		break;
	case TraceAnalyzer::EXPORT_TYPE_ALL:
		caption = tr("Export all filtered events");
		break;
	default:
		caption = tr("Unknown export");
		break;
	}

	options = QFileDialog::DontUseNativeDialog | QFileDialog::DontUseSheet;
	fileName = QFileDialog::getSaveFileName(this, caption, QString(),
						tr("ASCII Text (*.asc *.txt)"),
						nullptr, options);
	if (fileName.isEmpty())
		return;

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
	QList<int> taskGraphs;
	QList<int> legendPids;
	vtl::Time redtime, bluetime;
	bool selected = false;
	bool unified = false;
	int selected_pid = 0;
	unsigned int selected_cpu = 0;
	TaskGraph *selected_graph;
	enum TaskGraph::GraphType graph_type;

	if (!analyzer->isOpen()) {
		setupOpenGL();
		graphEnableDialog->checkConsumption();
		return;
	}

	/* Save the PIDs of the tasks that have a task graph */
	taskGraphs = taskRangeAllocator->getPidList();

	/* Save the Pids of the tasks that have a legend */
	legendPids = taskToolBar->legendPidList();

	/* Save the cursor time */
	Cursor *redCursor = cursors[TShark::RED_CURSOR];
	Cursor *blueCursor = cursors[TShark::BLUE_CURSOR];

	if (redCursor != nullptr)
		redtime = redCursor->getTime();
	if (blueCursor != nullptr)
		bluetime = blueCursor->getTime();

	/* Save the zoom */
	QCPRange savedRangeX = tracePlot->xAxis->range();

	/* Save wether a task was selected */
	selected_graph = selectedGraph();
	if (selected_graph != nullptr) {
		selected = true;
		selected_cpu = selected_graph->getCPU();
		selected_pid = selected_graph->getPid();
		graph_type = selected_graph->getGraphType();
		unified = graph_type == TaskGraph::GRAPH_UNIFIED;
	}

	clearPlot();
	setupOpenGL();
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
			task->uninterruptibleGraph = nullptr;
		}
	}

	computeLayout();
	setupCursors(redtime, bluetime);
	rescaleTrace();
	showTrace();
	tracePlot->show();

	tracePlot->xAxis->setRange(savedRangeX);
	/* Restore the task graphs from the list */
	QList<int>::const_iterator j;
	for (j = taskGraphs.begin(); j != taskGraphs.end(); j++)
		addTaskGraph(*j);

	/* Restore the legends from the list */
	for (j = legendPids.begin(); j != legendPids.end(); j++)
		addTaskToLegend(*j);

	if (selected) {
		/* Restore the graph selection */
		if (unified)
			selectTaskByPid(selected_pid, nullptr,
					PR_TRY_TASKGRAPH);
		else
			selectTaskByPid(selected_pid, &selected_cpu,
					PR_CPUGRAPH_ONLY);
	} else {
		/* No task was selected */
		tracePlot->replot();
		setTaskActionsEnabled(false);
		updateAddToLegendAction();
		updateTaskGraphActions();
	}
	graphEnableDialog->checkConsumption();
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

	taskGraph = new TaskGraph(tracePlot, 0, TaskGraph::GRAPH_UNIFIED);
	taskGraph->setTaskGraphForLegend(cpuTask->graph);
	QPen pen = QPen();

	pen.setColor(color);
	pen.setWidth(settingStore->getValue(Setting::LINE_WIDTH).intv());
	taskGraph->setPen(pen);
	taskGraph->setTask(task);

	task->offset = taskRange->lower;
	task->scale = schedHeight;
	task->doScale();
	task->doScaleWakeup();
	task->doScaleRunning();
	task->doScalePreempted();
	task->doScaleUnint();

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

	addStillRunningTaskGraph(task);
	addPreemptedTaskGraph(task);
	addUninterruptibleTaskGraph(task);

	/*
	 * We only modify the lower part of the range to show the newly
	 * added unified task graph.
	 */
	QCPRange range = tracePlot->yAxis->range();
	tracePlot->yAxis->setRange(QCPRange(bottom, range.upper));

	updateTaskGraphActions();
}

void MainWindow::doReplot()
{
	tracePlot->replot();
}

void MainWindow::doLegendCheck()
{
	updateAddToLegendAction();
}

void MainWindow::addAccessoryTaskGraph(QCPGraph **graphPtr,
				       const QString &name,
				       const QVector<double> &timev,
				       const QVector<double> &scaledData,
				       QCPScatterStyle::ScatterShape sshape,
				       double size,
				       const QColor &color)
{
	/* Add the still running graph on top of the other two... */
	QCPGraph *graph;
	QPen pen;
	QCPScatterStyle style = QCPScatterStyle(sshape, size);
	if (timev.size() <= 0) {
		*graphPtr = nullptr;
		return;
	}
	graph = tracePlot->addGraph(tracePlot->xAxis, tracePlot->yAxis);
	graph->setName(name);
	pen.setColor(color);
	style.setPen(pen);
	graph->setScatterStyle(style);
	graph->setLineStyle(QCPGraph::lsNone);
	graph->setAdaptiveSampling(true);
	graph->setData(timev, scaledData);
	*graphPtr = graph;
}

void MainWindow::addStillRunningTaskGraph(Task *task)
{
	addAccessoryTaskGraph(&task->runningGraph, RUNNING_NAME,
			      task->runningTimev, task->scaledRunningData,
			      RUNNING_SHAPE, RUNNING_SIZE, RUNNING_COLOR);
}

void MainWindow::addPreemptedTaskGraph(Task *task)
{
	addAccessoryTaskGraph(&task->preemptedGraph, PREEMPTED_NAME,
			      task->preemptedTimev, task->scaledPreemptedData,
			      PREEMPTED_SHAPE, PREEMPTED_SIZE, PREEMPTED_COLOR);
}

void MainWindow::addUninterruptibleTaskGraph(Task *task)
{
	addAccessoryTaskGraph(&task->uninterruptibleGraph, UNINT_NAME,
			      task->uninterruptibleTimev,
			      task->scaledUninterruptibleData,
			      UNINT_SHAPE, UNINT_SIZE, UNINT_COLOR);
}

void MainWindow::removeTaskGraph(int pid)
{
	Task *task = analyzer->findTask(pid);
	QCPGraph *qcpGraph;

	if (task == nullptr) {
		setTaskGraphClearActionEnabled(
			!taskRangeAllocator->isEmpty());
		return;
	}

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

	if (task->uninterruptibleGraph != nullptr) {
		tracePlot->removeGraph(task->uninterruptibleGraph);
		task->uninterruptibleGraph = nullptr;
	}

	taskRangeAllocator->putTaskRange(pid);
	bottom = taskRangeAllocator->getBottom();

	QCPRange range = tracePlot->yAxis->range();
	tracePlot->yAxis->setRange(QCPRange(bottom, range.upper));

	tracePlot->replot();
	updateTaskGraphActions();
}

void MainWindow::clearTaskGraphsTriggered()
{
	TaskRange r;
	int pid;
	Task *task;
	QCPGraph *qcpGraph;
	TaskRangeAllocator::iterator iter;

	for (iter = taskRangeAllocator->begin();
	     iter != taskRangeAllocator->end();
	     iter++) {
		r = iter.value();
		pid = r.pid;
		task = analyzer->findTask(pid);
		if (task == nullptr)
			continue;

		if (task->graph == nullptr)
			continue;

		qcpGraph = task->graph->getQCPGraph();
		if (qcpGraph != nullptr && qcpGraph->selected() &&
		    taskToolBar->getPid() == task->pid)
			taskToolBar->removeTaskGraph();
		task->graph->destroy();
		task->graph = nullptr;

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

		if (task->uninterruptibleGraph != nullptr) {
			tracePlot->removeGraph(task->uninterruptibleGraph);
			task->uninterruptibleGraph = nullptr;
		}
	}

	taskRangeAllocator->clearAll();
	bottom = taskRangeAllocator->getBottom();

	QCPRange range = tracePlot->yAxis->range();
	tracePlot->yAxis->setRange(QCPRange(bottom, range.upper));

	tracePlot->replot();
	updateTaskGraphActions();
}

void MainWindow::updateTaskGraphActions()
{
	setTaskGraphClearActionEnabled(!taskRangeAllocator->isEmpty());
	int spid = taskToolBar->getPid();
	if (spid != 0) {
		bool TaskGraph_selected = taskRangeAllocator->contains(spid);
		setTaskGraphRemovalActionEnabled(TaskGraph_selected);
		setAddTaskGraphActionEnabled(!TaskGraph_selected);
	} else {
		setTaskGraphRemovalActionEnabled(false);
		setAddTaskGraphActionEnabled(false);
	}
}

void MainWindow::updateAddToLegendAction()
{
	int pid = taskToolBar->getPid();
	if (pid == 0) {
		/* No task is selected */
		setAddToLegendActionEnabled(false);
		return;
	}
	setAddToLegendActionEnabled(!taskToolBar->legendContains(pid));
}

TaskGraph *MainWindow::selectedGraph()
{
	TaskGraph *graph = nullptr;
	QCPGraph *qcpGraph = nullptr;
	QCPAbstractPlottable *plottable;
	QList<QCPAbstractPlottable*> plist = tracePlot->selectedPlottables();
	QList<QCPAbstractPlottable*>::const_iterator iter;

	for (iter = plist.begin(); iter != plist.end(); iter++) {
		plottable = *iter;
		qcpGraph = qobject_cast<QCPGraph *>(plottable);
		if (qcpGraph == nullptr)
			continue;
		graph = TaskGraph::fromQCPGraph(qcpGraph);
		if (graph == nullptr)
			continue;
	}

	if (qcpGraph == nullptr || !qcpGraph->selected())
		return nullptr;
	return graph;
}

void MainWindow::showTaskSelector()
{
	if (taskSelectDialog->isVisible()) {
		taskSelectDialog->hide();
		return;
	}
	taskSelectDialog->show();
	if (dockWidgetArea(taskSelectDialog) == Qt::NoDockWidgetArea)
		addDockWidget(Qt::LeftDockWidgetArea, taskSelectDialog);

	if (dockWidgetArea(statsDialog) == Qt::LeftDockWidgetArea)
		tabifyDockWidget(statsDialog, taskSelectDialog);
}

void MainWindow::filterOnCPUs()
{
	if (cpuSelectDialog->isVisible())
		cpuSelectDialog->hide();
	else
		cpuSelectDialog->show();
}

void MainWindow::showEventFilter()
{
	if (eventSelectDialog->isVisible())
		eventSelectDialog->hide();
	else
		eventSelectDialog->show();
}

void MainWindow::showGraphEnable()
{
	if (graphEnableDialog->isVisible())
		graphEnableDialog->hide();
	else
		graphEnableDialog->show();
}

void MainWindow::showStats()
{
	if (statsDialog->isVisible()) {
		statsDialog->hide();
		return;
	}
	statsDialog->show();
	if (dockWidgetArea(statsDialog) == Qt::NoDockWidgetArea)
		addDockWidget(Qt::LeftDockWidgetArea, statsDialog);

	if (dockWidgetArea(taskSelectDialog) == Qt::LeftDockWidgetArea)
		tabifyDockWidget(taskSelectDialog, statsDialog);
}

void MainWindow::showStatsTimeLimited()
{
	if (statsLimitedDialog->isVisible()) {
		statsLimitedDialog->hide();
		return;
	}
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
		oops_warnx();
		return;
	}

	inactiveIdx = TShark::RED_CURSOR;
	if (activeIdx == inactiveIdx)
		inactiveIdx = TShark::BLUE_CURSOR;

	Cursor *activeCursor = cursors[activeIdx];
	Cursor *inactiveCursor = cursors[inactiveIdx];

	if (activeCursor == nullptr || inactiveCursor == nullptr) {
		oops_warnx();
		return;
	}

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

	selectTaskByPid(wpid, &wcpu, PR_TRY_TASKGRAPH);
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

	selectTaskByPid(wpid, &wcpu, PR_TRY_TASKGRAPH);
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

bool MainWindow::selectQCPGraph(QCPGraph *graph)
{
	int end = graph->dataCount() - 1;
	if (end < 0)
		return false;
	QCPDataRange wholeRange(0,  end);
	QCPDataSelection wholeSelection(wholeRange);
	graph->setSelection(wholeSelection);
	return true;
}

/* Add a task graph for the currently selected task */
void MainWindow::addTaskGraphTriggered()
{
	addTaskGraph(taskToolBar->getPid());
	doReplot();
}

void MainWindow::selectTaskByPid(int pid, const unsigned int *preferred_cpu,
				 preference_t preference)
{
	Task *task;
	QCPGraph *qcpGraph;
	CPUTask *cpuTask;
	TaskGraph *graph = nullptr;
	unsigned int cpu;
	int maxSize;
	CPUTask *maxTask;

	/* Deselect the selected task */
	tracePlot->deselectAll();

	/*
	 * If the task to be selected is pid 0, that is swapper, then remove
	 * the task from the task toolbar and disable the task actions.
	 */
	if (pid == 0)
		goto out;

	if (preference == PR_CPUGRAPH_ONLY)
		goto do_cpugraph;

	task = analyzer->findTask(pid);

	/* task is always supposed to be != nullptr, so display warning */
	if (task == nullptr) {
		oops_warnx();
		goto out;
	}

	if (task->graph == nullptr)
		goto do_cpugraph;
	qcpGraph = task->graph->getQCPGraph();
	if (qcpGraph == nullptr)
		goto do_cpugraph;
	selectQCPGraph(qcpGraph);
	graph = task->graph;
	goto out;

do_cpugraph:

	/*
	 * If no preference is given, we will selected the CPU graph with the
	 * highest number of scheduling events.
	 */
	if (preferred_cpu == nullptr) {
		maxTask = nullptr;
		maxSize = -1;
		for (cpu = 0; cpu < analyzer->getNrCPUs(); cpu++) {
			cpuTask = analyzer->findCPUTask(pid, cpu);
			if (cpuTask != nullptr) {
				if (cpuTask->schedTimev.size() > maxSize) {
					maxSize = cpuTask->schedTimev.size();
					maxTask = cpuTask;
				}
			}
		}
		cpuTask = maxTask;
	} else {
		cpuTask = analyzer->findCPUTask(pid, *preferred_cpu);
	}
	/* If we can't find what we expected we warn the user */
	if (cpuTask == nullptr || cpuTask->graph == nullptr) {
		oops_warnx();
		goto out;
	}
	qcpGraph = cpuTask->graph->getQCPGraph();
	if (qcpGraph == nullptr) {
		oops_warnx();
		goto out;
	}

	selectQCPGraph(qcpGraph);

	/* Finally update the TaskToolBar to reflect the change in selection */
	graph = TaskGraph::fromQCPGraph(qcpGraph);
	if (graph == nullptr) {
		oops_warnx();
	}

out:
	if (graph != nullptr) {
		taskToolBar->setTaskGraph(graph);
		setTaskActionsEnabled(true);
	} else {
		taskToolBar->removeTaskGraph();
		setTaskActionsEnabled(false);
	}
	updateTaskGraphActions();
	updateAddToLegendAction();
	tracePlot->replot();
}

bool MainWindow::isOpenGLEnabled()
{
	if (has_opengl())
		return tracePlot->openGl();
	else
		return false;
}

void MainWindow::setupOpenGL()
{
	if (has_opengl() &&
	    settingStore->getValue(Setting::OPENGL_ENABLED).boolv()) {
		if (!isOpenGLEnabled()) {
			tracePlot->setOpenGl(true, 4);
			if (tracePlot->openGl()) {
				printf("OpenGL rendering enabled\n");
			}
		}
	} else {
		if (isOpenGLEnabled()) {
			tracePlot->setOpenGl(false, 4);
			if (!tracePlot->openGl()) {
				printf("OpenGL rendering disabled\n");
			}
		}
	}
	settingStore->setBoolValue(Setting::OPENGL_ENABLED, isOpenGLEnabled());
}

/* Adds the currently selected task to the legend */
void MainWindow::addToLegendTriggered()
{
	taskToolBar->addCurrentTaskToLegend();
	doReplot();
	updateAddToLegendAction();
}

/* Clears the legend of all tasks */
void MainWindow::clearLegendTriggered()
{
	taskToolBar->clearLegend();
	updateAddToLegendAction();
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
