#
#
#  Traceshark - a visualizer for visualizing ftrace and perf traces
#  Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

###############################################################################
# Header files
#

HEADERS       = qcustomplot/qcustomplot.h

HEADERS      +=  ui/cursor.h
HEADERS      +=  ui/cursorinfo.h
HEADERS      +=  ui/eventinfodialog.h
HEADERS      +=  ui/eventsmodel.h
HEADERS      +=  ui/eventswidget.h
HEADERS      +=  ui/infowidget.h
HEADERS      +=  ui/legendgraph.h
HEADERS      +=  ui/licensedialog.h
HEADERS      +=  ui/mainwindow.h
HEADERS      +=  ui/migrationarrow.h
HEADERS      +=  ui/migrationline.h
HEADERS      +=  ui/taskgraph.h
HEADERS      +=  ui/taskinfo.h
HEADERS      +=  ui/taskrangeallocator.h
HEADERS      +=  ui/traceplot.h

HEADERS      +=  analyzer/abstracttask.h
HEADERS      +=  analyzer/cpufreq.h
HEADERS      +=  analyzer/cpu.h
HEADERS      +=  analyzer/cpuidle.h
HEADERS      +=  analyzer/cputask.h
HEADERS      +=  analyzer/migration.h
HEADERS      +=  analyzer/task.h
HEADERS      +=  analyzer/tcolor.h
HEADERS      +=  analyzer/traceanalyzer.h

HEADERS      +=  parser/genericparams.h
HEADERS      +=  parser/paramhelpers.h
HEADERS      +=  parser/traceevent.h
HEADERS      +=  parser/tracefile.h
HEADERS      +=  parser/tracelinedata.h
HEADERS      +=  parser/traceline.h
HEADERS      +=  parser/traceparser.h

HEADERS      +=  parser/grammar/argnode.h
HEADERS      +=  parser/grammar/cpunode.h
HEADERS      +=  parser/grammar/eventnode.h
HEADERS      +=  parser/grammar/grammar.h
HEADERS      +=  parser/grammar/grammarnode.h
HEADERS      +=  parser/grammar/grammarroot.h
HEADERS      +=  parser/grammar/pidnode.h
HEADERS      +=  parser/grammar/storenode.h

HEADERS      +=  parser/ftrace/ftracegrammar.h
HEADERS      +=  parser/ftrace/ftraceparams.h
HEADERS      +=  parser/ftrace/timenode.h

HEADERS      +=  parser/perf/perfeventnode.h
HEADERS      +=  parser/perf/perfgrammar.h
HEADERS      +=  parser/perf/perfparams.h
HEADERS      +=  parser/perf/perftimenode.h

HEADERS      +=  threads/indexwatcher.h
HEADERS      +=  threads/loadbuffer.h
HEADERS      +=  threads/loadthread.h
HEADERS      +=  threads/threadbuffer.h
HEADERS      +=  threads/tthread.h
HEADERS      +=  threads/workitem.h
HEADERS      +=  threads/workqueue.h
HEADERS      +=  threads/workthread.h

HEADERS      +=  mm/mempool.h
HEADERS      +=  mm/stringpool.h
HEADERS      +=  mm/stringtree.h

HEADERS      +=  misc/setting.h
HEADERS      +=  misc/tlist.h
HEADERS      +=  misc/traceshark.h
HEADERS      +=  misc/tstring.h

###############################################################################
# Source files
#

SOURCES       = qcustomplot/qcustomplot.cpp

SOURCES      +=  ui/cursor.cpp
SOURCES      +=  ui/cursorinfo.cpp
SOURCES      +=  ui/eventinfodialog.cpp
SOURCES      +=  ui/eventsmodel.cpp
SOURCES      +=  ui/eventswidget.cpp
SOURCES      +=  ui/infowidget.cpp
SOURCES      +=  ui/legendgraph.cpp
SOURCES      +=  ui/licensedialog.cpp
SOURCES      +=  ui/mainwindow.cpp
SOURCES      +=  ui/migrationarrow.cpp
SOURCES      +=  ui/migrationline.cpp
SOURCES      +=  ui/taskgraph.cpp
SOURCES      +=  ui/taskinfo.cpp
SOURCES      +=  ui/taskrangeallocator.cpp
SOURCES      +=  ui/traceplot.cpp

SOURCES      +=  analyzer/abstracttask.cpp
SOURCES      +=  analyzer/cpufreq.cpp
SOURCES      +=  analyzer/cpuidle.cpp
SOURCES      +=  analyzer/cputask.cpp
SOURCES      +=  analyzer/task.cpp
SOURCES      +=  analyzer/tcolor.cpp
SOURCES      +=  analyzer/traceanalyzer.cpp

SOURCES      +=  parser/traceevent.cpp
SOURCES      +=  parser/tracefile.cpp
SOURCES      +=  parser/traceparser.cpp

SOURCES      +=  parser/grammar/argnode.cpp
SOURCES      +=  parser/grammar/cpunode.cpp
SOURCES      +=  parser/grammar/eventnode.cpp
SOURCES      +=  parser/grammar/grammar.cpp
SOURCES      +=  parser/grammar/grammarnode.cpp
SOURCES      +=  parser/grammar/grammarroot.cpp
SOURCES      +=  parser/grammar/pidnode.cpp
SOURCES      +=  parser/grammar/storenode.cpp

SOURCES      +=  parser/ftrace/ftracegrammar.cpp
SOURCES      +=  parser/ftrace/ftraceparams.cpp
SOURCES      +=  parser/ftrace/timenode.cpp

SOURCES      +=  parser/perf/perfeventnode.cpp
SOURCES      +=  parser/perf/perfgrammar.cpp
SOURCES      +=  parser/perf/perfparams.cpp
SOURCES      +=  parser/perf/perftimenode.cpp

SOURCES      +=  threads/indexwatcher.cpp
SOURCES      +=  threads/loadbuffer.cpp
SOURCES      +=  threads/loadthread.cpp
SOURCES      +=  threads/tthread.cpp
SOURCES      +=  threads/workqueue.cpp

SOURCES      +=  mm/mempool.cpp
SOURCES      +=  mm/stringpool.cpp
SOURCES      +=  mm/stringtree.cpp

SOURCES      +=  misc/main.cpp

###############################################################################
# Qt Modules
#

QT           += core
QT           += widgets
QT           += printsupport

###############################################################################
# Directories
#

#DESTDIR=bin #Target file directory
OBJECTS_DIR=obj
MOC_DIR=obj

###############################################################################
# Flags
#

QMAKE_CXXFLAGS_RELEASE += -pedantic -Wall -g -std=c++11
QMAKE_CFLAGS_RELEASE += -pedantic -Wall -std=c99
QMAKE_LFLAGS_RELEASE =
CONFIG += DEBUG

###############################################################################
# Resources
#

RESOURCES     = traceshark.qrc
