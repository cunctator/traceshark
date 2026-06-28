// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2021, 2023, 2026  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <cstring>

#include <QString>
#include <QStringList>
#include <QTextStream>

#include "misc/errors.h"
#include "misc/qtcompat.h"
#include "misc/traceshark.h"
#include "misc/types.h"
#include "vtl/compiler.h"

namespace TShark {

/*
 * Search for needle within [hay, end). Returns a pointer to the first match or
 * nullptr. Used to locate the short stack-trace labels within a single line.
 */
static const char *findInRange(const char *hay, const char *end,
			       const char *needle, int nlen)
{
	for (const char *h = hay; h + nlen <= end; h++) {
		if (memcmp(h, needle, nlen) == 0)
			return h;
	}
	return nullptr;
}

int stripStackTraceHeaders(const char *in, int inlen, char *out)
{
	static const char kernelLabel[] = EVENTSTR_KERNEL_STACK ":";
	static const char userLabel[] = EVENTSTR_USER_STACK ":";
	const int kernelLen = (int) (sizeof(kernelLabel) - 1);
	const int userLen = (int) (sizeof(userLabel) - 1);
	const char *p = in;
	const char *end = in + inlen;
	char *o = out;

	while (p < end) {
		const char *nl = (const char*) memchr(p, '\n', end - p);
		const char *lineEnd = (nl != nullptr) ? nl : end;
		const char *content = p;
		const char *c;
		int n;

		/*
		 * Fast path: stack frame lines start with "=>" (after optional
		 * leading whitespace) and can never contain a header label, so
		 * copy them verbatim without searching.
		 */
		for (c = p; c < lineEnd && (*c == ' ' || *c == '\t'); c++)
			;
		if (c + 1 < lineEnd && c[0] == '=' && c[1] == '>')
			goto copy;

		if (findInRange(p, lineEnd, kernelLabel, kernelLen) != nullptr) {
			/* Drop the whole kernel_stack header line. */
			p = (nl != nullptr) ? nl + 1 : end;
			continue;
		}

		c = findInRange(p, lineEnd, userLabel, userLen);
		if (c != nullptr) {
			content = c + userLen;
			while (content < lineEnd &&
			       (*content == ' ' || *content == '\t'))
				content++;
			/*
			 * Insert a tab before copying the inlined item, since
			 * user space stacktrace lines are always indented by a
			 * tab.
			 */
			*o = '\t';
			o++;
		}
copy:
		n = (int) (lineEnd - content);
		memcpy(o, content, n);
		o += n;
		if (nl != nullptr)
			*o++ = '\n';
		p = (nl != nullptr) ? nl + 1 : end;
	}
	return (int) (o - out);
}

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
