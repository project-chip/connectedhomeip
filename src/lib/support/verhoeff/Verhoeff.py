#!/usr/bin/env python

#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2013-2017 Nest Labs, Inc.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    @file
#      This file implements a Python script for generating and
#      verifying check digits using Verhoeff's algorithm. In addition,
#      the script may be used to generate Verhoeff multiply tables for
#      a given base.
#

from __future__ import absolute_import, print_function

import sys

from six.moves import range

__all__ = ['ComputeCheckChar',   'VerifyCheckChar',
           'ComputeCheckChar16', 'VerifyCheckChar16',
           'ComputeCheckChar32', 'VerifyCheckChar32',
           'ComputeCheckChar36', 'VerifyCheckChar36']

CharSet_Base10 = "0123456789"
CharSet_Base16 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
CharSet_Base32 = "0123456789ABCDEFGHJKLMNPRSTUVWXY"  # Excludes I, O, Q and Z
CharSet_Base36 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

PermTable_Base10 = [1,   5,  7,  6,  2,  8,  3,  0,  9,  4]
PermTable_Base16 = [4,   7,  5, 14,  8, 12,
                    15,  0,  2, 11,  3, 13, 10,  6,  9,  1]
PermTable_Base32 = [7,   2,  1, 30, 16, 20, 27, 11, 31,  6,  8, 13, 29,  5,
                    10, 21, 22,  3, 24,  0, 23, 25, 12,  9, 28, 14,  4, 15, 17, 18, 19, 26]
PermTable_Base36 = [29,  0, 32, 11, 35, 20,  7, 27,  2,  4, 19, 28, 30,  1,  5, 12,
                    3,  9, 16, 22,  6, 33,  8, 24, 26, 21, 14, 10, 34, 31, 15, 25, 17, 13, 23, 18]


def DihedralMultiply(x, y, n):
    n2 = n * 2

    x = x % n2
    y = y % n2

    if (x < n):
        if (y < n):
            return (x + y) % n
        else:
            return ((x + (y - n)) % n) + n
    else:
        if (y < n):
            return ((n + (x - n) - y) % n) + n
        else:
            return (n + (x - n) - (y - n)) % n


def DihedralInvert(val, n):
    if (val > 0 and val < n):
        return n - val
    else:
        return val


def Permute(val, permTable, iterCount):
    val = val % len(permTable)
    if (iterCount == 0):
        return val
    else:
        return Permute(permTable[val], permTable, iterCount - 1)


def _ComputeCheckChar(str, strLen, polygonSize, permTable, charSet):
    str = str.upper()
    c = 0
    for i in range(1, strLen+1):
        ch = str[strLen - i]
        val = charSet.index(ch)
        p = Permute(val, permTable, i)
        c = DihedralMultiply(c, p, polygonSize)
    c = DihedralInvert(c, polygonSize)
    return charSet[c]


def ComputeCheckChar(str, charSet=CharSet_Base10):
    return _ComputeCheckChar(str, len(str), polygonSize=5, permTable=PermTable_Base10, charSet=charSet)


def VerifyCheckChar(str, charSet=CharSet_Base10):
    expectedCheckCh = _ComputeCheckChar(str, len(
        str)-1, polygonSize=5, permTable=PermTable_Base10, charSet=CharSet_Base10)
    return str[-1] == expectedCheckCh


def ComputeCheckChar16(str, charSet=CharSet_Base16):
    return _ComputeCheckChar(str, len(str), polygonSize=8, permTable=PermTable_Base16, charSet=charSet)


def VerifyCheckChar16(str, charSet=CharSet_Base16):
    expectedCheckCh = _ComputeCheckChar(
        str, len(str)-1, polygonSize=8, permTable=PermTable_Base16, charSet=charSet)
    return str[-1] == expectedCheckCh


def ComputeCheckChar32(str, charSet=CharSet_Base32):
    return _ComputeCheckChar(str, len(str), polygonSize=16, permTable=PermTable_Base32, charSet=charSet)


def VerifyCheckChar32(str, charSet=CharSet_Base32):
    expectedCheckCh = _ComputeCheckChar(
        str, len(str)-1, polygonSize=16, permTable=PermTable_Base32, charSet=charSet)
    return str[-1] == expectedCheckCh


def ComputeCheckChar36(str, charSet=CharSet_Base36):
    return _ComputeCheckChar(str, len(str), polygonSize=18, permTable=PermTable_Base36, charSet=charSet)


def VerifyCheckChar36(str, charSet=CharSet_Base36):
    expectedCheckCh = _ComputeCheckChar(
        str, len(str)-1, polygonSize=18, permTable=PermTable_Base36, charSet=charSet)
    return str[-1] == expectedCheckCh


if __name__ == "__main__":

    usage = """Usage: %s <command> [ <args> ]

Commands:
  generate <string>
  verify <string-with-check-digit>
  gen-multiply-table <base>
""" % (sys.argv[0])

    if (len(sys.argv) < 2):
        print(usage)
    elif (sys.argv[1] == "generate"):
        if (len(sys.argv) < 3):
            print(usage)
            sys.exit(-1)
        ch = ComputeCheckChar(sys.argv[2])
        print("%s%c" % (sys.argv[2], ch))
    elif (sys.argv[1] == "verify"):
        if (len(sys.argv) < 3):
            print(usage)
            sys.exit(-1)
        if (VerifyCheckChar(sys.argv[2])):
            print("%s is VALID" % (sys.argv[2]))
            sys.exit(0)
        else:
            print("%s is INVALID" % (sys.argv[2]))
            sys.exit(-1)
    elif (sys.argv[1] == "gen-multiply-table"):
        if (len(sys.argv) < 3):
            print(usage)
            sys.exit(-1)
        base = int(sys.argv[2])
        if (base % 2 != 0):
            print("Base must be even")
            sys.exit(-1)
        n = base / 2
        for x in range(0, base):
            sys.stdout.write("    ")
            for y in range(0, base):
                o = DihedralMultiply(x, y, n)
                sys.stdout.write("%2d, " % o)
            sys.stdout.write("\n")
    else:
        print(usage)
        sys.exit(-1)
    sys.exit(0)
