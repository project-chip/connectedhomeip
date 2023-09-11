/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
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

#include <stdint.h>
#include <stdlib.h>

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace Crypto {

uint64_t GetRandU64()
{
    uint64_t tmp = 0;
    VerifyOrDie(CHIP_NO_ERROR == DRBG_get_bytes(reinterpret_cast<uint8_t *>(&tmp), sizeof(tmp)));
    return tmp;
}

uint32_t GetRandU32()
{
    uint32_t tmp = 0;
    VerifyOrDie(CHIP_NO_ERROR == DRBG_get_bytes(reinterpret_cast<uint8_t *>(&tmp), sizeof(tmp)));
    return tmp;
}

uint16_t GetRandU16()
{
    uint16_t tmp = 0;
    VerifyOrDie(CHIP_NO_ERROR == DRBG_get_bytes(reinterpret_cast<uint8_t *>(&tmp), sizeof(tmp)));
    return tmp;
}

uint8_t GetRandU8()
{
    uint8_t tmp = 0;
    VerifyOrDie(CHIP_NO_ERROR == DRBG_get_bytes(&tmp, sizeof(tmp)));
    return tmp;
}

} // namespace Crypto
} // namespace chip
