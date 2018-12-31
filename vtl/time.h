// SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
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

#ifndef _VTL_TIME_H
#define _VTL_TIME_H

#include <QString>
#include <climits>
#include <cstdint>

#include "vtl/compiler.h"

namespace vtl {

#define VTL_TIME_MAX vtl::Time(false, UINT_MAX, UINT_MAX)
#define VTL_TIME_MIN vtl::Time(true, UINT_MAX, UINT_MAX)
#define VTL_TIME_ZERO vtl::Time(false, 0, 0)

#define _TIME_MAX(A, B) (A > B ? A:B)
#define _TIME_MIN(A, B) (A < B ? A:B)

#define _TIMEINT_REQ (1000000000000000000LL)

#if INT_MAX >= _TIMEINT_REQ && INT_MIN < -_TIMEINT_REQ
#define	_TIME_MILLE (1000)
#define TIME_USE_INT
#define _TIME_FMT_STRING  "%d"

#elif LONG_MAX >= _TIMEINT_REQ && LONG_MIN < -_TIMEINT_REQ
#define	_TIME_MILLE (1000L)
#define TIME_USE_LONG
#define _TIME_FMT_STRING  "%ld"

#elif LLONG_MAX >= _TIMEINT_REQ && LLONG_MIN < -_TIMEINT_REQ
#define	_TIME_MILLE (1000LL)
#define TIME_USE_LLONG
#define _TIME_FMT_STRING  "%lld"

#else
#error "A longer long long is required!"
#endif

#define USECS_PER_MSEC (_TIME_MILLE)
#define NSECS_PER_USEC (_TIME_MILLE)
#define MSECS_PER_SEC (_TIME_MILLE)
#define USECS_PER_SEC (MSECS_PER_SEC * USECS_PER_MSEC)
#define NSECS_PER_SEC (USECS_PER_SEC * NSECS_PER_USEC)

	typedef class Time final {
	public:

#ifdef TIME_USE_INT
		typedef int timeint_t;
#elif defined(TIME_USE_LONG)
		typedef long timeint_t;
#elif defined(TIME_USE_LLONG)
		typedef long long timeint_t;
#endif
	Time(bool n = false, timeint_t s = 0, timeint_t ns = 0,
	     unsigned int p = 0):
		time(n ? (- s * NSECS_PER_SEC + ns) : (s * NSECS_PER_SEC + ns)),
			precision(p)
		{}
		__always_inline Time operator+(const Time &other) const;
		__always_inline void operator+=(const Time &other);
		__always_inline Time operator-(const Time &other) const;
		__always_inline void operator-=(const Time &other);
		__always_inline bool operator<(const Time &other) const;
		__always_inline bool operator>(const Time &other) const;
		__always_inline bool operator<=(const Time &other) const;
		__always_inline bool operator>=(const Time &other) const;
		__always_inline bool operator==(const Time &other) const;
		__always_inline Time operator*(long other) const;
		__always_inline Time operator*(int other) const;
		__always_inline Time operator*(unsigned long other) const;
		__always_inline Time operator*(unsigned other) const;
		__always_inline void operator*=(long other);
		__always_inline void operator*=(int other);
		__always_inline void operator*=(unsigned long other);
		__always_inline void operator*=(unsigned other);
		__always_inline static Time fromDouble(const double &t);
		__always_inline static Time fromString(const char *str,
						       bool &ok);
		__always_inline static Time fromSpacedString(const char *str,
							     bool &ok);
		__always_inline bool isZero();
		__always_inline QString toQString() const;
		__always_inline bool sprint(char *buf) const;
		__always_inline double toDouble() const;
		__always_inline Time fabs() const;
		__always_inline unsigned int getPrecision() const;
		__always_inline void setPrecision(unsigned int p);
	private:
		__always_inline static Time __fromString(const char *str,
							 bool &ok,
							 bool spaced,
							 bool colonatend);
		timeint_t time;
		unsigned int precision : 4;
	} time_t;

	__always_inline Time Time::operator+(const Time &other) const
	{
		Time r;
		r.precision = _TIME_MAX(precision, other.precision);
		r.time = time + other.time;
		return r;
	}

	__always_inline void Time::operator+=(const Time &other)
	{
		precision = _TIME_MAX(precision, other.precision);
		time = time + other.time;
	}

	__always_inline Time Time::operator-(const Time &other) const
	{
		Time r;
		r.precision = _TIME_MAX(precision, other.precision);
		r.time = time - other.time;
		return r;
	}

	__always_inline void Time::operator-=(const Time &other)
	{
		precision = _TIME_MAX(precision, other.precision);
		time = time - other.time;
	}

	__always_inline bool Time::operator>(const Time &other) const
	{
		return time > other.time;
	}

	__always_inline bool Time::operator<(const Time &other) const
	{
		return time < other.time;
	}

	__always_inline bool Time::operator>=(const Time &other) const
	{
		return time >= other.time;
	}

