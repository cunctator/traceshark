// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef MISC_OSAPI_H
#define MISC_OSAPI_H

#include <cstring>

extern "C" {
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
}

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

/*
 * bzero() was removed from IEEE Std 1003.1-2008 (``POSIX.1'') and some
 * implementations remove bzero() if we have defined _POSIX_C_SOURCE=200809L
 */
#define tshark_bzero(ADDR, SIZE) ((void)memset(ADDR, 0, SIZE))

#if defined(__APPLE__) && TARGET_OS_MAC

#define lseek64(FD, OFFSET, WHENCE) lseek(FD, OFFSET, WHENCE)

/* These are for comparing mtime and ctime in a portable way */
#define cmp_ctimespec(s1, s2) TShark::cmp_timespec(s1.st_ctimespec,	\
						   s2.st_ctimespec)
#define cmp_mtimespec(s1, s2) TShark::cmp_timespec(s1.st_mtimespec,	\
						   s2.st_mtimespec)

#define tshark_pthread_setname_np(NAME) pthread_setname_np(NAME)

#elif __linux__

/* These are the Linux versions, note the difference in members names */
#define cmp_ctimespec(s1, s2) TShark::cmp_timespec(s1.st_ctim, s2.st_ctim)
#define cmp_mtimespec(s1, s2) TShark::cmp_timespec(s1.st_mtim, s2.st_mtim)

#define tshark_pthread_setname_np(NAME) pthread_setname_np(pthread_self(), \
							   NAME)

#elif __unix__

/*
 * For now what is here in __unix__ is just copies of whatever is in the mac
 * section but that's just because at this point I have not tried with the
 * other unices, so this section is kind of a placeholder. I assume that many
 * would resemble macOS more than Linux.
 */

#define lseek64(FD, OFFSET, WHENCE) lseek(FD, OFFSET, WHENCE)

#define cmp_ctimespec(s1, s2) TShark::cmp_timespec(s1.st_ctimespec,	\
						   s2.st_ctimespec)
#define cmp_mtimespec(s1, s2) TShark::cmp_timespec(s1.st_mtimespec,	\
						   s2.st_mtimespec)

#define tshark_pthread_setname_np(NAME) pthread_setname_np(NAME)

#else /* __unix__ */
#error "Unknown Operating system"
#endif

#endif /* MISC_OSAPI_H */
