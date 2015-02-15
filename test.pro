HEADERS       = ftraceparser.h
HEADERS      += mempool.h
HEADERS      += tracefile.h
HEADERS      += traceline.h

SOURCES       = main.cpp
SOURCES      += ftraceparser.cpp
SOURCES      += mempool.cpp
SOURCES      += tracefile.cpp
QT           += core

QMAKE_CXXFLAGS_RELEASE += -Wall -g
QMAKE_CFLAGS_RELEASE += -pedantic -Wall -std=c99
QMAKE_LFLAGS_RELEASE =
CONFIG += DEBUG
