// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2020, 2026  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
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
#include "misc/chunk.h"
#include "misc/traceshark.h"
#include "threads/indexwatcher.h"
#include "threads/threadbuffer.h"
#include "threads/workitem.h"
#include "threads/workthread.h"
#include "threads/workqueue.h"
#include "misc/tstring.h"
#include "vtl/compiler.h"

#define NR_TBUFFERS (4)

class TraceFile;
class TraceAnalyzer;
namespace vtl {
	template<class T> class TList;
}

class TraceParser
{
	friend class TraceAnalyzer;
public:
	TraceParser();
	~TraceParser();
	int open(const QString &fileName);
	bool isOpen() const;
	void close(int *ts_errno);
	void threadParser();
	void threadReader();
	vtl_always_inline vtl::TList<TraceEvent> *getEventsTList() const;
	const StringTree<> *getPerfEventTree();
	const StringTree<> *getFtraceEventTree();
protected:
	vtl_always_inline void waitForNextBatch(bool &eof, int &index);
	void waitForTraceType();
	tracetype_t traceType;
	TraceFile *traceFile;
private:
	void determineTraceType();
	void guessTraceType();
	void sendTraceType();
	void prepareParse();
	vtl_always_inline bool parseBuffer_(tracetype_t ttppe,
					    unsigned int index);
	vtl_always_inline bool parseFtraceBuffer(unsigned int index);
	vtl_always_inline bool parsePerfBuffer(unsigned int index);
	vtl_always_inline bool parseLineFtrace(TraceLine &line,
					       TraceEvent &event);
	vtl_always_inline
	bool parseLinePerf(TraceLine &line, TraceEvent &event);
	void fixLastEvent();
	bool parseBuffer(unsigned int index);
	bool parseLineBugFixup(TraceEvent* event, const vtl::Time &prevTime);
	void attachStackChunk(TraceEvent *origin, int64_t begin, int64_t end);
	void clearFtraceStackData();
	vtl_always_inline TraceEvent *ftraceLastEventForCPU(unsigned int cpu)
		const;
	vtl_always_inline void ftraceSetLastEventForCPU(unsigned int cpu,
							TraceEvent *event);
	MemPool *ptrPool;
	MemPool *postEventPool;
	TraceEvent fakeEvent;
	Chunk fakePostEventInfo;
	FtraceGrammar *ftraceGrammar;
	PerfGrammar *perfGrammar;
	ThreadBuffer<TraceLine> **tbuffers;
	WorkThread<TraceParser> *parserThread;
	WorkThread<TraceParser> *readerThread;
	TraceLineData ftraceLineData;
	TraceLineData perfLineData;
	/*
	 * The fields below are only used when parsing ftrace traces, in order
	 * to associate kernel_stack/user_stack events with the event whose
	 * stack trace they contain. Such an event belongs to the most recent
	 * ordinary event on the same CPU, which is tracked by
	 * ftraceLastEventByCPU. The ftraceStack* fields hold a stack-trace
	 * capture whose length is not yet known because it ends at the next
	 * event line.
	 */
	bool ftraceStackPending;
	int64_t ftraceStackInfoBegin;
	TraceEvent *ftraceStackOrigin;
	TraceEvent *ftraceLastEventByCPU[NR_CPUS_ALLOWED];
	vtl::TList<TraceEvent> *ftraceEvents;
	vtl::TList<TraceEvent> *perfEvents;
	vtl::TList<TraceEvent> *events;
	IndexWatcher *eventsWatcher;
	/* This IndexWatcher isn't really watching an index, it's to synchronize
	 * when traceType has been determined in the parser thread */
	IndexWatcher *traceTypeWatcher;
};

vtl_always_inline void TraceParser::waitForNextBatch(bool &eof, int &index)
{
	eventsWatcher->waitForNextBatch(eof, index);
}

vtl_always_inline
TraceEvent *TraceParser::ftraceLastEventForCPU(unsigned int cpu) const
{
	if (isValidCPU(cpu))
		return ftraceLastEventByCPU[cpu];
	return nullptr;
}

vtl_always_inline void TraceParser::ftraceSetLastEventForCPU(unsigned int cpu,
							    TraceEvent *event)
{
	if (isValidCPU(cpu))
		ftraceLastEventByCPU[cpu] = event;
}

