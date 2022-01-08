# SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
#
#  Traceshark - a visualizer for visualizing ftrace and perf traces
#  Copyright (C) 2014-2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
# Architecture Flags
#

# Architecture flags, uncomment those that suits your machine best
# NB: Only a few of these have been tested. Could be spelling mistakes in any
# of the commented out flags.

# Automatic detection. Does not necessarily work
# MARCH_FLAG = -march=native
# MTUNE_FLAG = -mtune=native

##### x86-32 Section

# i386
# MARCH_FLAG = -march=i386
# MTUNE_FLAG = -mtune=i386

# i486
# MARCH_FLAG = -march=i486
# MTUNE_FLAG = -mtune=i486

# Pentium
# MARCH_FLAG = -march=pentium
# MTUNE_FLAG = -mtune=pentium

# Lakemont
# MARCH_FLAG = -march=lakemont
# MTUNE_FLAG = -mtune=lakemont

# Pentium MMX
# MARCH_FLAG = -march=pentium-mmx
# MTUNE_FLAG = -mtune=pentium-mmx

# Pentium Pro
# MARCH_FLAG = -march=pentiumpro
# MTUNE_FLAG = -mtune=pentiumpro

# i686
# MARCH_FLAG = -march=i686
# MTUNE_FLAG = -mtune=i686

# Pentium 2
# MARCH_FLAG = -march=pentium2
# MTUNE_FLAG = -mtune=pentium2

# Pentium 3
# MARCH_FLAG = -march=pentium3
# MTUNE_FLAG = -mtune=pentium3

# Pentium M
# MARCH_FLAG = -march=pentium-m
# MTUNE_FLAG = -mtune=pentium-m

# Pentium 4
# MARCH_FLAG = -march=pentium4
# MTUNE_FLAG = -mtune=pentium4

# Prescott
# MARCH_FLAG = -march=prescott
# MTUNE_FLAG = -mtune=prescott

### x86-64 Section

# Generic 64-bit x86
# MARCH_FLAG = -march=x86-64
# MTUNE_FLAG = -mtune=x86-64

# Generic 64-bit x86
# MARCH_FLAG = -march=x86-64
# MTUNE_FLAG = -mtune=generic

### AMD

# Athlon 64
# MARCH_FLAG = -march=athlon64
# MTUNE_FLAG = -mtune=athlon64

# Athlon 64 SSE3
# MARCH_FLAG = -march=athlon64-sse3
# MTUNE_FLAG = -mtune=athlon64-sse3

# Barcelona
# MARCH_FLAG = -march=barcelona
# MTUNE_FLAG = -mtune=barcelona

# Bulldozer v1
# MARCH_FLAG = -march=bdver1
# MTUNE_FLAG = -mtune=bdver1

# Bulldozer v2
# MARCH_FLAG = -march=bdver2
# MTUNE_FLAG = -mtune=bdver2

# Bulldozer v3
# MARCH_FLAG = -march=bdver3
# MTUNE_FLAG = -mtune=bdver3

# Bulldozer v4
# MARCH_FLAG = -march=bdver4
# MTUNE_FLAG = -mtune=bdver4

# Zen v1
# MARCH_FLAG = -march=znver1
# MTUNE_FLAG = -mtune=znver1

# Zen v2
# MARCH_FLAG = -march=znver2
# MTUNE_FLAG = -mtune=znver2

# Bobcat v1
# MARCH_FLAG = -march=btver1
# MTUNE_FLAG = -mtune=btver1

# Bobcat v2
# MARCH_FLAG = -march=btver2
# MTUNE_FLAG = -mtune=btver2

### Intel

# Generic Intel
# MARCH_FLAG = -march=x86-64
# MTUNE_FLAG = -mtune=intel

# Cocona
# MARCH_FLAG = -march=nocona
# MTUNE_FLAG = -mtune=nocona

# Core2
# MARCH_FLAG = -march=core2
# MTUNE_FLAG = -mtune=core2

# Nehalem
# MARCH_FLAG = -march=nehalem
# MTUNE_FLAG = -mtune=nehalem

# Westmere
# MARCH_FLAG = -march=westmere
# MTUNE_FLAG = -mtune=westmere

# Sandybridge
# MARCH_FLAG = -march=sandybridge
# MTUNE_FLAG = -mtune=sandybridge

