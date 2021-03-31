// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2014-2019, 2021  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef MISC_TYPES_H_
#define MISC_TYPES_H_

typedef uint32_t taskstate_t;

/*
 * This is the maximum length of strings in TRACEEVENT_DEFS_. If a new string is
 * added and it is longer than the value below, then it must be increased.
 */
#define EVENTSTRINGS_MAXLEN 20

#define TRACEEVENTS_DEFS_						\
	TSHARK_ITEM_(CPU_FREQUENCY = 0,	"cpu_frequency"),		\
	TSHARK_ITEM_(CPU_IDLE,		"cpu_idle"),			\
	TSHARK_ITEM_(SCHED_MIGRATE_TASK,"sched_migrate_task"),		\
	TSHARK_ITEM_(SCHED_SWITCH,	"sched_switch"),		\
	TSHARK_ITEM_(SCHED_WAKEUP,	"sched_wakeup"),		\
	TSHARK_ITEM_(SCHED_WAKEUP_NEW,	"sched_wakeup_new"),		\
	TSHARK_ITEM_(SCHED_WAKING,	"sched_waking"),		\
	TSHARK_ITEM_(SCHED_PROCESS_FORK,"sched_process_fork"),		\
	TSHARK_ITEM_(SCHED_PROCESS_EXIT,"sched_process_exit"),		\
	TSHARK_ITEM_(IRQ_HANDLER_ENTRY,	"irq_handler_entry"),		\
	TSHARK_ITEM_(IRQ_HANDLER_EXIT,	"irq_handler_exit"),		\
	TSHARK_ITEM_(NR_EVENTS,		nullptr)

#undef TSHARK_ITEM_
#define TSHARK_ITEM_(A, B) A
typedef enum : int {
	TRACEEVENTS_DEFS_
} event_t;
#undef TSHARK_ITEM_

#define EVENT_ERROR ((event_t)-1)

#endif /* MISC_TYPES_H_  */
