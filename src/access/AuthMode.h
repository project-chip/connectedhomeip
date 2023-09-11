/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace chip {
namespace Access {

// Using bitfield values so auth mode and privilege set can be stored together.
// Auth mode should have only one value expressed, which should not be None.
enum class AuthMode : uint8_t
{
    kNone  = 0,
    kPase  = 1 << 5,
    kCase  = 1 << 6,
    kGroup = 1 << 7
};

} // namespace Access
} // namespace chip
