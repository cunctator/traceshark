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

#ifndef TTHREAD_H
#define TTHREAD_H

/* I hate this class. I would like to use QThread directly instead of inventing
 * my own thread class. However, QThread does not lend itself to the use of
 * templates since it inherits QObject, so here we go down the dirty road */

#include <QThread> /* We are going to piggyback */
#include <QtCore>

class __TThread;

class TThread {
	friend class __TThread;
public:
	TThread();
	virtual ~TThread();
	void exit(int returnCode = 0);
	bool isFinished();
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
	bool isInterruptionRequested();
	void requestInterruption();
#endif
	bool isRunning();
	void setPriority(QThread::Priority priority);
	void setStackSize(uint stackSize);
	uint stackSize();
	void start();
	void terminate();
	bool wait(unsigned long time = ULONG_MAX);
	void quit();
protected:
	virtual void run()=0;
private:
	__TThread *threadPtr;
};

class __TThread : public QThread {
	Q_OBJECT
public:
	__TThread(TThread *thr);
protected:
	void run();
private:
	TThread *tThread;
};

#endif /* TTHREAD_H */
