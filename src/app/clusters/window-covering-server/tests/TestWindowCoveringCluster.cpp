/*
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#include <app/clusters/window-covering-server/WindowCoveringCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/WindowCovering/Metadata.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BitMask.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using namespace chip::Testing;

using Status = chip::Protocols::InteractionModel::Status;

namespace {

constexpr EndpointId kTestEndpointId = 1;

// Records how it was called so tests can assert the cluster drives the delegate with the
// right parameters. Results are configurable to simulate delegate failure / in-progress motion.
class MockWindowCoveringDelegate : public WindowCoveringDelegate
{
public:
    CHIP_ERROR HandleMovement(WindowCoveringType type) override
    {
        mHandleMovementCallCount++;
        mLastMovementType = type;
        return mHandleMovementResult;
    }

    CHIP_ERROR HandleStopMotion() override
    {
        mHandleStopMotionCallCount++;
        return mHandleStopMotionResult;
    }

    void Reset()
    {
        mHandleMovementCallCount   = 0;
        mHandleStopMotionCallCount = 0;
    }

    int mHandleMovementCallCount         = 0;
    WindowCoveringType mLastMovementType = WindowCoveringType::Lift;
    CHIP_ERROR mHandleMovementResult     = CHIP_NO_ERROR;

    int mHandleStopMotionCallCount     = 0;
    CHIP_ERROR mHandleStopMotionResult = CHIP_NO_ERROR;
};

// Exposes the protected `SetType` / `SetEndProductType` setters, which otherwise have no
// Interaction Model write path and are only set during application integration.
class TestableWindowCoveringCluster : public WindowCoveringCluster
{
public:
    using WindowCoveringCluster::SetEndProductType;
    using WindowCoveringCluster::SetType;
    using WindowCoveringCluster::WindowCoveringCluster;
};

struct TestWindowCoveringCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }
};

} // namespace

TEST_F(TestWindowCoveringCluster, StartupShutdown)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    EXPECT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AttributesList_LiftOnly)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                        }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// PositionAwareLift pulls in exactly the lift Percent100ths pair (Target + Current), nothing on tilt.
TEST_F(TestWindowCoveringCluster, AttributesList_PositionAwareLift)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                            Attributes::TargetPositionLiftPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionLiftPercent100ths::kMetadataEntry,
                                        }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Symmetric case: PositionAwareTilt pulls in the tilt Percent100ths pair only.
TEST_F(TestWindowCoveringCluster, AttributesList_PositionAwareTilt)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kTilt, Feature::kPositionAwareTilt }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                            Attributes::TargetPositionTiltPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionTiltPercent100ths::kMetadataEntry,
                                        }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Both position-aware features plus every optional attribute: feature-gated and
// OptionalAttributeSet-gated attributes must coexist correctly.
TEST_F(TestWindowCoveringCluster, AttributesList_AllFeaturesAndOptionals)
{
    WindowCovering::OptionalAttributeSet optionals;
    optionals.Set<Attributes::NumberOfActuationsLift::Id>()
        .Set<Attributes::NumberOfActuationsTilt::Id>()
        .Set<Attributes::CurrentPositionLiftPercentage::Id>()
        .Set<Attributes::CurrentPositionTiltPercentage::Id>()
        .Set<Attributes::SafetyStatus::Id>();

    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate)
            .WithFeatures(
                BitFlags<Feature>{ Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift, Feature::kPositionAwareTilt })
            .WithOptionalAttributes(optionals));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::Type::kMetadataEntry,
                                            Attributes::ConfigStatus::kMetadataEntry,
                                            Attributes::OperationalStatus::kMetadataEntry,
                                            Attributes::EndProductType::kMetadataEntry,
                                            Attributes::Mode::kMetadataEntry,
                                            Attributes::NumberOfActuationsLift::kMetadataEntry,
                                            Attributes::NumberOfActuationsTilt::kMetadataEntry,
                                            Attributes::CurrentPositionLiftPercentage::kMetadataEntry,
                                            Attributes::CurrentPositionTiltPercentage::kMetadataEntry,
                                            Attributes::SafetyStatus::kMetadataEntry,
                                            Attributes::TargetPositionLiftPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionLiftPercent100ths::kMetadataEntry,
                                            Attributes::TargetPositionTiltPercent100ths::kMetadataEntry,
                                            Attributes::CurrentPositionTiltPercent100ths::kMetadataEntry,
                                        }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AcceptedCommands_LiftOnly)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::UpOrOpen::kMetadataEntry,
                                                  Commands::DownOrClose::kMetadataEntry,
                                                  Commands::StopMotion::kMetadataEntry,
                                                  Commands::GoToLiftPercentage::kMetadataEntry,
                                              }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AcceptedCommands_TiltOnly)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kTilt }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::UpOrOpen::kMetadataEntry,
                                                  Commands::DownOrClose::kMetadataEntry,
                                                  Commands::StopMotion::kMetadataEntry,
                                                  Commands::GoToTiltPercentage::kMetadataEntry,
                                              }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AcceptedCommands_LiftAndTilt)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kTilt }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::UpOrOpen::kMetadataEntry,
                                                  Commands::DownOrClose::kMetadataEntry,
                                                  Commands::StopMotion::kMetadataEntry,
                                                  Commands::GoToLiftPercentage::kMetadataEntry,
                                                  Commands::GoToTiltPercentage::kMetadataEntry,
                                              }));
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, ReadMandatoryDefaults)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kTilt }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Type type{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::Type::Id, type), CHIP_NO_ERROR);
    EXPECT_EQ(type, Type()); // default-constructed enum == kRollerShade (0)

    EndProductType productType{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::EndProductType::Id, productType), CHIP_NO_ERROR);
    EXPECT_EQ(productType, EndProductType());

    chip::BitMask<ConfigStatus> configStatus{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::ConfigStatus::Id, configStatus), CHIP_NO_ERROR);
    EXPECT_EQ(configStatus.Raw(), 0);

    chip::BitMask<OperationalStatus> opStatus{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::OperationalStatus::Id, opStatus), CHIP_NO_ERROR);
    EXPECT_EQ(opStatus.Raw(), 0);

    chip::BitMask<Mode> mode{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);
    EXPECT_EQ(mode.Raw(), 0);

    BitFlags<Feature> featureMap;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_TRUE(featureMap.HasAll(Feature::kLift, Feature::kTilt));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, ReadEnabledOptionalDefaults)
{
    WindowCovering::OptionalAttributeSet optionals;
    optionals.Set<Attributes::NumberOfActuationsLift::Id>().Set<Attributes::NumberOfActuationsTilt::Id>();

    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(kTestEndpointId,
                                          WindowCoveringCluster::Config(delegate)
                                              .WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kTilt })
                                              .WithOptionalAttributes(optionals));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t actuationsLift = 0xFFFF;
    ASSERT_EQ(tester.ReadAttribute(Attributes::NumberOfActuationsLift::Id, actuationsLift), CHIP_NO_ERROR);
    EXPECT_EQ(actuationsLift, 0u);

    uint16_t actuationsTilt = 0xFFFF;
    ASSERT_EQ(tester.ReadAttribute(Attributes::NumberOfActuationsTilt::Id, actuationsTilt), CHIP_NO_ERROR);
    EXPECT_EQ(actuationsTilt, 0u);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A disabled optional attribute is absent from Attributes(), so the IM (and therefore
// ClusterTester) rejects reading it as UnsupportedAttribute.
TEST_F(TestWindowCoveringCluster, ReadDisabledOptionalIsUnsupported)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t actuationsLift = 0;
    EXPECT_EQ(tester.ReadAttribute(Attributes::NumberOfActuationsLift::Id, actuationsLift), Status::UnsupportedAttribute);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, ReadNullablePositionRoundTrip)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    NPercent100ths value;
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, value), CHIP_NO_ERROR);
    EXPECT_TRUE(value.IsNull()); // default is null

    cluster.SetCurrentPositionLiftPercent100ths(NPercent100ths(4200));
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, value), CHIP_NO_ERROR);
    ASSERT_FALSE(value.IsNull());
    EXPECT_EQ(value.Value(), 4200);

    cluster.SetCurrentPositionLiftPercent100ths(NPercent100ths()); // back to null
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, value), CHIP_NO_ERROR);
    EXPECT_TRUE(value.IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Mode is constrained to the low nibble (bits 0..3); raw values <= 0x0F are accepted,
// anything above is rejected with ConstraintError.
TEST_F(TestWindowCoveringCluster, WriteMode_ValidAndConstraint)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::BitMask<Mode> minValue;
    minValue.SetRaw(0x00);
    EXPECT_EQ(tester.WriteAttribute(Attributes::Mode::Id, minValue), CHIP_NO_ERROR);

    chip::BitMask<Mode> maxValid;
    maxValid.SetRaw(0x0F);
    EXPECT_EQ(tester.WriteAttribute(Attributes::Mode::Id, maxValid), CHIP_NO_ERROR);

    chip::BitMask<Mode> tooLarge;
    tooLarge.SetRaw(0x10);
    EXPECT_EQ(tester.WriteAttribute(Attributes::Mode::Id, tooLarge), Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Writing Mode derives ConfigStatus; a changed value marks both dirty, a no-op write marks neither.
TEST_F(TestWindowCoveringCluster, WriteMode_DirtyAndNoOp)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    auto & dirtyList = tester.GetDirtyList();

    chip::BitMask<Mode> mode;
    mode.Set(Mode::kLedFeedback);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);

    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::Mode::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::ConfigStatus::Id));
    EXPECT_EQ(dirtyList.size(), 2u);

    dirtyList.clear();

    ASSERT_EQ(tester.WriteAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);
    EXPECT_TRUE(dirtyList.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetterDirtyAndNoOp)
{
    WindowCovering::OptionalAttributeSet optionals;
    optionals.Set<Attributes::NumberOfActuationsLift::Id>();

    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(kTestEndpointId,
                                          WindowCoveringCluster::Config(delegate)
                                              .WithFeatures(BitFlags<Feature>{ Feature::kLift })
                                              .WithOptionalAttributes(optionals));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    auto & dirtyList = tester.GetDirtyList();

    cluster.SetNumberOfActuationsLift(5);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::NumberOfActuationsLift::Id));
    EXPECT_EQ(cluster.GetNumberOfActuationsLift(), 5u);

    dirtyList.clear();

    cluster.SetNumberOfActuationsLift(5); // same value -> no-op
    EXPECT_TRUE(dirtyList.empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, Command_UpOrOpen_DownOrClose)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(
            BitFlags<Feature>{ Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift, Feature::kPositionAwareTilt }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // UpOrOpen -> fully open (minimum).
    delegate.Reset();
    auto up = tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
    ASSERT_TRUE(up.IsSuccess());
    EXPECT_EQ(delegate.mHandleMovementCallCount, 2); // once for Lift, once for Tilt
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMinOpen);
    EXPECT_EQ(cluster.GetTargetPositionTiltPercent100ths().Value(), kWcPercent100thsMinOpen);

    // DownOrClose -> fully closed (maximum).
    delegate.Reset();
    auto down = tester.Invoke<Commands::DownOrClose::Type>(Commands::DownOrClose::Type());
    ASSERT_TRUE(down.IsSuccess());
    EXPECT_EQ(delegate.mHandleMovementCallCount, 2);
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMaxClosed);
    EXPECT_EQ(cluster.GetTargetPositionTiltPercent100ths().Value(), kWcPercent100thsMaxClosed);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, Command_StopMotion)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    delegate.Reset();
    auto result = tester.Invoke<Commands::StopMotion::Type>(Commands::StopMotion::Type());
    ASSERT_TRUE(result.IsSuccess());
    EXPECT_EQ(delegate.mHandleStopMotionCallCount, 1);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, Command_GoToLiftPercentage)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Mid-range value.
    {
        Commands::GoToLiftPercentage::Type cmd;
        cmd.liftPercent100thsValue = 5000;
        delegate.Reset();
        auto result = tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(delegate.mHandleMovementCallCount, 1);
        EXPECT_EQ(delegate.mLastMovementType, WindowCoveringType::Lift);
        EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 5000);
    }

    // Boundaries 0 and 10000 are valid.
    {
        Commands::GoToLiftPercentage::Type cmd;
        cmd.liftPercent100thsValue = kWcPercent100thsMaxClosed;
        auto result                = tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMaxClosed);
    }

    // Above the maximum -> ConstraintError, target unchanged, delegate untouched.
    {
        Commands::GoToLiftPercentage::Type cmd;
        cmd.liftPercent100thsValue = static_cast<Percent100ths>(kWcPercent100thsMaxClosed + 1);
        delegate.Reset();
        auto result = tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);
        EXPECT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::ConstraintError);
        EXPECT_EQ(delegate.mHandleMovementCallCount, 0);
        EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), kWcPercent100thsMaxClosed);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Symmetric coverage for the tilt axis.
TEST_F(TestWindowCoveringCluster, Command_GoToTiltPercentage)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kTilt, Feature::kPositionAwareTilt }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    {
        Commands::GoToTiltPercentage::Type cmd;
        cmd.tiltPercent100thsValue = 2500;
        delegate.Reset();
        auto result = tester.Invoke<Commands::GoToTiltPercentage::Type>(cmd);
        ASSERT_TRUE(result.IsSuccess());
        EXPECT_EQ(delegate.mHandleMovementCallCount, 1);
        EXPECT_EQ(delegate.mLastMovementType, WindowCoveringType::Tilt);
        EXPECT_EQ(cluster.GetTargetPositionTiltPercent100ths().Value(), 2500);
    }

    {
        Commands::GoToTiltPercentage::Type cmd;
        cmd.tiltPercent100thsValue = static_cast<Percent100ths>(kWcPercent100thsMaxClosed + 500);
        delegate.Reset();
        auto result = tester.Invoke<Commands::GoToTiltPercentage::Type>(cmd);
        EXPECT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::ConstraintError);
        EXPECT_EQ(delegate.mHandleMovementCallCount, 0);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Mode drives the operational bit of ConfigStatus, which gates motion:
//   neither maintenance nor calibration -> Success, maintenance -> Busy, calibration -> Failure.
TEST_F(TestWindowCoveringCluster, MotionLock)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId, WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    // Default mode -> motion allowed.
    {
        auto result = tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
        EXPECT_TRUE(result.IsSuccess());
    }

    // Maintenance mode -> Busy.
    {
        chip::BitMask<Mode> maintenance;
        maintenance.Set(Mode::kMaintenanceMode);
        cluster.SetMode(maintenance);

        auto result = tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
        ASSERT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::Busy);
    }

    // Calibration mode -> Failure.
    {
        chip::BitMask<Mode> calibration;
        calibration.Set(Mode::kCalibrationMode);
        cluster.SetMode(calibration);

        auto result = tester.Invoke<Commands::UpOrOpen::Type>(Commands::UpOrOpen::Type());
        ASSERT_FALSE(result.IsSuccess());
        auto statusCode = result.GetStatusCode();
        ASSERT_TRUE(statusCode.has_value());
        // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
        EXPECT_EQ(statusCode->GetStatus(), Status::Failure);
    }

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A CHIP_ERROR_IN_PROGRESS result from HandleStopMotion means "still moving"; the target must
// not be snapped to the (still null) current position.
TEST_F(TestWindowCoveringCluster, Delegate_StopMotionInProgressKeepsTarget)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Commands::GoToLiftPercentage::Type goTo;
    goTo.liftPercent100thsValue = 3000;
    ASSERT_TRUE(tester.Invoke<Commands::GoToLiftPercentage::Type>(goTo).IsSuccess());
    ASSERT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 3000);

    delegate.mHandleStopMotionResult = CHIP_ERROR_IN_PROGRESS;
    auto result                      = tester.Invoke<Commands::StopMotion::Type>(Commands::StopMotion::Type());
    ASSERT_TRUE(result.IsSuccess());

    ASSERT_FALSE(cluster.GetTargetPositionLiftPercent100ths().IsNull());
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 3000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// A failing HandleMovement is logged only, not propagated to the command caller, and the
// target state is still committed. Guards this legacy-preserved contract against regression.
TEST_F(TestWindowCoveringCluster, Delegate_MovementFailureIsNotPropagated)
{
    MockWindowCoveringDelegate delegate;
    TestableWindowCoveringCluster cluster(
        kTestEndpointId,
        WindowCoveringCluster::Config(delegate).WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kPositionAwareLift }));
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    delegate.mHandleMovementResult = CHIP_ERROR_INTERNAL;

    Commands::GoToLiftPercentage::Type cmd;
    cmd.liftPercent100thsValue = 6000;
    auto result                = tester.Invoke<Commands::GoToLiftPercentage::Type>(cmd);

    EXPECT_TRUE(result.IsSuccess());                                       // failure swallowed (logged only)
    EXPECT_EQ(delegate.mHandleMovementCallCount, 1);                       // delegate was still called
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 6000); // target still committed

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Persistent attributes survive a restart; RAM-only attributes (Type) reset to default.
// A single TestServerClusterContext outlives two successive cluster instances.
TEST_F(TestWindowCoveringCluster, PersistenceRoundTrip)
{
    TestServerClusterContext sharedContext;
    MockWindowCoveringDelegate delegate;

    WindowCoveringCluster::Config config(delegate);
    config.WithFeatures(BitFlags<Feature>{ Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift });

    // First boot: mutate persistent state and a RAM-only attribute, then shut down.
    {
        TestableWindowCoveringCluster cluster(kTestEndpointId, config);
        ASSERT_EQ(cluster.Startup(sharedContext.Get()), CHIP_NO_ERROR);

        cluster.SetNumberOfActuationsLift(7);
        cluster.SetNumberOfActuationsTilt(9);
        cluster.SetCurrentPositionLiftPercent100ths(NPercent100ths(2500));
        cluster.SetType(Type::kRollerShade2Motor); // RAM-only

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    // Second boot: a brand-new cluster against the same storage.
    {
        TestableWindowCoveringCluster cluster(kTestEndpointId, config);
        ASSERT_EQ(cluster.Startup(sharedContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetNumberOfActuationsLift(), 7u);
        EXPECT_EQ(cluster.GetNumberOfActuationsTilt(), 9u);
        ASSERT_FALSE(cluster.GetCurrentPositionLiftPercent100ths().IsNull());
        EXPECT_EQ(cluster.GetCurrentPositionLiftPercent100ths().Value(), 2500);

        EXPECT_EQ(cluster.GetType(), Type()); // not persisted -> reset to default

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}
