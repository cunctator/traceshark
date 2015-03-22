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

#include "cpunode.h"
#include "traceevent.h"

CpuNode::CpuNode(const char *name)
	: GrammarNode(name) {};

bool CpuNode::match(char *str, TraceEvent *event)
{
	char *c;
	unsigned int cpu = 0;
	int digit;

	if (str[0] != '[')
		return false;

	cpu = 0;
	for (c = str + 1; *c != '\0' && *c != ']'; c++) {
		digit = *c - '0';
		if (digit > 9 || digit < 0)
			goto error;
		cpu *= 10;
		cpu += digit;
	}
	event->cpu = cpu;
	return true;
error:
	event->cpu = 0;
	return false;
}
