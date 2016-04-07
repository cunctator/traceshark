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

#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QString>
#include "threads/tthread.h"
#include "misc/traceshark.h"

#define WORKTHREAD_DEFAULTNAME "WorkThread"

template <class ObjType>
class WorkThread : public TThread
{
public:
	WorkThread();
	WorkThread(const QString &name);
	WorkThread(ObjType *p, DEFINE_MEMBER_FN(void, ObjType, oF));
	WorkThread(const QString &name, ObjType *p,
		   DEFINE_MEMBER_FN(void, ObjType, oF));
	~WorkThread();
	void setObjFn(ObjType *p, DEFINE_MEMBER_FN(void, ObjType, oF));
protected:
	void run();
private:
	ObjType *workObject;
	DEFINE_MEMBER_FN(void, ObjType, objectFunc);
};

template <class ObjType>
WorkThread<ObjType>::WorkThread():
TThread(QString(WORKTHREAD_DEFAULTNAME)), workObject(nullptr),
	objectFunc(nullptr) {}

template <class ObjType>
WorkThread<ObjType>::WorkThread(const QString &name):
TThread(name), workObject(nullptr), objectFunc(nullptr) {}

template <class ObjType>
WorkThread<ObjType>::WorkThread(ObjType *p,
				DEFINE_MEMBER_FN(void, ObjType, oF)):
TThread(QString(WORKTHREAD_DEFAULTNAME)), workObject(p), objectFunc(oF) {}

template <class ObjType>
WorkThread<ObjType>::WorkThread(const QString &name, ObjType *p,
				DEFINE_MEMBER_FN(void, ObjType, oF)):
TThread(name), workObject(p), objectFunc(oF) {}


template <class ObjType>
void WorkThread<ObjType>::run()
{
	CALL_MEMBER_FN(workObject, objectFunc)();
}

template <class ObjType>
WorkThread<ObjType>::~WorkThread()
{
}

template <class ObjType>
void WorkThread<ObjType>::setObjFn(ObjType *p,
				   DEFINE_MEMBER_FN(void, ObjType, oF)) {
	workObject = p;
	objectFunc = oF;
}

#endif /* WORKTHREAD */
