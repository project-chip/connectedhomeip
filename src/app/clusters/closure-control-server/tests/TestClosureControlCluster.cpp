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

#include <app/clusters/closure-control-server/ClosureControlCluster.h>
#include <app/clusters/closure-control-server/ClosureControlClusterDelegate.h>
#include <app/clusters/closure-control-server/ClosureControlClusterObjects.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemClock.h>
#include <system/SystemTimer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters;
using namespace chip::System::Clock::Literals;
using namespace chip::Testing;

using Status = chip::Protocols::InteractionModel::Status;

namespace chip {
namespace System {

// TODO: This might be worthwhile to generalize and put into the system layer, but it too will need unit tests.
// This was taken from the Valve control cluster rework - what we do there should be done here as well.
class TimerAndMockClock : public Clock::Internal::MockClock, public Layer
{
public:
    // System Layer overrides
    CriticalFailure Init() override { return CHIP_NO_ERROR; }
    void Shutdown() override { Clear(); }
    void Clear()
    {
        mTimerList.Clear();
        mTimerNodes.ReleaseAll();
    }
    bool IsInitialized() const override { return true; }

    CriticalFailure StartTimer(Clock::Timeout aDelay, TimerCompleteCallback aComplete, void * aAppState) override
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
    CriticalFailure ScheduleWork(TimerCompleteCallback aComplete, void * aAppState) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    // Clock overrides
    // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
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

    // NOLINTNEXTLINE(bugprone-derived-method-shadowing-base-method)
    void AdvanceMonotonic(Clock::Milliseconds64 increment) { SetMonotonic(GetMonotonicMilliseconds64() + increment); }

private:
    TimerPool<> mTimerNodes;
    TimerList mTimerList;
};

} // namespace System
} // namespace chip

// Mock callback functions
__attribute__((unused)) void
MatterClosureControlClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    // Mock implementation - no-op for tests
}

namespace {

// These are globals because SetUpTestSuite is static which requires static variables
System::TimerAndMockClock gSystemLayerAndClock = System::TimerAndMockClock();
System::Clock::ClockBase * gSavedClock         = nullptr;

// Simple mock implementation of DelegateBase
class MockDelegate : public ClosureControlClusterDelegate
{
public:
    virtual ~MockDelegate() = default;

    Status HandleStopCommand() override
    {
        ++stopCommandCalls;
        return stopCommandStatus;
    }
    Status HandleMoveToCommand(const Optional<chip::app::Clusters::ClosureControl::TargetPositionEnum> & tag,
                               const Optional<bool> & latch, const Optional<Globals::ThreeLevelAutoEnum> & speed) override
    {
        ++moveToCommandCalls;
        lastMoveToPosition = tag;
        lastMoveToLatch    = latch;
        lastMoveToSpeed    = speed;
        return moveToCommandStatus;
    }
    Status HandleCalibrateCommand() override
    {
        ++calibrateCommandCalls;
        return calibrateCommandStatus;
    }

    bool IsReadyToMove() override { return isReadyToMove; }
    ElapsedS GetCalibrationCountdownTime() override { return calibrationCountdownTime; }
    ElapsedS GetMovingCountdownTime() override { return movingCountdownTime; }
    ElapsedS GetWaitingForMotionCountdownTime() override { return waitingForMotionCountdownTime; }

    Status stopCommandStatus               = Status::Success;
    Status moveToCommandStatus             = Status::Success;
    Status calibrateCommandStatus          = Status::Success;
    bool isReadyToMove                     = true;
    ElapsedS calibrationCountdownTime      = 30;
    ElapsedS movingCountdownTime           = 20;
    ElapsedS waitingForMotionCountdownTime = 10;

    int stopCommandCalls      = 0;
    int moveToCommandCalls    = 0;
    int calibrateCommandCalls = 0;

