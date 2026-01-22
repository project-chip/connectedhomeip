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

#include <lib/support/TimerDelegateMock.h>

#include <pw_unit_test/framework.h>

#include <system/RAIIMockClock.h>

using namespace chip;

using namespace chip::app;

using namespace chip::app::Clusters;

using namespace chip::app::Clusters::LevelControl;

constexpr EndpointId kTestEndpointId = 1;

class MockLevelControlDelegate : public LevelControlDelegate

{

public:
    void OnOptionsChanged(BitMask<OptionsBitmap> options) override { mOptions = options; }

    void OnOnLevelChanged(DataModel::Nullable<uint8_t> onLevel) override { mOnLevel = onLevel; }

    void OnDefaultMoveRateChanged(DataModel::Nullable<uint8_t> defaultMoveRate) override { mDefaultMoveRate = defaultMoveRate; }

    void OnLevelChanged(uint8_t level) override
    {
        mLevel              = level;
        mLevelChangedCalled = true;
    }

    // OnOff methods

    void SetOnOff(bool on) override
    {
        mSetOnOffCalled = true;
        mOn             = on;
    }

    bool GetOnOff() override { return mOn; }

    BitMask<OptionsBitmap> mOptions;

    DataModel::Nullable<uint8_t> mOnLevel;

    DataModel::Nullable<uint8_t> mDefaultMoveRate;

    bool mLevelChangedCalled = false;

    uint8_t mLevel = 0;

    bool mSetOnOffCalled = false;

    bool mOn = false;
};

struct LevelControlTestBase : public ::testing::Test

{

    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    chip::TimerDelegateMock mockTimer;

    chip::System::Clock::Internal::RAIIMockClock mockClock;

    MockLevelControlDelegate mockDelegate;
};
