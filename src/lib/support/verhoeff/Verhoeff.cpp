/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
/**
 *    @file
 *      This file implements an object for the core Verhoeff
 *      check-digit algorithm operations.
 *
 */

#include "Verhoeff.h"

int Verhoeff::DihedralMultiply(int x, int y, int n)
{
    int n2 = n * 2;

    x = x % n2;
    y = y % n2;

    if (x < n)
    {
        if (y < n)
            return (x + y) % n;

        return ((x + (y - n)) % n) + n;
    }

    if (y < n)
        return ((n + (x - n) - y) % n) + n;

    return (n + (x - n) - (y - n)) % n;
}

int Verhoeff::DihedralInvert(int val, int n)
{
    if (val > 0 && val < n)
        return n - val;
    return val;
}

int Verhoeff::Permute(int val, const uint8_t * permTable, int permTableLen, uint64_t iterCount)
{
    val = val % permTableLen;
    if (iterCount == 0)
        return val;
    return Permute(permTable[val], permTable, permTableLen, iterCount - 1);
}
