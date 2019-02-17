// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "misc/errors.h"
#include <QFile>

/*
 * Do not change the order of these without updating the enum in
 * errors.h
 */

static const char noerror[] = "No error has occurred.";
static const char interne[] = "A serious internal error has occurred.";
static const char errerro[] = "An error in the error reporting has occurred.";
static const char errpars[] = "A parsing error has occurred.";
static const char errcpue[] = "Could not find cycles or cpu-cycles events.";
static const char errchan[] = "The tracefile has changed on disk.";
static const char errueof[] = "Unexpected end of file.";
static const char errffer[] = "Incorrect file format.";
static const char errnewf[] = \
	"The file format is from a newer unsupported version.";

/* The errors below are to accomodate the Qt QFileDevice::FileError codes */
static const char errfred[] = "An error occurred when reading from the file.";
static const char errfwri[] = "An error occurred when writing to the file.";
static const char errffat[] = "A fatal error occurred.";
static const char errfres[] = \
	"Out of resources (e.g., too many open files, out of memory, etc.)";
static const char errfope[] = "The file could not be opened.";
static const char errfabt[] = "The operation was aborted.";
static const char errftim[] = "A timeout occurred.";
static const char errfuns[] = "An unspecified error occurred.";
static const char errfrem[] = "The file could not be removed.";
static const char errfren[] = "The file could not be renamed.";
static const char errfpos[] = "The position in the file could not be changed.";
static const char errfrez[] = "The file could not be resized.";
static const char errfacc[] = "The file could not be accessed.";
static const char errfcop[] = "The file could not be copied.";

static const char *errorstrings[TS_NR_ERRORS] = {
	noerror,
	interne,
	errerro,
	errpars,
	errcpue,
	errchan,
	errueof,
	errffer,
	errnewf,
	errfred,
	errfwri,
	errffat,
	errfres,
	errfope,
	errfabt,
	errftim,
	errfuns,
	errfrem,
	errfren,
	errfpos,
	errfrez,
	errfacc,
	errfcop
};

const char *ts_strerror(int ts_errno)
{
	if (ts_errno >= TS_NR_ERRORS || ts_errno < 0)
		ts_errno = TS_ERROR_ERROR;
	return errorstrings[ts_errno];
}

static const char parsingerr[] = "parsing error";

/*
 * All places were we wish to have a null string that doesn't crash the program
 * like nullptr does, are essentially parsing problems.
 */
const char *const NullStr = &parsingerr[0];
