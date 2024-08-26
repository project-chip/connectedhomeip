/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <pw_unit_test/framework.h>

#include <lib/core/ErrorStr.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimeUtils.h>
#include <system/SystemClock.h>
#include <system/SystemConfig.h>

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

#if CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
#include <lwip/sys.h>
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
#include <errno.h>
#include <time.h>
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME

using namespace chip::System;

namespace {

TEST(TestSystemClock, TestRealClock)
{
    Clock::Milliseconds64 oldMilli = SystemClock().GetMonotonicMilliseconds64();
    Clock::Milliseconds64 newMilli = SystemClock().GetMonotonicMilliseconds64();
    EXPECT_GE(newMilli, oldMilli);

    Clock::Microseconds64 oldMicro = SystemClock().GetMonotonicMicroseconds64();
    Clock::Microseconds64 newMicro = SystemClock().GetMonotonicMicroseconds64();
    EXPECT_GE(newMicro, oldMicro);

    Clock::Microseconds64::rep microseconds = newMicro.count();
    EXPECT_EQ((microseconds & 0x8000'0000'0000'0000), 0UL);

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME &&                                                                                  \
    (CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME || CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS)
    constexpr int kDelayMilliseconds = 3;

#if CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
    sys_msleep(kDelayMilliseconds);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
    struct timespec delay = { 0, kDelayMilliseconds * chip::kNanosecondsPerMillisecond };
    while (nanosleep(&delay, &delay) == -1 && errno == EINTR)
        continue;
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

    newMilli = SystemClock().GetMonotonicMilliseconds64();
    EXPECT_GT(newMilli, oldMilli);

    newMicro = SystemClock().GetMonotonicMicroseconds64();
    EXPECT_GT(newMicro, oldMicro);

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME &&
       // (CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME || CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS)
}

TEST(TestSystemClock, TestMockClock)
{
    Clock::Internal::MockClock clock;

    Clock::ClockBase * savedRealClock = &SystemClock();
    Clock::Internal::SetSystemClockForTesting(&clock);

    EXPECT_EQ(SystemClock().GetMonotonicMilliseconds64(), Clock::kZero);
    EXPECT_EQ(SystemClock().GetMonotonicMicroseconds64(), Clock::kZero);

    constexpr Clock::Milliseconds64 k1234 = Clock::Milliseconds64(1234);
    clock.SetMonotonic(k1234);
    EXPECT_EQ(SystemClock().GetMonotonicMilliseconds64(), k1234);
    EXPECT_EQ(SystemClock().GetMonotonicMicroseconds64(), k1234);

    Clock::Internal::SetSystemClockForTesting(savedRealClock);
}

} // namespace
