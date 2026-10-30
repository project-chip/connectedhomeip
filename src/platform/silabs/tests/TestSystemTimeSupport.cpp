/*
 * Copyright (c) 2026 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <pw_unit_test/framework.h>
#include <system/SystemClock.h>

namespace chip::System::Clock {
class SilabsTestClockImpl : public ClockBase
{
public:
    Microseconds64 GetMonotonicMicroseconds64() override;
    Milliseconds64 GetMonotonicMilliseconds64() override;
    CHIP_ERROR GetClock_RealTime(Microseconds64 & time) override;
    CHIP_ERROR GetClock_RealTimeMS(Milliseconds64 & time) override;
    CHIP_ERROR SetClock_RealTime(Microseconds64 time) override;
};
} // namespace chip::System::Clock

// Compile the actual backend with isolated symbols alongside the host's clock.
#define ClockImpl SilabsTestClockImpl
#define gClockImpl gSilabsTestClockImpl
#define GetClock_Monotonic GetSilabsTestClock_Monotonic
#define GetClock_MonotonicMS GetSilabsTestClock_MonotonicMS
#define GetClock_MonotonicHiRes GetSilabsTestClock_MonotonicHiRes
#define InitClock_RealTime InitSilabsTestClock_RealTime
#include "../SystemTimeSupport.cpp"
#undef InitClock_RealTime
#undef GetClock_MonotonicHiRes
#undef GetClock_MonotonicMS
#undef GetClock_Monotonic
#undef gClockImpl
#undef ClockImpl

namespace MockSleeptimer {
uint32_t gCriticalDepth = 0;
uint64_t gTicks         = 0;
uint32_t gFrequency     = 32768;
} // namespace MockSleeptimer

uint32_t sl_sleeptimer_get_timer_frequency()
{
    return MockSleeptimer::gFrequency;
}

uint64_t sl_sleeptimer_get_tick_count64()
{
    return MockSleeptimer::gTicks;
}

namespace {
using namespace chip;
using namespace chip::System::Clock;
using namespace MockSleeptimer;

constexpr uint64_t kValidUTC = static_cast<uint64_t>(CHIP_SYSTEM_CONFIG_VALID_REAL_TIME_THRESHOLD) * kMicrosecondsPerSecond;

class TestSystemTimeSupport : public ::testing::Test
{
public:
    void SetUp() override
    {
        gTicks         = 0;
        gFrequency     = 32768;
        gCriticalDepth = 0;
        sBootTimeUS    = 0;
    }

    void TearDown() override { EXPECT_EQ(gCriticalDepth, 0u); }

    SilabsTestClockImpl clock;
};

TEST_F(TestSystemTimeSupport, StartupAndResetLeaveUTCUnsynchronized)
{
    ASSERT_EQ(InitSilabsTestClock_RealTime(), CHIP_NO_ERROR);
    Microseconds64 utc(123);
    Milliseconds64 utcMS(456);
    EXPECT_EQ(clock.GetClock_RealTime(utc), CHIP_ERROR_REAL_TIME_NOT_SYNCED);
    EXPECT_EQ(clock.GetClock_RealTimeMS(utcMS), CHIP_ERROR_REAL_TIME_NOT_SYNCED);
    EXPECT_EQ(utc.count(), 123u);
    EXPECT_EQ(utcMS.count(), 456u);

    ASSERT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC)), CHIP_NO_ERROR);
    gTicks = 32768;
    ASSERT_EQ(InitSilabsTestClock_RealTime(), CHIP_NO_ERROR);
    EXPECT_EQ(clock.GetClock_RealTime(utc), CHIP_ERROR_REAL_TIME_NOT_SYNCED);
    EXPECT_EQ(clock.GetClock_RealTimeMS(utcMS), CHIP_ERROR_REAL_TIME_NOT_SYNCED);
    EXPECT_EQ(clock.GetMonotonicMicroseconds64().count(), 1000000u);
}

TEST_F(TestSystemTimeSupport, UTCAdvancesAndAcceptsCorrections)
{
    gTicks = 32768 * 10;
    ASSERT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC)), CHIP_NO_ERROR);
    gTicks += 32768 * 60;
    Microseconds64 utc;
    Milliseconds64 utcMS;
    ASSERT_EQ(clock.GetClock_RealTime(utc), CHIP_NO_ERROR);
    ASSERT_EQ(clock.GetClock_RealTimeMS(utcMS), CHIP_NO_ERROR);
    EXPECT_EQ(utc.count(), kValidUTC + 60000000);
    EXPECT_EQ(utcMS.count(), (kValidUTC + 60000000) / kMicrosecondsPerMillisecond);

    ASSERT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC + 120000000)), CHIP_NO_ERROR);
    ASSERT_EQ(clock.GetClock_RealTime(utc), CHIP_NO_ERROR);
    EXPECT_EQ(utc.count(), kValidUTC + 120000000);
    ASSERT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC)), CHIP_NO_ERROR);
    gTicks += 32768;
    ASSERT_EQ(clock.GetClock_RealTime(utc), CHIP_NO_ERROR);
    EXPECT_EQ(utc.count(), kValidUTC + 1000000);
}

TEST_F(TestSystemTimeSupport, PlausibilityThresholdIsInclusive)
{
    EXPECT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC - 1)), CHIP_ERROR_INVALID_TIME);
    ASSERT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC)), CHIP_NO_ERROR);
    EXPECT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC - 1)), CHIP_ERROR_INVALID_TIME);
    Microseconds64 utc;
    ASSERT_EQ(clock.GetClock_RealTime(utc), CHIP_NO_ERROR);
    EXPECT_EQ(utc.count(), kValidUTC);
}

TEST_F(TestSystemTimeSupport, RejectsTimeAtOrBelowMonotonic)
{
    gFrequency = 1000000;
    gTicks     = kValidUTC + 1;
    EXPECT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC)), CHIP_ERROR_INVALID_TIME);
    EXPECT_EQ(clock.SetClock_RealTime(Microseconds64(gTicks)), CHIP_ERROR_INVALID_TIME);
    Microseconds64 utc;
    EXPECT_EQ(clock.GetClock_RealTime(utc), CHIP_ERROR_REAL_TIME_NOT_SYNCED);
}

TEST_F(TestSystemTimeSupport, FractionalTicksAndMillisecondsAreTruncated)
{
    gTicks = 33;
    EXPECT_EQ(clock.GetMonotonicMicroseconds64().count(), 1007u);
    EXPECT_EQ(clock.GetMonotonicMilliseconds64().count(), 1u);
    ASSERT_EQ(clock.SetClock_RealTime(Microseconds64(kValidUTC + 999)), CHIP_NO_ERROR);
    Milliseconds64 utc;
    ASSERT_EQ(clock.GetClock_RealTimeMS(utc), CHIP_NO_ERROR);
    EXPECT_EQ(utc.count(), kValidUTC / kMicrosecondsPerMillisecond);
}

TEST_F(TestSystemTimeSupport, LongUptimeDoesNotOverflowTickScaling)
{
    gTicks = uint64_t{ 1 } << 48;
    EXPECT_EQ(clock.GetMonotonicMicroseconds64().count(), 8589934592000000u);
}
} // namespace
