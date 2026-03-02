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
#include <lib/support/TimerDelegateMock.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters;
using namespace chip::Testing;

using Status = chip::Protocols::InteractionModel::Status;

// Mock callback functions
__attribute__((unused)) void
MatterClosureControlClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    // Mock implementation - no-op for tests
}

namespace {

TimerDelegateMock mockTimerDelegate;

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

class MockClusterConformance : public ClusterConformance
{
public:
    MockClusterConformance()
    {
        // We need at least one feature to be supported from Positioning or MotionLatching (O.a+)
        // So, we set the Positioning feature by default.
        FeatureMap().Set(Feature::kPositioning);
    }
};

class TestClosureControlCluster : public ::testing::Test
{
public:
    TestClosureControlCluster() :
        mCluster(kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, mockConformance, initParams }),
        mClusterTester(mCluster)
    {}

    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        // Initialize cluster with test context to enable event generation
        ASSERT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
        initParams.mMainState           = MainStateEnum::kStopped;
        initParams.mOverallCurrentState = DataModel::NullNullable;
    }

    void TearDown() override { mCluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

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
    MockClusterConformance mockConformance;
    ClusterInitParameters initParams;
    const EndpointId kTestEndpointId = 1;
    ClosureControlCluster mCluster;
    ClusterTester mClusterTester;
};
} // namespace

TEST_F(TestClosureControlCluster, TestAttributesList)
{
    std::vector<DataModel::AttributeEntry> expectedAttributes(ClosureControl::Attributes::kMandatoryMetadata.begin(),
                                                              ClosureControl::Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, expectedAttributes));

    MockClusterConformance testConformance;
    testConformance.OptionalAttributes().Set<Attributes::CountdownTime::Id>();
    ClosureControlCluster testCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });
    expectedAttributes.push_back(ClosureControl::Attributes::CountdownTime::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(testCluster, expectedAttributes));

    testConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster motionLatchingCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });
    expectedAttributes.push_back(ClosureControl::Attributes::LatchControlModes::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(motionLatchingCluster, expectedAttributes));
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
    BitFlags<Feature> featureMap;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, mockConformance.FeatureMap());
}

TEST_F(TestClosureControlCluster, TestCalibrationFeatureMapAndAcceptedCommands)
{
    MockClusterConformance calibrateConformance;
    calibrateConformance.FeatureMap().Set(Feature::kCalibration);
    ClosureControlCluster calibrateCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, calibrateConformance, initParams });
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
    MockClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, instantaneousConformance, initParams });
    ClusterTester tester(instantaneousCluster);
    BitFlags<Feature> featureMap;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, instantaneousConformance.FeatureMap());

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(instantaneousCluster,
                                              {
                                                  ClosureControl::Commands::MoveTo::kMetadataEntry,
                                              }));
}

TEST_F(TestClosureControlCluster, TestMainState)
{
    TestServerClusterContext testContext;
    MockClusterConformance stateConformance;
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, stateConformance, initParams });
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kCalibrating), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kProtected), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kDisengaged), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    stateConformance.FeatureMap().Set(Feature::kCalibration).Set(Feature::kProtection).Set(Feature::kManuallyOperable);
    ClosureControlCluster featureCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, stateConformance, initParams });
    ASSERT_EQ(featureCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.GetMainState(), MainStateEnum::kCalibrating);
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kProtected), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.GetMainState(), MainStateEnum::kProtected);
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.GetMainState(), MainStateEnum::kDisengaged);
}

TEST_F(TestClosureControlCluster, TestSetMainStateUpdatesCountdownTime)
{
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kCalibration);
    mockDelegate.calibrationCountdownTime      = 33;
    mockDelegate.movingCountdownTime           = 22;
    mockDelegate.waitingForMotionCountdownTime = 11;
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    DataModel::Nullable<ElapsedS> countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 22u);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kWaitingForMotion), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 11u);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 33u);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);
}

