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

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <system/SystemConfig.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/TimeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <nlunit-test.h>
#include <system/SystemClock.h>

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

void TestRealClock(nlTestSuite * inSuite, void * inContext)
{
    Clock::MonotonicMilliseconds oldMilli = SystemClock().GetMonotonicMilliseconds();
    Clock::MonotonicMilliseconds newMilli = SystemClock().GetMonotonicMilliseconds();
    NL_TEST_ASSERT(inSuite, newMilli >= oldMilli);

    Clock::MonotonicMicroseconds oldMicro = SystemClock().GetMonotonicMicroseconds();
    Clock::MonotonicMicroseconds newMicro = SystemClock().GetMonotonicMicroseconds();
    NL_TEST_ASSERT(inSuite, newMicro >= oldMicro);

#if !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME &&                                                                                  \
    (CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME || CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS)
    constexpr int kDelayMilliseconds = 3;

#if CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME
    sys_msleep(kDelayMilliseconds);
#endif // CHIP_SYSTEM_CONFIG_USE_LWIP

#if CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS
    struct timespec delay = { 0, kDelayMilliseconds * chip::kNanosecondsPerMillisecond };
    while (nanosleep(&delay, &delay) == -1 && errno == EINTR)
    {
    }
#endif // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS

    newMilli = SystemClock().GetMonotonicMilliseconds();
    NL_TEST_ASSERT(inSuite, newMilli > oldMilli);

    newMicro = SystemClock().GetMonotonicMicroseconds();
    NL_TEST_ASSERT(inSuite, newMicro > oldMicro);

#endif // !CHIP_SYSTEM_CONFIG_PLATFORM_PROVIDES_TIME && (CHIP_SYSTEM_CONFIG_USE_LWIP_MONOTONIC_TIME ||
       // CHIP_SYSTEM_CONFIG_USE_POSIX_TIME_FUNCTS)
}

void TestMockClock(nlTestSuite * inSuite, void * inContext)
{
    class MockClock : public ClockBase
    {
    public:
        MonotonicMicroseconds GetMonotonicMicroseconds() override { return mTime * chip::kMicrosecondsPerMillisecond; }
        MonotonicMilliseconds GetMonotonicMilliseconds() override { return mTime; }
        MonotonicMilliseconds mTime = 0;
    };
    MockClock clock;

    ClockBase * savedRealClock = &SystemClock();
    SetSystemClockForTesting(&clock);

    NL_TEST_ASSERT(inSuite, SystemClock().GetMonotonicMilliseconds() == 0);
    NL_TEST_ASSERT(inSuite, SystemClock().GetMonotonicMicroseconds() == 0);

    clock.mTime = 1234;
    NL_TEST_ASSERT(inSuite, SystemClock().GetMonotonicMilliseconds() == 1234);
    NL_TEST_ASSERT(inSuite, SystemClock().GetMonotonicMicroseconds() == 1234 * chip::kMicrosecondsPerMillisecond);

    SetSystemClockForTesting(savedRealClock);
}

} // namespace

/**
 *   Test Suite. It lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestRealClock", TestRealClock),
    NL_TEST_DEF("TestMockClock", TestMockClock),
    NL_TEST_SENTINEL()
};
// clang-format on

int TestSystemClock(void)
{
    nlTestSuite theSuite = {
        "chip-timesource", &sTests[0], nullptr /* setup */, nullptr /* teardown */
    };

    // Run test suit againt one context.
    nlTestRunner(&theSuite, nullptr /* context */);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestSystemClock)
