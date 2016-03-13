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

#ifndef TRACEPARSER_H
#define TRACEPARSER_H

#include <QVector>

#include "genericparams.h"
#include "grammar.h"
#include "grammarnode.h"
#include "mm/mempool.h"
#include "traceline.h"
#include "traceevent.h"
#include "tlist.h"
#include "traceshark.h"
#include "threads/indexwatcher.h"
#include "threads/threadbuffer.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"
#include "tstring.h"

#define NR_TBUFFERS (3)
#define TBUFSIZE (256)

class TraceFile;
class TraceAnalyzer;

class TraceParser
{
	friend class TraceAnalyzer;
public:
	TraceParser(TList<TraceEvent> *analyzerEvents);
	~TraceParser();
	bool open(const QString &fileName);
	bool isOpen();
	void close();
	void threadParser();
	void threadReader();
protected:
	unsigned long nrFtraceEvents;
	unsigned long nrPerfEvents;
	tracetype_t traceType;
	__always_inline void waitForNextBatch(bool &eof, int &index);
	void waitForTraceType();
private:
	void determineTraceType();
	void sendTraceType();
	void prepareParse();
	__always_inline bool __parseBuffer(tracetype_t ttppe,
					   unsigned int index);
	__always_inline bool parseFtraceBuffer(unsigned int index);
	__always_inline bool parsePerfBuffer(unsigned int index);
	__always_inline bool parseLine(TraceLine* line, TraceEvent* event,
				       GrammarNode *root);
	__always_inline bool parseLineFtrace(TraceLine* line,
					     TraceEvent &event);
	__always_inline bool parseLinePerf(TraceLine *line, TraceEvent &event);
	void fixLastEvent();
	bool parseBuffer(unsigned int index);
	bool parseLineBugFixup(TraceEvent* event);
	TraceFile *traceFile;
	MemPool *ptrPool;
	MemPool *postEventPool;
	TraceEvent fakeEvent;
	TString fakePostEventInfo;
	Grammar *ftraceGrammar;
	Grammar *perfGrammar;
	ThreadBuffer<TraceLine> **tbuffers;
	WorkThread<TraceParser> *parserThread;
	WorkThread<TraceParser> *readerThread;
	char *infoBegin;
	bool prevLineIsEvent;
	double prevTime;
	TraceEvent *prevEvent;
	TList<TraceEvent> *events;
	IndexWatcher *eventsWatcher;
	/* This IndexWatcher isn't really watching an index, it's to synchronize
	 * when traceType has been determined in the parser thread */
	IndexWatcher *traceTypeWatcher;
};

__always_inline void TraceParser::waitForNextBatch(bool &eof, int &index)
{
	eventsWatcher->waitForNextBatch(eof, index);
}

/* This parses a buffer */
__always_inline bool TraceParser::parseFtraceBuffer(unsigned int index)
{
	return __parseBuffer(TRACE_TYPE_FTRACE, index);
}

/* This parses a buffer */
__always_inline bool TraceParser::parsePerfBuffer(unsigned int index)
{
	return __parseBuffer(TRACE_TYPE_PERF, index);
}

/* This parses a buffer */
__always_inline bool TraceParser::__parseBuffer(tracetype_t ttype,
						unsigned int index)
{
	unsigned int i, s;

	ThreadBuffer<TraceLine> *tbuf = tbuffers[index];
	if (tbuf->beginConsumeBuffer()) {
		tbuf->endConsumeBuffer(); /* Uncessary but beatiful */
		return true;
	}

	s = tbuf->nRead;

	for(i = 0; i < s; i++) {
		TraceLine *line = &tbuf->buffer[i];
		TraceEvent &event = events->preAlloc();
		event.argc = 0;
		event.argv = (TString**) ptrPool->preallocN(256);
		if (ttype == TRACE_TYPE_FTRACE)
			parseLineFtrace(line, event);
		else if (ttype == TRACE_TYPE_PERF)
			parseLinePerf(line, event);
	}
	tbuf->endConsumeBuffer();
	return false;
}
		


__always_inline bool TraceParser::parseLine(TraceLine* line, TraceEvent* event,
					    GrammarNode *root)
{
	unsigned int i,j;
	GrammarNode *node = root;
	bool retval = root->isLeaf;
	event->argc = 0;

	for (i = 0; i < line->nStrings; i++)
	{
		TString *str = &line->strings[i];
		for (j = 0; j < node->nChildren; j++) {
			if (node->children[j]->match(str, event)) {
				node = node->children[j];
				retval = node->children[j]->isLeaf;
				goto cont;
			}
		}
		return false;
	cont:
		continue;
	}
	return retval;
}

__always_inline bool TraceParser::parseLineFtrace(TraceLine* line,
						  TraceEvent &event)
{
	if (parseLine(line, &event, ftraceGrammar->root)) {
		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < prevTime) {
			if (!parseLineBugFixup(&event))
				return true;
		}
		prevTime = event.time;
		ptrPool->commitN(event.argc);
		event.postEventInfo = NULL;
		events->commit();
		nrFtraceEvents++;
		/* probably not necessary because ftrace traces doesn't
		 * have backtraces and stuff but do it anyway */
		prevLineIsEvent = true;
		return true;
	}
	return false;
}

__always_inline bool TraceParser::parseLinePerf(TraceLine* line,
						TraceEvent & event)
{
	if (parseLine(line, &event, perfGrammar->root)) {
		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < prevTime) {
			if (!parseLineBugFixup(&event))
				return true;
		}
		prevTime = event.time;
		ptrPool->commitN(event.argc);
		if (prevLineIsEvent) {
			prevEvent->postEventInfo = NULL;
		} else {
			TString *str = (TString*) postEventPool->
				allocObj();
			str->ptr = infoBegin;
			str->len = line->begin - infoBegin;
			prevEvent->postEventInfo = str;
			prevLineIsEvent = true;
		}
		events->commit();
		prevEvent = &event;
		nrPerfEvents++;
		return true;
	} else {
		if (prevLineIsEvent) {
			infoBegin = line->begin;
			prevLineIsEvent = false;
		}
		return false;
	}
}

#endif /* TRACEPARSER_H */