# Ivybridge
# MARCH_FLAG = -march=ivybridge
# MTUNE_FLAG = -mtune=ivybridge

# Haswell
# MARCH_FLAG = -march=haswell
# MTUNE_FLAG = -mtune=haswell

# Broadwell
# MARCH_FLAG = -march=broadwell
# MTUNE_FLAG = -mtune=broadwell

# Skylake
# MARCH_FLAG = -march=skylake
# MTUNE_FLAG = -mtune=skylake

# Bonnell
# MARCH_FLAG = -march=bonnell
# MTUNE_FLAG = -mtune=bonnell

# Silvermont
# MARCH_FLAG = -march=silvermont
# MTUNE_FLAG = -mtune=silvermont

# Goldmont
# MARCH_FLAG = -march=goldmont
# MTUNE_FLAG = -mtune=goldmont

# Goldmont Plus
# MARCH_FLAG = -march=goldmont-plus
# MTUNE_FLAG = -mtune=goldmont-plus

# Tremont
# MARCH_FLAG = -march=tremont
# MTUNE_FLAG = -mtune=tremont

# KNL
# MARCH_FLAG = -march=knl
# MTUNE_FLAG = -mtune=knl

# KNM
# MARCH_FLAG = -march=knm
# MTUNE_FLAG = -mtune=knm

# Skylake-AVX512
# MARCH_FLAG = -march=skylake-avx512
# MTUNE_FLAG = -mtune=skylake-avx512

# Cannonlake
# MARCH_FLAG = -march=cannonlake
# MTUNE_FLAG = -mtune=cannonlake

# Icelake
# MARCH_FLAG = -march=icelake
# MTUNE_FLAG = -mtune=icelake

# Icelake Client
# MARCH_FLAG = -march=icelake-client
# MTUNE_FLAG = -mtune=icelake-client

# Icelake Server
# MARCH_FLAG = -march=icelake-server
# MTUNE_FLAG = -mtune=icelake-server

# Cascadelake
# MARCH_FLAG = -march=cascadelake
# MTUNE_FLAG = -mtune=cascadelake

# Cooperlake
# MARCH_FLAG = -march=cooperlake
# MTUNE_FLAG = -mtune=cooperlake

# Tigerlake
# MARCH_FLAG = -march=tigerlake
# MTUNE_FLAG = -mtune=tigerlake

### Raspberry PI section

# RPI 3 - this does not seem to help much, if at all
# MARCH_FLAG = -mcpu=cortex-a53
# MTUNE_FLAG = -mtune=cortex-a53

# RPI 4
# MARCH_FLAG = -mcpu=cortex-a72
# MTUNE_FLAG = -mtune=cortex-a72

# Asus Tinkerboard
# MARCH_FLAG = -mcpu=cortex-a17
# MTUNE_FLAG = -mtune=cortex-a17

###############################################################################
# Build configuration options used to compute generic compiler flags
# These may be edited by the user in order to configure the build
#

# Uncomment the line below to customize the installation root directory. The
# default is /usr
# CUSTOM_INSTALL_PREFIX = /usr/local

# Uncomment this to disable the usage of OpenGL rendering. If you disable this,
# the line width of the scheduling graphs will always be 1 pixel. You should
# disable this if your computer is not OpenGL capable, or if you are happy to
# always have the scheduling graphs drawn with a width of 1.
# DISABLE_OPENGL = yes

# Uncomment this to use the libqcustomplot of your system. At the time of
# writing, this is a bad idea. Only do this if you know exactly what you are
# doing. traceshark has its own QCustomPlot, which contains important
# performance fixes that are unlikely to be in the system libqcustomplot.
# On the other hand, if you do not care about performance and have at least one
# other application that uses libqcustomplot and you want to save about 1.3 MB
# of disk space, then this is for you. If libqcustomplot was compiled without
# OpenGL support, then you probably want to disable OpenGL above as well.
# USE_SYSTEM_QCUSTOMPLOT = yes

# If you have uncommented the previous, you can uncomment this to customize
# the flag for linking with libqcustomplot. It defaults to -lqcustomplot.
# Make sure that you compile traceshark with the same Qt version as your
# libqcustomplot. Currently it is better to use Qt 5.
# USE_CUSTOM_QCUSTOMPLOT_FLAG = -lqcustomplot-qt5

# Uncomment this for debug symbols
# USE_DEBUG_FLAG = -g

