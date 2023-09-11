/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

namespace chip {

// TODO: Consider making this a class and the various utility methods static
// methods.
using PasscodeId = uint16_t;

constexpr PasscodeId kDefaultCommissioningPasscodeId = 0;

} // namespace chip
