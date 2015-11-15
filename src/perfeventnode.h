/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
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

#ifndef PERFEVENTNODE_H
#define PERFEVENTNODE_H

#include "grammarnode.h"

class StringTree;

class PerfEventNode: public GrammarNode
{
public:
	PerfEventNode(const char *name);
	~PerfEventNode();
	bool match(TString *str, TraceEvent *event);
	void clearStringPool();
private:
	StringTree *eventTree;
	void setupTree();
};

#endif /* PERFEVENTNODE_H */
