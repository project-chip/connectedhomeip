/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

namespace chip {
namespace test_utils {

void SleepMicros(uint64_t microsecs);
void SleepMillis(uint64_t millisecs);
uint64_t TimeMonotonicMillis();

} // namespace test_utils
} // namespace chip