# Uncomment this for debug symbols and without optimization:
# USE_DEBUG_FLAG = -g -O0

# Uncomment this for debug build. This affects Qt.
# QT_DEBUG_BUILD = yes

# Uncomment if you want to use hardening flags
# Not really needed, unless browsing data controlled by a non-trusted source
# or for testing purposes.
# USE_HARDENING_CXXFLAGS = yes

# If you want to compile with another compiler than the defaul g++, then
# uncomment and change to the compiler of your choice
# USE_ALTERNATIVE_COMPILER = clang++-10
# USE_ALTERNATIVE_COMPILER = g++-10

# These optimization options do not seem to help, so leave them commented out.
# Only play with these if you are interested in playing with obscure compiler
# optimizations.
# USE_EXTRA_OPTS  = -fpredictive-commoning -fvect-cost-model -fsplit-paths -ftree-vectorize -funswitch-loops -floop-interchange
# USE_EXTRA_OPTS += -funsafe-math-optimizations
# USE_EXTRA_OPTS += -O3

############################# ATTENTION !!!!! ##################################
############################# ATTENTION !!!!! ##################################
############################# ATTENTION !!!!! ##################################
# Do not edit anything below this, unless you are developing traceshark, it's
# not meant to be changed by regular users.
################################################################################

################################################################################
# Header files
#

!equals(USE_SYSTEM_QCUSTOMPLOT, yes) {
HEADERS       = qcustomplot/qcustomplot.h
HEADERS      += qcustomplot/qcppointer.h
HEADERS      += qcustomplot/qcppointer_impl.h
HEADERS      += qcustomplot/qcplist.h
}

HEADERS      +=  ui/abstracttaskmodel.h
HEADERS      +=  ui/cpuselectdialog.h
HEADERS      +=  ui/cpuselectmodel.h
HEADERS      +=  ui/cursor.h
HEADERS      +=  ui/cursorinfo.h
HEADERS      +=  ui/errordialog.h
HEADERS      +=  ui/eventinfodialog.h
HEADERS      +=  ui/eventselectdialog.h
HEADERS      +=  ui/eventselectmodel.h
HEADERS      +=  ui/eventsmodel.h
HEADERS      +=  ui/eventswidget.h
HEADERS      +=  ui/graphenabledialog.h
HEADERS      +=  ui/infowidget.h
HEADERS      +=  ui/latencymodel.h
HEADERS      +=  ui/latencywidget.h
HEADERS      +=  ui/licensedialog.h
HEADERS      +=  ui/mainwindow.h
HEADERS      +=  ui/migrationarrow.h
HEADERS      +=  ui/migrationline.h
HEADERS      +=  ui/qcustomplot.h
HEADERS      +=  ui/regexdialog.h
HEADERS      +=  ui/regexwidget.h
HEADERS      +=  ui/statslimitedmodel.h
HEADERS      +=  ui/statsmodel.h
HEADERS      +=  ui/tableview.h
HEADERS      +=  ui/taskgraph.h
HEADERS      +=  ui/taskmodel.h
HEADERS      +=  ui/taskrangeallocator.h
HEADERS      +=  ui/taskselectdialog.h
HEADERS      +=  ui/tasktoolbar.h
HEADERS      +=  ui/traceplot.h
HEADERS      +=  ui/tracesharkstyle.h
HEADERS      +=  ui/valuebox.h
HEADERS      +=  ui/yaxisticker.h

HEADERS      +=  analyzer/abstracttask.h
HEADERS      +=  analyzer/cpufreq.h
HEADERS      +=  analyzer/cpu.h
HEADERS      +=  analyzer/cpuidle.h
HEADERS      +=  analyzer/cputask.h
HEADERS      +=  analyzer/filterstate.h
HEADERS      +=  analyzer/latency.h
HEADERS      +=  analyzer/latencycomp.h
HEADERS      +=  analyzer/migration.h
HEADERS      +=  analyzer/regexfilter.h
HEADERS      +=  analyzer/task.h
HEADERS      +=  analyzer/tcolor.h
HEADERS      +=  analyzer/traceanalyzer.h

HEADERS      +=  parser/fileinfo.h
HEADERS      +=  parser/genericparams.h
HEADERS      +=  parser/paramhelpers.h
HEADERS      +=  parser/traceevent.h
HEADERS      +=  parser/tracefile.h
HEADERS      +=  parser/tracelinedata.h
HEADERS      +=  parser/traceline.h
HEADERS      +=  parser/traceparser.h

