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

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/level-control/level-control.h> // nogncheck
#include <app/server-cluster/testing/MockCommandHandler.h>
#include <app/tests/test-ember-api.h>
#include <app/util/mock/Functions.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/StatusCode.h>
#include <pw_unit_test/framework.h>
#include <system/RAIIMockClock.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kEndpoint = 0;
uint8_t gMinLevel;
uint8_t gMaxLevel;
DataModel::Nullable<uint8_t> gCurrentLevel;
uint16_t gRemainingTime;
bool gOn;
MarkAttributeDirty gLevelDirty;
Status gLimitReadStatus;
Status gLevelWriteStatus;
PumpConfigurationAndControl::OperationModeEnum gOperationMode;

const Testing::MockNodeConfig kNodeConfig({
    Testing::MockEndpointConfig(
        kEndpoint,
        {
            Testing::MockClusterConfig(LevelControl::Id,
                                       {
                                           Testing::MockAttributeConfig(LevelControl::Attributes::RemainingTime::Id),
                                           Testing::MockAttributeConfig(LevelControl::Attributes::Options::Id),
                                       }),
            Testing::MockClusterConfig(OnOff::Id),
            Testing::MockClusterConfig(PumpConfigurationAndControl::Id,
                                       {
                                           Testing::MockAttributeConfig(PumpConfigurationAndControl::Attributes::ControlMode::Id),
                                       }),
        }),
});

class TransitionTimer : public System::LayerImpl
{
public:
    CriticalFailure StartTimer(System::Clock::Timeout, System::TimerCompleteCallback callback, void * context) override
    {
        mCallback = callback;
        mContext  = context;
        return CHIP_NO_ERROR;
    }

    void CancelTimer(System::TimerCompleteCallback callback, void * context) override
    {
        if (mCallback == callback && mContext == context)
        {
            mCallback = nullptr;
        }
    }

    bool HasTimer() const { return mCallback != nullptr; }

    void Fire()
    {
        ASSERT_TRUE(HasTimer());
        auto callback = mCallback;
        mCallback     = nullptr;
        callback(this, mContext);
    }

private:
    System::TimerCompleteCallback mCallback = nullptr;
    void * mContext                         = nullptr;
};

class TestPumpLevelTransition : public ::testing::Test
{
protected:
    void SetUp() override
    {
        gMinLevel         = 1;
        gMaxLevel         = 254;
        gCurrentLevel     = DataModel::NullNullable;
        gRemainingTime    = 0;
        gOn               = true;
        gLevelDirty       = MarkAttributeDirty::kNo;
        gLimitReadStatus  = Status::Success;
        gLevelWriteStatus = Status::Success;
        gOperationMode    = PumpConfigurationAndControl::OperationModeEnum::kNormal;
        Testing::SetMockNodeConfig(kNodeConfig);
        Testing::numEndpoints = 1;
        DeviceLayer::SetSystemLayerForTesting(&mTimer);
        emberAfLevelControlClusterServerInitCallback(kEndpoint);
        // Initialize the reporting cache through an existing command so these tests also build before the fix.
        LevelControl::Commands::MoveToLevel::DecodableType command;
        command.level          = 128;
        command.transitionTime = DataModel::MakeNullable<uint16_t>(0);
        ASSERT_EQ(LevelControlServer::MoveToLevel(kEndpoint, command), Status::Success);
        mTimer.Fire();
        ASSERT_FALSE(HasFailure());
        ASSERT_EQ(gCurrentLevel.Value(), 128u);
        ASSERT_FALSE(mTimer.HasTimer());
    }

    void TearDown() override
    {
        MatterLevelControlClusterServerShutdownCallback(kEndpoint);
        DeviceLayer::SetSystemLayerForTesting(nullptr);
        Testing::numEndpoints = 0;
        Testing::ResetMockNodeConfig();
    }

    void StartTransition(uint8_t target)
    {
        LevelControl::Commands::MoveToLevel::DecodableType command;
        command.level          = target;
        command.transitionTime = DataModel::MakeNullable<uint16_t>(100);
        ASSERT_EQ(LevelControlServer::MoveToLevel(kEndpoint, command), Status::Success);
        ASSERT_TRUE(mTimer.HasTimer());
        mClock.AdvanceMonotonic(System::Clock::Milliseconds64(100));
        mTimer.Fire();
        ASSERT_TRUE(mTimer.HasTimer());
        ASSERT_GT(gRemainingTime, 0);
    }

