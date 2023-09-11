/*
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <system/SystemClock.h>
#include <transport/Session.h>

namespace chip {
namespace app {

// This is an expected upper bound of time-to-first-byte for IM transactions, the actual processing time should never exceed this
// value unless specified elsewhere (e.g. async command handling for some clusters).
static constexpr System::Clock::Timeout kExpectedIMProcessingTime = System::Clock::Seconds16(2);

} // namespace app
} // namespace chip
