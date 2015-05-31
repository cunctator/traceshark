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

#include <QTextStream>
#include <QDateTime>
#include <cmath>

#include "cursor.h"
#include "eventswidget.h"
#include "ftraceparser.h"
#include "infowidget.h"
#include "mainwindow.h"
#include "traceshark.h"
#include "threads/workqueue.h"
#include "threads/workitem.h"
#include "qcustomplot/qcustomplot.h"

MainWindow::MainWindow():
	customPlot(NULL)
{
	parser = new FtraceParser;

	//setCentralWidget(traceLabel);

	createActions();
	createToolBars();
	createMenus();

	schedItem = new WorkItem<FtraceParser> (parser,
						&FtraceParser::processSched);
	migItem = new WorkItem<FtraceParser> (parser,
					      &FtraceParser::processMigration);
	freqItem = new WorkItem<FtraceParser> (parser,
					       &FtraceParser::processCPUfreq);
	workQueue = new WorkQueue();
	workQueue->addDefaultWorkItem(schedItem);
	workQueue->addDefaultWorkItem(migItem);
	workQueue->addDefaultWorkItem(freqItem);

	customPlot = new QCustomPlot();
	customPlot->hide();
	customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom |
				    QCP::iSelectAxes | QCP::iSelectLegend |
				    QCP::iSelectPlottables);

	tsconnect(customPlot, mouseWheel(QWheelEvent*), this, mouseWheel());
	tsconnect(customPlot->xAxis, rangeChanged(QCPRange), customPlot->xAxis2,
		  setRange(QCPRange));
	tsconnect(customPlot, mousePress(QMouseEvent*), this, mousePress());

	plotWidget = new QWidget;
	plotLayout = new QHBoxLayout;
	plotWidget->setLayout(plotLayout);
	setCentralWidget(plotWidget);
	plotLayout->addWidget(customPlot);

	eventsWidget = new EventsWidget(this);
	addDockWidget(Qt::BottomDockWidgetArea, eventsWidget);

	infoWidget = new InfoWidget();
	addDockWidget(Qt::TopDockWidgetArea, infoWidget);

	cursors[RED_CURSOR] = NULL;
	cursors[BLUE_CURSOR] = NULL;

	tsconnect(customPlot, mouseDoubleClick(QMouseEvent*),
		  this, plotDoubleClicked(QMouseEvent*));
	tsconnect(infoWidget, valueChanged(double, int),
		  this, infoValueChanged(double, int));
	tsconnect(eventsWidget, timeSelected(double), this,
		  eventTimeSelected(double));
}

MainWindow::~MainWindow()
{
	delete parser;
	delete schedItem;
	delete migItem;
	delete freqItem;
	delete workQueue;
	delete customPlot;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	/* Here is a great place to save settings, if we ever want to do it */
	event->accept();
	/* event->ignore() could be used to refuse to close the window */
}

void MainWindow::openTrace()
{
	QString name = QFileDialog::getOpenFileName(this);
	if (!name.isEmpty()) {
		eventsWidget->beginResetModel();
		eventsWidget->setEvents(NULL);
		eventsWidget->endResetModel();
		if (parser->isOpen())
			parser->close();
		loadTraceFile(name);
	}
	if (parser->isOpen()) {
		QTextStream qout(stdout);
		qout.setRealNumberPrecision(6);
		qout.setRealNumberNotation(QTextStream::FixedNotation);
		quint64 start, process, layout, rescale, show;

		start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
		processTrace();
		process = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
		computeLayout();
		layout = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
		rescaleTrace();
		rescale = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
		showTrace();
		show = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
		qout << "processTrace() took "
		     << (double) (process - start) / 1000;
		qout << " s\n";
		qout << "computeLayout() took "
		     << (double) (layout - process) / 1000;
		qout << " s\n";
		qout << "rescaleTrace() took "
		     << (double) (rescale - layout) / 1000;
		qout << " s\n";
		qout << "showTrace() took "
		     << (double) (show - rescale) / 1000;
		qout << " s\n";
		eventsWidget->beginResetModel();
		eventsWidget->setEvents(&parser->events);
		eventsWidget->endResetModel();
		setupCursors();
		customPlot->show();
	}
}

