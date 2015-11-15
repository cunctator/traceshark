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

#include "namenode.h"
#include "mm/stringpool.h"
#include "traceevent.h"
#include "tstring.h"

NameNode::NameNode(const char *name)
	: GrammarNode(name)
{
	namePool = new StringPool(1024, 65536);
}

NameNode::~NameNode()
{
	delete namePool;
}

bool NameNode::match(TString *str, TraceEvent *event)
{
	TString *newstr;

	/* This is the "magic" that saves a ton of string allocations
	 * with exactly the same string */
	newstr = namePool->allocString(str, TShark::StrHash32(str), 65536);
	if (newstr == NULL)
		return false;
	event->taskName = newstr;
	return true;
}

void NameNode::clearStringPool()
{
	namePool->clear();
}
