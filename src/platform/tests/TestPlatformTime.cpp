/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file implements a unit test suite for the Platform Time
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>
#include <system/SystemClock.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::System;
using namespace chip::System::Clock::Literals;

#ifdef CONFIG_WIDER_SLEEP_MARGIN
// SleepMillis can undersleep by as much as 10ms on esp32-qemu.
constexpr Clock::Milliseconds64 kTestTimeMarginMs = 10_ms64;
#else
constexpr Clock::Milliseconds64 kTestTimeMarginMs = 2_ms64;
#endif
constexpr Clock::Microseconds64 kTestTimeMarginUs = 500_us64;

// =================================
//      Unit tests
// =================================

TEST(TestDevice, GetMonotonicMicroseconds)
{
    static const Clock::Microseconds64 kTestVectorSystemTimeUs[] = {
        600_us64,
        900_us64,
        1500_us64,
    };
    int numOfTestsRan                      = 0;
    constexpr Clock::Microseconds64 margin = kTestTimeMarginUs;

    for (const Clock::Microseconds64 & Tdelay : kTestVectorSystemTimeUs)
    {
        const Clock::Microseconds64 Tstart = System::SystemClock().GetMonotonicMicroseconds64();

        chip::test_utils::SleepMicros(Tdelay.count());

        const Clock::Microseconds64 Tend   = System::SystemClock().GetMonotonicMicroseconds64();
        const Clock::Microseconds64 Tdelta = Tend - Tstart;

        ChipLogProgress(DeviceLayer,
                        "Start=0x" ChipLogFormatX64 " End=0x" ChipLogFormatX64 " Delta=0x" ChipLogFormatX64
                        " Expected=0x" ChipLogFormatX64,
                        ChipLogValueX64(Tstart.count()), ChipLogValueX64(Tend.count()), ChipLogValueX64(Tdelta.count()),
                        ChipLogValueX64(Tdelay.count()));

        // verify that timers don't fire early
        EXPECT_GT(Tdelta, (Tdelay - margin));
        // verify they're not too late
        //        EXPECT_LT(Tdelta, (Tdelay + margin));
        numOfTestsRan++;
    }
    EXPECT_GT(numOfTestsRan, 0);
}

TEST(TestDevice, GetMonotonicMilliseconds)
{
    static const System::Clock::Milliseconds64 kTestVectorSystemTimeMs[] = {
        10_ms64,
        100_ms64,
        250_ms64,
    };
    int numOfTestsRan                      = 0;
    constexpr Clock::Milliseconds64 margin = kTestTimeMarginMs;

    for (const Clock::Milliseconds64 & Tdelay : kTestVectorSystemTimeMs)
    {
        const Clock::Milliseconds64 Tstart = System::SystemClock().GetMonotonicMilliseconds64();

        chip::test_utils::SleepMillis(Tdelay.count());

        const Clock::Milliseconds64 Tend   = System::SystemClock().GetMonotonicMilliseconds64();
        const Clock::Milliseconds64 Tdelta = Tend - Tstart;

        ChipLogProgress(DeviceLayer,
                        "Start=0x" ChipLogFormatX64 " End=0x" ChipLogFormatX64 " Delta=0x" ChipLogFormatX64
                        " Expected=0x" ChipLogFormatX64,
                        ChipLogValueX64(Tstart.count()), ChipLogValueX64(Tend.count()), ChipLogValueX64(Tdelta.count()),
                        ChipLogValueX64(Tdelay.count()));

        // verify that timers don't fire early
        EXPECT_GT(Tdelta, (Tdelay - margin));
        // verify they're not too late
        //        EXPECT_LT(Tdelta, (Tdelay + margin));
        numOfTestsRan++;
    }
    EXPECT_GT(numOfTestsRan, 0);
}
