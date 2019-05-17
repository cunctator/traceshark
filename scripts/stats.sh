#!/bin/sh
# SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
#
#  average.sh - a script to compute the average of traceshark processing time
#  Copyright (C) 2017, 2018  Viktor Rosendahl <viktor.rosendahl@gmail.com>
#
#  This file is dual licensed: you can use it either under the terms of
#  the GPL, or the BSD license, at your option.
#
#   a) This program is free software; you can redistribute it and/or
#      modify it under the terms of the GNU General Public License as
#      published by the Free Software Foundation; either version 2 of the
#      License, or (at your option) any later version.
#
#      This program is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#      GNU General Public License for more details.
#
#      You should have received a copy of the GNU General Public
#      License along with this library; if not, write to the Free
#      Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
#      MA 02110-1301 USA
#
#  Alternatively,
#
#   b) Redistribution and use in source and binary forms, with or
#      without modification, are permitted provided that the following
#      conditions are met:
#
#      1. Redistributions of source code must retain the above
#         copyright notice, this list of conditions and the following
#         disclaimer.
#      2. Redistributions in binary form must reproduce the above
#         copyright notice, this list of conditions and the following
#         disclaimer in the documentation and/or other materials
#         provided with the distribution.
#
#      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#      CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#      INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#      DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
#      CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
#      NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#      LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
#      HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#      CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
#      OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#      EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

if [ ! $# -eq 1 ];then
    echo "Usage $0 <file>"
    exit 0
fi

file=$1

average=$(cat $file|LC_NUMERIC="C" awk '/processTrace/ { sum+=$3;n++ } END {print sum/n}')

minimum=$(cat $file|LC_NUMERIC="C" awk 'BEGIN {min=-1} $1~/processTrace/ && (min == -1 || $3 < min) {  min=$3 } END {print min}')

maximum=$(cat $file|LC_NUMERIC="C" awk 'BEGIN {max=-1} $1~/processTrace/ && (max == -1 || $3 > max) {  max=$3 } END {print max}')


st_average=$(cat $file|LC_NUMERIC="C" awk '/showTrace/ { sum+=$3;n++ } END {print sum/n}')

st_minimum=$(cat $file|LC_NUMERIC="C" awk 'BEGIN {min=-1} $1~/showTrace/ && (min == -1 || $3 < min) {  min=$3 } END {print min}')

st_maximum=$(cat $file|LC_NUMERIC="C" awk 'BEGIN {max=-1} $1~/showTrace/ && (min == -1 || $3 > max) {  max=$3 } END {print max}')

tps_average=$(cat $file|LC_NUMERIC="C" awk '/tracePlot->show/ { sum+=$3;n++ } END {print sum/n}')

tps_minimum=$(cat $file|LC_NUMERIC="C" awk 'BEGIN {min=-1} $1~/tracePlot->show/ && (min == -1 || $3 < min) {  min=$3 } END {print min}')

tps_maximum=$(cat $file|LC_NUMERIC="C" awk 'BEGIN {max=-1} $1~/tracePlot->show/ && (max == -1 || $3 > max) {  max=$3 } END {print max}')

echo "\nprocessTrace():"
echo "minimum = "$minimum
echo "average = "$average
echo "maximum = "$maximum
echo "\n"
echo "showTrace():"
echo "show trace minimum = "$st_minimum
echo "show trace average = "$st_average
echo "show trace maximum = "$st_maximum
echo "\n"
echo "tracePlot->show():"
echo "show trace minimum = "$tps_minimum
echo "show trace average = "$tps_average
echo "show trace maximum = "$tps_maximum
echo "\n"
