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
#include <QtWidgets>

#include "ftraceparser.h"
#include "mainwindow.h"
#include "traceshark.h"
#include "parserthread.h"

MainWindow::MainWindow()
{
	parser = new FtraceParser;

	traceLabel = new QLabel;
	setCentralWidget(traceLabel);

	createActions();
	createToolBars();
	createMenus();	
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
		if (parser->isOpen())
			parser->close();
		loadTraceFile(name);
	}
	if (parser->isOpen()) {
		processTrace();
	}
}

void MainWindow::processTrace()
{
	QTextStream qout(stdout);
	quint64 start, pre, process;
	ParserThread *schedThread, *migThread, *freqThread;

	qout.setRealNumberPrecision(6);
	qout.setRealNumberNotation(QTextStream::FixedNotation);

	start = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
	parser->preScan();
	pre = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	schedThread = new ParserThread(parser, &FtraceParser::processSched);
	schedThread->start();
	migThread = new ParserThread(parser, &FtraceParser::processMigration);
	migThread->start();
	freqThread = new ParserThread(parser, &FtraceParser::processCPUfreq);
	freqThread->start();

	migThread->wait();
	freqThread->wait();
	schedThread->wait();

	process = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();

	qout << "preScan() took " << (double) (pre - start) / 1000 << " s\n";
	qout << "processing took " << (double) (process - pre) / 1000 << 
		" s\n";
	qout.flush();
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
	       "<p>This is free software, and you are welcome to redistribute it"	       "under certain conditions; select \"License\" under the \"Help\""
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
