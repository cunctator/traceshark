/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl
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
#include <QtWidgets>

#include "mainwindow.h"
#include "traceshark.h"
#include "ftraceparser.h"

MainWindow::MainWindow()
{
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
	if (!name.isEmpty())
		loadTraceFile(name);
}

void MainWindow::closeTrace()
{	
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Traceshark"),
            tr("<h2>Traceshark</h2>"
               "<p>Copyright &copy; 2014-2015 Viktor Rosendahl"
	       "<p>This program comes with ABSOLUTELY NO WARRANTY; details below."
	       "<p>This is free software, and you are welcome to redistribute it"	       "under certain conditions; details below."));
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

	aboutAction = new QAction(tr("&About"), this);
	aboutAction->setStatusTip(tr("Show info about Traceshark"));
	tsconnect(aboutAction, triggered(), this, about());
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
}


void MainWindow::loadTraceFile(QString &fileName)
{
	parser = new FtraceParser();
	QTextStream(stdout) << "opening " << fileName << "\n";
	
	if (!parser->open(fileName)) {
		QTextStream(stdout) << "failed to open " << fileName << "\n";
	}
}
