// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015-2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef VTL_COMPILER_H
#define VTL_COMPILER_H

#ifdef __has_cpp_attribute
#define TS_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define TS_HAS_CPP_ATTRIBUTE(x) 0
#endif

#if TS_HAS_CPP_ATTRIBUTE(fallthrough)
#define ts_fallthrough [[fallthrough]]
#elif TS_HAS_CPP_ATTRIBUTE(gnu::fallthrough)
#define ts_fallthrough [[gnu::fallthrough]]
#else
#define ts_fallthrough (void)0
#endif

#if defined(__GNUC__) || defined (__clang__)

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

/*
 * locality must be between [0, 3]
 *
 * 0 no temporal locality
 * 1 low temporal locality
 * 2 moderate degree of temporal locality
 * 3 high degree of temporal locality
 */
#define prefetch_read(addr, locality) \
	__builtin_prefetch(addr, 0, locality)
#define prefetch_write(addr, locality) \
	__builtin_prefetch(addr, 1, locality)
#define prefetch(addr) \
	__builtin_prefetch(addr)

#else /* __GNUC__ not defined */

#define likely(x)   (x)
#define unlikely(x) (x)
#define prefetch_read(addr, locality)
#define prefetch_write(addr, locality)
#define prefetch(addr)

#endif /* __GNUC__ */

#define vtl_str(a) __vtl_str(a)
#define __vtl_str(a) #a

#if defined(__clang__)
#define VTL_COMPILER "clang-" vtl_str(__clang_major__) "." \
	vtl_str(__clang_minor__) "." vtl_str(__clang_patchlevel__)
#elif defined(__GNUC__)
#define VTL_COMPILER "gcc-" vtl_str(__GNUC__) "." vtl_str(__GNUC_MINOR__) "." \
	vtl_str(__GNUC_PATCHLEVEL__)
#else
#define VTL_COMPILER "unknown compiler, how about updating " __FILE__
#endif

#endif /* VTL_COMPILER_H */
