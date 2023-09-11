/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

namespace chip {
namespace Crypto {

/**
 *  This function generates 64-bit unsigned random number.
 *
 * @return  64-bit unsigned random integer.
 *
 */
extern uint64_t GetRandU64();

/**
 *  This function generates 32-bit unsigned random number.
 *
 * @return  32-bit unsigned random integer.
 *
 */
extern uint32_t GetRandU32();

/**
 *  This function generates 16-bit unsigned random number.
 *
 * @return  16-bit unsigned random integer.
 *
 */
extern uint16_t GetRandU16();

/**
 *  This function generates 8-bit unsigned random number.
 *
 * @return  8-bit unsigned random integer.
 *
 */
extern uint8_t GetRandU8();

} // namespace Crypto
} // namespace chip
