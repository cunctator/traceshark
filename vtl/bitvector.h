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

#ifndef _VTL_BITVECTOR_H
#define _VTL_BITVECTOR_H

#include <cstdint>
#include <QVector>

namespace vtl {
class BitVector
{
public:
	BitVector();
	__always_inline bool readbool(unsigned int index) const;
	__always_inline void appendbool(bool value);
	__always_inline unsigned int read(unsigned int index) const;
	__always_inline void append(unsigned int value);
	__always_inline unsigned int size() const;
	void clear();
	void softclear();
private:
	static const unsigned int INCREASE_NR = 1024;
	typedef unsigned int word_t;
	static const unsigned int BITVECTOR_BITS_PER_WORD = sizeof(word_t)
		* 8;
	unsigned int nrElements;
	unsigned int nrWords;
	QVector<word_t> array;
};

__always_inline bool BitVector::readbool(unsigned int index) const
{
	return BitVector::read(index) == 0x1;
}

__always_inline void BitVector::appendbool(bool value)
{
	unsigned int bitnr = nrElements % BITVECTOR_BITS_PER_WORD;
	word_t mask;
	unsigned int windex = nrElements / BITVECTOR_BITS_PER_WORD;

	if (windex >= nrWords) {
		nrWords += INCREASE_NR;
		array.resize(nrWords);
	}

	word_t &word = array[windex];

	if (value) {
		mask = 0x1 << bitnr;
		word = word | mask;
	} else {
		mask = ~(0x1 << bitnr);
		word = word & mask;
	}

	nrElements++;
}

__always_inline unsigned int BitVector::read(unsigned int index) const
{
	unsigned int windex = index / BITVECTOR_BITS_PER_WORD;
        unsigned int r;
	unsigned int bitnr = index % BITVECTOR_BITS_PER_WORD;

	const word_t &word = array[windex];
	r = (word >> bitnr) & 0x1;
	return r;
}

__always_inline void BitVector::append(unsigned int value)
{
	unsigned int bitnr = nrElements % BITVECTOR_BITS_PER_WORD;
	word_t mask_and, mask_or;
	unsigned int windex = nrElements / BITVECTOR_BITS_PER_WORD;

	if (windex >= nrWords) {
		nrWords += INCREASE_NR;
		array.resize(nrWords);
	}

	word_t &word = array[windex];

	mask_or  = (value & 0x1) << bitnr;
	mask_and = ~ (0x1 << bitnr);
	word &= mask_and;
	word |= mask_or;

	nrElements++;
}

__always_inline unsigned int BitVector::size() const
{
	return nrElements;
}

}

#endif /* _BITVECTOR_H */
