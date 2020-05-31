// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
 *
 * This file is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 *
 *  a) This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License as
 *     published by the Free Software Foundation; either version 2 of the
 *     License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public
 *     License along with this library; if not, write to the Free
 *     Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 *     MA 02110-1301 USA
 *
 * Alternatively,
 *
 *  b) Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *     1. Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *     2. Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 *     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *     CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *     INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *     MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PERFHELPERS_H
#define PERFHELPERS_H

#include "misc/string.h"
#include "parser/paramhelpers.h"
#include "vtl/compiler.h"

/*
 * The functions in this file are not meant to be used directly. The are only
 *  helper functions meant to be used by the functions in perfparams.h.
 */

/*
 * This function finds the index of the '==>' in the switch event arguments
 * that has this format:
 *
 * prev_pid=... prev_prio=... prev_state=... ==> next_comm=...
 *
 * This is quite paranoid but I don't see a choice. Basically, it would be
 * sufficient to check for the '==>' but then if some weird person would be
 * running a task that has a name like 'x x x ==>' then the parsing could get
 * fooled, checking that the tokens before and after the '==>' have the correct
 * prefixes protects us against such idiotic cases. I believe that the maximum
 * taskname length in the kernel (16 characters?)  will protect us from from
 * somebody creating the truly lunatic taskname that would be able to fool
 * this parsing, i.e. a taskname such as :
 *
 * "prev_pid= prev_prio= prev_state= ==> next_comm="
 *
 * In a nutshell, this protects us against weirdos but not against lunatics :)
 */
static vtl_always_inline int
perf_sched_switch_find_arrow_(const TraceEvent &event, bool &is_distro_style)
{
	int i;
	for (i = 2; i < event.argc - 2; i++) {
		const TString *arrow = event.argv[i];
		if (!isArrowStr(arrow))
			continue;
		const char *c1 = event.argv[i - 2]->ptr;
		const char *c2 = event.argv[i - 1]->ptr;
		const char *c3 = event.argv[i + 1]->ptr;
		/* Check if it is regular mainline format */
		if (!prefixcmp(c1, SWITCH_PREV_PFIX) &&
		    !prefixcmp(c2, SWITCH_PREV_PFIX) &&
		    !prefixcmp(c3, SWITCH_NEXT_PFIX)) {
			is_distro_style = false;
			break;
		} else {
			/*
			 * Check if it is distro format. We do this by
			 * checking that the priority fields have their
			 * [] braces
			 */
			const TString *t1 = event.argv[i - 2];
			const TString *t2 = event.argv[event.argc - 1];
			if (is_param_inside_braces(t1) &&
			    is_param_inside_braces(t2)) {
				is_distro_style = true;
				break;
			}
		}
		/*
		 * If we reach this point, there are two possibilities:
		 * - Some weirdo has a ' ==> ' inside a task name
		 * - Unknown format
		 *
		 * However, we do not give up as a subsequent iteration may
		 * find the correct '==>'
		 */
	}
	if (!(i < event.argc - 2))
		return 0;
	return i;
}

static vtl_always_inline int
perf_sched_switch_handle_oldpid_newformat_(const TraceEvent &event,
					   const sched_switch_handle &handle)
{
	int idx = handle.perf.index;
	int i;

	/* Normat case */
	if (idx >= 3) {
		/*
		 * In the normal case of the of a known format, the
		 * "prev_pid=" prefix should be found at position idx - 3
		 * but we will anyway scan also idx - 2 and idx - 1. The idx
		 * event should point to the "==>" string.
		 */
		for (i = idx - 3; i < idx; i++) {
			if (!prefixcmp(event.argv[i]->ptr, SWITCH_PPID_PFIX))
				return int_after_char(event, i, '=');
		}
	}
	/*
	 * Probably/hopefully we will never get here. If we do, then we have
	 * some unknown sched_switch argument format.
	 */
	for (i = 0; i < idx - 3; i++) {
		if (prefixcmp(event.argv[i]->ptr, SWITCH_PPID_PFIX) != 0)
			continue;
		/*
		 * We require that either the previous or next string start
		 * with "prev_". This is to guard against people with task
		 * names that contain "prev_pid="
		 */
		if ((i > 0 && prefixcmp(event.argv[i - 1]->ptr,
					SWITCH_PREV_PFIX)) ||
		    (i < (idx - 2) && prefixcmp(event.argv[i + 1]->ptr,
						SWITCH_PREV_PFIX)))
			return int_after_char(event, i, '=');
	}
	return ABSURD_INT;
}

static vtl_always_inline int
perf_sched_switch_handle_newpid_newformat_(const TraceEvent &event,
					   const sched_switch_handle &handle)
{
	int i;

	/* Normat case */
	if (event.argc > 2) {
		/*
		 * In the normal case of the of a known format, the
		 * "next_pid=" prefix should be found at position event.argc - 2
		 * but we will anyway scan also event.argc - 1.
		 */
		for (i = event.argc - 2; i < event.argc; i++) {
			if (!prefixcmp(event.argv[i]->ptr, SWITCH_NPID_PFIX))
				return int_after_char(event, i, '=');
		}
	}
	int idx = handle.perf.index;
	for (i = idx + 1; i < event.argc - 2; i++) {
		if (prefixcmp(event.argv[i]->ptr, SWITCH_NPID_PFIX) != 0)
			continue;
		/*
		 * We require that either the previous or next string start
		 * with "next_". This is to guard against people with task
		 * names that contain "next_pid="
		 */
		if ((i > (idx + 1) && prefixcmp(event.argv[i - 1]->ptr,
						SWITCH_NEXT_PFIX)) ||
		    (i < (event.argc - 1) && prefixcmp(event.argv[i + 1]->ptr,
						       SWITCH_NEXT_PFIX)))
			return int_after_char(event, i, '=');
	}
	return ABSURD_INT;
}

#endif /* PERFHELPERS_H */
