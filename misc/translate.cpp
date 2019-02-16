// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2019  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include "translate.h"

tserror_t translate_FileError(QFileDevice::FileError err)
{
	tserror_t ecode;
	switch(err) {
	case QFileDevice::NoError:
		ecode = TS_ERROR_NOERROR;
		break;
	case QFileDevice::ReadError:
		ecode = TS_ERROR_FILE_READ;
		break;
	case QFileDevice::WriteError:
		ecode = TS_ERROR_FILE_WRITE;
		break;
	case QFileDevice::FatalError:
		ecode = TS_ERROR_FATALERROR;
		break;
	case QFileDevice::ResourceError:
		ecode = TS_ERROR_FILE_RESOURCE;
		break;
	case QFileDevice::OpenError:
		ecode = TS_ERROR_OPEN;
		break;
	case QFileDevice::AbortError:
		ecode = TS_ERROR_ABORT;
		break;
	case QFileDevice::TimeOutError:
		ecode = TS_ERROR_TIMEOUT;
		break;
	case QFileDevice::UnspecifiedError:
		ecode = TS_ERROR_UNSPEC;
		break;
	case QFileDevice::RemoveError:
		ecode = TS_ERROR_FILE_REMOVE;
		break;
	case QFileDevice::RenameError:
		ecode = TS_ERROR_FILE_RENAME;
		break;
	case QFileDevice::PositionError:
		ecode = TS_ERROR_FILE_POS;
		break;
	case QFileDevice::ResizeError:
		ecode = TS_ERROR_FILE_RESIZE;
		break;
	case QFileDevice::PermissionsError:
		ecode = TS_ERROR_FILE_PERM;
		break;
	case QFileDevice::CopyError:
		ecode = TS_ERROR_FILE_COPY;
		break;
	default:
		ecode = TS_ERROR_UNSPEC;
		break;
	}
	return ecode;
}
