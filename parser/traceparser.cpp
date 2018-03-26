/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <climits>
#include <cstring>
#include <limits>

#include "misc/tstring.h"
#include "parser/genericparams.h"
#include "mm/mempool.h"
#include "parser/ftrace/ftracegrammar.h"
#include "parser/perf/perfgrammar.h"
#include "parser/tracefile.h"
#include "parser/traceparser.h"
#include "misc/errors.h"
#include "misc/traceshark.h"
#include "threads/indexwatcher.h"
#include "threads/threadbuffer.h"

#define CLEAR_VARIABLE(VAR) memset(&VAR, 0, sizeof(VAR))

TraceParser::TraceParser(vtl::TList<TraceEvent> *analyzerEvents)
	: traceType(TRACE_TYPE_NONE)
{
	traceFile = nullptr;
	ptrPool = new MemPool(16384, sizeof(TString*));
	postEventPool = new MemPool(16384, sizeof(TString));

	ftraceGrammar = new FtraceGrammar();
	perfGrammar = new PerfGrammar();

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<TraceParser>
		(QString("parserThread"), this, &TraceParser::threadParser);
	readerThread = new WorkThread<TraceParser>
		(QString("readerThread"), this, &TraceParser::threadReader);
	events = analyzerEvents;
	eventsWatcher = new IndexWatcher(10000);
	traceTypeWatcher = new IndexWatcher;

	CLEAR_VARIABLE(fakeEvent);
	CLEAR_VARIABLE(fakePostEventInfo);
	CLEAR_VARIABLE(ftraceLineData);
	CLEAR_VARIABLE(perfLineData);
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
		tbuffers[i] = new ThreadBuffer<TraceLine>(TBUFSIZE);
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

void TraceParser::close()
{
	if (traceFile != nullptr) {
		delete traceFile;
		traceFile = nullptr;
	}
	ptrPool->reset();
	perfGrammar->clear();
	ftraceGrammar->clear();
	traceType = TRACE_TYPE_NONE;
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
	 * trace if we end up here
	 */
	goto out;

	/*
	 * The purpose of jumping to these loops is to  be able to use the
	 * (hopefully faster) specialized parse functions
	 */
ftrace:
	while(true) {
		if (parseFtraceBuffer(i))
			break;
		eventsWatcher->sendNextIndex(events->size());
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
	goto out;

perf:
	while(true) {
		if (parsePerfBuffer(i))
			break;
		eventsWatcher->sendNextIndex(events->size());
		i++;
		if (i == NR_TBUFFERS)
			i = 0;
	}
out:
	/*
	 * Make sure that the processing thread can continue even if no trace
	 * type was detected, otherwise it would wait forever in
	 * waitForTraceType()
	 */
	sendTraceType();

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
	fakePostEventInfo.ptr = traceFile->mappedFile;
	fakeEvent.postEventInfo = &fakePostEventInfo;

	perfLineData.infoBegin = traceFile->mappedFile;
	perfLineData.prevEvent = &fakeEvent;
	perfLineData.nrEvents = 0;
	perfLineData.prevLineIsEvent = true;
	perfLineData.prevTime = VTL_TIME_MIN;

	ftraceLineData.infoBegin = traceFile->mappedFile;
	ftraceLineData.prevEvent = &fakeEvent;
	ftraceLineData.nrEvents = 0;
	ftraceLineData.prevLineIsEvent = true;
	ftraceLineData.prevTime = VTL_TIME_MIN;

	events->clear();
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
	char *infoBegin = nullptr;

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
	TraceEvent &lastEvent = events->last();
	if (prevLineIsEvent) {
		lastEvent.postEventInfo = nullptr;
	} else {
		TString *str = (TString*) postEventPool->
			allocObj();
		str->ptr = infoBegin;
		str->len = traceFile->mappedFile + traceFile->fileSize
			- infoBegin;
		lastEvent.postEventInfo = str;
	}
}

#define CORR_DELTA vtl::Time(false, 0, 900000000)
#define TIME_10MS  vtl::Time(false, 0, 10000000)

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
	if (ftraceLineData.nrEvents > 0 && perfLineData.nrEvents == 0) {
		traceType = TRACE_TYPE_FTRACE;
		TraceEvent::setStringTree(ftraceGrammar->eventTree);
		sendTraceType();
		return;
	} else if (ftraceLineData.nrEvents  == 0 && perfLineData.nrEvents > 0) {
		traceType = TRACE_TYPE_PERF;
		TraceEvent::setStringTree(perfGrammar->eventTree);
		sendTraceType();
		return;
	}
	traceType = TRACE_TYPE_NONE;
}

/* This parses a buffer regardless if it's perf or ftrace */
bool TraceParser::parseBuffer(unsigned int index)
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
		if (parseLineFtrace(line, event))
			continue;
		else {
			event.argc = 0;
			parseLinePerf(line, event);
		}
	}
	eof = tbuf->loadBuffer->isEOF();
	tbuf->endConsumeBuffer();
	return eof;
}
