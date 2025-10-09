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
 *      This file implements a unit test suite for the Platform Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <atomic>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestUtils.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::Inet;
using namespace chip::DeviceLayer;

// =================================
//      Unit tests
// =================================

class TestPlatformMgr : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        CHIP_ERROR error = chip::Platform::MemoryInit();
        EXPECT_EQ(error, CHIP_NO_ERROR);

        // Set up a fake commissionable data provider since required by internals of several
        // Device/SystemLayer components.
        static chip::DeviceLayer::TestOnlyCommissionableDataProvider commissionable_data_provider;
        chip::DeviceLayer::SetCommissionableDataProvider(&commissionable_data_provider);
    }

    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

TEST_F(TestPlatformMgr, InitShutdown)
{
    CHIP_ERROR err = PlatformMgr().InitChipStack();
    EXPECT_EQ(err, CHIP_NO_ERROR);

    PlatformMgr().Shutdown();
}

TEST_F(TestPlatformMgr, BasicEventLoopTask)
{
    std::atomic<int> counterRun{ 0 };

    EXPECT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

    // Start/stop the event loop task a few times.
    for (size_t i = 0; i < 3; i++)
    {
        EXPECT_EQ(PlatformMgr().StartEventLoopTask(), CHIP_NO_ERROR);

        std::atomic<int> counterSync{ 2 };

        // Verify that the event loop will not exit until we tell it to by
        // scheduling few lambdas (for the test to pass, the event loop will
        // have to process more than one event).
        DeviceLayer::SystemLayer().ScheduleLambda([&]() {
            counterRun++;
            counterSync--;
        });

        // Sleep for a short time to allow the event loop to process the
        // scheduled event and go to idle state. Without this sleep, the
        // event loop may process both scheduled lambdas during single
        // iteration of the event loop which would defeat the purpose of
        // this test on POSIX platforms where the event loop is implemented
        // using a "do { ... } while (shouldRun)" construct.
        chip::test_utils::SleepMillis(10);

        DeviceLayer::SystemLayer().ScheduleLambda([&]() {
            counterRun++;
            counterSync--;
        });

        // Wait for the event loop to process the scheduled events.
        // Note, that we can not use any synchronization primitives like
        // condition variables or barriers, because the test has to compile
        // on all platforms. Instead we use a busy loop with a timeout.
        for (size_t t = 0; counterSync != 0 && t < 1000; t++)
            chip::test_utils::SleepMillis(1);

        EXPECT_EQ(PlatformMgr().StopEventLoopTask(), CHIP_NO_ERROR);

        // Sleep for a short time to allow the event loop to stop.
        // Note, in some platform implementations the event loop thread
        // is self-terminating. We need time to process the stopping event
        // inside event loop.
        chip::test_utils::SleepMillis(10);
    }

    EXPECT_EQ(counterRun, (3 * 2));

    PlatformMgr().Shutdown();
}

static bool stopRan;
static CHIP_ERROR stopResult = CHIP_NO_ERROR;

static void StopTheLoop(intptr_t)
{
    // Testing the return value here would involve multi-threaded access to the
    // nlTestSuite, and it's not clear whether that's OK.
    stopRan    = true;
    stopResult = PlatformMgr().StopEventLoopTask();
}

TEST_F(TestPlatformMgr, BasicRunEventLoop)
{
    stopRan = false;

    EXPECT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

    PlatformMgr().ScheduleWork(StopTheLoop);

    EXPECT_FALSE(stopRan);
    PlatformMgr().RunEventLoop();
    EXPECT_TRUE(stopRan);
    EXPECT_EQ(stopResult, CHIP_NO_ERROR);

    PlatformMgr().Shutdown();
}

static bool sleepRan;

static void SleepSome(intptr_t)
{
    chip::test_utils::SleepMillis(1000);
    sleepRan = true;
}

TEST_F(TestPlatformMgr, RunEventLoopTwoTasks)
{
    stopRan  = false;
    sleepRan = false;

    EXPECT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

    PlatformMgr().ScheduleWork(SleepSome);
    PlatformMgr().ScheduleWork(StopTheLoop);

    EXPECT_FALSE(stopRan);
    EXPECT_FALSE(sleepRan);
    PlatformMgr().RunEventLoop();
    EXPECT_TRUE(stopRan);
    EXPECT_TRUE(sleepRan);

    PlatformMgr().Shutdown();
}

TEST_F(TestPlatformMgr, RunEventLoopStopBeforeSleep)
{
    stopRan  = false;
    sleepRan = false;

    EXPECT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

    PlatformMgr().ScheduleWork([](intptr_t arg) {
        // Ensure that we don't proceed after stopping until the sleep is done too.
        StopTheLoop(arg);
        SleepSome(arg);
    });

    EXPECT_FALSE(stopRan);
    EXPECT_FALSE(sleepRan);
    PlatformMgr().RunEventLoop();
    EXPECT_TRUE(stopRan);
    EXPECT_TRUE(sleepRan);

    PlatformMgr().Shutdown();
}

TEST_F(TestPlatformMgr, TryLockChipStack)
{
    EXPECT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

    bool locked = PlatformMgr().TryLockChipStack();
    EXPECT_EQ(locked, !CHIP_SYSTEM_CONFIG_NO_LOCKING);

    if (locked)
        PlatformMgr().UnlockChipStack();

    PlatformMgr().Shutdown();
}

static int sEventRecieved = 0;

void DeviceEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    EXPECT_EQ(arg, 12345);
    sEventRecieved++;
}

TEST_F(TestPlatformMgr, AddEventHandler)
{
    sEventRecieved = 0;
    EXPECT_EQ(PlatformMgr().AddEventHandler(DeviceEventHandler, 12345), CHIP_NO_ERROR);
}

class MockSystemLayer : public System::LayerImpl
{
public:
    CHIP_ERROR StartTimer(System::Clock::Timeout aDelay, System::TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_APPLICATION_ERROR(1);
    }
    CHIP_ERROR ScheduleWork(System::TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_APPLICATION_ERROR(2);
    }
};

TEST_F(TestPlatformMgr, MockSystemLayerTest)
{
    MockSystemLayer systemLayer;

    DeviceLayer::SetSystemLayerForTesting(&systemLayer);
    EXPECT_EQ(&DeviceLayer::SystemLayer(), static_cast<chip::System::Layer *>(&systemLayer));

    CHIP_ERROR err = PlatformMgr().InitChipStack();
    EXPECT_EQ(err, CHIP_NO_ERROR);
    EXPECT_EQ(&DeviceLayer::SystemLayer(), static_cast<chip::System::Layer *>(&systemLayer));

    EXPECT_EQ(DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::kZero, nullptr, nullptr), CHIP_APPLICATION_ERROR(1));
    EXPECT_EQ(DeviceLayer::SystemLayer().ScheduleWork(nullptr, nullptr), CHIP_APPLICATION_ERROR(2));

    PlatformMgr().Shutdown();

    DeviceLayer::SetSystemLayerForTesting(nullptr);
}
