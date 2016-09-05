/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

/* Let's piggy back on QThread. This will make TThread a mere container which
 * through a pointer isolates us from having to intherit QObject in our thread
 * classes */
#include <QThread>
#include "threads/tthread.h"

extern "C" {
#include <sys/prctl.h>
}

__TThread::__TThread(TThread *thr, const QString &name):
	tThread(thr), threadName(name) {}

void __TThread::run()
{
	prctl(PR_SET_NAME, (unsigned long) threadName.toLocal8Bit().data(), 0,
	      0, 0);
	tThread->run();
}

TThread::TThread()
{
	const QString name("TThread");
	threadPtr = new __TThread(this, name);
}

TThread::TThread(const QString &name)
{
	threadPtr = new __TThread(this, name);
}

TThread::~TThread()
{
	delete threadPtr;
}

void TThread::exit(int returnCode)
{
	threadPtr->exit(returnCode);
}

void TThread::start()
{
	threadPtr->start();
}

void TThread::terminate()
{
	threadPtr->terminate();
}

bool TThread::isFinished() const
{
	return threadPtr->isFinished();
}

bool TThread::isRunning() const
{
	return threadPtr->isRunning();
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
bool TThread::isInterruptionRequested() const
{
	return threadPtr->isInterruptionRequested();
}

void TThread::requestInterruption()
{
	threadPtr->requestInterruption();
}
#endif

void TThread::setPriority(QThread::Priority priority)
{
	threadPtr->setPriority(priority);
}

void TThread::setStackSize(uint stackSize)
{
	threadPtr->setStackSize(stackSize);
}

uint TThread::stackSize()
{
	return threadPtr->stackSize();
}

bool TThread::wait(unsigned long time)
{
	return threadPtr->wait(time);
}

void TThread::quit()
{
	threadPtr->quit();
}
