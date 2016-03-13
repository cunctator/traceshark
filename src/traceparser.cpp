/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <climits>
#include <limits>

#include <QTextStream>

#include "tstring.h"
#include "genericparams.h"
#include "grammarroot.h"
#include "mm/mempool.h"
#include "ftracegrammar.h"
#include "perfgrammar.h"
#include "tracefile.h"
#include "traceparser.h"
#include "traceshark.h"
#include "threads/indexwatcher.h"
#include "threads/threadbuffer.h"

TraceParser::TraceParser(TList<TraceEvent> *analyzerEvents)
	: traceType(TRACE_TYPE_NONE)
{
	traceFile = NULL;
	ptrPool = new MemPool(16384, sizeof(TString*));
	postEventPool = new MemPool(16384, sizeof(TString));

	ftraceGrammar = new FtraceGrammar();
	perfGrammar = new PerfGrammar();

	tbuffers = new ThreadBuffer<TraceLine>*[NR_TBUFFERS];
	parserThread = new WorkThread<TraceParser>
		(this, &TraceParser::threadParser);
	readerThread = new WorkThread<TraceParser>
		(this, &TraceParser::threadReader);
	events = analyzerEvents;
	eventsWatcher = new IndexWatcher(10000);
	traceTypeWatcher = new IndexWatcher;
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

bool TraceParser::open(const QString &fileName)
{
	bool ok = false;
	unsigned int i;

	if (traceFile != NULL)
		return ok;

	traceFile = new TraceFile(fileName.toLocal8Bit().data(), ok,
				  1024 * 1024, NR_TBUFFERS);

	if (!ok) {
		delete traceFile;
		traceFile = NULL;
		return ok;
	}

	/* These buffers will be deleted by the parserThread */
	for (i = 0; i < NR_TBUFFERS; i++)
		tbuffers[i] = new ThreadBuffer<TraceLine>(TBUFSIZE,
							  NR_TBUFFERS);
	eventsWatcher->reset();
	traceTypeWatcher->reset();
	readerThread->start();
	parserThread->start();

	return true;
}

bool TraceParser::isOpen()
{
	return (traceFile != NULL);
}

void TraceParser::close()
{
	if (traceFile != NULL) {
		delete traceFile;
		traceFile = NULL;
	}
	ptrPool->reset();
	perfGrammar->clearPools();
	ftraceGrammar->clearPools();
	traceType = TRACE_TYPE_NONE;
}


void TraceParser::threadReader()
{
	unsigned long long nr = 0;
	unsigned int i = 0;
	unsigned int curbuf = 0;

	tbuffers[curbuf]->beginProduceBuffer();
	while(!traceFile->atEnd()) {
		TraceLine *line = &tbuffers[curbuf]->buffer[i];
		quint32 n = traceFile->ReadLine(line, curbuf);
		nr += n;
		i++;
		if (i == (TBUFSIZE - 1)) {
			tbuffers[curbuf]->endProduceBuffer(i);
			curbuf++;
			if (curbuf == NR_TBUFFERS)
				curbuf = 0;
			i = 0;
			tbuffers[curbuf]->beginProduceBuffer();
			traceFile->clearPool(curbuf);
		}
	}
	tbuffers[curbuf]->endProduceBuffer(i);
	/* We must send an empty buffer at the end to signal that we are EOF */
	if (i != 0) {
		curbuf++;
		if (curbuf == NR_TBUFFERS)
			curbuf = 0;
		tbuffers[curbuf]->beginProduceBuffer();
		tbuffers[curbuf]->endProduceBuffer(0);
	}

	QTextStream(stdout) << nr << "\n";
}


/* This function does prescanning as well, to determine number of events,
 * number of CPUs, max/min CPU frequency etc */
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
	/* Must have been a short trace or a lot of unknown garbage in the
	 * trace if we end up here */
	goto out;

	/* The purpose of jumping to these loops is to  be able to use the
	 * (hopefully faster) specialized parse functions */
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
	/* Make sure that the processing thread can continue even if no trace
	 * type was detected, otherwise it would wait forever in
	 * waitForTraceType() */
	sendTraceType();
	eventsWatcher->sendNextIndex(events->size());
	eventsWatcher->sendEOF();
	for (i = 0; i < NR_TBUFFERS; i++)
		delete tbuffers[i];

	fixLastEvent();
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
	infoBegin = traceFile->mappedFile;
	prevLineIsEvent = true;
	fakePostEventInfo.ptr = traceFile->mappedFile;
	fakeEvent.postEventInfo = &fakePostEventInfo;
	prevEvent = &fakeEvent;
	nrFtraceEvents = 0;
	nrPerfEvents = 0;
	events->clear();
	prevTime = std::numeric_limits<double>::lowest();
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
	/* Only perf traces will have backtraces after events, I think */
	if (traceType != TRACE_TYPE_PERF)
		return;
	TraceEvent &lastEvent = events->last();
	if (prevLineIsEvent) {
		lastEvent.postEventInfo = NULL;
	} else {
		TString *str = (TString*) postEventPool->
			allocObj();
		str->ptr = infoBegin;
		str->len = traceFile->mappedFile + traceFile->fileSize
			- infoBegin;
		lastEvent.postEventInfo = str;
	}
}

bool TraceParser::parseLineBugFixup(TraceEvent* event)
{
	double corrtime = event->time + 0.9;
	double delta = corrtime - prevTime;
	bool retval = false;

	if (delta >= (double)0 && delta < 0.00001) {
		event->time = corrtime;
		retval = true;
	}
	return retval;
}

void TraceParser::determineTraceType()
{
	if (nrFtraceEvents > 0 && nrPerfEvents == 0) {
		traceType = TRACE_TYPE_FTRACE;
		sendTraceType();
		return;
	} else if (nrFtraceEvents  == 0 && nrPerfEvents > 0) {
		traceType = TRACE_TYPE_PERF;
		sendTraceType();
		return;
	}
	traceType = TRACE_TYPE_NONE;
}

/* This parses a buffer regardless if it's perf or ftrace */
bool TraceParser::parseBuffer(unsigned int index)
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
		if (parseLineFtrace(line, event))
			continue;
		else {
			parseLinePerf(line, event);
		}
	}
	tbuf->endConsumeBuffer();
	return false;
}
