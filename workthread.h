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

#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "tthread.h"

/* C++ syntax for calling the pointer to a member function for an object */
#define CALL_MEMBER_FN(ptrObject, ptrToMember) ((ptrObject)->*(ptrToMember))
/* C++ syntax for declaring a pointer to a member function */
#define DECLARE_MEMBER_FN(className, name) void (className::* name)()

template <class ObjType>
class WorkThread : public TThread
{
public:
	WorkThread(ObjType *p, DECLARE_MEMBER_FN(ObjType, oF));
	~WorkThread();
protected:
	void run();
private:
	ObjType *workObject;
	DECLARE_MEMBER_FN(ObjType, objectFunc);
};

template <class ObjType>
WorkThread<ObjType>::WorkThread(ObjType *p, DECLARE_MEMBER_FN(ObjType, oF)):
workObject(p), objectFunc(oF) {}


template <class ObjType>
void WorkThread<ObjType>::run()
{
	CALL_MEMBER_FN(workObject, objectFunc)();
}

template <class ObjType>
WorkThread<ObjType>::~WorkThread()
{
}

#endif /* WORKTHREAD */
