// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2016, 2017, 2019-2021
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

#include "parser/traceevent.h"
#include "misc/types.h"
#include "mm/stringtree.h"

/* TSHARK_ITEM_ is used by the TRACEEVENT_DEFS_ macro */
#undef TSHARK_ITEM_
#define TSHARK_ITEM_(A, B) B
/*
 * The maximum length of these strings should be in the macro
 * EVENTSTRINGS_MAXLEN, which is defined misc/types.h
 */
const char * const eventstrings[] = {
	TRACEEVENTS_DEFS_
};
#undef TSHARK_ITEM_

StringTree<> *TraceEvent::stringTree = nullptr;

void TraceEvent::setStringTree(StringTree<> *sTree)
{
	stringTree = sTree;
}

const StringTree<> *TraceEvent::getStringTree()
{
	return stringTree;
}

const TString *TraceEvent::getEventName() const
{
	return stringTree->stringLookup(TraceEvent::type);
}

void TraceEvent::clear()
{
	taskName = nullptr;
	pid = 0;
	cpu = 0;
	time = VTL_TIME_ZERO;
	intArg = 0;
	type = EVENT_ERROR;
	argv = nullptr;
	argc = 0;
	postEventInfo = nullptr;
}

const TString *TraceEvent::getEventName(event_t event)
{
	return stringTree->stringLookup(event);
}

int TraceEvent::getNrEvents()
{
	return stringTree->getMaxEvent() + 1;
}
