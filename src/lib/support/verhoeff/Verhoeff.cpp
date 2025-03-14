/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 *    @file
 *      This file implements an object for the core Verhoeff
 *      check-digit algorithm operations.
 *
 */

#include "Verhoeff.h"

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
