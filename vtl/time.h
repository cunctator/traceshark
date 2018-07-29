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

#ifndef _VTL_TIME_H
#define _VTL_TIME_H

#include <QString>
#include <climits>

#include "vtl/compiler.h"

namespace vtl {

#define VTL_TIME_MAX vtl::Time(false, UINT_MAX, UINT_MAX)
#define VTL_TIME_MIN vtl::Time(true, UINT_MAX, UINT_MAX)
#define VTL_TIME_ZERO vtl::Time(false, 0, 0)

#define __TIME_MAX(A, B) (A > B ? A:B)
#define __TIME_MIN(A, B) (A < B ? A:B)

#define USECS_PER_MSEC (1000)
#define NSECS_PER_USEC (1000)
#define MSECS_PER_SEC (1000)
#define USECS_PER_SEC (MSECS_PER_SEC * USECS_PER_MSEC)
#define NSECS_PER_SEC (USECS_PER_SEC * NSECS_PER_USEC)

	typedef class Time final {
	public:
#if UINT_MAX > NSECS_PER_SEC
		typedef unsigned int timeuint_t;
#define __TIME_FMT_STRING          "%u"
#else
		typedef unsigned long timeuint_t;
#define __TIME_FMT_STRING          "%lu"
#endif
	Time(bool n = false, timeuint_t s = 0, timeuint_t ns = 0,
	     unsigned int p = 0):
		sec(s), nsec(ns), precision(p)
		{
			negative = n ? 0x1 : 0x0;
		}
		__always_inline Time operator+(const Time &other) const;
		__always_inline Time operator-(const Time &other) const;
		__always_inline bool operator<(const Time &other) const;
		__always_inline bool operator>(const Time &other) const;
		__always_inline bool operator<=(const Time &other) const;
		__always_inline bool operator>=(const Time &other) const;
		__always_inline bool operator==(const Time &other) const;
		__always_inline static Time fromDouble(const double &t);
		__always_inline static Time fromString(const char *str,
						       bool &ok);
		__always_inline static Time fromSpacedString(const char *str,
							     bool &ok);
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
		timeuint_t sec;
		timeuint_t nsec;
		unsigned int negative : 1;
		unsigned int precision : 4;
	} time_t;

	__always_inline Time Time::operator+(const Time &other) const
	{
		Time r;
		r.precision = __TIME_MAX(precision, other.precision);

		if (likely(negative == other.negative)) {
			r.sec = sec + other.sec;
			r.nsec = nsec + other.nsec;
			r.negative = negative;
			if (r.nsec > NSECS_PER_SEC) {
				r.nsec -= NSECS_PER_SEC;
				r.sec++;
			}
			return r;
		}
		bool greater = sec > other.sec ||
			(sec == other.sec && nsec > other.nsec);
		if (greater) {
			r.sec = sec;
			r.nsec = nsec;
			r.negative = negative;
			if (other.nsec > nsec) {
				r.nsec += NSECS_PER_SEC;
				r.sec--;
			}
			r.sec -= other.sec;
			r.nsec -= other.nsec;
			return r;
		}
		bool smaller = sec < other.sec ||
			(sec == other.sec && nsec < other.nsec);
		if (smaller) {
			r.sec = other.sec;
			r.nsec = other.nsec;
			r.negative = other.negative;
			if (nsec > other.nsec) {
				r.nsec += NSECS_PER_SEC;
				r.sec--;
			}
			r.sec -= sec;
			r.nsec -= nsec;
			return r;
		}
		r.sec = 0;
		r.nsec = 0;
		r.negative = 0x0;
		return r;
	}

	__always_inline Time Time::operator-(const Time &other) const
	{
		Time r;
		r.precision = __TIME_MAX(precision, other.precision);

		if (unlikely(negative != other.negative)) {
			r.sec = sec + other.sec;
			r.nsec = nsec + other.nsec;
			if (r.nsec >= NSECS_PER_SEC) {
				r.nsec -= NSECS_PER_SEC;
				r.sec++;
			}
			r.negative = negative;
			return r;
		}
		bool greater = sec > other.sec ||
			(sec == other.sec && nsec > other.nsec);
		if (greater) {
			r.sec = sec;
			r.nsec = nsec;
			r.negative = negative;
			if (other.nsec > nsec) {
				r.nsec += NSECS_PER_SEC;
				r.sec--;
			}
			r.sec -= other.sec;
			r.nsec -= other.nsec;
			return r;
		}
		bool smaller = sec < other.sec ||
			(sec == other.sec && nsec < other.nsec);
		if (smaller) {
			r.sec = other.sec;
			r.nsec = other.nsec;
			r.negative = other.negative;
			if (nsec > other.nsec) {
				r.nsec += NSECS_PER_SEC;
				r.sec--;
			}
			r.sec -= sec;
			r.nsec -= nsec;
			return r;
		}
		r.sec = 0;
		r.nsec = 0;
		r.negative = 0x0;
		return r;
	}

