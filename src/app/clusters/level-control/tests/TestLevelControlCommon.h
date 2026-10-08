/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#pragma once

#include <app/clusters/level-control/LevelControlCluster.h>
#include <app/clusters/level-control/LevelControlDelegate.h>
#include <app/clusters/on-off-server/OnOffCluster.h>
#include <lib/support/TimerDelegateMock.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

constexpr chip::EndpointId kTestEndpointId = 1;

class MockLevelControlDelegate : public chip::app::Clusters::LevelControlDelegate
{
public:
    void OnOptionsChanged(chip::BitMask<chip::app::Clusters::LevelControl::OptionsBitmap> options) override { mOptions = options; }
    void OnOnLevelChanged(chip::app::DataModel::Nullable<uint8_t> onLevel) override { mOnLevel = onLevel; }
    void OnDefaultMoveRateChanged(chip::app::DataModel::Nullable<uint8_t> defaultMoveRate) override
    {
        mDefaultMoveRate = defaultMoveRate;
    }
    void OnLevelChanged(uint8_t level) override
    {
        mLevel              = level;
        mLevelChangedCalled = true;
    }

    chip::BitMask<chip::app::Clusters::LevelControl::OptionsBitmap> mOptions;
    chip::app::DataModel::Nullable<uint8_t> mOnLevel;
    chip::app::DataModel::Nullable<uint8_t> mDefaultMoveRate;

    bool mLevelChangedCalled = false;
    uint8_t mLevel           = 0;
};

struct LevelControlTestBase : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    chip::TimerDelegateMock mockTimer;
    chip::System::Clock::Internal::RAIIMockClock mockClock;
    MockLevelControlDelegate mockDelegate;

    void AdvanceClock(chip::System::Clock::Milliseconds64 duration)
    {
        mockClock.AdvanceMonotonic(duration);
        mockTimer.AdvanceClock(duration);
    }
};
