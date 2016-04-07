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

#include "parser/argnode.h"
#include "mm/stringpool.h"
#include "parser/traceevent.h"

ArgNode::ArgNode(const char *name) 
	: GrammarNode(name)
{
	argPool = new StringPool(2048, 1024 * 1024);
}

ArgNode::~ArgNode()
{
	delete argPool;
}

bool ArgNode::match(TString *str, TraceEvent *event)
{
	TString *newstr;
	if (event->argc < 255) {
		newstr = argPool->allocString(str, TShark::StrHash32(str), 16);
		if (newstr == nullptr)
			return false;
		event->argv[event->argc] = newstr;
		event->argc++;
		return true;
	}
	return false;
}

void ArgNode::clearStringPool()
{
	argPool->clear();
}
