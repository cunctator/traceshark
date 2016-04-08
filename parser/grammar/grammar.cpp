/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014, 2015, 2016  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "parser/grammar/grammar.h"
#include "parser/grammar/grammarnode.h"
#include "parser/grammar/grammarroot.h"

Grammar::Grammar()
{
	root = new GrammarRoot("RootNode");
}

Grammar::~Grammar()
{
	deleteTree(root);
}

void Grammar::clearPools()
{
	__clearPools(root);
	resetReapedFlag(root);
}

void Grammar::__clearPools(GrammarNode *tree)
{
	unsigned int i;
	tree->reaped = true;
	tree->clearStringPool();
	for (i = 0; i < tree->nChildren; i++) {
		/* Clear subtree if it hasn't been visited */
		if (!tree->children[i]->reaped)
			__clearPools(tree->children[i]);
	}
}

void Grammar::resetReapedFlag(GrammarNode *tree)
{
	unsigned int i;
	tree->reaped = false;
	for (i = 0; i < tree->nChildren; i++) {
		/* Reset subtree if it hasn't been visited, N.B. since we are
		 * resetting, reaped flag will be false for visited nodes */
		if (tree->children[i]->reaped)
			resetReapedFlag(tree->children[i]);
	}
}

void Grammar::deleteTree(GrammarNode* node) {
	unsigned int i;
	node->reaped = true;
	for (i = 0; i < node->nChildren; i++) {
		/* Delete subtree if it has not been visited */
		if (!node->children[i]->reaped)
			deleteTree(node->children[i]);
	}
	delete node;
}
