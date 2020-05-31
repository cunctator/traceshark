// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2018, 2020
 *  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QList>
#include <QMap>
#include <QThread>
#include "threads/tthread.h"
#include "misc/osapi.h"

/*
 * Let's piggy back on QThread. This will make TThread a mere container which
 * through a pointer isolates us from having to intherit QObject in our thread
 * classes.
 */

TThread_::TThread_(TThread *thr, const QString &name):
	tThread(thr), threadName(name) {}

void TThread_::run()
{
	tshark_pthread_setname_np(threadName.toLocal8Bit().data());
	tThread->run();
}

TThread::TThread()
{
	const QString name("TThread");
	threadPtr = new TThread_(this, name);
	threadMap[threadPtr] = threadPtr;
}

TThread::TThread(const QString &name)
{
	threadPtr = new TThread_(this, name);
	threadMap[threadPtr] = threadPtr;
}

TThread::~TThread()
{
	threadMap.remove(threadPtr);
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

void TThread::listThreads(QList<QThread*> &list)
{
	QMap<TThread_*, TThread_*>::iterator iter;
	for (iter = threadMap.begin(); iter != threadMap.end(); iter++)
		list.append(static_cast<QThread*>(iter.value()));
}

QMap<TThread_*, TThread_*> TThread::threadMap;
