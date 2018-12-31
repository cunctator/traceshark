#!/bin/sh
# SPDX-License-Identifier: (GPL-2.0-or-later OR BSD-2-Clause)
#
#  testrun.sh - a script to launch traceshark multiple times
#  Copyright (C) 2017  Viktor Rosendahl <viktor.rosendahl@gmail.com>
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

if [ $# -lt 3 ];then
    echo "$0 <prgname> <perf filename> <nr of iterations>"
    exit
fi

sleeptime=7

if [ $# -gt 3 ];then
    sleeptime=$4
fi

prg_name=$1
perf_name=$2
nr_iterations=$3


for index in $(seq $nr_iterations)
do
    LC_NUMERIC="C" $prg_name $perf_name&
    sleep $sleeptime
    killall -9 $prg_name
done
