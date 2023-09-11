/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2013-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file implements utility functions for deriving a fibonacci
 *      number from an index.
 *
 */

#pragma once

#include <cstdint>

namespace chip {

/**
 *  This function generates 32-bit Fibonacci number
 *  for a given 32 bit index. The index boundary is 47, then it would overflow.
 *
 * @return  32-bit unsigned fibonacci number.
 *
 */
extern uint32_t GetFibonacciForIndex(uint32_t index);

} // namespace chip
