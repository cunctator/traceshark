// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2022  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef SVGRESOURCES_H
#define SVGRESOURCES_H

/* All pixmap resources for icons should be defined here  */
#define RESSRC_GPH_ADD_TASK      ":/traceshark/images/addtask.svg"
#define RESSRC_GPH_ADD_TO_LEGEND ":/traceshark/images/addtolegend.svg"
#define RESSRC_GPH_ARGFILTER     ":/traceshark/images/argfilter.svg"
#define RESSRC_GPH_CLEAR_LEGEND  ":/traceshark/images/clearlegend.svg"
#define RESSRC_GPH_CLEAR_TASK    ":/traceshark/images/cleartasksgraphs.svg"
#define RESSRC_GPH_CLOSE         ":/traceshark/images/close.svg"
#define RESSRC_GPH_CPUFILTER     ":/traceshark/images/cpufilter.svg"
#define RESSRC_GPH_CURSOR_ZOOM   ":/traceshark/images/cursorzoom.svg"
#define RESSRC_GPH_DEFAULT_ZOOM  ":/traceshark/images/defaultzoom.svg"
#define RESSRC_GPH_EVENTBTRACE   ":/traceshark/images/eventbacktrace.svg"
#define RESSRC_GPH_EVENTFILTER   ":/traceshark/images/eventfilter.svg"
#define RESSRC_GPH_EVENTFLTCPU   ":/traceshark/images/eventfiltercpu.svg"
#define RESSRC_GPH_EVENTFLTNAME  ":/traceshark/images/eventfiltername.svg"
#define RESSRC_GPH_EVENTFLTPID   ":/traceshark/images/eventfilterpid.svg"
#define RESSRC_GPH_EVENTFLTTYPE  ":/traceshark/images/eventfiltertype.svg"
#define RESSRC_GPH_EVENTMOVEBLUE ":/traceshark/images/eventmoveblue.svg"
#define RESSRC_GPH_EVENTMOVERED  ":/traceshark/images/eventmovered.svg"
#define RESSRC_GPH_EXPORTCPUEVENTS  \
	                         ":/traceshark/images/exportcpuevents.svg"
#define RESSRC_GPH_EXPORTEVENTS  ":/traceshark/images/exportevents.svg"
#define RESSRC_GPH_FILTERCURRENT ":/traceshark/images/filtercurrent.svg"
#define RESSRC_GPH_FILTERCURRENT_LIMIT \
	                         ":/traceshark/images/filtercurrentlimited.svg"
#define RESSRC_GPH_FULL_ZOOM     ":/traceshark/images/fullzoom.svg"
#define RESSRC_GPH_GETSTATS      ":/traceshark/images/getstats.svg"
#define RESSRC_GPH_GETSTATS_TIMELIMIT \
	                         ":/traceshark/images/getstatstimelimit.svg"
#define RESSRC_GPH_GRAPHENABLE   ":/traceshark/images/graphenabledialog.svg"
#define RESSRC_GPH_LATENCY       ":/traceshark/images/latency.svg"
#define RESSRC_GPH_WAKEUP_LATENCY \
                                 ":/traceshark/images/latency-wakeup30x30.png"
#define RESSRC_GPH_MOVE_BLUE     ":/traceshark/images/moveblue.svg"
#define RESSRC_GPH_MOVE_RED      ":/traceshark/images/movered.svg"
#define RESSRC_GPH_OPEN          ":/traceshark/images/open.svg"
#define RESSRC_GPH_REMOVE_TASK   ":/traceshark/images/removetask.svg"
#define RESSRC_GPH_RESETFILTERS  ":/traceshark/images/resetfilters.svg"
#define RESSRC_GPH_SCREENSHOT    ":/traceshark/images/screenshot.svg"
#define RESSRC_GPH_SHARK         ":/traceshark/images/shark.png"
#define RESSRC_GPH_TASKSELECT    ":/traceshark/images/taskselector.svg"
#define RESSRC_GPH_TIMEFILTER    ":/traceshark/images/timefilter.svg"
#define RESSRC_GPH_FIND_SLEEP    ":/traceshark/images/findsleep.svg"
#define RESSRC_GPH_VERTICAL_ZOOM ":/traceshark/images/verticalzoom.svg"
#define RESSRC_GPH_FIND_WAKEUP   ":/traceshark/images/wakeup.svg"
#define RESSRC_GPH_FIND_WAKING   ":/traceshark/images/waking.svg"
#define RESSRC_GPH_FIND_WAKING_DIRECT \
                                 ":/traceshark/images/wakingdirect.svg"
#define RESSRC_GPH_QCP_LOGO      ":/traceshark/images/qcp-logo.png"
#define RESSRC_GPH_QT_LOGO       ":/traceshark/images/qtlogo-64.png"

#endif /* SVGRESOURCES_H */