HEADERS      +=  parser/ftrace/ftraceparams.h
HEADERS      +=  parser/ftrace/ftracegrammar.h

HEADERS      +=  parser/perf/helpers.h
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

HEADERS      +=  misc/chunk.h
HEADERS      +=  misc/errors.h
HEADERS      +=  misc/maplist.h
HEADERS      +=  misc/osapi.h
HEADERS      +=  misc/pngresources.h
HEADERS      +=  misc/resources.h
HEADERS      +=  misc/setting.h
HEADERS      +=  misc/settingstore.h
HEADERS      +=  misc/string.h
HEADERS      +=  misc/svgresources.h
HEADERS      +=  misc/traceshark.h
HEADERS      +=  misc/translate.h
HEADERS      +=  misc/tstring.h
HEADERS      +=  misc/types.h

HEADERS      +=  vtl/avltree.h
HEADERS      +=  vtl/bitvector.h
HEADERS      +=  vtl/bsdexits.h
HEADERS      +=  vtl/compiler.h
HEADERS      +=  vtl/error.h
HEADERS      +=  vtl/heapsort.h
HEADERS      +=  vtl/tlist.h
HEADERS      +=  vtl/time.h

###############################################################################
# Source files
#

!equals(USE_SYSTEM_QCUSTOMPLOT, yes) {
SOURCES       = qcustomplot/qcustomplot.cpp
}

SOURCES      +=  ui/abstracttaskmodel.cpp
SOURCES      +=  ui/cpuselectdialog.cpp
SOURCES      +=  ui/cpuselectmodel.cpp
SOURCES      +=  ui/cursor.cpp
SOURCES      +=  ui/cursorinfo.cpp
SOURCES      +=  ui/errordialog.cpp
SOURCES      +=  ui/eventinfodialog.cpp
SOURCES      +=  ui/eventselectdialog.cpp
SOURCES      +=  ui/eventselectmodel.cpp
SOURCES      +=  ui/eventsmodel.cpp
SOURCES      +=  ui/eventswidget.cpp
SOURCES      +=  ui/graphenabledialog.cpp
SOURCES      +=  ui/infowidget.cpp
SOURCES      +=  ui/latencymodel.cpp
SOURCES      +=  ui/latencywidget.cpp
SOURCES      +=  ui/licensedialog.cpp
SOURCES      +=  ui/mainwindow.cpp
SOURCES      +=  ui/migrationarrow.cpp
SOURCES      +=  ui/migrationline.cpp
SOURCES      +=  ui/regexdialog.cpp
SOURCES      +=  ui/regexwidget.cpp
SOURCES      +=  ui/statslimitedmodel.cpp
SOURCES      +=  ui/statsmodel.cpp
SOURCES      +=  ui/tableview.cpp
SOURCES      +=  ui/taskgraph.cpp
SOURCES      +=  ui/taskmodel.cpp
SOURCES      +=  ui/taskrangeallocator.cpp
SOURCES      +=  ui/taskselectdialog.cpp
SOURCES      +=  ui/tasktoolbar.cpp
SOURCES      +=  ui/traceplot.cpp
SOURCES      +=  ui/tracesharkstyle.cpp
SOURCES      +=  ui/valuebox.cpp
SOURCES      +=  ui/yaxisticker.cpp


SOURCES      +=  analyzer/abstracttask.cpp
SOURCES      +=  analyzer/cpufreq.cpp
SOURCES      +=  analyzer/cpuidle.cpp
SOURCES      +=  analyzer/cputask.cpp
SOURCES      +=  analyzer/filterstate.cpp
SOURCES      +=  analyzer/latencycomp.cpp
SOURCES      +=  analyzer/regexfilter.cpp
SOURCES      +=  analyzer/task.cpp
SOURCES      +=  analyzer/tcolor.cpp
SOURCES      +=  analyzer/traceanalyzer.cpp

SOURCES      +=  parser/fileinfo.cpp
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

SOURCES      +=  misc/errors.cpp
SOURCES      +=  misc/main.cpp
SOURCES      +=  misc/setting.cpp
SOURCES      +=  misc/settingstore.cpp
SOURCES      +=  misc/traceshark.cpp

SOURCES      +=  misc/translate.cpp