TEST_F(TestClosureControlCluster, TestSetCountdownTimeFromDelegateUnsupportedFeatures)
{
    MockClusterConformance motionOnlyConformance;
    motionOnlyConformance.FeatureMap().ClearAll().Set(Feature::kMotionLatching).Set(Feature::kInstantaneous);
    ClosureControlCluster motionOnlyCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, motionOnlyConformance, initParams });
    EXPECT_EQ(motionOnlyCluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(5)),
              CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MockClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, instantaneousConformance, initParams });
    EXPECT_EQ(instantaneousCluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(5)),
              CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestSetCountdownTimeFromDelegateAndRead)
{
    MockClusterConformance positioningConformance;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });

    EXPECT_EQ(cluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(1)), CHIP_NO_ERROR);
    DataModel::Nullable<ElapsedS> countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 1u);

    EXPECT_EQ(cluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(2)), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 2u);
}

TEST_F(TestClosureControlCluster, TestSetOverallCurrentStateFeatureValidation)
{
    MockClusterConformance positioningConformance;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });

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
    TestServerClusterContext testContext;
    MockClusterConformance positioningConformance;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

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
    MockClusterConformance positioningConformance;
    ClosureControlCluster positioningCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    EXPECT_EQ(positioningCluster.HandleCalibrate(), Status::UnsupportedCommand);

    MockClusterConformance calibratingConformance;
    calibratingConformance.FeatureMap().Set(Feature::kCalibration);
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, calibratingConformance, initParams });
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::InvalidInState);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::Success);
    EXPECT_EQ(mockDelegate.calibrateCommandCalls, 1);

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kCalibrating);
}

TEST_F(TestClosureControlCluster, TestHandleCalibrateDelegateFailure)
{
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kCalibration);
    mockDelegate.calibrateCommandStatus = Status::Busy;
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::Busy);
    EXPECT_EQ(mockDelegate.calibrateCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleStop)
{
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kCalibration);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleStop(), Status::Success);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kStopped);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleStop(), Status::Success);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleStopFeatureAndDelegateFailure)
{
    MockClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, instantaneousConformance, initParams });
    EXPECT_EQ(instantaneousCluster.HandleStop(), Status::UnsupportedCommand);

    MockClusterConformance positioningConformance;
    mockDelegate.stopCommandStatus = Status::Busy;
    ClosureControlCluster busyCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    EXPECT_EQ(busyCluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(busyCluster.HandleStop(), Status::Busy);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToNoArgumentsAndInvalidState)
{
    MockClusterConformance positioningConformance;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(NullOptional, NullOptional, NullOptional), Status::InvalidCommand);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::InvalidInState);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToAllFeatures)
{
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });
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

    DataModel::Nullable<GenericOverallTargetState> targetState = cluster.GetOverallTargetState();
    EXPECT_FALSE(targetState.IsNull());
    EXPECT_EQ(targetState.Value().position.Value().Value(), TargetPositionEnum::kMoveToFullyOpen);
    EXPECT_EQ(targetState.Value().latch.Value().Value(), false);
    EXPECT_EQ(targetState.Value().speed.Value(), Globals::ThreeLevelAutoEnum::kHigh);

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kMoving);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToTransitionsToWaitingWhenNotReady)
{
    MockClusterConformance positioningConformance;
    mockDelegate.isReadyToMove = false;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional), Status::Success);
    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kWaitingForMotion);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToLatchedPositionChangeRequiresUnlatch)
{
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });
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
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });
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
    MockClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    mockDelegate.moveToCommandStatus = Status::Busy;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional), Status::Busy);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToConstraintValidation)
{
    MockClusterConformance testConformance;
    testConformance.FeatureMap().Set(Feature::kMotionLatching).Set(Feature::kSpeed);
    ClosureControlCluster cluster(kTestEndpointId,
                                  ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, testConformance, initParams });
    EXPECT_EQ(cluster.SetOverallCurrentState(
                  LatchedState(CurrentPositionEnum::kPartiallyOpened, false, Optional(Globals::ThreeLevelAutoEnum::kLow))),
              CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kUnknownEnumValue), NullOptional, NullOptional),
              Status::ConstraintError);
    EXPECT_EQ(cluster.HandleMoveTo(NullOptional, NullOptional, Optional(Globals::ThreeLevelAutoEnum::kUnknownEnumValue)),
              Status::ConstraintError);
}

