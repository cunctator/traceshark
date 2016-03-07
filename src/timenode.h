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

#ifndef TIMENODE_H
#define TIMENODE_H

#include "tstring.h"
#include "grammarnode.h"

class StringPool;

class TimeNode: public GrammarNode
{
public:
	TimeNode(const char *name);
	~TimeNode();
	bool match(TString *str, TraceEvent *event);
	void clearStringPool() {};
private:
	__always_inline bool extractNameAndPid(TString &dstr,
					       unsigned int &pid,
					       TString &compound,
					       unsigned int maxlen);
	StringPool *namePool;
};

__always_inline bool TimeNode::extractNameAndPid(TString &dstr,
						 unsigned int &pid,
						 TString &compound,
						 unsigned int maxlen)
{
	char *nullChr = compound.ptr + compound.len;
	char *lastChr = nullChr - 1;
	char *c;
	char *beginPid;
	int digit;

	if (compound.len < 2)
		return false;

	for (c = lastChr - 1; c >= compound.ptr; c--) {
		if (*c == '-')
			goto found1;
	}
	return false;
found1:
	/* Cut the compound string so that only the name remains */
	compound.len = compound.len - (nullChr - c);
	*c = '\0';
	beginPid = c + 1;

	/* Use the remainder, that was cutoff from the name, as separated by the
	 * '-' sign get the pid */
	pid = 0;
	for (c = beginPid; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return false;
	}

	dstr.merge(&compound, maxlen);
	return true;
}

#endif
