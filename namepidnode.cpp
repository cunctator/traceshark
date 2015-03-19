/*
 * Traceshark - a visualizer for visualizing ftrace traces
 * Copyright (C) 2015  Viktor Rosendahl
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

#include <cstring>

#include "namepidnode.h"
#include "traceevent.h"

NamePidNode::NamePidNode(const char *name)
	: GrammarNode(name) {};

bool NamePidNode::match(char *str, TraceEvent *event)
{
	int len = strlen(str);
	char *lastChr = str + len - 1;
	char *c;
	char *beginPid;
	int pid;
	int digit;

	if (len < 3)
		return false;

	for (c = lastChr - 1; c >= str; c--) {
		if (*c == '-')
			goto found1;
	}
	return false;
found1:
	*c = '\0';
	beginPid = c + 1;
	
	pid = 0;
	for (c = beginPid; c < lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return false;
	}

	event->taskName = str;
	event->pid = pid;
	return true;
}
