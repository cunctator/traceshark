/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TRACEPARSER_H
#define TRACEPARSER_H

#include <QVector>

#include "parser/genericparams.h"
#include "parser/ftrace/ftracegrammar.h"
#include "parser/perf/perfgrammar.h"
#include "mm/mempool.h"
#include "parser/tracelinedata.h"
#include "parser/traceline.h"
#include "parser/traceevent.h"
#include "misc/traceshark.h"
#include "threads/indexwatcher.h"
#include "threads/threadbuffer.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"
#include "misc/tstring.h"

#define NR_TBUFFERS (4)
#define TBUFSIZE (256)

class TraceFile;
class TraceAnalyzer;
namespace vtl {
	template<class T> class TList;
}

class TraceParser
{
	friend class TraceAnalyzer;
public:
	TraceParser(vtl::TList<TraceEvent> *analyzerEvents);
	~TraceParser();
	int open(const QString &fileName);
	bool isOpen() const;
	void close();
	void threadParser();
	void threadReader();
protected:
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
	__always_inline bool parseLineFtrace(TraceLine &line,
					     TraceEvent &event);
	__always_inline bool parseLinePerf(TraceLine &line, TraceEvent &event);
	void fixLastEvent();
	bool parseBuffer(unsigned int index);
	bool parseLineBugFixup(TraceEvent* event, const vtl::Time &prevTime);
	TraceFile *traceFile;
	MemPool *ptrPool;
	MemPool *postEventPool;
	TraceEvent fakeEvent;
	TString fakePostEventInfo;
	FtraceGrammar *ftraceGrammar;
	PerfGrammar *perfGrammar;
	ThreadBuffer<TraceLine> **tbuffers;
	WorkThread<TraceParser> *parserThread;
	WorkThread<TraceParser> *readerThread;
	TraceLineData ftraceLineData;
	TraceLineData perfLineData;
	vtl::TList<TraceEvent> *events;
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
	bool eof;

	ThreadBuffer<TraceLine> *tbuf = tbuffers[index];
	tbuf->beginConsumeBuffer();

	s = tbuf->list.size();

	for(i = 0; i < s; i++) {
		TraceLine &line = tbuf->list[i];
		TraceEvent &event = events->preAlloc();
		event.argc = 0;
		event.argv = (const TString**)
			ptrPool->preallocN(EVENT_MAX_NR_ARGS);
		if (ttype == TRACE_TYPE_FTRACE)
			parseLineFtrace(line, event);
		else if (ttype == TRACE_TYPE_PERF)
			parseLinePerf(line, event);
	}
	eof = tbuf->loadBuffer->isEOF();
	tbuf->endConsumeBuffer();
	return eof;
}

__always_inline bool TraceParser::parseLineFtrace(TraceLine &line,
						  TraceEvent &event)
{
	if (ftraceGrammar->parseLine(line, event)) {
		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < ftraceLineData.prevTime) {
			if (!parseLineBugFixup(&event, ftraceLineData.prevTime))
				return true;
		}
		ftraceLineData.prevTime = event.time;

		ptrPool->commitN(event.argc);
		events->commit();

		event.postEventInfo = nullptr;
		ftraceLineData.nrEvents++;
		/* probably not necessary because ftrace traces doesn't
		 * have backtraces and stuff but do it anyway */
		ftraceLineData.prevLineIsEvent = true;
		return true;
	}
	return false;
}

__always_inline bool TraceParser::parseLinePerf(TraceLine &line,
						TraceEvent &event)
{
	if (perfGrammar->parseLine(line, event)) {
		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < perfLineData.prevTime) {
			if (!parseLineBugFixup(&event, perfLineData.prevTime))
				return true;
		}
		perfLineData.prevTime = event.time;

		ptrPool->commitN(event.argc);
		events->commit();

		if (perfLineData.prevLineIsEvent) {
			perfLineData.prevEvent->postEventInfo = nullptr;
		} else {
			TString *str = (TString*) postEventPool->
				allocObj();
			str->ptr = perfLineData.infoBegin;
			str->len = line.begin - perfLineData.infoBegin;
			perfLineData.prevEvent->postEventInfo = str;
			perfLineData.prevLineIsEvent = true;
		}
		perfLineData.prevEvent = &event;
		perfLineData.nrEvents++;
		return true;
	} else {
		if (perfLineData.prevLineIsEvent) {
			perfLineData.infoBegin = line.begin;
			perfLineData.prevLineIsEvent = false;
		}
		return false;
	}
}

#endif /* TRACEPARSER_H */
