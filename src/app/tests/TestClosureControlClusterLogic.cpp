/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/core/StringBuilderAdapters.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/closure-control-server/closure-control-cluster-delegate.h>
#include <app/clusters/closure-control-server/closure-control-cluster-logic.h>
#include <app/clusters/closure-control-server/closure-control-cluster-objects.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>
#include <unordered_set>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters;
using namespace chip::System::Clock::Literals;

using Status = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace System {

// TODO: This might be worthwhile to generalize and put into the system layer, but it too will need unit tests.
// This was taken from the Valve control cluster rework - what we do there should be done here as well.
class TimerAndMockClock : public Clock::Internal::MockClock, public Layer
{
public:
    // System Layer overrides
    CHIP_ERROR Init() override { return CHIP_NO_ERROR; }
    void Shutdown() override { Clear(); }
    void Clear()
    {
        mTimerList.Clear();
        mTimerNodes.ReleaseAll();
    }
    bool IsInitialized() const override { return true; }

    CHIP_ERROR StartTimer(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
    {
        Clock::Timestamp awakenTime = GetMonotonicMilliseconds64() + std::chrono::duration_cast<Clock::Milliseconds64>(aDelay);
        TimerList::Node * node      = mTimerNodes.Create(*this, awakenTime, aComplete, aAppState);
        mTimerList.Add(node);
        return CHIP_NO_ERROR;
    }
    void CancelTimer(TimerCompleteCallback aComplete, void * aAppState) override
    {
        TimerList::Node * cancelled = mTimerList.Remove(aComplete, aAppState);
        if (cancelled != nullptr)
        {
            mTimerNodes.Release(cancelled);
        }
    }
    CHIP_ERROR ExtendTimerTo(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }
    bool IsTimerActive(TimerCompleteCallback onComplete, void * appState) override
    {
        return mTimerList.GetRemainingTime(onComplete, appState) != Clock::Timeout(0);
    }
    Clock::Timeout GetRemainingTime(TimerCompleteCallback onComplete, void * appState) override
    {
        return mTimerList.GetRemainingTime(onComplete, appState);
    }
    CHIP_ERROR ScheduleWork(TimerCompleteCallback aComplete, void * aAppState) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // Clock overrides
    void SetMonotonic(Clock::Milliseconds64 timestamp)
    {
        MockClock::SetMonotonic(timestamp);
        // Find all the timers that fired at this time or before and invoke the callbacks
        TimerList::Node * node;
        while ((node = mTimerList.Earliest()) != nullptr && node->AwakenTime() <= timestamp)
        {
            mTimerList.PopEarliest();
            // Invoke auto-releases
            mTimerNodes.Invoke(node);
        }
    }

    void AdvanceMonotonic(Clock::Milliseconds64 increment) { SetMonotonic(GetMonotonicMilliseconds64() + increment); }

private:
    TimerPool<> mTimerNodes;
    TimerList mTimerList;
};

} // namespace System
} // namespace chip

namespace {

// These are globals because SetUpTestSuite is static which requires static variables
System::TimerAndMockClock gSystemLayerAndClock = System::TimerAndMockClock();
System::Clock::ClockBase * gSavedClock         = nullptr;

// Simple mock implementation of DelegateBase
class MockDelegate : public DelegateBase
{
public:
    virtual ~MockDelegate() = default;

    Status HandleStopCommand() override { return Status::Success; }
    Status HandleMoveToCommand(const Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> & tag,
                               const Optional<bool> & latch, const Optional<Globals::ThreeLevelAutoEnum> & speed) override
    {
        return Status::Success;
    }
    Status HandleCalibrateCommand() override { return Status::Success; }
    CHIP_ERROR GetCurrentErrorAtIndex(size_t index, ClosureErrorEnum & closureError) override { return CHIP_NO_ERROR; }

    bool IsManualLatchingNeeded() override { return false; }
    bool IsReadyToMove() override { return true; }
    ElapsedS GetCalibrationCountdownTime() override { return 30; }
    ElapsedS GetMovingCountdownTime() override { return 20; }
    ElapsedS GetWaitingForMotionCountdownTime() override { return 10; }

private:
    std::unordered_set<ClosureErrorEnum> currentErrors;
};

// Simple mock implementation of MatterContext
class MockMatterContext : public MatterContext
{
public:
    MockMatterContext() : MatterContext(kInvalidEndpointId) {}
    void MarkDirty(AttributeId attributeId) override
    {
        hasBeenMarkedDirty   = true;
        mReportedAttributeId = attributeId;
    }

    bool HasBeenMarkedDirty() const { return hasBeenMarkedDirty; }
    void ResetDirtyFlag() { hasBeenMarkedDirty = false; }

