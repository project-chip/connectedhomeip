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
 *      This file implements utility functions for deriving random integers.
 *
 *  @note These utility functions do not generate cryptographically strong
 *        random number. To get cryptographically strong random data use
 *        chip::Crypto::DRBG_get_bytes().
 *
 */

#include "RandUtils.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

namespace chip {

/**
 *  @def NORMALIZED_RAND_RANGE(reqRange)
 *
 *  This macro calculates normalized range for the output of rand() function
 *  based on the requested random range [0, reqRange].
 *
 *  @note
 *  For most of the platforms we support, RAND_MAX is usually 0x7FFF or 0x7FFFFFFF.
 *  In these cases normalization for ranges [0, UINT8_MAX] or [0, UINT16_MAX]
 *  is not needed.
 *
 *  @param[in] reqRange  The requested random number range.
 *
 *  @return              normalized random range.
 *
 */
#define NORMALIZED_RAND_RANGE(reqRange) (((reqRange) + 1) * ((RAND_MAX + 1) / ((reqRange) + 1)))

#if RAND_MAX < UINT8_MAX
#error "RAND_MAX value is too small. RandUtils functions assume that RAND_MAX is greater or equal to UINT8_MAX."
#endif

uint64_t GetRandU64()
{
    // rand() returns int, which is always smaller than the size of uint64_t
    // and rand() cannot be used directly to generate random uint64_t number.
    return static_cast<uint64_t>(GetRandU32()) ^ (static_cast<uint64_t>(GetRandU32()) << (sizeof(uint32_t) * CHAR_BIT));
}

uint32_t GetRandU32()
{
    // Check if (RAND_MAX == UINT32_MAX) but it is unlikely because rand() returns signed int,
    // which maximum possible value is 0x7FFFFFFF (smaller that UINT32_MAX = 0xFFFFFFFF).
#if RAND_MAX == UINT32_MAX
    return static_cast<uint32_t>(rand());
#else
    return static_cast<uint32_t>(GetRandU16()) ^ (static_cast<uint32_t>(GetRandU16()) << (sizeof(uint16_t) * CHAR_BIT));
#endif
}

uint16_t GetRandU16()
{
#if RAND_MAX >= UINT16_MAX
#if (RAND_MAX == INT_MAX) || (RAND_MAX == NORMALIZED_RAND_RANGE(UINT16_MAX))
    // rand() random output range normalization is not needed.
    return static_cast<uint16_t>(rand());
#else
    // Otherwise, Normilize the output range of rand() and reject rand() outputs outside of that range.
    while (true)
    {
        int r = rand();
        if (r < NORMALIZED_RAND_RANGE(UINT16_MAX))
            return static_cast<uint16_t>(r);
    }
#endif
#else
    return static_cast<uint16_t>(GetRandU8()) ^ (static_cast<uint16_t>(GetRandU8()) << CHAR_BIT);
#endif
}

uint8_t GetRandU8()
{
#if (RAND_MAX == INT_MAX) || (RAND_MAX == NORMALIZED_RAND_RANGE(UINT8_MAX))
    // rand() random output range normalization is not needed.
    return static_cast<uint8_t>(rand());
#else
    // Otherwise, Normilize the output range of rand() and reject rand() outputs outside of that range.
    while (true)
    {
        int r = rand();
        if (r < NORMALIZED_RAND_RANGE(UINT8_MAX))
            return static_cast<uint8_t>(r);
    }
#endif
}

} // namespace chip
