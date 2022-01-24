// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2018, 2020, 2022
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <climits>
#include <cstring>
#include <cstdio>
#include <limits>

#include "misc/tstring.h"
#include "parser/genericparams.h"
#include "mm/mempool.h"
#include "parser/ftrace/ftracegrammar.h"
#include "parser/perf/perfgrammar.h"
#include "parser/tracefile.h"
#include "parser/traceparser.h"
#include "misc/errors.h"
#include "misc/chunk.h"
#include "misc/traceshark.h"
#include "threads/indexwatcher.h"
#include "threads/threadbuffer.h"

#define TRACE_TYPE_CONFIDENCE_FACTOR (100)

TraceParser::TraceParser()
	: traceType(TRACE_TYPE_UNKNOWN), events(nullptr)
{
	traceFile = nullptr;
	ptrPool = new MemPool(16384, sizeof(TString*));
	postEventPool = new MemPool(16384, sizeof(Chunk));

	ftraceGrammar = new FtraceGrammar();
	perfGrammar = new PerfGrammar();

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<TraceParser>
		(QString("parserThread"), this, &TraceParser::threadParser);
	readerThread = new WorkThread<TraceParser>
		(QString("readerThread"), this, &TraceParser::threadReader);
	eventsWatcher = new IndexWatcher(10000);
	traceTypeWatcher = new IndexWatcher;
	ftraceEvents = new vtl::TList<TraceEvent>();
	perfEvents = new vtl::TList<TraceEvent>();

	fakeEvent.clear();

	fakePostEventInfo.offset = 0;
	fakePostEventInfo.len = 0;

	ftraceLineData.clear();
	perfLineData.clear();
}

TraceParser::~TraceParser()
{
	delete ftraceGrammar;
	delete perfGrammar;
	delete ptrPool;
	delete postEventPool;
	delete[] tbuffers;
	delete parserThread;
	delete readerThread;
	delete eventsWatcher;
	delete traceTypeWatcher;
	delete ftraceEvents;
	delete perfEvents;
}

int TraceParser::open(const QString &fileName)
{
	int ts_errno;
	unsigned int i;

	if (traceFile != nullptr)
		return -TS_ERROR_INTERNAL;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ts_errno,
				  1024 * 1024 * 2);

	if (ts_errno != 0) {
		delete traceFile;
		traceFile = nullptr;
		return ts_errno;
	}

	/* These buffers will be deleted by the parserThread */
	for (i = 0; i < NR_TBUFFERS; i++)
		tbuffers[i] = new ThreadBuffer<TraceLine>();
	eventsWatcher->reset();
	traceTypeWatcher->reset();
	readerThread->start();
	parserThread->start();

	return 0;
}

bool TraceParser::isOpen() const
{
	return (traceFile != nullptr);
}

void TraceParser::close(int *ts_errno)
{
	if (traceFile != nullptr) {
		traceFile->close(ts_errno);
		delete traceFile;
		traceFile = nullptr;
	} else {
		*ts_errno = 0;
	}
	ptrPool->reset();
	perfGrammar->clear();
	perfEvents->clear();
	ftraceGrammar->clear();
	ftraceEvents->clear();
	events = nullptr;
	traceType = TRACE_TYPE_UNKNOWN;
}


void TraceParser::threadReader()
{
	unsigned long long nr = 0;
	unsigned int i = 0;
	unsigned int curbuf = 0;
	bool eof;

	for (i = 0; i < NR_TBUFFERS; i++)
		tbuffers[i]->loadBuffer = traceFile->getLoadBuffer(i);

	tbuffers[curbuf]->beginProduceBuffer();

	while(true) {
		TraceLine *line = &tbuffers[curbuf]->list.increase();
		quint32 n = traceFile->ReadLine(line, tbuffers[curbuf]);
		nr += n;
		if (traceFile->getBufferSwitch()) {
			eof = tbuffers[curbuf]->loadBuffer->isEOF();
			tbuffers[curbuf]->endProduceBuffer();
			if (eof)
				break;
			curbuf++;
			if (curbuf == NR_TBUFFERS)
				curbuf = 0;
			traceFile->clearBufferSwitch();
			tbuffers[curbuf]->beginProduceBuffer();
			eof = tbuffers[curbuf]->loadBuffer->isEOF();
			/*
			 * This is were EOF will be detected in practice, with
			 * the current implementation of LoadBuffer
			 */
			if (eof && tbuffers[curbuf]->loadBuffer->nRead == 0) {
				tbuffers[curbuf]->endProduceBuffer();
				break;
			}
		}
	}

	printf("%llu\n", nr);
}


