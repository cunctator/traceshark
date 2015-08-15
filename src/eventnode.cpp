/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "eventnode.h"
#include "mm/stringpool.h"
#include "traceevent.h"
#include "tstring.h"

EventNode::EventNode(const char *name)
	: GrammarNode(name)
{
	eventPool = new StringPool(8, 256);
}

EventNode::~EventNode()
{
	delete eventPool;
}

bool EventNode::match(TString *str, TraceEvent *event)
{
	char *lastChr = str->ptr + str->len - 1;
	TString *newstr;

	if (str->len < 1)
		return false;

	if (*lastChr == ':') {
		*lastChr = '\0';
		str->len--;
	}

	newstr = eventPool->allocString(str, StringHashFuncSimple32(str));
	if (newstr == NULL)
		return false;
	event->eventName = newstr;
	return true;
}
