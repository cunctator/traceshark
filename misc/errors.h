// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018, 2019, 2021, 2022
 * Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef ERRORS_H
#define ERRORS_H

#define TSHARK_ERROR_DEFS_						\
	TSHARK_ITEM_(TS_ERROR_NOERROR = 0,				\
		"No error has occurred."),				\
	TSHARK_ITEM_(TS_ERROR_INTERNAL,					\
		"A serious internal error has occurred."),		\
	TSHARK_ITEM_(TS_ERROR_ERROR,					\
		"An error in the error reporting has occurred."),	\
	TSHARK_ITEM_(TS_ERROR_PARSER,					\
		"A parsing error has occurred."),			\
	TSHARK_ITEM_(TS_ERROR_NOCPUEV,					\
		"Could not find cycles or cpu-cycles events."),		\
	TSHARK_ITEM_(TS_ERROR_FILECHANGED,				\
		"The tracefile has changed on disk."),			\
	TSHARK_ITEM_(TS_ERROR_EOF,					\
		"Unexpected end of file."),				\
	TSHARK_ITEM_(TS_ERROR_FILEFORMAT,				\
		"Incorrect file format."),				\
	TSHARK_ITEM_(TS_ERROR_NEWFORMAT,				\
		"The file format is from a newer unsupported version."),\
/* The errors below are to accomodate the Qt QFileDevice::FileError codes */ \
	TSHARK_ITEM_(TS_ERROR_FILE_READ,				\
		"An error occurred when reading from the file."),	\
	TSHARK_ITEM_(TS_ERROR_FILE_WRITE,				\
		"An error occurred when writing to the file."),		\
	TSHARK_ITEM_(TS_ERROR_FATALERROR,				\
		"A fatal error occurred."),				\
	TSHARK_ITEM_(TS_ERROR_FILE_RESOURCE,				\
	"Out of resources (e.g., too many open files, out of memory, etc.)"),\
	TSHARK_ITEM_(TS_ERROR_OPEN,					\
		"The file could not be opened."),			\
	TSHARK_ITEM_(TS_ERROR_ABORT,					\
		"The operation was aborted."),				\
	TSHARK_ITEM_(TS_ERROR_TIMEOUT,					\
		"A timeout occurred."),					\
	TSHARK_ITEM_(TS_ERROR_UNSPEC,					\
		"An unspecified error occurred."),			\
	TSHARK_ITEM_(TS_ERROR_FILE_REMOVE,				\
		"The file could not be removed."),			\
	TSHARK_ITEM_(TS_ERROR_FILE_RENAME,				\
		"The file could not be renamed."),			\
	TSHARK_ITEM_(TS_ERROR_FILE_POS,					\
		"The position in the file could not be changed."),	\
	TSHARK_ITEM_(TS_ERROR_FILE_RESIZE,				\
		"The file could not be resized."),			\
	TSHARK_ITEM_(TS_ERROR_FILE_PERM,				\
		"The file could not be accessed."),			\
	TSHARK_ITEM_(TS_ERROR_FILE_COPY,				\
		"The file could not be copied."),			\
/* The errors below are to accomodate the regcomp() function */		\
	TSHARK_ITEM_(TS_ERROR_REG_BADBR,				\
		     "Invalid use of back reference operator."),	\
	TSHARK_ITEM_(TS_ERROR_REG_BADPAT,				\
		     "Invalid use of pattern operators."),		\
	TSHARK_ITEM_(TS_ERROR_REG_BADRPT,				\
		     "Invalid use of repetition operators."),		\
	TSHARK_ITEM_(TS_ERROR_REG_EBRACE,				\
		     "Un-matched brace interval operators."),		\
	TSHARK_ITEM_(TS_ERROR_REG_EBRACK,				\
		     "Un-matched bracket list operators."),		\
	TSHARK_ITEM_(TS_ERROR_REG_ECOLLATE,				\
		     "Invalid collating element."),			\
	TSHARK_ITEM_(TS_ERROR_REG_ECTYPE,				\
		     "Unknown character class name."),			\
	TSHARK_ITEM_(TS_ERROR_REG_EEND,					\
		     "Nonspecific error."),				\
	TSHARK_ITEM_(TS_ERROR_REG_EESCAPE,				\
		     "Trailing backslash."),				\
	TSHARK_ITEM_(TS_ERROR_REG_EPAREN,				\
		     "Un-matched parenthesis group operators."),	\
	TSHARK_ITEM_(TS_ERROR_REG_ERANGE,				\
		     "Invalid use of the range operator."),		\
	TSHARK_ITEM_(TS_ERROR_REG_ESIZE,				\
		     "Pattern buffer too large."),			\
	TSHARK_ITEM_(TS_ERROR_REG_ESPACE,				\
		     "The regex routines ran out of memory."),		\
	TSHARK_ITEM_(TS_ERROR_REG_ESUBREG,				\
		     "Invalid back reference to a subexpression."),	\
	TSHARK_ITEM_(TS_ERROR_BUF_NOSPACE,				\
		     "The program ran out of space in an internal buffer."), \
	TSHARK_ITEM_(TS_NR_ERRORS,					\
		     nullptr)

#undef TSHARK_ITEM_
#define TSHARK_ITEM_(A, B) A
typedef enum : int {
		TSHARK_ERROR_DEFS_
} tserror_t;
#undef TSHARK_ITEM_

const char *ts_strerror(int ts_errno);

extern const char * const NullStr;

#endif /* ERRORS_H */
