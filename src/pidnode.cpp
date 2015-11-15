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

#include "pidnode.h"
#include "traceevent.h"
#include "tstring.h"

PidNode::PidNode(const char *name)
	: GrammarNode(name)
{}

bool PidNode::match(TString *str, TraceEvent *event)
{
	char *lastChr = str->ptr + str->len - 1;
	int pid;
	int digit;
	char *c;

	if (str->len < 1 || str->len > 10)
		return false;

	pid = 0;
	for (c = str->ptr; c <= lastChr; c++) {
		pid *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			pid += digit;
		else
			return false;
	}

	event->pid = pid;
	return true;
}
