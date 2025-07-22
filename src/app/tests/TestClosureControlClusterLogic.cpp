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

TEST_F(TestClosureControlClusterLogic, ReadFeatureMap_Init)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
}

TEST_F(TestClosureControlClusterLogic, GetFeatureMap_WithoutInit)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    BitFlags<Feature> featureMap;
    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, GetFeatureMap_WithInit)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    BitFlags<Feature> featureMap;
    EXPECT_EQ(logic->GetFeatureMap(featureMap), CHIP_NO_ERROR);

    EXPECT_EQ(featureMap, conformance.FeatureMap());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, GetClusterRevision_WithoutInit)
{
    conformance.FeatureMap().Set(Feature::kPositioning);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;
    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_ERROR_INCORRECT_STATE);

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, GetClusterRevision_WithInit)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    uint16_t kExpectedClusterRevision = 1u;
    Attributes::ClusterRevision::TypeInfo::Type clusterRevision;
    EXPECT_EQ(logic->GetClusterRevision(clusterRevision), CHIP_NO_ERROR);

    EXPECT_EQ(clusterRevision, kExpectedClusterRevision);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
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

TEST_F(TestClosureControlClusterLogic, SetCountdownTimeFromDelegate_NoPositioningFeature)
{
    conformance.FeatureMap().Set(Feature::kMotionLatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> countdownTime;

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(countdownTime), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlClusterLogic, SetCountdownTimeFromDelegate_InstantaneousFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kInstantaneous);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    DataModel::Nullable<ElapsedS> countdownTime;

    EXPECT_EQ(logic->SetCountdownTimeFromDelegate(countdownTime), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(logic->GetCountdownTime(countdownTime), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
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

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kFullyOpened);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidSpeedPositioningOnly)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional,
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidLatchPositioningOnly)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), Optional(true), NullOptional));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidPositioningAndMotionLatching)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true), NullOptional));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidSpeedPositioningAndMotionLatching)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidPositioningAndSpeed)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyClosed)), NullOptional,
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kFullyClosed);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_InvalidLatchPositioningAndSpeed)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyClosed)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidAllFeatures)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kHigh)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kFullyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_ValidNullToNull)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set the attribute to null
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState;
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

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
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kFullyOpened);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Reset the value
    overallCurrentState.SetNull();
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesChangePositioning)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    // Change position
    overallCurrentState.Value().position.Value().SetNonNull(CurrentPositionEnum::kFullyOpened);
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kFullyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesChangeLatching)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    // Change latching
    overallCurrentState.Value().latch.Value().SetNonNull(false);
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), false);
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesChangeSpeed)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    // Change speed
    overallCurrentState.Value().speed.SetValue(Globals::ThreeLevelAutoEnum::kHigh);
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    EXPECT_TRUE(mockContext.MatchesReportedAttributeId(Attributes::OverallCurrentState::Id));
}

TEST_F(TestClosureControlClusterLogic, SetOverallState_AllFeaturesNoChange)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    // Set the same value again
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> readValue;
    EXPECT_EQ(logic->GetOverallCurrentState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value().Value(), CurrentPositionEnum::kPartiallyOpened);
    EXPECT_EQ(readValue.Value().latch.Value().Value(), true);
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kLow);

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
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

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
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::InvalidInState);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());

    EXPECT_EQ(logic->SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::InvalidInState);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_AllFeatures)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    BitFlags<LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureControl::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureControl::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kMoveToFullyOpen);
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
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kMoveToFullyOpen);
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
    BitFlags<LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureControl::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureControl::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(NullOptional, Optional(true), NullOptional));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_FALSE(readValue.Value().position.HasValue());
    EXPECT_EQ(readValue.Value().latch.Value(), false);
    EXPECT_FALSE(readValue.Value().speed.HasValue());

    EXPECT_EQ(state, MainStateEnum::kMoving);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_PositioningAndSpeedFeature)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), NullOptional,
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(false),
                                  Optional<Globals::ThreeLevelAutoEnum>(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kMoveToFullyOpen);
    EXPECT_FALSE(readValue.Value().latch.HasValue());
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_EQ(state, MainStateEnum::kMoving);

    EXPECT_FALSE(coundowntime.IsNull());
    EXPECT_EQ(coundowntime.Value(), static_cast<ElapsedS>(20));

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_OnlyPosititonField)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(false),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(logic->GetCountdownTime(coundowntime), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().position.Value(), TargetPositionEnum::kMoveToFullyOpen);
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
    BitFlags<LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureControl::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureControl::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, Optional(true), NullOptional), Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
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
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(false),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, NullOptional, Optional(Globals::ThreeLevelAutoEnum::kLow)), Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    MainStateEnum state;
    DataModel::Nullable<ElapsedS> coundowntime;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);
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

// HandleMoveTo command when the cluster is in a latched state and incoming command changes position without unlatching.
TEST_F(TestClosureControlClusterLogic, TestHandleMoveToCommand_ChangePositionWithoutIncomingLatchWhenLatched)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::InvalidInState);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