void MainWindow::processTrace()
{
	QTextStream qout(stdout);
	quint64 start, pre, process, colorize;

	qout.setRealNumberPrecision(6);
	qout.setRealNumberNotation(QTextStream::FixedNotation);

	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	parser->preScan();
	pre = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	workQueue->setWorkItemsDefault();
	workQueue->start();

	process = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	qout << "preScan() took " << (double) (pre - start) / 1000 << " s\n";
	qout << "processing took " << (double) (process - pre) / 1000 << 
		" s\n";
	qout.flush();

	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	parser->colorizeTasks();
	colorize = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	qout << "colorize() took " << (double) (colorize - start) / 1000 <<
		" s\n";
}

void MainWindow::computeLayout()
{
	unsigned int cpu;
	unsigned int nrCPUs;
	unsigned int offset = schedSectionSpace;
	QString label;
	bottom = 0;

	ticks.resize(0);
	tickLabels.resize(0);
	nrCPUs = parser->getNrCPUs();

	/* Set the offset and scale of the scheduling graphs */
	for (cpu = 0; cpu < nrCPUs; cpu++) {
		parser->setSchedOffset(cpu, offset);
		parser->setSchedScale(cpu, schedHeight);
		label = QString("cpu") + QString::number(cpu);
		ticks.append(offset);
		tickLabels.append(label);
		offset += schedHeight + schedSpacing;
	}

	offset += cpuSectionOffset;

	for (cpu = 0; cpu < nrCPUs; cpu++) {
		parser->setCpuFreqOffset(cpu, offset);
		parser->setCpuIdleOffset(cpu, offset);
		parser->setCpuFreqScale(cpu, cpuHeight);
		parser->setCpuIdleScale(cpu, cpuHeight);
		label = QString("cpu") + QString::number(cpu);
		ticks.append(offset);
		tickLabels.append(label);
		offset += cpuHeight + cpuSpacing;
	}

	top = offset;
}

void MainWindow::rescaleTrace()
{
	parser->doScale();
}

void MainWindow::clearPlot()
{
	cursors[RED_CURSOR] = NULL;
	cursors[BLUE_CURSOR] = NULL;
	customPlot->clearItems();
	customPlot->clearPlottables();
	customPlot->hide();
}