/* This parses a buffer */
vtl_always_inline bool TraceParser::parseFtraceBuffer(unsigned int index)
{
	return parseBuffer_(TRACE_TYPE_FTRACE, index);
}

/* This parses a buffer */
vtl_always_inline bool TraceParser::parsePerfBuffer(unsigned int index)
{
	return parseBuffer_(TRACE_TYPE_PERF, index);
}

/* This parses a buffer */
vtl_always_inline bool TraceParser::parseBuffer_(tracetype_t ttype,
						 unsigned int index)
{
	unsigned int i, s;
	bool eof;
	const TString **argv;

	ThreadBuffer<TraceLine> *tbuf = tbuffers[index];
	tbuf->beginConsumeBuffer();

	s = tbuf->list.size();
	argv = (const TString**) ptrPool->preallocN(EVENT_MAX_NR_ARGS);

	for(i = 0; i < s; i++) {
		TraceLine &line = tbuf->list[i];
		if (ttype == TRACE_TYPE_FTRACE) {
			TraceEvent &event = ftraceEvents->preAlloc();
			event.argc = 0;
			event.argv = argv;
			if (parseLineFtrace(line, event)) {
				argv = (const TString**)
					ptrPool->preallocN(EVENT_MAX_NR_ARGS);
			}
		} else if (ttype == TRACE_TYPE_PERF) {
			TraceEvent &event = perfEvents->preAlloc();
			event.argc = 0;
			event.argv = argv;
			if (parseLinePerf(line, event)) {
				argv = (const TString**)
					ptrPool->preallocN(EVENT_MAX_NR_ARGS);
			}
		}
	}
	eof = tbuf->loadBuffer->isEOF();
	tbuf->endConsumeBuffer();
	return eof;
}

vtl_always_inline bool TraceParser::parseLineFtrace(TraceLine &line,
						    TraceEvent &event)
{
	if (ftraceGrammar->parseLine(line, event)) {
		/*
		 * This event line terminates any pending stack-trace capture
		 * from a preceding kernel_stack/user_stack event; the captured
		 * range ends where this line begins.
		 */
		if (ftraceStackPending) {
			attachStackChunk(ftraceStackOrigin,
					 ftraceStackInfoBegin,
					 line.begin);
			ftraceStackPending = false;
		}

		/* Check if the timestamp of this event is affected by
		 * the infamous ftrace timestamp rollover bug and
		 * try to correct it */
		if (event.time < ftraceLineData.prevTime) {
			if (!parseLineBugFixup(&event, ftraceLineData.prevTime))
				return true;
		}
		ftraceLineData.prevTime = event.time;

		if (event.type == KERNEL_STACK || event.type == USER_STACK) {
			/*
			 * The stack trace belongs to the most recent ordinary
			 * event on the same CPU. Begin capturing its text
			 * (which spans this line and the trailing frame lines,
			 * up to the next event line) and discard the stack
			 * event itself by not committing it. If there is no
			 * such ordinary event yet, there is nothing to attach
			 * it to, so just drop it.
			 */
			TraceEvent *origin = ftraceLastEventForCPU(event.cpu);
			if (origin != nullptr) {
				ftraceStackPending = true;
				ftraceStackInfoBegin = line.begin;
				ftraceStackOrigin = origin;
			}
			return false;
		}

		ptrPool->commitN(event.argc);
		ftraceEvents->commit();

		event.postEventInfo = nullptr;
		ftraceSetLastEventForCPU(event.cpu, &event);
		ftraceLineData.nrEvents++;
		ftraceLineData.prevLineIsEvent = true;
		return true;
	}
	return false;
}

vtl_always_inline bool TraceParser::parseLinePerf(TraceLine &line,
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
		perfEvents->commit();

		if (perfLineData.prevLineIsEvent) {
			perfLineData.prevEvent->postEventInfo = nullptr;
		} else {
			Chunk *chunk = (Chunk*) postEventPool->
				allocObj();
			chunk->offset = perfLineData.infoBegin;
			chunk->len = line.begin - perfLineData.infoBegin;
			chunk->next = nullptr;
			perfLineData.prevEvent->postEventInfo = chunk;
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

vtl_always_inline vtl::TList<TraceEvent> *TraceParser::getEventsTList() const
{
	return events;
}

#endif /* TRACEPARSER_H */
