/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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
 *     DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *     CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *     SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *     NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *     LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *     HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *     OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *     EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>

extern "C" {
#include <err.h>
}
#include <cstdio>
#include <cstring>
#include "vtl/error.h"

static vtl::ErrorHandler *handler;
static const char *(*strerror_func)(int errnum);

void vtl::set_strerror(const char *(*func)(int errnum)) {
	strerror_func = func;
}

void vtl::set_error_handler(vtl::ErrorHandler *eh)
{
	handler = eh;
}

static void __vwarnx(const char *fmt, va_list args)
{
	if (handler != nullptr)
		handler->ErrorX(fmt, args);
	else
		vwarnx(fmt, args);
}

static void __vwarn(int vtl_errno, const char *fmt, va_list args)
{
	if (handler != nullptr)
		handler->Error(vtl_errno, fmt, args);
	else {
		vwarn(fmt, args);
		if (vtl_errno > 0) {
			const char *msg = strerror(vtl_errno);
			fprintf(stderr, "%s", msg);
		} else if (vtl_errno < 0) {
			const char *msg = strerror_func(-vtl_errno);
			fprintf(stderr, "%s", msg);
		} else {
			/* vtl_errno = 0, do nothing */
		}
	}
}

void vtl::errx(int ecode, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	__vwarnx(fmt, args);
	va_end(args);
	exit(ecode);
}

void vtl::warnx(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	__vwarnx(fmt, args);
	va_end(args);
}

void vtl::err(int ecode, int vtl_errno, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	__vwarn(vtl_errno, fmt, args);
	va_end(args);
	exit(ecode);
}


void vtl::warn(int vtl_errno, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	__vwarn(vtl_errno, fmt, args);
	va_end(args);
}