	__always_inline bool Time::operator>(const Time &other) const
	{
		if (unlikely(negative != other.negative))
			return other.negative == 0x1;
		bool greater = sec > other.sec ||
			(sec == other.sec && nsec > other.nsec);
		if (greater)
			return negative == 0x0;
		bool smaller = sec < other.sec ||
			(sec == other.sec && nsec < other.nsec);
		if (smaller)
			return negative == 0x1;
		return false;
	}

	__always_inline bool Time::operator<(const Time &other) const
	{
		if (unlikely(negative != other.negative))
			return negative == 0x1;
		bool greater = sec > other.sec ||
			(sec == other.sec && nsec > other.nsec);
		if (greater)
			return negative == 0x1;
		bool smaller = sec < other.sec ||
			(sec == other.sec && nsec < other.nsec);
		if (smaller)
			return negative == 0x0;
		return false;
	}

	__always_inline bool Time::operator>=(const Time &other) const
	{
		if (unlikely(negative != other.negative))
			return other.negative == 0x1;
		bool greater = sec > other.sec ||
			(sec == other.sec && nsec > other.nsec);
		if (greater)
			return negative == 0x0;
		bool smaller = sec < other.sec ||
			(sec == other.sec && nsec < other.nsec);
		if (smaller)
			return negative == 0x1;
		return true;
	}

	__always_inline bool Time::operator<=(const Time &other) const
	{
		if (unlikely(negative != other.negative))
			return negative == 0x1;
		bool greater = sec > other.sec ||
			(sec == other.sec && nsec > other.nsec);
		if (greater)
			return negative == 0x1;
		bool smaller = sec < other.sec ||
			(sec == other.sec && nsec < other.nsec);
		if (smaller)
			return negative == 0x0;
		return true;
	}

	__always_inline bool Time::operator==(const Time &other) const
	{
		if (other.sec == 0 && sec == 0 && other.nsec == 0 &&
		    nsec == 0)
			return true;
		return sec == other.sec && nsec == other.nsec &&
		negative == other.negative;
	}


	__always_inline Time Time::fromDouble(const double &t)
	{
		Time r;
		double a = t;

		r.negative = 0x0;
		if (t < 0) {
			r.negative = 0x1;
			a = - a;
		}
		r.sec = (unsigned long) a;
		r.nsec = (a - (double) r.sec) * NSECS_PER_SEC;
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

	__always_inline Time Time::__fromString(const char *str, bool &ok,
						bool spaced, bool colonatend)
	{
		Time r;
		timeuint_t base = 0;
		unsigned int d;
		timeuint_t mulint;
		const char *c;

		r.sec = 0;
		r.nsec = 0;
		r.negative = 0x0;
		ok = true;

		if (*str == '-') {
			str++;
			r.negative = 0x1;
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

		r.sec = base;
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
			r.nsec = mulint * base;
		}

		if (colonatend && *c != ':')
			goto error;

		return r;
	error:
		ok = false;
		return 0;
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
		unsigned int i, len;
		timeuint_t acc;
		timeuint_t mdiv;
		timeuint_t rdiv;
		int digit;

		char *b = &buf[0];
		int s;

		if (nsec > NSECS_PER_SEC)
			return false;

		if (negative == 0x1) {
			*b = '-';
			b++;
		}

		s = sprintf(b, __TIME_FMT_STRING, sec);
		if (s < 0)
			return false;
		b += s;

		*b = '.';
		b++;

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
		*b = '\0';

		return true;
	}

	__always_inline double Time::toDouble() const
	{
		double r = (double)sec;
		r += (double) nsec / NSECS_PER_SEC;
		return r;
	}

	__always_inline Time Time::fabs() const
	{
		Time t;

		t.sec = sec;
		t.nsec = nsec;
		t.negative = 0x0;
		t.precision = precision;
		return t;
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
