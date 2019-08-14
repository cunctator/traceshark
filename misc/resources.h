// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2016-2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef RESOURCES_H
#define RESOURCES_H

/* All pixmap resources for icons should be defined here  */
#define RESSRC_PNG_ADD_TASK      ":/traceshark/images/addtask30x30.png"
#define RESSRC_PNG_ADD_TO_LEGEND ":/traceshark/images/addtolegend30x30.png"
#define RESSRC_PNG_CLEAR_LEGEND  ":/traceshark/images/clearlegend30x30.png"
#define RESSRC_PNG_CLEAR_TASK    ":/traceshark/images/cleartasksgraphs30x30.png"
#define RESSRC_PNG_CLOSE         ":/traceshark/images/close30x30.png"
#define RESSRC_PNG_CPUFILTER     ":/traceshark/images/cpufilter30x30.png"
#define RESSRC_PNG_CURSOR_ZOOM   ":/traceshark/images/cursorzoom30x30.png"
#define RESSRC_PNG_DEFAULT_ZOOM  ":/traceshark/images/defaultzoom30x30.png"
#define RESSRC_PNG_EVENTFILTER   ":/traceshark/images/eventfilter30x30.png"
#define RESSRC_PNG_EXPORTCPUEVENTS  \
	                         ":/traceshark/images/exportcpuevents30x30.png"
#define RESSRC_PNG_EXPORTEVENTS  ":/traceshark/images/exportevents30x30.png"
#define RESSRC_PNG_FILTERCURRENT ":/traceshark/images/filtercurrent30x30.png"
#define RESSRC_PNG_FILTERCURRENT_LIMIT \
	                         ":/traceshark/images/filtercurrentlimited30x30.png"
#define RESSRC_PNG_GETSTATS      ":/traceshark/images/getstats30x30.png"
#define RESSRC_PNG_GETSTATS_TIMELIMIT \
	                         ":/traceshark/images/getstatstimelimit30x30.png"
#define RESSRC_PNG_GRAPHENABLE   ":/traceshark/images/graphenabledialog30x30.png"
#define RESSRC_PNG_MOVE_BLUE     ":/traceshark/images/moveblue30x30.png"
#define RESSRC_PNG_MOVE_RED      ":/traceshark/images/movered30x30.png"
#define RESSRC_PNG_OPEN          ":/traceshark/images/open30x30.png"
#define RESSRC_PNG_REMOVE_TASK   ":/traceshark/images/removetask30x30.png"
#define RESSRC_PNG_RESETFILTERS  ":/traceshark/images/resetfilters30x30.png"
#define RESSRC_PNG_SCREENSHOT    ":/traceshark/images/screenshot30x30.png"
#define RESSRC_PNG_SHARK         ":/traceshark/images/shark.png"
#define RESSRC_PNG_TASKSELECT    ":/traceshark/images/taskselector30x30.png"
#define RESSRC_PNG_TIMEFILTER    ":/traceshark/images/timefilter30x30.png"
#define RESSRC_PNG_FIND_SLEEP    ":/traceshark/images/findsleep30x30.png"
#define RESSRC_PNG_FIND_WAKEUP   ":/traceshark/images/wakeup30x30.png"
#define RESSRC_PNG_FIND_WAKING   ":/traceshark/images/waking30x30.png"
#define RESSRC_PNG_FIND_WAKING_DIRECT \
				 ":/traceshark/images/wakingdirect30x30.png"
#define RESSRC_PNG_QCP_LOGO      ":/traceshark/images/qcp-logo.png"
#define RESSRC_PNG_QT_LOGO       ":/traceshark/images/qtlogo-64.png"

#define RESSRC_TEXT_LICENSE      ":/traceshark/LICENSE"

#endif /* RESOURCES_H */
