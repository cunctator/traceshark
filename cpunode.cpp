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
#include <cstring>

CpuNode::CpuNode(const char *name)
	: GrammarNode(name) {};

bool CpuNode::match(char *str, TraceEvent *event)
{
	int len = strlen(str);
	char *lastChr = str + len - 1;
	char *c;
	unsigned int cpu = 0;
	int digit;

	if (*lastChr != ']' || str[0] != '[')
		return false;

	cpu = 0;
	for (c = str + 1; c < lastChr; c++) {
		cpu *= 10;
		digit = *c - '0';
		if (digit <= 9 && digit >= 0)
			cpu += digit;
		else
			return false;
	}

	event->cpu = cpu;
	return true;
}