    bool MatchesReportedAttributeId(AttributeId attributeId) const { return attributeId == mReportedAttributeId; }
    void ResetReportedAttributeId() { mReportedAttributeId = kInvalidAttributeId; }

private:
    bool hasBeenMarkedDirty          = false;
    AttributeId mReportedAttributeId = kInvalidAttributeId;
};

class TestClosureControlClusterLogic : public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        gSavedClock = &System::SystemClock();
        System::Clock::Internal::SetSystemClockForTesting(&gSystemLayerAndClock);
        DeviceLayer::SetSystemLayerForTesting(&gSystemLayerAndClock);
    }

    static void TearDownTestSuite()
    {
        gSystemLayerAndClock.Shutdown();
        DeviceLayer::SetSystemLayerForTesting(nullptr);
        System::Clock::Internal::SetSystemClockForTesting(gSavedClock);
        Platform::MemoryShutdown();
    }

    void SetUp() override
    {
        mockDelegate = MockDelegate();
        mockContext  = MockMatterContext();
        conformance  = ClusterConformance();
        logic        = std::make_unique<ClusterLogic>(mockDelegate, mockContext);
    }

    void TearDown() override { logic.reset(); }

    MockDelegate mockDelegate;
    MockMatterContext mockContext;
    ClusterConformance conformance;
    ClusterInitParameters initParams;
    std::unique_ptr<ClusterLogic> logic;
};
} // namespace

TEST_F(TestClosureControlClusterLogic, Init_ValidConformance)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
}

TEST_F(TestClosureControlClusterLogic, Init_InvalidConformance)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed).Set(Feature::kInstantaneous);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR);
}

TEST_F(TestClosureControlClusterLogic, SetMainState_Stopped)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kStopped);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_Moving)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kMoving);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_GT(countdownTime.Value(), 0u);
    EXPECT_LE(countdownTime.Value(), 30u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_WaitingForMotion)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kWaitingForMotion), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kWaitingForMotion);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_GT(countdownTime.Value(), 0u);
    EXPECT_LE(countdownTime.Value(), 30u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_Error)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kError);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_SetupRequired)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kSetupRequired), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kSetupRequired);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_CalibratingValid)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kCalibrating);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_GT(countdownTime.Value(), 0u);
    EXPECT_LE(countdownTime.Value(), 30u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_CalibratingInvalid)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kCalibrating), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kStopped);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_TRUE(countdownTime.IsNull());

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetMainState_ProtectedValid)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kProtection);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kProtected), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kProtected);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_ProtectedInvalid)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kProtected), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kStopped);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_TRUE(countdownTime.IsNull());

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetMainState_DisengagedValid)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kManuallyOperable);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kDisengaged);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetMainState_DisengagedInvalid)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kDisengaged), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kStopped);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_TRUE(countdownTime.IsNull());

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetMainState_UnknownEnumValue)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kUnknownEnumValue), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kStopped);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_TRUE(countdownTime.IsNull());

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetMainState_WithoutInit)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kStopped), CHIP_ERROR_INCORRECT_STATE);

    MainStateEnum readValue = MainStateEnum::kMoving; // Other enum to validate no changes to the read value
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(readValue, MainStateEnum::kMoving);

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetMainState_NoChanges)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    mockContext.ResetDirtyFlag();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue, MainStateEnum::kError);

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, GetSetMainState_ValidChange)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);

    MainStateEnum readValue = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(logic->GetMainState(readValue), CHIP_NO_ERROR);

    EXPECT_EQ(readValue, MainStateEnum::kMoving);

    DataModel::Nullable<ElapsedS> countdownTime = DataModel::NullNullable;
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_GT(countdownTime.Value(), 0u);
    EXPECT_LE(countdownTime.Value(), 30u);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
}

