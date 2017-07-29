#
#
#  Traceshark - a visualizer for visualizing ftrace and perf traces
#  Copyright (C) 2014, 2015, 2016, 2017  Viktor Rosendahl
#  <viktor.rosendahl@gmail.com>
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

HEADERS      +=  ./qcp/axis/axis.h
HEADERS      +=  ./qcp/axis/axisticker.h
HEADERS      +=  ./qcp/axis/axistickerdatetime.h
HEADERS      +=  ./qcp/axis/axistickerfixed.h
HEADERS      +=  ./qcp/axis/axistickerlog.h
HEADERS      +=  ./qcp/axis/axistickerpi.h
HEADERS      +=  ./qcp/axis/axistickertext.h
HEADERS      +=  ./qcp/axis/axistickertime.h
HEADERS      +=  ./qcp/axis/range.h
HEADERS      +=  ./qcp/colorgradient.h
HEADERS      +=  ./qcp/core.h
HEADERS      +=  ./qcp/datacontainer.h
HEADERS      +=  ./qcp/global.h
HEADERS      +=  ./qcp/item.h
HEADERS      +=  ./qcp/items/item-bracket.h
HEADERS      +=  ./qcp/items/item-curve.h
HEADERS      +=  ./qcp/items/item-ellipse.h
HEADERS      +=  ./qcp/items/item-line.h
HEADERS      +=  ./qcp/items/item-pixmap.h
HEADERS      +=  ./qcp/items/item-rect.h
HEADERS      +=  ./qcp/items/item-straightline.h
HEADERS      +=  ./qcp/items/item-text.h
HEADERS      +=  ./qcp/items/item-tracer.h
HEADERS      +=  ./qcp/layer.h
HEADERS      +=  ./qcp/layout.h
HEADERS      +=  ./qcp/layoutelements/layoutelement-axisrect.h
HEADERS      +=  ./qcp/layoutelements/layoutelement-colorscale.h
HEADERS      +=  ./qcp/layoutelements/layoutelement-legend.h
HEADERS      +=  ./qcp/layoutelements/layoutelement-textelement.h
HEADERS      +=  ./qcp/lineending.h
HEADERS      +=  ./qcp/paintbuffer.h
HEADERS      +=  ./qcp/painter.h
HEADERS      +=  ./qcp/plottable.h
HEADERS      +=  ./qcp/plottable1d.h
HEADERS      +=  ./qcp/plottables/plottable-bars.h
HEADERS      +=  ./qcp/plottables/plottable-colormap.h
HEADERS      +=  ./qcp/plottables/plottable-curve.h
HEADERS      +=  ./qcp/plottables/plottable-errorbar.h
HEADERS      +=  ./qcp/plottables/plottable-financial.h
HEADERS      +=  ./qcp/plottables/plottable-graph.h
HEADERS      +=  ./qcp/plottables/plottable-statisticalbox.h
HEADERS      +=  ./qcp/qcp.h
HEADERS      +=  ./qcp/qcppointer.h
HEADERS      +=  ./qcp/scatterstyle.h
HEADERS      +=  ./qcp/selection.h
HEADERS      +=  ./qcp/selectiondecorator-bracket.h
HEADERS      +=  ./qcp/selectionrect.h
HEADERS      +=  ./qcp/vector2d.h

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

SOURCES      +=  ./qcp/axis/axis.cpp
SOURCES      +=  ./qcp/axis/axisticker.cpp
SOURCES      +=  ./qcp/axis/axistickerdatetime.cpp
SOURCES      +=  ./qcp/axis/axistickerfixed.cpp
SOURCES      +=  ./qcp/axis/axistickerlog.cpp
SOURCES      +=  ./qcp/axis/axistickerpi.cpp
SOURCES      +=  ./qcp/axis/axistickertext.cpp
SOURCES      +=  ./qcp/axis/axistickertime.cpp
SOURCES      +=  ./qcp/axis/range.cpp
SOURCES      +=  ./qcp/colorgradient.cpp
SOURCES      +=  ./qcp/core.cpp
SOURCES      +=  ./qcp/datacontainer.cpp
SOURCES      +=  ./qcp/item.cpp
SOURCES      +=  ./qcp/items/item-bracket.cpp
SOURCES      +=  ./qcp/items/item-curve.cpp
SOURCES      +=  ./qcp/items/item-ellipse.cpp
SOURCES      +=  ./qcp/items/item-line.cpp
SOURCES      +=  ./qcp/items/item-pixmap.cpp
SOURCES      +=  ./qcp/items/item-rect.cpp
SOURCES      +=  ./qcp/items/item-straightline.cpp
SOURCES      +=  ./qcp/items/item-text.cpp
SOURCES      +=  ./qcp/items/item-tracer.cpp
SOURCES      +=  ./qcp/layer.cpp
SOURCES      +=  ./qcp/layout.cpp
SOURCES      +=  ./qcp/layoutelements/layoutelement-axisrect.cpp
SOURCES      +=  ./qcp/layoutelements/layoutelement-colorscale.cpp
SOURCES      +=  ./qcp/layoutelements/layoutelement-legend.cpp
SOURCES      +=  ./qcp/layoutelements/layoutelement-textelement.cpp
SOURCES      +=  ./qcp/lineending.cpp
SOURCES      +=  ./qcp/paintbuffer.cpp
SOURCES      +=  ./qcp/painter.cpp
SOURCES      +=  ./qcp/plottable.cpp
SOURCES      +=  ./qcp/plottable1d.cpp
SOURCES      +=  ./qcp/plottables/plottable-bars.cpp
SOURCES      +=  ./qcp/plottables/plottable-colormap.cpp
SOURCES      +=  ./qcp/plottables/plottable-curve.cpp
SOURCES      +=  ./qcp/plottables/plottable-errorbar.cpp
SOURCES      +=  ./qcp/plottables/plottable-financial.cpp
SOURCES      +=  ./qcp/plottables/plottable-graph.cpp
SOURCES      +=  ./qcp/plottables/plottable-statisticalbox.cpp
SOURCES      +=  ./qcp/scatterstyle.cpp
SOURCES      +=  ./qcp/selection.cpp
SOURCES      +=  ./qcp/selectiondecorator-bracket.cpp
SOURCES      +=  ./qcp/selectionrect.cpp
SOURCES      +=  ./qcp/vector2d.cpp

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
