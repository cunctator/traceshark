// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2021, 2023  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <QString>
#include <QStringList>
#include <QTextStream>

#include "misc/errors.h"
#include "misc/qtcompat.h"
#include "misc/traceshark.h"
#include "vtl/compiler.h"

namespace TShark {

#undef TSHARK_LOGIC_ITEM_
#define TSHARK_LOGIC_ITEM_(a) vtl_str(a)
	const char * const logic_names[] = {
		TSHARK_LOGIC_DEFS_,
		nullptr
	};
#undef TSHARK_LOGIC_ITEM_

	void checkSuffix(QString *string, const QString &suffix) {
		if (!string->endsWith(suffix.toLower()) &&
		    !string->endsWith(suffix.toUpper()))
			string->append(suffix);
	}

	void checkSuffix(QString *string, const QString &suffix,
			 const QString &rsuffix) {
		if (!string->endsWith(suffix.toLower()) &&
		    !string->endsWith(suffix.toUpper()) &&
		    !string->endsWith(rsuffix.toLower()) &&
		    !string->endsWith(rsuffix.toUpper()))
			string->append(suffix);
	}

	int readKeyValuePair(QTextStream &stream, QString &key, QString &value)
	{
		QString line;
		QStringList lineList;

		line = stream.readLine();
		do {
			lineList = line.split(' ', QtCompat::SkipEmptyParts);
		} while(lineList.size() == 0 && !stream.atEnd());
		if (lineList.size() != 2)
			return -TS_ERROR_FILEFORMAT;
		key = lineList[0];
		value = lineList[1];
		return 0;
	}
}