TEST_F(TestClosureControlClusterLogic, SetCountdownTimeFromDelegate_Invalid)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    DataModel::Nullable<ElapsedS> countdownTime;

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(countdownTime), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_TRUE(countdownTime.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetCountdownTimeFromDelegate_ValidChange)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(DataModel::NullNullable), CHIP_NO_ERROR);
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Change from null
    DataModel::Nullable<ElapsedS> countdownTime = DataModel::MakeNullable<ElapsedS>(0);
    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(countdownTime), CHIP_NO_ERROR);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<ElapsedS> readValue;
    EXPECT_EQ(logic->GetCountdownTime(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue.Value(), 0u);

    // change from 0
    countdownTime.SetNonNull(1);
    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(countdownTime), CHIP_NO_ERROR);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->GetCountdownTime(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue.Value(), 1u);

    // validate Increase
    countdownTime.SetNonNull(2);
    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(countdownTime), CHIP_NO_ERROR);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::CountdownTime::Id));
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->GetCountdownTime(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(readValue.Value(), 2u);
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidPositioningOnly)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kFullyOpened);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidSpeedPositioningOnly)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyOpened)), NullOptional,
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidLatchPositioningOnly)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyOpened)), Optional(true), NullOptional));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidPositioningAndMotionLatching)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true), NullOptional));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidSpeedPositioningAndMotionLatching)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidPositioningAndSpeed)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyClosed)), NullOptional,
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kFullyClosed);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_EQ(readValue.Value().speed.Value().Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidLatchPositioningAndSpeed)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyClosed)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidAllFeatures)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kHigh))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kFullyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value().Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidNullToNull)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set the attribute to null
    DataModel::Nullable<GenericOverallState> overallState;
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidValueThenNull)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set a valid value
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kFullyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kFullyOpened);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Reset the value
    overallState.SetNull();
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesChangePositioning)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    // Change positioning
    overallState.Value().positioning.Value().SetNonNull(PositioningEnum::kFullyOpened);
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kFullyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value().Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesChangeLatching)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    // Change latching
    overallState.Value().latch.Value().SetNonNull(false);
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), false);
    EXPECT_EQ(readValue.Value().speed.Value().Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesChangeSpeed)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    // Change speed
    overallState.Value().speed.Value().SetNonNull(Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value().Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesNoChange)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set the same value again
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallState> readValue;
    EXPECT_EQ(logic->GetOverallState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().positioning.Value().Value(), PositioningEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value().Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, CalibrateCommand_NoCalibrationFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleCalibrate(), Status::UnsupportedCommand);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, CalibrateCommand_InCalibrationMainState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleCalibrate(), Status::Success);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, CalibrateCommand_InNonCompatibleMainState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();
    EXPECT_EQ(logic->HandleCalibrate(), Status::InvalidInState);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kWaitingForMotion), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();
    EXPECT_EQ(logic->HandleCalibrate(), Status::InvalidInState);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, Calibrate)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    EXPECT_EQ(logic->SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleCalibrate(), Status::Success);

    MainStateEnum state;
    DataModel::Nullable<ElapsedS> countdownTime;
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_NO_ERROR);

    EXPECT_EQ(state, MainStateEnum::kCalibrating);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), static_cast<ElapsedS>(30));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, StopCommand_InstatenousFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kInstantaneous);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleStop(), Status::UnsupportedCommand);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, StopCommand_InCalibrationMainState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    MainStateEnum state;

    EXPECT_EQ(logic->HandleStop(), Status::Success);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);

    EXPECT_EQ(state, MainStateEnum::kStopped);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, StopCommand_InMovingMainState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    MainStateEnum state;

    EXPECT_EQ(logic->HandleStop(), Status::Success);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);

    EXPECT_EQ(state, MainStateEnum::kStopped);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, StopCommand_InWaitingForMotionMainState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    MainStateEnum state;

    EXPECT_EQ(logic->HandleStop(), Status::Success);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);

    EXPECT_EQ(state, MainStateEnum::kStopped);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, StopCommand_InNonCompatibleMainState)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kManuallyOperable);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();
    EXPECT_EQ(logic->HandleStop(), Status::Success);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();
    EXPECT_EQ(logic->HandleStop(), Status::Success);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_NoArguments)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, NullOptional, NullOptional), Status::InvalidCommand);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_InvalidState)
{
    conformance.FeatureMap()
        .Set(Feature::kPositioning)
        .Set(Feature::kMotionLatching)
        .Set(Feature::kSpeed)
        .Set(Feature::kManuallyOperable);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::InvalidInState);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::InvalidInState);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_AllFeatures)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kOpenInFull);
    EXPECT_EQ(readValue.Value().latch.Value(), false);
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_OnlyPositioningFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kOpenInFull);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_OnlyMotionLatchingFeature)
{
    conformance.FeatureMap().Set(Feature::kMotionLatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(GenericOverallState(NullOptional, Optional(true), NullOptional));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_FALSE(readValue.Value().position.HasValue());
    EXPECT_EQ(readValue.Value().latch.Value(), false);
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_PositioningAndSpeedFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), NullOptional,
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kOpenInFull);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_OnlyPosiitonField)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kOpenInFull), NullOptional, NullOptional),
              Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kOpenInFull);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_OnlyLatchField)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, Optional(true), NullOptional), Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_FALSE(readValue.Value().position.HasValue());
    EXPECT_EQ(readValue.Value().latch.Value(), true);
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_OnlySpeedField)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallState> overallState(
        GenericOverallState(Optional(DataModel::MakeNullable(PositioningEnum::kPartiallyOpened)), Optional(true),
                            Optional(DataModel::MakeNullable(Globals::ThreeLevelAutoEnum::kLow))));
    EXPECT_EQ(logic->SetOverallState(overallState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, NullOptional, Optional(Globals::ThreeLevelAutoEnum::kLow)), Status::Success);

    DataModel::Nullable<GenericOverallTarget> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTarget(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_FALSE(readValue.Value().position.HasValue());
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}