    void ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum mode)
    {
        gOperationMode = mode;
        MatterPumpConfigurationAndControlClusterServerAttributeChangedCallback(ConcreteAttributePath(
            kEndpoint, PumpConfigurationAndControl::Id, PumpConfigurationAndControl::Attributes::OperationMode::Id));
    }

    TransitionTimer mTimer;
    System::Clock::Internal::RAIIMockClock mClock;
};

} // namespace

bool emberAfContainsServer(EndpointId endpoint, ClusterId cluster)
{
    return kNodeConfig.clusterByIds(endpoint, cluster) != nullptr;
}

namespace chip::app::Clusters::LevelControl::Attributes {

Status CurrentLevel::Get(EndpointId, DataModel::Nullable<uint8_t> & value)
{
    value = gCurrentLevel;
    return Status::Success;
}

Status CurrentLevel::Set(EndpointId, const DataModel::Nullable<uint8_t> & value, MarkAttributeDirty dirty)
{
    if (gLevelWriteStatus == Status::Success)
    {
        gCurrentLevel = value;
        gLevelDirty   = dirty;
    }
    return gLevelWriteStatus;
}

Status CurrentLevel::Set(EndpointId endpoint, uint8_t value)
{
    return Set(endpoint, DataModel::MakeNullable(value), MarkAttributeDirty::kYes);
}

Status MinLevel::Get(EndpointId, uint8_t * value)
{
    *value = gMinLevel;
    return gLimitReadStatus;
}

Status MaxLevel::Get(EndpointId, uint8_t * value)
{
    *value = gMaxLevel;
    return gLimitReadStatus;
}

Status RemainingTime::Set(EndpointId, uint16_t value, MarkAttributeDirty)
{
    gRemainingTime = value;
    return Status::Success;
}

Status FeatureMap::Get(EndpointId, uint32_t * value)
{
    *value = 0;
    return Status::Success;
}

Status Options::Get(EndpointId, BitMask<OptionsBitmap> * value)
{
    *value = BitMask<OptionsBitmap>();
    return Status::Success;
}

Status OnOffTransitionTime::Get(EndpointId, uint16_t * value)
{
    *value = 0;
    return Status::Success;
}

} // namespace chip::app::Clusters::LevelControl::Attributes

namespace chip::app::Clusters::OnOff::Attributes::OnOff {

Status Get(EndpointId, bool * value)
{
    *value = gOn;
    return Status::Success;
}

} // namespace chip::app::Clusters::OnOff::Attributes::OnOff

namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes {

Status OperationMode::Get(EndpointId, OperationModeEnum * value)
{
    *value = gOperationMode;
    return Status::Success;
}

Status ControlMode::Get(EndpointId, ControlModeEnum * value)
{
    *value = ControlModeEnum::kConstantSpeed;
    return Status::Success;
}

Status PumpStatus::Get(EndpointId, BitMask<PumpStatusBitmap> * value)
{
    *value = BitMask<PumpStatusBitmap>();
    return Status::Success;
}

Status PumpStatus::Set(EndpointId, BitMask<PumpStatusBitmap>)
{
    return Status::Success;
}

Status EffectiveOperationMode::Set(EndpointId, OperationModeEnum)
{
    return Status::Success;
}

Status EffectiveControlMode::Set(EndpointId, ControlModeEnum)
{
    return Status::Success;
}

} // namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes

namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes::FeatureMap {

Status Get(EndpointId, uint32_t * value)
{
    *value = 0;
    return Status::Success;
}

} // namespace chip::app::Clusters::PumpConfigurationAndControl::Attributes::FeatureMap

