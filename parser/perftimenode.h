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

#ifndef PERFTIMENODE_H
#define PERFTIMENODE_H

#include "misc/tstring.h"
#include "parser/grammarnode.h"

class StringPool;

class PerfTimeNode: public GrammarNode
{
public:
	PerfTimeNode(const char *name);
	~PerfTimeNode();
	bool match(TString *str, TraceEvent *event);
	void clearStringPool() {};
private:
	__always_inline int pidFromString(const TString &str);
	StringPool *namePool;
};

__always_inline int PerfTimeNode::pidFromString(const TString &str)
{
	char *lastChr = str.ptr + str.len - 1;
	int pid;
	int digit;
	char *c;

	if (str.len < 1 || str.len > 10)
		return false;

	pid = 0;
	for (c = str.ptr; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return -1;
	}
	return pid;
}

#endif