void MainWindow::showTrace()
{
	unsigned int cpu;
	double start, end;
	int precision = 7;
	double extra = 0;

	start = parser->getStartTime();
	end = parser->getEndTime();

	if (end >= 10)
		extra = floor (log(end) / log(10));

	precision += (int) extra;

	clearPlot();
	customPlot->yAxis->setRange(QCPRange(bottom, top));
	customPlot->xAxis->setRange(QCPRange(start, end));
	customPlot->xAxis->setNumberPrecision(precision);
	customPlot->yAxis->setTicks(false);
	customPlot->yAxis->setAutoTicks(false);
	customPlot->yAxis->setAutoTickLabels(false);
	customPlot->yAxis->setTickVector(ticks);
	customPlot->yAxis->setTickVectorLabels(tickLabels);
	customPlot->yAxis->setTickLabels(true);
	customPlot->yAxis->setTicks(true);

	/* Show CPU frequency graphs */
	for (cpu = 0; cpu <= parser->getMaxCPU(); cpu++) {
		QCPGraph *graph = new QCPGraph(customPlot->xAxis,
					       customPlot->yAxis);
		customPlot->addPlottable(graph);
		graph->setLineStyle(QCPGraph::lsStepLeft);
		graph->setData(parser->cpuFreq[cpu].timev,
			       parser->cpuFreq[cpu].scaledData);
	}

	/* Show scheduling graphs */
	for (cpu = 0; cpu <= parser->getMaxCPU(); cpu++) {
		DEFINE_TASKMAP_ITERATOR(iter) = parser->
			cpuTaskMaps[cpu].begin();
		while(iter != parser->cpuTaskMaps[cpu].end()) {
			CPUTask &task = iter.value();
			unsigned int pid = task.pid;
			iter++;

			/* Add scheduling graph */
			QCPGraph *graph = new QCPGraph(customPlot->xAxis,
						       customPlot->yAxis);
			QColor color = parser->getTaskColor(pid);
			QPen pen = QPen();
			pen.setColor(color);
			graph->setPen(pen);
			customPlot->addPlottable(graph);
			graph->setLineStyle(QCPGraph::lsStepLeft);
			graph->setAdaptiveSampling(true);
			graph->setData(task.timev, task.scaledData);

			/* Add wakeup graph on top of scheduling */
			graph = new QCPGraph(customPlot->xAxis,
						       customPlot->yAxis);
			customPlot->addPlottable(graph);
			QCPScatterStyle style =
				QCPScatterStyle(QCPScatterStyle::ssDot);
			style.setPen(pen);
			graph->setScatterStyle(style);
			graph->setLineStyle(QCPGraph::lsNone);
			graph->setAdaptiveSampling(true);
			graph->setDataKeyError(task.wakeTimev, task.wakeHeight,
					       task.wakeDelay, task.wakeZero);
			graph->setErrorType(QCPGraph::etKey);
			graph->setErrorBarSize(4);
			graph->setErrorPen(pen);

			/* Add still running graph on top of the other two...*/
			if (task.runningTimev.size() == 0)
				continue;
			graph = new QCPGraph(customPlot->xAxis,
					     customPlot->yAxis);
			customPlot->addPlottable(graph);
			style = QCPScatterStyle(QCPScatterStyle::ssCircle, 5);
			pen.setColor(Qt::red);
			style.setPen(pen);
			graph->setScatterStyle(style);
			graph->setLineStyle(QCPGraph::lsNone);
			graph->setAdaptiveSampling(true);
			graph->setData(task.runningTimev,
				       task.scaledRunningData);
		}
	}
}

void MainWindow::setupCursors()
{
	double start, end, red, blue;

	start = parser->getStartTime();
	end = parser->getEndTime();

	cursors[RED_CURSOR] = new Cursor(customPlot);
	cursors[BLUE_CURSOR] = new Cursor(customPlot);
	cursors[RED_CURSOR]->setColor(Qt::red);
	cursors[BLUE_CURSOR]->setColor(Qt::blue);

	customPlot->addItem(cursors[RED_CURSOR]);
	customPlot->addItem(cursors[BLUE_CURSOR]);

	red = (start + end) / 2;
	cursors[RED_CURSOR]->setPosition(red);
	infoWidget->setTime(red, RED_CURSOR);
	blue = (start + end) / 2 + (end - start) / 10;
	cursors[BLUE_CURSOR]->setPosition(blue);
	infoWidget->setTime(blue, BLUE_CURSOR);
	/* Fixme:
	 * For some reason the EventsWidget doesn't want to make its first 
	 * scroll to somewhere in the middle of the trace. As a work around
	 * we first scroll to the beginning and to the end, and then to 
	 * where we want */
	eventsWidget->scrollTo(start);
	eventsWidget->scrollTo(end);
	eventsWidget->scrollTo(red);
}

void MainWindow::closeTrace()
{
	if(parser->isOpen())
		parser->close();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Traceshark"),
            tr("<h1>Traceshark</h1>"
               "<p>Copyright &copy; 2014-2015 Viktor Rosendahl"
	       "<p>This program comes with ABSOLUTELY NO WARRANTY; details below."
	       "<p>This is free software, and you are welcome to redistribute it"
	       " under certain conditions; select \"License\" under the \"Help\""
	       "menu for details."

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
	       "Program in return for a fee."));
}

void MainWindow::license()
{
	// Figure out some way to display the whole GPL nicely here
}

void MainWindow::mouseWheel()
{
	customPlot->axisRect()->setRangeZoom(Qt::Horizontal);
}

void MainWindow::mousePress()
{
	customPlot->axisRect()->setRangeDrag(Qt::Horizontal);
}

