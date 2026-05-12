/**
 *
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
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::ClosureControl;
using namespace chip::app::Clusters;
using namespace chip::Testing;

using Status = chip::Protocols::InteractionModel::Status;
using Config = ClosureControlCluster::Config;

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

    bool OnCountdownTimeChanged(DataModel::Nullable<ElapsedS> newCountdownTime) override
    {
        mCountdownTimeValue         = newCountdownTime;
        mCountdownTimeChangedCalled = true;
        return true;
    }
    bool OnMainStateChanged(MainStateEnum newState) override
    {
        mMainStateValue         = newState;
        mMainStateChangedCalled = true;
        return true;
    }
    bool OnOverallCurrentStateChanged(DataModel::Nullable<GenericOverallCurrentState> newState) override
    {
        mOverallCurrentStateValue         = newState;
        mOverallCurrentStateChangedCalled = true;
        return true;
    }
    bool OnOverallTargetStateChanged(DataModel::Nullable<GenericOverallTargetState> newState) override
    {
        mOverallTargetStateValue         = newState;
        mOverallTargetStateChangedCalled = true;
        return true;
    }
    bool OnCurrentErrorListChanged(DataModel::List<const ClosureErrorEnum> newCurrentErrorList) override
    {
        mCurrentErrorListCopy.clear();
        for (auto it = newCurrentErrorList.begin(); it != newCurrentErrorList.end(); ++it)
        {
            mCurrentErrorListCopy.push_back(*it);
        }
        mCurrentErrorListCount         = mCurrentErrorListCopy.size();
        mCurrentErrorListChangedCalled = true;
        return true;
    }

    bool GetCountdownTimeChangedCalled() const { return mCountdownTimeChangedCalled; }
    bool GetMainStateChangedCalled() const { return mMainStateChangedCalled; }
    bool GetOverallCurrentStateChangedCalled() const { return mOverallCurrentStateChangedCalled; }
    bool GetOverallTargetStateChangedCalled() const { return mOverallTargetStateChangedCalled; }
    bool GetCurrentErrorListChangedCalled() const { return mCurrentErrorListChangedCalled; }
    DataModel::Nullable<ElapsedS> GetCountdownTimeValue() const { return mCountdownTimeValue; }
    MainStateEnum GetMainStateValue() const { return mMainStateValue; }
    DataModel::Nullable<GenericOverallCurrentState> GetOverallCurrentStateValue() const { return mOverallCurrentStateValue; }
    DataModel::Nullable<GenericOverallTargetState> GetOverallTargetStateValue() const { return mOverallTargetStateValue; }
    size_t GetCurrentErrorListCount() const { return mCurrentErrorListCount; }
    const std::vector<ClosureErrorEnum> & GetCurrentErrorListCopy() const { return mCurrentErrorListCopy; }

    void Reset()
    {
        mCountdownTimeChangedCalled       = false;
        mCountdownTimeValue               = DataModel::NullNullable;
        mMainStateChangedCalled           = false;
        mMainStateValue                   = MainStateEnum::kUnknownEnumValue;
        mOverallCurrentStateChangedCalled = false;
        mOverallCurrentStateValue         = DataModel::NullNullable;
        mOverallTargetStateChangedCalled  = false;
        mOverallTargetStateValue          = DataModel::NullNullable;
        mCurrentErrorListChangedCalled    = false;
        mCurrentErrorListCopy.clear();
        mCurrentErrorListCount = 0;
    }

private:
    bool mCountdownTimeChangedCalled = false;
    DataModel::Nullable<ElapsedS> mCountdownTimeValue;
    bool mMainStateChangedCalled = false;
    MainStateEnum mMainStateValue;
    bool mOverallCurrentStateChangedCalled = false;
    DataModel::Nullable<GenericOverallCurrentState> mOverallCurrentStateValue;
    bool mOverallTargetStateChangedCalled = false;
    DataModel::Nullable<GenericOverallTargetState> mOverallTargetStateValue;
    bool mCurrentErrorListChangedCalled = false;
    std::vector<ClosureErrorEnum> mCurrentErrorListCopy;
    size_t mCurrentErrorListCount = 0;

    // Move to command parameters
    Optional<TargetPositionEnum> lastMoveToPosition       = NullOptional;
    Optional<bool> lastMoveToLatch                        = NullOptional;
    Optional<Globals::ThreeLevelAutoEnum> lastMoveToSpeed = NullOptional;
};

constexpr EndpointId kTestEndpointId = 1;

class TestClosureControlCluster : public ::testing::Test
{
public:
    TestClosureControlCluster() :
        mCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning()), mClusterTester(mCluster)
    {}

    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }

    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        // Initialize cluster with test context to enable event generation
        ASSERT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        mockDelegate.Reset();
    }

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
    ClosureControlCluster mCluster;
    ClusterTester mClusterTester;
};
} // namespace

TEST_F(TestClosureControlCluster, TestAttributesList)
{
    std::vector<DataModel::AttributeEntry> expectedAttributes(ClosureControl::Attributes::kMandatoryMetadata.begin(),
                                                              ClosureControl::Attributes::kMandatoryMetadata.end());
    EXPECT_TRUE(IsAttributesListEqualTo(mCluster, expectedAttributes));

    ClosureControlCluster countdownCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCountdownTime());
    expectedAttributes.push_back(ClosureControl::Attributes::CountdownTime::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(countdownCluster, expectedAttributes));

    ClosureControlCluster motionLatchingCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                                    .WithPositioning()
                                                    .WithCountdownTime()
                                                    .WithMotionLatching(BitFlags<LatchControlModesBitmap>()));
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
    EXPECT_EQ(featureMap, BitFlags<Feature>(Feature::kPositioning));
}

TEST_F(TestClosureControlCluster, TestCalibrationFeatureMapAndAcceptedCommands)
{
    ClosureControlCluster calibrateCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCalibration());
    ClusterTester tester(calibrateCluster);
    BitFlags<Feature> featureMap;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitFlags<Feature>(Feature::kPositioning).Set(Feature::kCalibration));

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(calibrateCluster,
                                              {
                                                  ClosureControl::Commands::Stop::kMetadataEntry,
                                                  ClosureControl::Commands::MoveTo::kMetadataEntry,
                                                  ClosureControl::Commands::Calibrate::kMetadataEntry,
                                              }));
}

TEST_F(TestClosureControlCluster, TestInstantaneousFeatureMapAndAcceptedCommands)
{
    ClosureControlCluster instantaneousCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithInstantaneous());
    ClusterTester tester(instantaneousCluster);
    BitFlags<Feature> featureMap;
    EXPECT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, BitFlags<Feature>(Feature::kPositioning).Set(Feature::kInstantaneous));

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(instantaneousCluster,
                                              {
                                                  ClosureControl::Commands::MoveTo::kMetadataEntry,
                                              }));
}

TEST_F(TestClosureControlCluster, TestMainState)
{
    TestServerClusterContext testContext;
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kCalibrating), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kProtected), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kDisengaged), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ClosureControlCluster featureCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                             .WithPositioning()
                                             .WithCalibration()
                                             .WithProtection()
                                             .WithManuallyOperable());
    ASSERT_EQ(featureCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.GetMainState(), MainStateEnum::kCalibrating);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), featureCluster.GetMainState());
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kProtected), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.GetMainState(), MainStateEnum::kProtected);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), featureCluster.GetMainState());
    EXPECT_EQ(featureCluster.SetMainState(MainStateEnum::kDisengaged), CHIP_NO_ERROR);
    EXPECT_EQ(featureCluster.GetMainState(), MainStateEnum::kDisengaged);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), featureCluster.GetMainState());
}

TEST_F(TestClosureControlCluster, TestSetMainStateUpdatesCountdownTime)
{
    mockDelegate.calibrationCountdownTime      = 33;
    mockDelegate.movingCountdownTime           = 22;
    mockDelegate.waitingForMotionCountdownTime = 11;
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCalibration());

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    DataModel::Nullable<ElapsedS> countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 22u);
    EXPECT_EQ(mockDelegate.GetCountdownTimeValue(), countdownTime);
    EXPECT_EQ(mockDelegate.GetCountdownTimeChangedCalled(), true);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kWaitingForMotion), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 11u);
    EXPECT_EQ(mockDelegate.GetCountdownTimeValue(), countdownTime);
    EXPECT_EQ(mockDelegate.GetCountdownTimeChangedCalled(), true);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kCalibrating), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 33u);
    EXPECT_EQ(mockDelegate.GetCountdownTimeValue(), countdownTime);
    EXPECT_EQ(mockDelegate.GetCountdownTimeChangedCalled(), true);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 0u);
    EXPECT_EQ(mockDelegate.GetCountdownTimeValue(), countdownTime);
    EXPECT_EQ(mockDelegate.GetCountdownTimeChangedCalled(), true);
}

TEST_F(TestClosureControlCluster, TestSetCountdownTimeFromDelegateUnsupportedFeatures)
{
    ClosureControlCluster motionOnlyCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                                .WithMotionLatching(BitFlags<LatchControlModesBitmap>())
                                                .WithInstantaneous());
    EXPECT_EQ(motionOnlyCluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(5)),
              CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ClosureControlCluster instantaneousCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithInstantaneous());
    EXPECT_EQ(instantaneousCluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(5)),
              CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestSetCountdownTimeFromDelegateAndRead)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());

    EXPECT_EQ(cluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(1)), CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetCountdownTimeChangedCalled(), true);
    DataModel::Nullable<ElapsedS> countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 1u);
    EXPECT_EQ(mockDelegate.GetCountdownTimeValue(), countdownTime);

    EXPECT_EQ(cluster.SetCountdownTimeFromDelegate(DataModel::MakeNullable<ElapsedS>(2)), CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetCountdownTimeChangedCalled(), true);
    countdownTime = cluster.GetCountdownTime();
    EXPECT_FALSE(countdownTime.IsNull());
    EXPECT_EQ(countdownTime.Value(), 2u);
    EXPECT_EQ(mockDelegate.GetCountdownTimeValue(), countdownTime);
}

TEST_F(TestClosureControlCluster, TestSetOverallCurrentStateFeatureValidation)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());

    DataModel::Nullable<GenericOverallCurrentState> validState(GenericOverallCurrentState(
        Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional, NullOptional));
    EXPECT_EQ(cluster.SetOverallCurrentState(validState), CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetOverallCurrentStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetOverallCurrentStateValue(), cluster.GetOverallCurrentState());

    mockDelegate.Reset();
    DataModel::Nullable<GenericOverallCurrentState> invalidSpeed(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)), NullOptional,
                                   Optional(Globals::ThreeLevelAutoEnum::kLow)));
    EXPECT_EQ(cluster.SetOverallCurrentState(invalidSpeed), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(mockDelegate.GetOverallCurrentStateChangedCalled(), false);

    mockDelegate.Reset();
    DataModel::Nullable<GenericOverallCurrentState> invalidLatch(
        GenericOverallCurrentState(Optional(DataModel::MakeNullable(CurrentPositionEnum::kFullyOpened)),
                                   Optional(DataModel::MakeNullable(true)), NullOptional));
    EXPECT_EQ(cluster.SetOverallCurrentState(invalidLatch), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    EXPECT_EQ(mockDelegate.GetOverallCurrentStateChangedCalled(), false);
}

TEST_F(TestClosureControlCluster, TestSetOverallCurrentStateSecureStateValidation)
{
    TestServerClusterContext testContext;
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
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
    ClosureControlCluster positioningCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(positioningCluster.HandleCalibrate(), Status::UnsupportedCommand);

    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCalibration());
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), cluster.GetMainState());

    mockDelegate.Reset();
    EXPECT_EQ(cluster.HandleCalibrate(), Status::InvalidInState);
    EXPECT_EQ(cluster.GetMainState(), MainStateEnum::kMoving);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), false);

    mockDelegate.Reset();
    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), cluster.GetMainState());
    EXPECT_EQ(cluster.HandleCalibrate(), Status::Success);
    EXPECT_EQ(mockDelegate.calibrateCommandCalls, 1);

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kCalibrating);
}

TEST_F(TestClosureControlCluster, TestHandleCalibrateDelegateFailure)
{
    mockDelegate.calibrateCommandStatus = Status::Busy;
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCalibration());

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kStopped), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleCalibrate(), Status::Busy);
    EXPECT_EQ(mockDelegate.calibrateCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleStop)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCalibration());

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), cluster.GetMainState());
    EXPECT_EQ(cluster.HandleStop(), Status::Success);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kStopped);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleStop(), Status::Success);
    EXPECT_EQ(mockDelegate.GetMainStateChangedCalled(), true);
    EXPECT_EQ(mockDelegate.GetMainStateValue(), cluster.GetMainState());
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleStopFeatureAndDelegateFailure)
{
    ClosureControlCluster instantaneousCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithInstantaneous());
    EXPECT_EQ(instantaneousCluster.HandleStop(), Status::UnsupportedCommand);

    mockDelegate.stopCommandStatus = Status::Busy;
    ClosureControlCluster busyCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(busyCluster.SetMainState(MainStateEnum::kMoving), CHIP_NO_ERROR);
    EXPECT_EQ(busyCluster.HandleStop(), Status::Busy);
    EXPECT_EQ(mockDelegate.stopCommandCalls, 1);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToNoArgumentsAndInvalidState)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(NullOptional, NullOptional, NullOptional), Status::InvalidCommand);

    EXPECT_EQ(cluster.SetMainState(MainStateEnum::kError), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::InvalidInState);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToAllFeatures)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                      .WithPositioning()
                                      .WithMotionLatching(BitFlags<LatchControlModesBitmap>()
                                                              .Set(LatchControlModesBitmap::kRemoteLatching)
                                                              .Set(LatchControlModesBitmap::kRemoteUnlatching))
                                      .WithSpeed());
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
    EXPECT_EQ(mockDelegate.GetOverallTargetStateValue(), targetState);

    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kMoving);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToTransitionsToWaitingWhenNotReady)
{
    mockDelegate.isReadyToMove = false;
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional), Status::Success);
    MainStateEnum state = cluster.GetMainState();
    EXPECT_EQ(state, MainStateEnum::kWaitingForMotion);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToLatchedPositionChangeRequiresUnlatch)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                      .WithPositioning()
                                      .WithMotionLatching(BitFlags<LatchControlModesBitmap>()
                                                              .Set(LatchControlModesBitmap::kRemoteLatching)
                                                              .Set(LatchControlModesBitmap::kRemoteUnlatching)));
    EXPECT_EQ(cluster.SetOverallCurrentState(LatchedState(CurrentPositionEnum::kPartiallyOpened, true)), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional),
              Status::InvalidInState);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), Optional(true), NullOptional),
              Status::InvalidInState);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), Optional(false), NullOptional), Status::Success);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToRemoteLatchingConformanceChecks)
{
    ClosureControlCluster latchOnlyCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
            .WithPositioning()
            .WithMotionLatching(BitFlags<LatchControlModesBitmap>().Set(LatchControlModesBitmap::kRemoteLatching)));
    EXPECT_EQ(latchOnlyCluster.SetOverallCurrentState(DataModel::Nullable<GenericOverallCurrentState>(
                  GenericOverallCurrentState(NullOptional, Optional(DataModel::MakeNullable(true)), NullOptional))),
              CHIP_NO_ERROR);
    EXPECT_EQ(latchOnlyCluster.HandleMoveTo(NullOptional, Optional(false), NullOptional), Status::InvalidInState);

    ClosureControlCluster unlatchOnlyCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
            .WithPositioning()
            .WithMotionLatching(BitFlags<LatchControlModesBitmap>().Set(LatchControlModesBitmap::kRemoteUnlatching)));
    EXPECT_EQ(unlatchOnlyCluster.SetOverallCurrentState(DataModel::Nullable<GenericOverallCurrentState>(
                  GenericOverallCurrentState(NullOptional, Optional(DataModel::MakeNullable(true)), NullOptional))),
              CHIP_NO_ERROR);
    EXPECT_EQ(unlatchOnlyCluster.HandleMoveTo(NullOptional, Optional(true), NullOptional), Status::InvalidInState);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToDelegateFailure)
{
    mockDelegate.moveToCommandStatus = Status::Busy;
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(cluster.SetOverallCurrentState(PositioningState(CurrentPositionEnum::kPartiallyOpened)), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.HandleMoveTo(Optional(TargetPositionEnum::kMoveToFullyOpen), NullOptional, NullOptional), Status::Busy);
}

TEST_F(TestClosureControlCluster, TestHandleMoveToConstraintValidation)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                      .WithPositioning()
                                      .WithMotionLatching(BitFlags<LatchControlModesBitmap>())
                                      .WithSpeed());
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
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());

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
    EXPECT_EQ(mockDelegate.GetCurrentErrorListChangedCalled(), true);
    // Cluster calls OnCurrentErrorListChanged with the list before each add; the last call (before 3rd add) had 2 elements.
    const std::vector<ClosureErrorEnum> & delegateList = mockDelegate.GetCurrentErrorListCopy();
    EXPECT_EQ(delegateList.size(), 2u);
    EXPECT_EQ(delegateList[0], ClosureErrorEnum::kPhysicallyBlocked);
    EXPECT_EQ(delegateList[1], ClosureErrorEnum::kTemperatureLimited);
}

TEST_F(TestClosureControlCluster, TestErrorListBufferSizeValidation)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_NE(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kBlockedBySensor), CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED);

    ClosureErrorEnum shortList[1] = {};
    Span<ClosureErrorEnum> shortSpan(shortList);
    EXPECT_EQ(cluster.GetCurrentErrorList(shortSpan), CHIP_ERROR_BUFFER_TOO_SMALL);
}

TEST_F(TestClosureControlCluster, TestErrorListInvalidUnknownValue)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());

    EXPECT_EQ(cluster.AddErrorToCurrentErrorList(ClosureErrorEnum::kUnknownEnumValue), CHIP_ERROR_INVALID_ARGUMENT);
}

TEST_F(TestClosureControlCluster, TestLatchControlModesFeatureValidation)
{
    // Without MotionLatching, GetLatchControlModes() returns an empty bitmap (the feature is not supported
    // and there is no way to supply modes since WithMotionLatching was not called).
    ClosureControlCluster positioningCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(positioningCluster.GetLatchControlModes(), BitFlags<LatchControlModesBitmap>());

    // With MotionLatching, the value supplied via Config is reflected verbatim (Fixed quality).
    BitFlags<LatchControlModesBitmap> configuredModes = BitFlags<LatchControlModesBitmap>()
                                                            .Set(LatchControlModesBitmap::kRemoteLatching)
                                                            .Set(LatchControlModesBitmap::kRemoteUnlatching);
    ClosureControlCluster latchCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithMotionLatching(configuredModes));
    EXPECT_EQ(latchCluster.GetLatchControlModes(), configuredModes);
}

// Testing only feature validation.
// Event generation failure is expected.
TEST_F(TestClosureControlCluster, TestGenerateMovementCompletedEvent)
{
    TestServerClusterContext testContext;
    ClosureControlCluster positioningCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    ASSERT_EQ(positioningCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(positioningCluster.GenerateMovementCompletedEvent(), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ClosureControlCluster instantaneousCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithInstantaneous());
    ASSERT_EQ(instantaneousCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(instantaneousCluster.GenerateMovementCompletedEvent(), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestGenerateEngageStateChangedEvent)
{
    TestServerClusterContext testContext;
    ClosureControlCluster positioningCluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    ASSERT_EQ(positioningCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_EQ(positioningCluster.GenerateEngageStateChangedEvent(true), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    ClosureControlCluster manuallyOperableCluster(
        Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithManuallyOperable());
    ASSERT_EQ(manuallyOperableCluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    EXPECT_NE(manuallyOperableCluster.GenerateEngageStateChangedEvent(true), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
}

TEST_F(TestClosureControlCluster, TestConformancePositioningOnly)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning());
    EXPECT_EQ(cluster.GetFeatureMap(), BitFlags<Feature>(Feature::kPositioning));
}

TEST_F(TestClosureControlCluster, TestConformanceMotionLatchingWithoutPositioning)
{
    // MotionLatching without Positioning is valid per spec (PS/LT conformance is O.a+).
    // Instantaneous is additionally set here to skip the constructor's CountdownTime reset
    // path, which currently assumes Positioning whenever !Instantaneous.
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                      .WithMotionLatching(BitFlags<LatchControlModesBitmap>())
                                      .WithInstantaneous());
    EXPECT_EQ(cluster.GetFeatureMap(), BitFlags<Feature>(Feature::kMotionLatching).Set(Feature::kInstantaneous));
}

TEST_F(TestClosureControlCluster, TestConformanceSpeedRequiresPositioningWithoutInstantaneous)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithSpeed());
    EXPECT_EQ(cluster.GetFeatureMap(), BitFlags<Feature>(Feature::kPositioning).Set(Feature::kSpeed));
}

TEST_F(TestClosureControlCluster, TestConformanceVentilationRequiresPositioning)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithVentilation());
    EXPECT_EQ(cluster.GetFeatureMap(), BitFlags<Feature>(Feature::kPositioning).Set(Feature::kVentilation));
}

TEST_F(TestClosureControlCluster, TestConformancePedestrianRequiresPositioning)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithPedestrian());
    EXPECT_EQ(cluster.GetFeatureMap(), BitFlags<Feature>(Feature::kPositioning).Set(Feature::kPedestrian));
}

TEST_F(TestClosureControlCluster, TestConformanceCalibrationRequiresPositioning)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCalibration());
    EXPECT_EQ(cluster.GetFeatureMap(), BitFlags<Feature>(Feature::kPositioning).Set(Feature::kCalibration));
}

TEST_F(TestClosureControlCluster, TestConformanceVentilationPedestrianCalibrationWithPositioning)
{
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                      .WithPositioning()
                                      .WithVentilation()
                                      .WithPedestrian()
                                      .WithCalibration());
    EXPECT_EQ(
        cluster.GetFeatureMap(),
        BitFlags<Feature>(Feature::kPositioning).Set(Feature::kVentilation).Set(Feature::kPedestrian).Set(Feature::kCalibration));
}

TEST_F(TestClosureControlCluster, TestConformanceCountdownTimeRequiresPositioningWithoutInstantaneous)
{
    // CountdownTime optional attribute requires Positioning enabled and Instantaneous disabled.
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate).WithPositioning().WithCountdownTime());

    std::vector<DataModel::AttributeEntry> expected(ClosureControl::Attributes::kMandatoryMetadata.begin(),
                                                    ClosureControl::Attributes::kMandatoryMetadata.end());
    expected.push_back(ClosureControl::Attributes::CountdownTime::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, expected));
}

TEST_F(TestClosureControlCluster, TestConformanceMotionLatchingExposesLatchControlModes)
{
    // Enabling MotionLatching must expose the LatchControlModes attribute alongside it.
    ClosureControlCluster cluster(Config(kTestEndpointId, mockDelegate, mockTimerDelegate)
                                      .WithPositioning()
                                      .WithMotionLatching(BitFlags<LatchControlModesBitmap>()));

    std::vector<DataModel::AttributeEntry> expected(ClosureControl::Attributes::kMandatoryMetadata.begin(),
                                                    ClosureControl::Attributes::kMandatoryMetadata.end());
    expected.push_back(ClosureControl::Attributes::LatchControlModes::kMetadataEntry);
    EXPECT_TRUE(IsAttributesListEqualTo(cluster, expected));
}
