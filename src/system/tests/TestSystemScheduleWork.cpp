/*
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
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemConfig.h>

class TestSystemScheduleWork : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }
};

static void IncrementIntCounter(chip::System::Layer *, void * state)
{
    ++(*static_cast<int *>(state));
}

static void StopEventLoop(chip::System::Layer *, void *)
{
    chip::DeviceLayer::PlatformMgr().StopEventLoopTask();
}

TEST_F(TestSystemScheduleWork, CheckScheduleWork)
{
    int callCount = 0;
    EXPECT_EQ(chip::DeviceLayer::SystemLayer().ScheduleWork(IncrementIntCounter, &callCount), CHIP_NO_ERROR);
    EXPECT_EQ(chip::DeviceLayer::SystemLayer().ScheduleWork(IncrementIntCounter, &callCount), CHIP_NO_ERROR);
    EXPECT_EQ(chip::DeviceLayer::SystemLayer().ScheduleWork(StopEventLoop, nullptr), CHIP_NO_ERROR);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    EXPECT_EQ(callCount, 2);
}
