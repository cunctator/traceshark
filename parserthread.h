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

#ifndef PARSERTHREAD_H
#define PARSERTHREAD_H

#include <QThread>
#include "ftraceparser.h"

/* C++ syntax for declaring a pointer to a member function */
typedef void (FtraceParser::*FtraceParserMemFn)();
/* C++ syntax for calling the pointer to a member function for an object */
#define CALL_MEMBER_FN(ptrObject, ptrToMember) ((ptrObject)->*(ptrToMember))

class ParserThread : public QThread
{
	Q_OBJECT
public:
	ParserThread(FtraceParser *p, FtraceParserMemFn pF);
protected:
	void run();
private:
	FtraceParser *ftraceParser;
	FtraceParserMemFn workFunc;
};

#endif /* PARSERTHREAD */