TEST_F(TestClosureControlCluster, TestErrorListLifecycle)
{
    MockClusterConformance positioningConformance;
    positioningConformance.FeatureMap().Set(Feature::kPositioning);
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });

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

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kError);

    cluster.ClearCurrentErrorList();
    errorSpan = Span<ClosureErrorEnum>(list);
    EXPECT_EQ(cluster.GetCurrentErrorList(errorSpan), CHIP_NO_ERROR);
    EXPECT_TRUE(errorSpan.empty());

    // Event generation errors are expected hence only logging the errors
    LogErrorOnFailure(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kPhysicallyBlocked));
    LogErrorOnFailure(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kTemperatureLimited));
    LogErrorOnFailure(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor));
    errorSpan = Span<ClosureErrorEnum>(list);
    EXPECT_EQ(cluster.GetCurrentErrorList(errorSpan), CHIP_NO_ERROR);
    EXPECT_EQ(errorSpan.size(), 3u);
    EXPECT_EQ(errorSpan[0], ClosureErrorEnum::kPhysicallyBlocked);
    EXPECT_EQ(errorSpan[1], ClosureErrorEnum::kTemperatureLimited);
    EXPECT_EQ(errorSpan[2], ClosureErrorEnum::kBlockedBySensor);
}

TEST_F(TestClosureControlCluster, TestErrorListBufferSizeValidation)
{
    MockClusterConformance positioningConformance;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    EXPECT_NE(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

    ClosureErrorEnum shortList[1] = {};
    Span<ClosureErrorEnum> shortSpan(shortList);
    EXPECT_EQ(cluster.GetCurrentErrorList(shortSpan), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST_F(TestClosureControlCluster, TestErrorListInvalidUnknownValue)
{
    MockClusterConformance positioningConformance;
    ClosureControlCluster cluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });

    EXPECT_EQ(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kUnknownEnumValue), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestClosureControlCluster, TestLatchControlModesFeatureValidation)
{
    MockClusterConformance positioningConformance;
    ClosureControlCluster positioningCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });

    EXPECT_EQ(
        positioningCluster.SetLatchControlModes(BitFlags<LatchControlModesBitmap>().Set(LatchControlModesBitmap::kRemoteLatching)),
        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    BitFlags<LatchControlModesBitmap> modes = positioningCluster.GetLatchControlModes();
    EXPECT_EQ(modes, BitFlags<LatchControlModesBitmap>());

    MockClusterConformance latchConformance;
    latchConformance.FeatureMap().Set(Feature::kMotionLatching);
    ClosureControlCluster latchCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, latchConformance, initParams });
    BitFlags<LatchControlModesBitmap> configuredModes = latchCluster.GetLatchControlModes();
    configuredModes.Set(LatchControlModesBitmap::kRemoteLatching).Set(LatchControlModesBitmap::kRemoteUnlatching);
    EXPECT_EQ(latchCluster.SetLatchControlModes(configuredModes), CHIP_NO_ERROR);
    EXPECT_EQ(latchCluster.GetLatchControlModes(), configuredModes);
}

// Testing only feature validation.
// Event generation failure is expected.
TEST_F(TestClosureControlCluster, TestGenerateMovementCompletedEvent)
{
    TestServerClusterContext testContext;
    MockClusterConformance positioningConformance;
    ClosureControlCluster positioningCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    ASSERT_EQ(positioningCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(positioningCluster.GenerateMovementCompletedEvent(), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MockClusterConformance instantaneousConformance;
    instantaneousConformance.FeatureMap().Set(Feature::kInstantaneous);
    ClosureControlCluster instantaneousCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, instantaneousConformance, initParams });
    ASSERT_EQ(instantaneousCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(instantaneousCluster.GenerateMovementCompletedEvent(), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestGenerateEngageStateChangedEvent)
{
    TestServerClusterContext testContext;
    MockClusterConformance positioningConformance;
    ClosureControlCluster positioningCluster(
        kTestEndpointId, ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, positioningConformance, initParams });
    ASSERT_EQ(positioningCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(positioningCluster.GenerateEngageStateChangedEvent(true), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    MockClusterConformance manuallyOperableConformance;
    manuallyOperableConformance.FeatureMap().Set(Feature::kManuallyOperable);
    ClosureControlCluster manuallyOperableCluster(
        kTestEndpointId,
        ClosureControlCluster::Context{ mockDelegate, mockTimerDelegate, manuallyOperableConformance, initParams });
    ASSERT_EQ(manuallyOperableCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(manuallyOperableCluster.GenerateEngageStateChangedEvent(true), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}