void MainWindow::plotDoubleClicked(QMouseEvent *event)
{
	int cursorIdx;

	cursorIdx = infoWidget->getCursorIdx();
	if (cursorIdx != RED_CURSOR && cursorIdx != BLUE_CURSOR)
		return;

	Cursor *cursor = cursors[cursorIdx];
	if (cursor != NULL) {
		double pixel = (double) event->x();
		double coord = customPlot->xAxis->pixelToCoord(pixel);
		cursor->setPosition(coord);
		eventsWidget->scrollTo(coord);
		infoWidget->setTime(coord, cursorIdx);
	}
}

void MainWindow::infoValueChanged(double value, int nr)
{
	Cursor *cursor;
	if (nr == RED_CURSOR || nr == BLUE_CURSOR) {
		cursor = cursors[nr];
		cursor->setPosition(value);
		eventsWidget->scrollTo(value);
	}
}

void MainWindow::eventTimeSelected(double time)
{
	int cursorIdx;

	cursorIdx = infoWidget->getCursorIdx();
	if (cursorIdx != RED_CURSOR && cursorIdx != BLUE_CURSOR)
		return;

	Cursor *cursor = cursors[cursorIdx];
	if (cursor != NULL) {
		cursor->setPosition(time);
		infoWidget->setTime(time, cursorIdx);
	}
}

void MainWindow::createActions()
{
	openAction = new QAction(tr("&Open"), this);
	openAction->setShortcuts(QKeySequence::Open);
	openAction->setStatusTip(tr("Open a trace file..."));
	tsconnect(openAction, triggered(), this, openTrace());

	closeAction = new QAction(tr("&Close"), this);
	closeAction->setShortcuts(QKeySequence::Close);
	closeAction->setStatusTip(tr("Close the trace"));
	tsconnect(closeAction, triggered(), this, closeTrace());

	exitAction = new QAction(tr("E&xit"), this);
	exitAction->setShortcuts(QKeySequence::Quit);
	exitAction->setStatusTip(tr("Exit traceshark"));
	tsconnect(exitAction, triggered(), this, close());

	aboutQtAction = new QAction(tr("About &Qt"), this);
	aboutQtAction->setStatusTip(tr("Show info about Qt"));
	tsconnect(aboutQtAction, triggered(), qApp, aboutQt());

	aboutAction = new QAction(tr("&About Traceshark"), this);
	aboutAction->setStatusTip(tr("Show info about Traceshark"));
	tsconnect(aboutAction, triggered(), this, about());

	licenseAction = new QAction(tr("&License"), this);
	aboutAction->setStatusTip(tr("Show the license of Traceshark"));
	tsconnect(licenseAction, triggered(), this, license());
}

void MainWindow::createToolBars()
{
}

void MainWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAction);
	fileMenu->addAction(closeAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAction);

	viewMenu = menuBar()->addMenu(tr("&View"));

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(aboutQtAction);
	helpMenu->addAction(aboutAction);
	helpMenu->addAction(licenseAction);
}


void MainWindow::loadTraceFile(QString &fileName)
{
	qint64 start, stop;
	QTextStream qout(stdout);

	qout.setRealNumberPrecision(6);
	qout.setRealNumberNotation(QTextStream::FixedNotation);

	qout << "opening " << fileName << "\n";
	
	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	if (!parser->open(fileName)) {
		qout << "failed to open " << fileName << "\n";
	}
	stop = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	stop = stop - start;

	qout << "Loading took " << (double) stop / 1000 << " s\n";
	qout.flush();

	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	parser->parse();
	stop = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	stop = stop - start;

	qout << "Parsing took " << (double) stop / 1000 << " s\n";
	qout.flush();

#if 0
	int i, s;
	unsigned int j;
	s = parser->events.size();
	for (i = 0; i < s; i++) {
		TraceEvent &event = parser->events[i];
		qout << event.taskName->ptr << " " << event.pid << " " <<
			event.time << " " << event.eventName->ptr;
		for (j = 0; j < event.argc; j++) {
			qout << " " << event.argv[j]->ptr;
		}
		qout << "\n";
        }
#endif
}