// HandleMoveTo command when the cluster is in a latched state and incoming command changes the position.
TEST_F(TestClosureControlClusterLogic, MoveToCommand_UpdatePositionWhenLatched)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(
        logic->HandleMoveTo(Optional<TargetPositionEnum>(TargetPositionEnum::kMoveToFullyOpen), Optional<bool>(true), NullOptional),
        Status::InvalidInState);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);

    EXPECT_TRUE(readValue.IsNull());

    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_UpdateSpeedWhenLatched)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, NullOptional, Optional(Globals::ThreeLevelAutoEnum::kHigh)), Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, MoveToCommand_UpdateLatchWhenLatched)
{
    conformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    BitFlags<LatchControlModesBitmap> latchControlModes;
    latchControlModes.Set(ClosureControl::LatchControlModesBitmap::kRemoteLatching)
        .Set(ClosureControl::LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    EXPECT_EQ(logic->SetLatchControlModes(latchControlModes), CHIP_NO_ERROR);

    // Set initial state
    DataModel::Nullable<GenericOverallCurrentState> overallCurrentState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), Optional(true),
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(logic->SetOverallCurrentState(overallCurrentState), CHIP_NO_ERROR);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->HandleMoveTo(NullOptional, Optional(false), NullOptional), Status::Success);

    DataModel::Nullable<GenericOverallTargetState> readValue;
    EXPECT_EQ(logic->GetOverallTargetState(readValue), CHIP_NO_ERROR);

    EXPECT_FALSE(readValue.IsNull());
    EXPECT_EQ(readValue.Value().latch.Value().Value(), false);

    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
}

TEST_F(TestClosureControlClusterLogic, ReadCurrentErrorList_WhenErrorListIsEmpty)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    ClosureErrorEnum list[10] = {};
    Span<ClosureErrorEnum> currentErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(currentErrorListSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(currentErrorListSpan.empty());
}

TEST_F(TestClosureControlClusterLogic, ReadCurrentErrorList_WhenErrorListIsNotEmpty)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    mockContext.ResetDirtyFlag();
    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kInternalInterference), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());

    ClosureErrorEnum list[10] = {};
    Span<ClosureErrorEnum> currentErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(currentErrorListSpan), CHIP_NO_ERROR);
    EXPECT_FALSE(currentErrorListSpan.empty());

    EXPECT_EQ(static_cast<int>(currentErrorListSpan.size()), 2);
    EXPECT_EQ(currentErrorListSpan[0], ClosureErrorEnum::kBlockedBySensor);
    EXPECT_EQ(currentErrorListSpan[1], ClosureErrorEnum::kInternalInterference);
}

TEST_F(TestClosureControlClusterLogic, AddErrorToCurrentErrorList)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    ClosureErrorEnum list[10] = {};
    Span<ClosureErrorEnum> currentErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(currentErrorListSpan), CHIP_NO_ERROR);
    EXPECT_FALSE(currentErrorListSpan.empty());

    EXPECT_EQ(static_cast<int>(currentErrorListSpan.size()), 1);
    EXPECT_EQ(currentErrorListSpan[0], ClosureErrorEnum::kBlockedBySensor);
}

TEST_F(TestClosureControlClusterLogic, AddDuplicateErrorToCurrentErrorList)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    logic->ClearCurrentErrorList();

    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
    EXPECT_FALSE(mockContext.HasBeenMarkedDirty());
    ClosureErrorEnum list[10] = {};
    Span<ClosureErrorEnum> currentErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(currentErrorListSpan), CHIP_NO_ERROR);
    EXPECT_FALSE(currentErrorListSpan.empty());

    EXPECT_EQ(static_cast<int>(currentErrorListSpan.size()), 1);
    EXPECT_EQ(currentErrorListSpan[0], ClosureErrorEnum::kBlockedBySensor);
}

TEST_F(TestClosureControlClusterLogic, ClearCurrentErrorList)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);
    logic->ClearCurrentErrorList();

    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());

    ClosureErrorEnum list[10] = {};
    Span<ClosureErrorEnum> currentErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(currentErrorListSpan), CHIP_NO_ERROR);
    EXPECT_FALSE(currentErrorListSpan.empty());

    EXPECT_EQ(static_cast<int>(currentErrorListSpan.size()), 1);
    EXPECT_EQ(currentErrorListSpan[0], ClosureErrorEnum::kBlockedBySensor);

    mockContext.ResetDirtyFlag();
    mockContext.ResetReportedAttributeId();

    logic->ClearCurrentErrorList();
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    Span<ClosureErrorEnum> emptyErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(emptyErrorListSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(emptyErrorListSpan.empty());
}

// This test is to ensure that the CurrentErrorList cannot be read when the span buffer is not equal to the expected size.
// Span buffer should be equal to kCurrentErrorListMaxSize to read the current error list(since max-size of the current-error-list
// is 10 as per the spec). If buffer is not equal to 10, it should return CHIP_ERROR_BUFFER_TOO_SMALL.
TEST_F(TestClosureControlClusterLogic, ReadCurrentErrorList_InvalidBufferSize)
{
    conformance.FeatureMap().Set(Feature::kPositioning);
    EXPECT_EQ(logic->Init(conformance, initParams), CHIP_NO_ERROR);

    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    mockContext.ResetDirtyFlag();
    EXPECT_EQ(logic->AddErrorToCurrentErrorList(ClosureErrorEnum::kInternalInterference), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mockContext.HasBeenMarkedDirty());
    ClosureErrorEnum list[1] = {};
    Span<ClosureErrorEnum> currentErrorListSpan(list);
    EXPECT_EQ(logic->GetCurrentErrorList(currentErrorListSpan), CHIP_ERROR_BUFFER_TOO_SMALL);
    ClosureErrorEnum list2[11] = {};
    Span<ClosureErrorEnum> currentErrorListSpan2(list2);
    EXPECT_NE(logic->GetCurrentErrorList(currentErrorListSpan2), CHIP_NO_ERROR);
}
