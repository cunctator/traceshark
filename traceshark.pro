#
#
#  Traceshark - a visualizer for visualizing ftrace and perf traces
#  Copyright (C) 2014, 2015, 2016, 2017  Viktor Rosendahl
#  <viktor.rosendahl@gmail.com>
#
# This file is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
#
#  a) This program is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public License as
#     published by the Free Software Foundation; either version 2 of the
#     License, or (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public
#     License along with this library; if not, write to the Free
#     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
#     MA 02110-1301 USA
#
# Alternatively,
#
#  b) Redistribution and use in source and binary forms, with or
#     without modification, are permitted provided that the following
#     conditions are met:
#
#     1. Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#     2. Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials
#        provided with the distribution.
#
#     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
#     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
HEADERS      +=  ui/taskmodel.h
HEADERS      +=  ui/taskrangeallocator.h
HEADERS      +=  ui/taskselectdialog.h
HEADERS      +=  ui/taskview.h
HEADERS      +=  ui/traceplot.h
HEADERS      +=  ui/tracesharkstyle.h
HEADERS      +=  ui/yaxisticker.h

HEADERS      +=  analyzer/abstracttask.h
HEADERS      +=  analyzer/cpufreq.h
HEADERS      +=  analyzer/cpu.h
HEADERS      +=  analyzer/cpuidle.h
HEADERS      +=  analyzer/cputask.h
HEADERS      +=  analyzer/filterstate.h
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

HEADERS      +=  parser/ftrace/ftraceparams.h
HEADERS      +=  parser/ftrace/ftracegrammar.h

HEADERS      +=  parser/perf/perfparams.h
HEADERS      +=  parser/perf/perfgrammar.h

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

HEADERS      +=  misc/resources.h
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
SOURCES      +=  ui/taskmodel.cpp
SOURCES      +=  ui/taskrangeallocator.cpp
SOURCES      +=  ui/taskselectdialog.cpp
SOURCES      +=  ui/taskview.cpp
SOURCES      +=  ui/traceplot.cpp
SOURCES      +=  ui/tracesharkstyle.cpp
SOURCES      +=  ui/yaxisticker.cpp


SOURCES      +=  analyzer/abstracttask.cpp
SOURCES      +=  analyzer/cpufreq.cpp
SOURCES      +=  analyzer/cpuidle.cpp
SOURCES      +=  analyzer/cputask.cpp
SOURCES      +=  analyzer/filterstate.cpp
SOURCES      +=  analyzer/task.cpp
SOURCES      +=  analyzer/tcolor.cpp
SOURCES      +=  analyzer/traceanalyzer.cpp

SOURCES      +=  parser/traceevent.cpp
SOURCES      +=  parser/tracefile.cpp
SOURCES      +=  parser/traceparser.cpp

SOURCES      +=  parser/ftrace/ftraceparams.cpp
SOURCES      +=  parser/ftrace/ftracegrammar.cpp

SOURCES      +=  parser/perf/perfparams.cpp
SOURCES      +=  parser/perf/perfgrammar.cpp

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
# Uncomment the line below to enable OpenGl through the new method. Seems flaky,
# so for now is better to use Qt4 and rely on on OpenGl being enabled in main()
# DEFINES += QCUSTOMPLOT_USE_OPENGL

###############################################################################
# Resources
#

RESOURCES     = traceshark.qrc