    Optional<TargetPositionEnum> lastMoveToPosition       = NullOptional;
    Optional<bool> lastMoveToLatch                        = NullOptional;
    Optional<Globals::ThreeLevelAutoEnum> lastMoveToSpeed = NullOptional;
};

class TestClosureControlCluster : public ::testing::Test
{
public:
    TestClosureControlCluster() :
        mCluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, conformance, initParams }), mClusterTester(mCluster)
    {}

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

    void SetUp() override {}

    void TearDown() override {}

    static DataModel::Nullable<GenericOverallCurrentState> PositioningState(CurrentPositionEnum position)
    {
        return DataModel::Nullable<GenericOverallCurrentState>(
            GenericOverallCurrentState(Optional(DataModel::MakeNullable(position)), NullOptional, NullOptional));
    }

    static DataModel::Nullable<GenericOverallCurrentState> LatchedState(CurrentPositionEnum position, bool latched,
                                                                        Optional<Globals::ThreeLevelAutoEnum> speed = NullOptional)
    {
        return DataModel::Nullable<GenericOverallCurrentState>(GenericOverallCurrentState(
            Optional(DataModel::MakeNullable(position)), Optional(DataModel::MakeNullable(latched)), speed));
    }

    MockDelegate mockDelegate;
    ClusterConformance conformance;
    ClusterInitParameters initParams;
    const EndpointId kTestEndpointId = 1;
    ClosureControlCluster mCluster;
    ClusterTester mClusterTester;
};
} // namespace

TEST_F(TestClosureControlCluster, TestAttributesList)
{
    std::vector<DataModel::AttributeEntry> mandatoryAttributes(ClosureControl::Attributes::kMandatoryMetadata.begin(),
                                                               ClosureControl::Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, mandatoryAttributes));
}

TEST_F(TestClosureControlCluster, TestMandatoryAcceptedCommands)
{
    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(mCluster,
                                              {
                                                  ClosureControl::Commands::Stop::kMetadataEntry,
                                                  ClosureControl::Commands::MoveTo::kMetadataEntry,
                                              }));
}

TEST_F(TestClosureControlCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestClosureControlCluster, TestReadFeatureMap)
{
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestClosureControlCluster, TestCalibrationFeatureMapAndAcceptedCommands)
{
    ClusterConformance calibrateConformance;
    calibrateConformance.FeatureMap().Set(Feature::kCalibration);
    ClosureControlCluster calibrateCluster(kTestEndpointId,
                                           ClosureControlCluster::Context{ mockDelegate, calibrateConformance, initParams });
    ClusterTester tester(calibrateCluster);
    BitFlags<Feature> featureMap;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, calibrateConformance.FeatureMap());

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(calibrateCluster,
                                              {
                                                  ClosureControl::Commands::Stop::kMetadataEntry,
                                                  ClosureControl::Commands::MoveTo::kMetadataEntry,
                                                  ClosureControl::Commands::Calibrate::kMetadataEntry,
                                              }));
}

TEST_F(TestClosureControlCluster, TestInstantaneousFeatureMapAndAcceptedCommands)
{
    ClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, instantaneousConformance, initParams });
    ClusterTester tester(instantaneousCluster);
    BitFlags<Feature> featureMap;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, instantaneousConformance.FeatureMap());

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(instantaneousCluster,
                                              {
                                                  ClosureControl::Commands::MoveTo::kMetadataEntry,
                                              }));
}

TEST_F(TestClosureControlCluster, TestMainStateFeatureGating)
{
    ClusterConformance stateConformance;
    stateConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, stateConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kCalibrating), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kProtected), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kDisengaged), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    stateConformance.FeatureMap().Set(Feature::kCalibration).Set(Feature::kProtection).Set(Feature::kManuallyOperable);
    ClosureControlCluster featureCluster(kTestEndpointId,
                                         ClosureControlCluster::Context{ mockDelegate, stateConformance, initParams });
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kProtected), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);
}

TEST_F(TestClosureControlCluster, TestSetMainStateUpdatesCountdownTime)
{
    ClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    mockDelegate.calibrationCountdownTime      = 33;
    mockDelegate.movingCountdownTime           = 22;
    mockDelegate.waitingForMotionCountdownTime = 11;
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, testConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    DataModel::Nullable<ElapsedS> countdownTime;
    EXPECT_EQ(cluster.GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 22u);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kWaitingForMotion), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 11u);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 33u);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);
}

TEST_F(TestClosureControlCluster, TestSetCountdownTimeFromDelegateFeatureGating)
{
    ClusterConformance motionOnlyConformance;
    motionOnlyConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster motionOnlyCluster(kTestEndpointId,
                                            ClosureControlCluster::Context{ mockDelegate, motionOnlyConformance, initParams });
    EXPECT_EQ(motionOnlyCluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(5)),
              CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, instantaneousConformance, initParams });
    EXPECT_EQ(instantaneousCluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(5)),
              CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestSetCountdownTimeFromDelegateAndRead)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });

    EXPECT_EQ(cluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(1)), CHIP_NO_ERROR);
    DataModel::Nullable<ElapsedS> countdownTime;
    EXPECT_EQ(cluster.GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 1u);

    EXPECT_EQ(cluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(2)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetCountdownTime(countdownTime), CHIP_NO_ERROR);
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 2u);
}

