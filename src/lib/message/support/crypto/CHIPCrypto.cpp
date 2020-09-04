/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2013-2017 Nest Labs, Inc.
 *    All rights reserved.
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
 *      This file implements general purpose cryptographic functions for the Message layer.
 *
 */

#include "CHIPCrypto.h"
#include <string.h>

namespace chip {
namespace Crypto {

/**
 * Compares the first `len` bytes of memory area `buf1` and memory area `buf2`.
 *
 * The time taken by this function is independent of the data in memory areas `buf1` and `buf2`.
 *
 * @param[in]  buf1   Pointer to a memory block.
 *
 * @param[in]  buf2   Pointer to a memory block.
 *
 * @param[in]  len    Size of memory area to compare in bytes.
 *
 * @retval  true      if first \c len bytes of memory area \c buf1 and \c buf2 are equal.
 * @retval  false     otherwise.
 *
 */
bool ConstantTimeCompare(const uint8_t * buf1, const uint8_t * buf2, uint16_t len)
{
    uint8_t c = 0;
    for (uint16_t i = 0; i < len; i++)
        c |= buf1[i] ^ buf2[i];
    return c == 0;
}

} // namespace Crypto
} // namespace chip
