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

#ifndef WORKITEM_H
#define WORKITEM_H

//#include <QObject>
#include "src/traceshark.h"

class WorkQueue;

/****************************** AbstractWorkItem ******************************
 * This class must be a virtual class, because it can't be a template, that 
 * would drag the WorkQueue class into the template business.
 */
class AbstractWorkItem {
	friend class WorkQueue;
public:
	virtual ~AbstractWorkItem() {}
protected:
	__always_inline bool __runWork();
	virtual bool run() = 0;
};

__always_inline bool AbstractWorkItem::__runWork() {
	return run();
}

/****************************** WorkItem **************************************
 * This class needs to be a template to be able to call functions in different
 * classes. The WorkQueue uses it through the AbstractWorkItem class interface
 * in order to avoid dealing with templates.
 */
template <class W>
class WorkItem : public AbstractWorkItem {
public:
	WorkItem(W *obj, DEFINE_MEMBER_FN(bool, W, fn));
	WorkItem() {}
	virtual ~WorkItem() {}
	void setObjFn(W *obj, DEFINE_MEMBER_FN(bool, W, fn));
protected:
	__always_inline bool run();
private:
	W *workObj;
	DEFINE_MEMBER_FN(bool, W,  workObjFn);
};

template <class W>
WorkItem<W>::WorkItem(W *obj, DEFINE_MEMBER_FN(bool, W, fn)):
workObj(obj), workObjFn(fn) {}

template <class W>
void WorkItem<W>::setObjFn(W *obj, DEFINE_MEMBER_FN(bool, W, fn)) {
	workObj = obj;
	workObjFn = fn;
}

template <class W>
__always_inline bool WorkItem<W>::run() {
	return CALL_MEMBER_FN(workObj, workObjFn)();
}

#endif /* WORKITEM_H */