TEST_F(TestClosureControlCluster, TestSetOverallCurrentStateFeatureValidation)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });

    DataModel::Nullable<GenericOverallCurrentState> validState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(cluster.SetOverallCurrentState(validState), CHIP_NO_ERROR);

    DataModel::Nullable<GenericOverallCurrentState> invalidSpeed(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional,
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(cluster.SetOverallCurrentState(invalidSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    DataModel::Nullable<GenericOverallCurrentState> invalidLatch(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)),
                                   Optional(DataModel::MakeNullable(true)), NullOptional));
    EXPECT_EQ(cluster.SetOverallCurrentState(invalidLatch), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestSetOverallCurrentStateSecureStateValidation)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });

    DataModel::Nullable<GenericOverallCurrentState> invalidSecureState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kPartiallyOpened)), NullOptional,
                                   NullOptional, DataModel::MakeNullable(true)));
    EXPECT_EQ(cluster.SetOverallCurrentState(invalidSecureState), CHIP_ERROR_INVALID_ARGUMENT);

    DataModel::Nullable<GenericOverallCurrentState> validSecureState(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyClosed)), NullOptional, NullOptional,
                                   DataModel::MakeNullable(true)));
    EXPECT_EQ(cluster.SetOverallCurrentState(validSecureState), CHIP_NO_ERROR);
}

TEST_F(TestClosureControlCluster, TestHandleCalibrate)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster positioningCluster(kTestEndpointId,
                                             ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });
    EXPECT_EQ(positioningCluster.HandleCalibrate(), Status::UnsupportedCommand);

    ClusterConformance calibratingConformance;
    calibratingConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, calibratingConformance, initParams });
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::InvalidInState);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::Success);
    EXPECT_EQ(mockDelegate.calibrateCommandCalls, 1);

    MainStateEnum state = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(cluster.GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MainStateEnum::kCalibrating);
}

TEST_F(TestClosureControlCluster, TestHandleCalibrateDelegateFailure)
{
    ClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    mockDelegate.calibrateCommandStatus = Status::Busy;
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, testConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::Busy);
    EXPECT_EQ(mockDelegate.calibrateCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleStop)
{
    ClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kCalibration);
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, testConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleStop(), Status::Success);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);

    MainStateEnum state = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(cluster.GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MainStateEnum::kStopped);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleStop(), Status::Success);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleStopFeatureAndDelegateFailure)
{
    ClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, instantaneousConformance, initParams });
    EXPECT_EQ(instantaneousCluster.HandleStop(), Status::UnsupportedCommand);

    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    mockDelegate.stopCommandStatus = Status::Busy;
    ClosureControlCluster busyCluster(kTestEndpointId,
                                      ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });
    EXPECT_EQ(busyCluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(busyCluster.HandleStop(), Status::Busy);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToNoArgumentsAndInvalidState)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(NullOptional, NullOptional, NullOptional), Status::InvalidCommand);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::InvalidInState);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToAllFeatures)
{
    ClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, testConformance, initParams });
    EXPECT_EQ(cluster.SetLatchControlModes(BitFlags<LatchControlModesBitmap>()
                                               .Set(LatchControlModesBitmap::kRemoteLatching)
                                               .Set(LatchControlModesBitmap::kRemoteUnlatching)),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetOverallCurrentState(
                  LatchedState(CurrentPositionEnum::kPartiallyOpened, true, Optional(Globals::ThreeLevelAutoEnum::kLow))),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), Optional(false),
                                   Optional(Globals::ThreeLevelAutoEnum::kHigh)),
              Status::Success);
    EXPECT_EQ(mockDelegate.moveToCommandCalls, 1);

    DataModel::Nullable<GenericOverallTargetState> targetState;
    EXPECT_EQ(cluster.GetOverallTargetState(targetState), CHIP_NO_ERROR);
    EXPECT_FALSE(targetState.IsNull());
    EXPECT_EQ(targetState.Value().position.Value().Value(), TargetPositionEnum::kMoveToFullyOpen);
    EXPECT_EQ(targetState.Value().latch.Value().Value(), false);
    EXPECT_EQ(targetState.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    MainStateEnum state = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(cluster.GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MainStateEnum::kMoving);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToTransitionsToWaitingWhenNotReady)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    mockDelegate.isReadyToMove = false;
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional), Status::Success);
    MainStateEnum state = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(cluster.GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MainStateEnum::kWaitingForMotion);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToLatchedPositionChangeRequiresUnlatch)
{
    ClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kPositioning).Set(Feature::kMotionLatching);
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, testConformance, initParams });
    EXPECT_EQ(cluster.SetLatchControlModes(BitFlags<LatchControlModesBitmap>()
                                               .Set(LatchControlModesBitmap::kRemoteLatching)
                                               .Set(LatchControlModesBitmap::kRemoteUnlatching)),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.SetOverallCurrentState(LatchedState(CurrentPositionEnum::kPartiallyOpened, true)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::InvalidInState);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), Optional(true), NullOptional),
              Status::InvalidInState);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), Optional(false), NullOptional), Status::Success);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToRemoteLatchingConformanceChecks)
{
    ClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster cluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, testConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(DataModel::Nullable<GenericOverallCurrentState>(
                  GenericOverallCurrentState(NullOptional, Optional(DataModel::MakeNullable(true)), NullOptional))),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetLatchControlModes(BitFlags<LatchControlModesBitmap>().Set(LatchControlModesBitmap::kRemoteLatching)),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(NullOptional, Optional(false), NullOptional), Status::InvalidInState);

    EXPECT_EQ(cluster.SetLatchControlModes(BitFlags<LatchControlModesBitmap>().Set(LatchControlModesBitmap::kRemoteUnlatching)),
              CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(NullOptional, Optional(true), NullOptional), Status::InvalidInState);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToDelegateFailure)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    mockDelegate.moveToCommandStatus = Status::Busy;
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional), Status::Busy);
}

