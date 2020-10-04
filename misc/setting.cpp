// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2018-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#include <stdlib.h>

#include "misc/errors.h"
#include "misc/osapi.h"
#include "misc/traceshark.h"
#include "vtl/error.h"
#include "setting.h"
#include "translate.h"

Setting::Value::Value() :
	type_(TYPE_INT)
{
	value.int_value = 0;
}

Setting::Value::Value(bool b) :
	type_(TYPE_BOOL)
{
	value.bool_value = b;
}

Setting::Value::Value(int i) :
	type_(TYPE_INT)
{
	value.int_value = i;
}

Setting::Setting(): supported(true), flags(FLAG_NO_FLAG), nrDep(0),
		    nrDependents(0)
{}


bool Setting::isWideScreen()
{
	QRect geometry;

	geometry = QApplication::desktop()->availableGeometry();
	return geometry.width() > 1800;
}

bool Setting::isLowResScreen()
{
	QRect geometry;

	geometry = QApplication::desktop()->availableGeometry();
	/* This is a heuristic */
	return geometry.width() < 1700 && geometry.height() < 1220;
}


Setting::Dependency::Dependency():
	type_(DESIRED_VALUE), index_(SHOW_SCHED_GRAPHS)
{}

Setting::Dependency::Dependency(Setting::index_t i, bool desired_val) :
	type_(DESIRED_VALUE), index_(i), desired_value(desired_val),
	low_value(0), high_value(0)
{}

Setting::Dependency::Dependency(Setting::index_t i, int desired_val) :
	type_(DESIRED_VALUE), index_(i), desired_value(desired_val),
	low_value(0), high_value(0)
{}

Setting::Dependency::Dependency(Setting::index_t i, int low, int high) :
	type_(DESIRED_INTERVAL), index_(i), desired_value(0),
	low_value(low), high_value(high)
{}

bool Setting::Dependency::getDesiredBool() const
{
	assert_bool(desired_value);
	assert_desired();
	return desired_value.value.bool_value;
}

int Setting::Dependency::getDesiredInt() const
{
	assert_int(desired_value);
	assert_desired();
	return desired_value.value.int_value;
}

bool Setting::Dependency::check(const Value &val) const
{
	if (type_ == DESIRED_VALUE) {
		return (val == desired_value);
	} else { /* type == DESIRED_INTERVAL */
		return (val <= high_value &&
			val >= low_value);
	}
}

const char *Setting::getValueTypeStr(Value::type_t type)
{
	static const char *boolstr = "bool";
	static const char *intstr = "int";
	static const char *unknownstr = "unknown";

	if (type == Value::TYPE_BOOL)
		return boolstr;
	if (type == Value::TYPE_INT)
		return intstr;
	return unknownstr;
}

void Setting::error_type(Value::type_t expected, Value::type_t was)
{
	const char *expstr = getValueTypeStr(expected);
	const char *wasstr = getValueTypeStr(was);

	vtl::errx(BSD_EX_SOFTWARE, "Error at %s:%d. Expected type %s but the type was %s\n", __FILE__, __LINE__, expstr, wasstr);
}

void Setting::Dependency::error_dep_type() const
{
	vtl::errx(BSD_EX_SOFTWARE,  "Error at %s:%d. Wrong dependency type\n");
}