/*
 * This function does prescanning as well, to determine number of events,
 * number of CPUs, max/min CPU frequency etc.
 */
void TraceParser::threadParser()
{
	unsigned int i = 0;
	bool eof;

	prepareParse();
	while(true) {
		eof = parseBuffer(i);
		determineTraceType();
		if (eof)
			break;
		/*
		 * We cannot send next index, unless trace type has been
		 * determined, because before that the events pointer will not
		 * be determined either.
		 */
		if (traceType != TRACE_TYPE_UNKNOWN)
			eventsWatcher->sendNextIndex(events->size());
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
		if (traceType == TRACE_TYPE_FTRACE)
			goto ftrace;
		if (traceType == TRACE_TYPE_PERF)
			goto perf;
	}
	/*
	 * Must have been a short trace or a lot of unknown garbage in the
	 * trace if we end up here. Make sure that the processing thread can
	 * continue even if no trace type was detected, otherwise it would wait
	 * forever in waitForTraceType()
	 */
	guessTraceType();
	goto out;

	/*
	 * The purpose of jumping to these loops is to  be able to use the
	 * (hopefully faster) specialized parse functions
	 */
ftrace:
	while(true) {
		if (parseFtraceBuffer(i))
			break;
		eventsWatcher->sendNextIndex(ftraceEvents->size());
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
	goto out;

perf:
	while(true) {
		if (parsePerfBuffer(i))
			break;
		eventsWatcher->sendNextIndex(perfEvents->size());
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
out:
	/*
	 * It's probable that at this point, one of the sendNextIndex() calls
	 * above has already been issued with and index value that corresponds
	 * to the last event but that is OK because the fixLastEvent() does
	 * not touch data that would be read by the code that waits for
	 * the sendNextIndex, i.e. the analyzer thread, so it's OK to call
	 * fixLastEvent while the analyzer thread is reading the last event.
	 */
	fixLastEvent();

	eventsWatcher->sendNextIndex(events->size());
	eventsWatcher->sendEOF();

	for (i = 0; i < NR_TBUFFERS; i++)
		delete tbuffers[i];
}

void TraceParser::waitForTraceType()
{
	int index;
	bool eof = false;
	while (!eof)
		traceTypeWatcher->waitForNextBatch(eof, index);
}

void TraceParser::sendTraceType()
{
	traceTypeWatcher->sendEOF();
}

void TraceParser::prepareParse()
{
	fakePostEventInfo.offset = 0;
	fakePostEventInfo.len = 0;
	fakeEvent.postEventInfo = &fakePostEventInfo;

	perfLineData.clear();
	perfLineData.prevEvent = &fakeEvent;

	ftraceLineData.clear();
	ftraceLineData.prevEvent = &fakeEvent;

	ftraceEvents->clear();
	perfEvents->clear();
	events = nullptr;
}

/*
 * This function is to be called after the parsing of all the events, it's
 * for fixing the postEventInfo pointer of the last event, because that info is
 * normally set when processing the next event in the
 * parse[Ftrace|Perf]Buffer() functions and for the last event there will of
 * course not be any next event.
 */
void TraceParser::fixLastEvent()
{
	bool prevLineIsEvent = false;
	int64_t infoBegin = 0;

	switch (traceType) {
	case TRACE_TYPE_FTRACE:
		prevLineIsEvent = ftraceLineData.prevLineIsEvent;
		infoBegin = ftraceLineData.infoBegin;
		break;
	case TRACE_TYPE_PERF:
		prevLineIsEvent = perfLineData.prevLineIsEvent;
		infoBegin = perfLineData.infoBegin;
		break;
	default:
		break;
	};

	/* Only perf traces will have backtraces after events, I think */
	if (traceType != TRACE_TYPE_PERF)
		return;
	/* If no events were found in the trace, then there is nothing to fix */
	if (events->size() <= 0)
		return;
	TraceEvent &lastEvent = events->last();
	if (prevLineIsEvent) {
		lastEvent.postEventInfo = nullptr;
	} else {
		Chunk *chunk = (Chunk*) postEventPool->
			allocObj();
		chunk->offset = infoBegin;
		chunk->len =  traceFile->getFileSize() - infoBegin;
		lastEvent.postEventInfo = chunk;
	}
}

#define CORR_DELTA vtl::Time(900000000)
#define TIME_10MS  vtl::Time(10000000)

bool TraceParser::parseLineBugFixup(TraceEvent* event,
				    const vtl::Time &prevTime)
{
	vtl::Time corrtime = event->time + CORR_DELTA;
	vtl::Time delta = corrtime - prevTime;
	bool retval = false;

	if (delta >= VTL_TIME_ZERO && delta < TIME_10MS) {
		event->time = corrtime;
		retval = true;
	}
	return retval;
}

void TraceParser::determineTraceType()
{
	if (ftraceLineData.nrEvents > (TSMAX(1, perfLineData.nrEvents)
				       * TRACE_TYPE_CONFIDENCE_FACTOR)) {
		traceType = TRACE_TYPE_FTRACE;
		TraceEvent::setStringTree(ftraceGrammar->eventTree);
		events = ftraceEvents;
		sendTraceType();
		return;
	} else if (perfLineData.nrEvents > (TSMAX(1, ftraceLineData.nrEvents)
					    * TRACE_TYPE_CONFIDENCE_FACTOR)) {
		traceType = TRACE_TYPE_PERF;
		TraceEvent::setStringTree(perfGrammar->eventTree);
		events = perfEvents;
		sendTraceType();
		return;
	}
	traceType = TRACE_TYPE_UNKNOWN;
}

void TraceParser::guessTraceType()
{
	if (perfLineData.nrEvents > ftraceLineData.nrEvents) {
		traceType = TRACE_TYPE_PERF;
		TraceEvent::setStringTree(perfGrammar->eventTree);
		events = perfEvents;
	} else if (perfLineData.nrEvents < ftraceLineData.nrEvents) {
		traceType = TRACE_TYPE_FTRACE;
		TraceEvent::setStringTree(ftraceGrammar->eventTree);
		events = ftraceEvents;
	} else {
		traceType = TRACE_TYPE_UNKNOWN;
		TraceEvent::setStringTree(perfGrammar->eventTree);
		events = perfEvents;
	}
	sendTraceType();
}

/* This parses a buffer regardless if it's perf or ftrace */
bool TraceParser::parseBuffer(unsigned int index)
{
	unsigned int i, s;
	bool eof;
	const TString **argv;

	ThreadBuffer<TraceLine> *tbuf = tbuffers[index];
	tbuf->beginConsumeBuffer();

	s = tbuf->list.size();
	argv = (const TString**)
		ptrPool->preallocN(EVENT_MAX_NR_ARGS);;

	for(i = 0; i < s; i++) {
		TraceLine &line = tbuf->list[i];
		TraceEvent &ft_event = ftraceEvents->preAlloc();
		ft_event.argc = 0;
		ft_event.argv = argv;
		if (parseLineFtrace(line, ft_event)) {
			argv = (const TString**)
				ptrPool->preallocN(EVENT_MAX_NR_ARGS);;
		}
		TraceEvent &p_event = perfEvents->preAlloc();
		p_event.argc = 0;
		p_event.argv = argv;
		if (parseLinePerf(line, p_event)) {
			argv = (const TString**)
				ptrPool->preallocN(EVENT_MAX_NR_ARGS);;
		}
	}
	eof = tbuf->loadBuffer->isEOF();
	tbuf->endConsumeBuffer();
	return eof;
}
