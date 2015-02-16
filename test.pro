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

HEADERS       = ftraceparser.h
HEADERS      += mempool.h
HEADERS      += tracefile.h
HEADERS      += traceline.h
HEADERS      += mainwindow.h
HEADERS      += traceshark.h

SOURCES       = main.cpp
SOURCES      += ftraceparser.cpp
SOURCES      += mempool.cpp
SOURCES      += tracefile.cpp
SOURCES      += mainwindow.cpp
QT           += core
QT           += widgets

QMAKE_CXXFLAGS_RELEASE += -Wall -g
QMAKE_CFLAGS_RELEASE += -pedantic -Wall -std=c99
QMAKE_LFLAGS_RELEASE =
CONFIG += DEBUG
