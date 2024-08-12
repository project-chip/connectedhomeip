/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *      This file implements a unit test suite for the Configuration Manager
 *      code functionality.
 *
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pw_unit_test/framework.h>

#include <app/FailSafeContext.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::Logging;
using namespace chip::DeviceLayer;

namespace {

constexpr FabricIndex kTestAccessingFabricIndex1 = 1;
constexpr FabricIndex kTestAccessingFabricIndex2 = 2;

class TestFailSafeContext : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(PlatformMgr().InitChipStack(), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }
};

// =================================
//      Unit tests
// =================================

TEST_F(TestFailSafeContext, TestFailSafeContext_ArmFailSafe)
{
    chip::app::FailSafeContext failSafeContext;

    EXPECT_EQ(failSafeContext.ArmFailSafe(kTestAccessingFabricIndex1, System::Clock::Seconds16(1)), CHIP_NO_ERROR);
    EXPECT_TRUE(failSafeContext.IsFailSafeArmed());
    EXPECT_EQ(failSafeContext.GetFabricIndex(), kTestAccessingFabricIndex1);
    EXPECT_TRUE(failSafeContext.IsFailSafeArmed(kTestAccessingFabricIndex1));
    EXPECT_FALSE(failSafeContext.IsFailSafeArmed(kTestAccessingFabricIndex2));

    failSafeContext.DisarmFailSafe();
    EXPECT_FALSE(failSafeContext.IsFailSafeArmed());
}

TEST_F(TestFailSafeContext, TestFailSafeContext_NocCommandInvoked)
{
    chip::app::FailSafeContext failSafeContext;

    EXPECT_EQ(failSafeContext.ArmFailSafe(kTestAccessingFabricIndex1, System::Clock::Seconds16(1)), CHIP_NO_ERROR);
    EXPECT_EQ(failSafeContext.GetFabricIndex(), kTestAccessingFabricIndex1);

    failSafeContext.SetAddNocCommandInvoked(kTestAccessingFabricIndex2);
    EXPECT_TRUE(failSafeContext.NocCommandHasBeenInvoked());
    EXPECT_TRUE(failSafeContext.AddNocCommandHasBeenInvoked());
    EXPECT_EQ(failSafeContext.GetFabricIndex(), kTestAccessingFabricIndex2);

    failSafeContext.SetUpdateNocCommandInvoked();
    EXPECT_TRUE(failSafeContext.NocCommandHasBeenInvoked());
    EXPECT_TRUE(failSafeContext.UpdateNocCommandHasBeenInvoked());

    failSafeContext.DisarmFailSafe();
}

} // namespace