SOURCES      +=  vtl/bitvector.cpp
SOURCES      +=  vtl/error.cpp

###############################################################################
# Directories
#

#DESTDIR=bin #Target file directory
OBJECTS_DIR=obj
MOC_DIR=obj


#############################################################################
# Compute generic compiler flags
#

GIT_VERSION_HEADERS = misc/gitversion-template.h
gitversion.output =  obj/gitversion.h
gitversion.dependency_type = TYPE_C
gitversion.variable_out = HEADERS
gitversion.commands = ./scripts/gitversion --input ${QMAKE_FILE_NAME} --output ${QMAKE_FILE_OUT}
gitversion.input = GIT_VERSION_HEADERS
QMAKE_EXTRA_COMPILERS += gitversion

equals(QT_DEBUG_BUILD, yes) {
CONFIG += debug
} else {
CONFIG += release
}

HARDENING_CXXFLAGS += -fPIE -pie
HARDENING_CXXFLAGS += -D_FORTIFY_SOURCE=2
HARDENING_CXXFLAGS += -Wformat -Wformat-security -Werror=format-security
HARDENING_CXXFLAGS += -fstack-protector-strong

HARDENING_LFLAGS += -Wl,-z,relro,-z,now

OUR_FLAGS = $${MARCH_FLAG} $${MTUNE_FLAG} $${USE_DEBUG_FLAG} $${USE_EXTRA_OPTS}

equals(USE_HARDENING_CXXFLAGS, yes) {
OUR_FLAGS += $${HARDENING_CXXFLAGS}
}

equals(USE_SYSTEM_QCUSTOMPLOT, yes) {

isEmpty(USE_CUSTOM_QCUSTOMPLOT_FLAG) {
QCUSTOM_FLAG = -lqcustomplot
} else {
QCUSTOM_FLAG = $${USE_CUSTOM_QCUSTOMPLOT_FLAG}
}

OUR_FLAGS += -DCONFIG_SYSTEM_QCUSTOMPLOT
LIBS += -lqcustomplot
}

OUR_NORMAL_CXXFLAGS = -pedantic -Wall -std=c++11
OUR_NORMAL_CFLAGS = -pedantic -Wall -std=c11

QMAKE_CXXFLAGS_RELEASE += $${OUR_NORMAL_CXXFLAGS} $${OUR_FLAGS}
QMAKE_CFLAGS_RELEASE += -$${OUR_NORMAL_CFLAGS} $${OUR_FLAGS}
QMAKE_LFLAGS_RELEASE += -fwhole-program -O2 -std=c++11 $${OUR_FLAGS}

equals (USE_HARDENING_CXXFLAGS, yes) {
QMAKE_LFLAGS_RELEASE += $${HARDENING_LFLAGS}
}

!isEmpty (USE_ALTERNATIVE_COMPILER) {
QMAKE_CXX = $${USE_ALTERNATIVE_COMPILER}
QMAKE_LINK = $${USE_ALTERNATIVE_COMPILER}
}

OUR_POSIX_DEFINES = _FILE_OFFSET_BITS=64 _POSIX_C_SOURCE=200809L

mac {
OUR_POSIX_DEFINES += _DARWIN_C_SOURCE
}

# Compute the defines to be set with -D flag at the compiler command line
DEFINES += $${OUR_POSIX_DEFINES}
!equals(DISABLE_OPENGL, yes) {
equals(QT_MAJOR_VERSION, 4) {
DEFINES += TRACESHARK_QT4_OPENGL
} else {
DEFINES += QCUSTOMPLOT_USE_OPENGL
}
}

###############################################################################
# Qt Modules
#

QT           += core
QT           += widgets
QT           += printsupport
!equals(DISABLE_OPENGL, yes): equals(QT_MAJOR_VERSION, 4) {
QT           += opengl
}


###############################################################################
# Resources
#

RESOURCES     = traceshark.qrc


###############################################################################
# Installation
#

isEmpty (CUSTOM_INSTALL_PREFIX) {
INSTALL_PREFIX = /usr
} else {
INSTALL_PREFIX = $${CUSTOM_INSTALL_PREFIX}
}

documentation.path = $${INSTALL_PREFIX}/share/man/man1
documentation.files = doc/traceshark.1

target.path = $${INSTALL_PREFIX}/bin
target.files = traceshark

INSTALLS += documentation
INSTALLS += target