namespace {

TEST(TestPumpConfigurationAndControlCluster, RejectsUndefinedControlModeWithinAttributeRange)
{
    uint8_t value = 6;
    ConcreteAttributePath path(1, PumpConfigurationAndControl::Id, PumpConfigurationAndControl::Attributes::ControlMode::Id);

    EXPECT_EQ(MatterPumpConfigurationAndControlClusterServerPreAttributeChangedCallback(path, ZCL_ENUM8_ATTRIBUTE_TYPE,
                                                                                        sizeof(value), &value),
              Status::ConstraintError);
}

TEST_F(TestPumpLevelTransition, MaximumTakesOverTransition)
{
    StartTransition(200);
    ASSERT_FALSE(HasFailure());
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMaximum);
    EXPECT_FALSE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel.Value(), gMaxLevel);
    EXPECT_EQ(gRemainingTime, 0u);
    EXPECT_EQ(gLevelDirty, MarkAttributeDirty::kYes);
}

TEST_F(TestPumpLevelTransition, MinimumTakesOverTransition)
{
    StartTransition(50);
    ASSERT_FALSE(HasFailure());
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMinimum);
    EXPECT_FALSE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel.Value(), gMinLevel);
    EXPECT_EQ(gRemainingTime, 0u);
    EXPECT_EQ(gLevelDirty, MarkAttributeDirty::kYes);
}

TEST_F(TestPumpLevelTransition, TakeoverDoesNotDependOnExecuteIfOff)
{
    StartTransition(200);
    ASSERT_FALSE(HasFailure());
    gOn = false;
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMaximum);
    EXPECT_FALSE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel.Value(), gMaxLevel);
    EXPECT_FALSE(gOn);
}

TEST_F(TestPumpLevelTransition, MinimumKeepsPumpRunningWhenMinLevelIsZero)
{
    gMinLevel = 0;
    emberAfLevelControlClusterServerInitCallback(kEndpoint);
    StartTransition(50);
    ASSERT_FALSE(HasFailure());
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMinimum);
    EXPECT_FALSE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel.Value(), 1u);
    EXPECT_EQ(gRemainingTime, 0u);
}

TEST_F(TestPumpLevelTransition, NormalModeLeavesTransitionRunning)
{
    StartTransition(200);
    ASSERT_FALSE(HasFailure());
    auto level = gCurrentLevel;
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kNormal);
    EXPECT_TRUE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel, level);
    EXPECT_GT(gRemainingTime, 0u);
}

TEST_F(TestPumpLevelTransition, LimitReadFailureLeavesTransitionRunning)
{
    StartTransition(200);
    ASSERT_FALSE(HasFailure());
    auto level       = gCurrentLevel;
    gLimitReadStatus = Status::Failure;
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMaximum);
    EXPECT_TRUE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel, level);
}

TEST_F(TestPumpLevelTransition, TakeoverSynchronizesQuietReportingForStop)
{
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMinimum);
    ASSERT_EQ(gCurrentLevel.Value(), gMinLevel);
    Testing::MockCommandHandler handler;
    LevelControl::Commands::Stop::DecodableType command;
    EXPECT_TRUE(emberAfLevelControlClusterStopCallback(
        &handler, ConcreteCommandPath(kEndpoint, LevelControl::Id, LevelControl::Commands::Stop::Id), command));
    ASSERT_TRUE(handler.HasStatus());
    EXPECT_EQ(handler.GetLastStatus().status.GetStatus(), Status::Success);
    EXPECT_EQ(gCurrentLevel.Value(), gMinLevel);
}

TEST_F(TestPumpLevelTransition, MaximumUsesConfiguredLimit)
{
    gMaxLevel = 240;
    emberAfLevelControlClusterServerInitCallback(kEndpoint);
    StartTransition(200);
    ASSERT_FALSE(HasFailure());
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMaximum);
    EXPECT_FALSE(mTimer.HasTimer());
    EXPECT_EQ(gCurrentLevel.Value(), 240u);
    EXPECT_EQ(gRemainingTime, 0u);
}

TEST_F(TestPumpLevelTransition, LevelWriteFailureEndsTransition)
{
    StartTransition(200);
    ASSERT_FALSE(HasFailure());
    auto level        = gCurrentLevel;
    gLevelWriteStatus = Status::Failure;
    ChangeOperationMode(PumpConfigurationAndControl::OperationModeEnum::kMaximum);
    EXPECT_FALSE(mTimer.HasTimer());
    EXPECT_EQ(gRemainingTime, 0u);
    EXPECT_EQ(gCurrentLevel, level);
}

} // namespace
