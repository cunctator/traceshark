// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
/*
 * Traceshark - a visualizer for visualizing ftrace and perf traces
 * Copyright (C) 2015, 2018-2020  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

#ifndef SETTING_H
#define SETTING_H

#include <QString>
#include <QMap>

#include "vtl/compiler.h"
#include "vtl/error.h"

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

class SettingStore;

class Setting
{
	friend class SettingStore;
	friend class ValueBox;
public:
	typedef enum Index : int {
		SHOW_SCHED_GRAPHS = 0,
		HORIZONTAL_WAKEUP,
		VERTICAL_WAKEUP,
		MAX_VRT_WAKEUP_LATENCY,
		SHOW_CPUFREQ_GRAPHS,
		SHOW_CPUIDLE_GRAPHS,
		SHOW_MIGRATION_GRAPHS,
		SHOW_MIGRATION_UNLIMITED,
		OPENGL_ENABLED,
		LINE_WIDTH,
		IDLE_LINE_WIDTH,
		FREQ_LINE_WIDTH,
		MIGRATION_WIDTH,
		EVENT_PID_FLT_INCL_ON,
		NR_SETTINGS,
	} index_t;
        class Value;
	class Dependency;
	class Value {
		friend class Dependency;
		friend class Setting;
		friend class SettingStore;
		friend class ValueBox;
	public:
		typedef enum Type {
			TYPE_BOOL,
			TYPE_INT,
		} type_t;
		Value();
		Value(bool b);
		Value(int i);
		vtl_always_inline bool operator<(const Value &other) const;
		vtl_always_inline bool operator>(const Value &other) const;
		vtl_always_inline bool operator<=(const Value &other) const;
		vtl_always_inline bool operator>=(const Value &other) const;
		vtl_always_inline bool operator==(const Value &other) const;
		vtl_always_inline bool operator!=(const Value &other) const;
		vtl_always_inline bool boolv() const;
		vtl_always_inline int intv() const;
		vtl_always_inline type_t type() const;
	protected:
		type_t type_;
		union {
			bool bool_value;
			int int_value;
		} value;
	};
	typedef enum Flag : unsigned int {
		FLAG_NO_FLAG          = 0,
		FLAG_MUST_BE_CONSUMED = 1
	} flag_t;
	class Dependency
	{
		friend class Setting;
		friend class SettingStore;
	public:
		typedef enum Type : int {
			DESIRED_VALUE = 0,
			DESIRED_INTERVAL,
		} type_t;
		Dependency();
		Dependency(index_t i, bool desired_val);
		Dependency(index_t i, int desired_val);
		Dependency(index_t i, int low, int high);
		bool getDesiredBool() const;
		int getDesiredInt() const;
		vtl_always_inline enum Type type() const;
		vtl_always_inline int index() const;
		vtl_always_inline const Value &desired() const;
		vtl_always_inline const Value &low() const;
		vtl_always_inline const Value &high() const;
		const Value &getLowerBound() const;
		const Value &getHigherBound() const;
		bool check(const Value &val) const;
		vtl_always_inline void assert_desired() const;
		vtl_always_inline void assert_interval() const;
	protected:
		type_t type_;
		int index_;
		Value desired_value;
		Value low_value;
		Value high_value;
	private:
		void error_dep_type() const;
	};
	Setting();

	static bool isWideScreen();
	static bool isLowResScreen();

	static const char *getValueTypeStr(Value::type_t type);
private:
	static void error_type(Value::type_t expected, Value::type_t was);
protected:
	vtl_always_inline static void assert_bool(const Value &val);
	vtl_always_inline static void assert_int(const Value &val);
	vtl_always_inline static void assert_same(Value::type_t a,
						  Value::type_t b);
	bool supported;
	Value value;
	Value min_value;
	Value max_value;
	Value disabled_value;
	flag_t flags;
	QString name;
	QString unit;
	Dependency dependency[4];
	Dependency dependent[4];
	unsigned int nrDep;
	unsigned int nrDependents;
};

vtl_always_inline
bool Setting::Value::operator<(const Setting::Value &other) const
{
	assert_same(type_, other.type_);
	switch (type_) {
	case TYPE_BOOL:
		vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
		break;
	case TYPE_INT:
		return value.int_value < other.value.int_value;
		break;
	default:
		break;
	}
	vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
	return true;
}

vtl_always_inline
bool Setting::Value::operator>(const Setting::Value &other) const
{
	assert_same(type_, other.type_);
	switch (type_) {
	case TYPE_BOOL:
		vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
		break;
	case TYPE_INT:
		return value.int_value > other.value.int_value;
		break;
	default:
		break;
	}
	vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
	return true;
}

vtl_always_inline
bool Setting::Value::operator<=(const Setting::Value &other) const
{
	assert_same(type_, other.type_);
	switch (type_) {
	case TYPE_BOOL:
		vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
		break;
	case TYPE_INT:
		return value.int_value <= other.value.int_value;
		break;
	default:
		break;
	}
	vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
	return true;
}

vtl_always_inline
bool Setting::Value::operator>=(const Setting::Value &other) const
{
	assert_same(type_, other.type_);
	switch (type_) {
	case TYPE_BOOL:
		vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
		break;
	case TYPE_INT:
		return value.int_value >= other.value.int_value;
		break;
	default:
		break;
	}
	vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
	return true;
}

vtl_always_inline
bool Setting::Value::operator!=(const Setting::Value &other) const
{
	assert_same(type_, other.type_);
	switch (type_) {
	case TYPE_BOOL:
		return value.bool_value != other.value.bool_value;
		break;
	case TYPE_INT:
		return value.int_value != other.value.int_value;
		break;
	default:
		break;
	}
	vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
	return true;
}

vtl_always_inline
bool Setting::Value::operator==(const Setting::Value &other) const
{
	assert_same(type_, other.type_);
	switch (type_) {
	case TYPE_BOOL:
		return value.bool_value == other.value.bool_value;
		break;
	case TYPE_INT:
		return value.int_value == other.value.int_value;
		break;
	default:
		break;
	}
	vtl::errx(BSD_EX_SOFTWARE, "%s:%d", __FILE__, __LINE__);
	return true;
}

vtl_always_inline
bool Setting::Value::boolv() const
{
	assert_same(type_, TYPE_BOOL);
	return value.bool_value;
}

vtl_always_inline
int Setting::Value::intv() const
{
	assert_same(type_, TYPE_INT);
	return value.int_value;
}

vtl_always_inline
Setting::Value::type_t Setting::Value::type() const
{
	return type_;
}

vtl_always_inline void Setting::assert_bool(const Setting::Value &val)
{
	if (val.type_ != Setting::Value::TYPE_BOOL)
		error_type(Setting::Value::TYPE_BOOL, val.type_);
}

 vtl_always_inline void Setting::assert_int(const Value &val)
{
	if (val.type_ != Setting::Value::TYPE_INT)
		error_type(Setting::Value::TYPE_INT, val.type_);
}

vtl_always_inline void Setting::assert_same(Value::type_t a, Value::type_t b)
{
	if (a != b)
		error_type(a, b);
}

vtl_always_inline void Setting::Dependency::assert_desired() const
{
	if (type_ != Setting::Dependency::DESIRED_VALUE)
		error_dep_type();
}

vtl_always_inline void Setting::Dependency::assert_interval() const
{
	if (type_ != Setting::Dependency::DESIRED_INTERVAL)
		error_dep_type();
}

vtl_always_inline enum Setting::Dependency::Type Setting::Dependency::type()
	const
{
	return type_;
}

vtl_always_inline int Setting::Dependency::index() const
{
	return index_;
}

vtl_always_inline const Setting::Value &Setting::Dependency::desired() const
{
	assert_desired();
	return desired_value;
}

vtl_always_inline const Setting::Value &Setting::Dependency::low() const
{
	assert_interval();
	return low_value;
}

vtl_always_inline const Setting::Value &Setting::Dependency::high() const
{
	assert_interval();
	return high_value;
}

#endif /* SETTING_H */