	__always_inline bool Time::operator<=(const Time &other) const
	{
		return time <= other.time;
	}

	__always_inline bool Time::operator==(const Time &other) const
	{
		return time == other.time;
	}

	__always_inline Time Time::operator*(long other) const
	{
		Time r;
		r.precision = precision;
		r.time = time * other;
		return r;
	}

	__always_inline Time Time::operator*(int other) const
	{
		Time r;
		r.precision = precision;
		r.time = time * other;
		return r;
	}

	__always_inline Time Time::operator*(unsigned long other) const
	{
		Time r;
		r.precision = precision;
		r.time = time * other;
		return r;
	}

	__always_inline Time Time::operator*(unsigned other) const
	{
		Time r;
		r.precision = precision;
		r.time = time * other;
		return r;
	}

	__always_inline void Time::operator*=(long other)
	{
		time *= other;
	}

	__always_inline void Time::operator*=(int other)
	{
		time *= other;
	}

	__always_inline void Time::operator*=(unsigned long other)
	{
		time *= other;
	}

	__always_inline void Time::operator*=(unsigned other)
	{
		time *= other;
	}

	__always_inline Time Time::fromDouble(const double &t)
	{
		Time r;

		r.time = (timeint_t)
			(t * NSECS_PER_SEC +
			 (((double) 5) / ((double)(NSECS_PER_SEC * 10))));
		r.precision = 9;
		return r;
	}

	__always_inline Time Time::fromString(const char *str, bool &ok)
	{
		return __fromString(str, ok, false, true);
	}

	__always_inline Time Time::fromSpacedString(const char *str, bool &ok)
	{
		return __fromString(str, ok, true, false);
	}

	__always_inline bool Time::isZero()
	{
		return time == 0;
	}

	__always_inline Time Time::__fromString(const char *str, bool &ok,
						bool spaced, bool colonatend)
	{
		Time r;
		uint32_t base = 0;
		uint32_t d;
		uint32_t mulint;
		const char *c;
		timeint_t tt;

		uint32_t sec = 0;
		uint32_t nsec = 0;
		bool negative = false;
		ok = true;

		if (*str == '-') {
			str++;
			negative = true;
		}

		for (c = str; *c != '\0'; c++) {
			if (spaced && *c == ' ')
				continue;
			if (*c < '0' || *c > '9')
				break;
			d = *c - '0';
			base *= 10;
			base += d;
		}

		sec = base;
		r.precision = 0;

		if (*c == '.') {
			mulint = NSECS_PER_SEC;
			base = 0;
			for (c++; *c != '\0'; c++) {
				if (spaced && *c == ' ')
					continue;
				if (*c < '0' || *c > '9')
					break;
				r.precision++;
				d = *c - '0';
				base *= 10;
				base += d;
				mulint /= 10;
			}
			nsec = mulint * base;
		}

		tt = sec * NSECS_PER_SEC + nsec;
		r.time = negative ? -tt : tt;
		if (colonatend && *c != ':')
			ok = false;

		return r;
	}

	__always_inline QString Time::toQString() const
	{
		QString qstr;
		char buf[40];

		if (sprint(buf))
			qstr = QString(buf);

		return qstr;
	}

	__always_inline bool Time::sprint(char *buf) const
	{
		int i, len;
		uint32_t acc;
		uint32_t mdiv;
		uint32_t rdiv;
		unsigned int sec;
		uint32_t nsec;
		timeint_t t = time;
		int digit;

		char *b = &buf[0];
		int s;

		if (t < 0) {
			*b = '-';
			b++;
			t = -t;
		}

		sec = t / NSECS_PER_SEC;
		nsec = t % NSECS_PER_SEC;

		s = sprintf(b, "%u", sec);
		if (s < 0)
			return false;
		b += s;

		if (precision > 0) {
			*b = '.';
			b++;
		} else {
			goto out;
		}

		len = precision;
		acc = nsec;
		mdiv = NSECS_PER_SEC / 10;
		rdiv = mdiv;

		/* rdiv is used for rounding */
		for (i = 0; i < len; i++) {
			rdiv /= 10;
		}
		acc += 5 * rdiv;

		for (i = 0; i < len; i++) {
			digit = acc / mdiv;
			acc = acc % mdiv;
			*b = '0' + digit;
			b++;
			mdiv /= 10;
		}
	out:
		*b = '\0';

		return true;
	}

	__always_inline double Time::toDouble() const
	{
		double r = ((double) time) / NSECS_PER_SEC;
		return r;
	}

	__always_inline Time Time::fabs() const
	{
		Time r;

		r.time = time < 0 ? -time : time;
		r.precision = precision;
		return r;
	}

	__always_inline unsigned int Time::getPrecision() const
	{
		return precision;
	}

	__always_inline void Time::setPrecision(unsigned int p)
	{
		if (p < 10)
			precision = p;
	}
}

#endif /* _VTL_TIME_H */