TEST_F(TestClosureControlCluster, TestErrorListLifecycle)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });

    ClosureErrorEnum list[kCurrentErrorListMaxSize] = {};
    Span<ClosureErrorEnum> errorSpan(list);
    EXPECT_EQ(cluster.GetCurrentErrorList(errorSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(errorSpan.empty());

    EXPECT_NE(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);
    EXPECT_EQ(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

    errorSpan = Span<ClosureErrorEnum>(list);
    EXPECT_EQ(cluster.GetCurrentErrorList(errorSpan), CHIP_NO_ERROR);
    EXPECT_EQ(errorSpan.size(), 1u);
    EXPECT_EQ(errorSpan[0], ClosureErrorEnum::kBlockedBySensor);

    MainStateEnum state = MainStateEnum::kUnknownEnumValue;
    EXPECT_EQ(cluster.GetMainState(state), CHIP_NO_ERROR);
    EXPECT_EQ(state, MainStateEnum::kError);

    cluster.ClearCurrentErrorList();
    errorSpan = Span<ClosureErrorEnum>(list);
    EXPECT_EQ(cluster.GetCurrentErrorList(errorSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(errorSpan.empty());
}

TEST_F(TestClosureControlCluster, TestErrorListBufferSizeValidation)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });
    EXPECT_NE(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

    ClosureErrorEnum shortList[1] = {};
    Span<ClosureErrorEnum> shortSpan(shortList);
    EXPECT_EQ(cluster.GetCurrentErrorList(shortSpan), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST_F(TestClosureControlCluster, TestLatchControlModesFeatureValidation)
{
    ClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster positioningCluster(kTestEndpointId,
                                             ClosureControlCluster::Context{ mockDelegate, positioningConformance, initParams });

    EXPECT_EQ(
        positioningCluster.SetLatchControlModes(BitFlags<LatchControlModesBitmap>().Set(LatchControlModesBitmap::kRemoteLatching)),
        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    BitFlags<LatchControlModesBitmap> modes;
    EXPECT_EQ(positioningCluster.GetLatchControlModes(modes), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ClusterConformance latchConformance;
    latchConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster latchCluster(kTestEndpointId,
                                       ClosureControlCluster::Context{ mockDelegate, latchConformance, initParams });
    BitFlags<LatchControlModesBitmap> configuredModes;
    configuredModes.Set(LatchControlModesBitmap::kRemoteLatching).Set(LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(latchCluster.SetLatchControlModes(configuredModes), CHIP_NO_ERROR);
    EXPECT_EQ(latchCluster.GetLatchControlModes(modes), CHIP_NO_ERROR);
    EXPECT_EQ(modes, configuredModes);
}
