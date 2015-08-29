#
#
#  Traceshark - a visualizer for visualizing ftrace traces
#  Copyright (C) 2014-2015  Viktor Rosendahl
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

HEADERS       = src/argnode.h
HEADERS      += src/cpu.h
HEADERS      += src/cpufreq.h
HEADERS      += src/cpuidle.h
HEADERS      += src/cpunode.h
HEADERS      += src/cursor.h
HEADERS      += src/cursorinfo.h
HEADERS      += src/eventnode.h
HEADERS      += src/eventsmodel.h
HEADERS      += src/eventswidget.h
HEADERS      += src/ftraceparams.h
HEADERS      += src/ftraceparser.h
HEADERS      += src/grammarnode.h
HEADERS      += src/grammarroot.h
HEADERS      += src/infowidget.h
HEADERS      += src/licensedialog.h
HEADERS      += threads/loadbuffer.h
HEADERS      += threads/loadthread.h
HEADERS      += src/mainwindow.h
HEADERS      += src/migration.h
HEADERS      += src/migrationarrow.h
HEADERS      += src/migrationline.h
HEADERS      += src/namepidnode.h
HEADERS      += mm/mempool.h
HEADERS      += mm/stringpool.h
HEADERS      += src/cputask.h
HEADERS      += src/setting.h
HEADERS      += src/task.h
HEADERS      += src/tcolor.h
HEADERS      += threads/threadbuffer.h
HEADERS      += threads/tthread.h
HEADERS      += src/timenode.h
HEADERS      += src/traceevent.h
HEADERS      += src/tracefile.h
HEADERS      += src/traceline.h
HEADERS      += src/traceshark.h
HEADERS      += src/tstring.h
HEADERS      += threads/workitem.h
HEADERS      += threads/workthread.h
HEADERS      += threads/workqueue.h
HEADERS      += qcustomplot/qcustomplot.h

SOURCES       = src/argnode.cpp
SOURCES      += src/cpufreq.cpp
SOURCES      += src/cpuidle.cpp
SOURCES      += src/cpunode.cpp
SOURCES      += src/cursor.cpp
SOURCES      += src/cursorinfo.cpp
SOURCES      += src/eventnode.cpp
SOURCES      += src/eventsmodel.cpp
SOURCES      += src/eventswidget.cpp
SOURCES      += src/ftraceparser.cpp
SOURCES      += src/ftraceparams.cpp
SOURCES      += src/grammarnode.cpp
SOURCES      += src/grammarroot.cpp
SOURCES      += src/infowidget.cpp
SOURCES      += src/licensedialog.cpp
SOURCES      += threads/loadbuffer.cpp
SOURCES      += threads/loadthread.cpp
SOURCES      += src/main.cpp
SOURCES      += src/mainwindow.cpp
SOURCES      += src/migrationarrow.cpp
SOURCES      += src/migrationline.cpp
SOURCES      += mm/mempool.cpp
SOURCES      += mm/stringpool.cpp
SOURCES      += src/namepidnode.cpp
SOURCES      += src/cputask.cpp
SOURCES      += src/tcolor.cpp
SOURCES      += threads/tthread.cpp
SOURCES      += src/timenode.cpp
SOURCES      += src/tracefile.cpp
SOURCES      += threads/workqueue.cpp
SOURCES      += qcustomplot/qcustomplot.cpp

QT           += core
QT           += widgets
QT           += printsupport

QMAKE_CXXFLAGS_RELEASE += -pedantic -Wall -g -std=c++11
QMAKE_CFLAGS_RELEASE += -pedantic -Wall -std=c99
QMAKE_LFLAGS_RELEASE =
CONFIG += DEBUG

RESOURCES     = traceshark.qrc
