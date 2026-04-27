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
#include <app/clusters/window-covering-server/WindowCoveringCluster.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <clusters/WindowCovering/Metadata.h>
#include <pw_unit_test/framework.h>

namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WindowCovering;
using namespace chip::Testing;

using chip::Protocols::InteractionModel::Status;

// Feature combinations
static constexpr BitFlags<Feature> kFeaturesLiftOnly(Feature::kLift);
static constexpr BitFlags<Feature> kFeaturesTiltOnly(Feature::kTilt);
static constexpr BitFlags<Feature> kFeaturesPaLift(Feature::kLift, Feature::kPositionAwareLift);
static constexpr BitFlags<Feature> kFeaturesPaTilt(Feature::kTilt, Feature::kPositionAwareTilt);
static constexpr BitFlags<Feature> kFeaturesAll(Feature::kLift, Feature::kTilt, Feature::kPositionAwareLift,
                                                Feature::kPositionAwareTilt);

class MockDelegate : public Delegate
{
public:
    CHIP_ERROR HandleMovement(WindowCoveringType type) override
    {
        mLastMovementType     = type;
        mHandleMovementCalled = true;
        return mMovementError;
    }

    CHIP_ERROR HandleStopMotion() override
    {
        mHandleStopMotionCalled = true;
        return mStopMotionError;
    }

    void Reset()
    {
        mHandleMovementCalled   = false;
        mHandleStopMotionCalled = false;
        mMovementError          = CHIP_NO_ERROR;
        mStopMotionError        = CHIP_NO_ERROR;
    }

    bool mHandleMovementCalled   = false;
    bool mHandleStopMotionCalled = false;
    WindowCoveringType mLastMovementType{};
    CHIP_ERROR mMovementError   = CHIP_NO_ERROR;
    CHIP_ERROR mStopMotionError = CHIP_NO_ERROR;
};

WindowCovering::Config MakeDefaultConfig()
{
    WindowCovering::Config config;
    config.type              = Type::kRollerShade;
    config.configStatus      = chip::BitMask<ConfigStatus>(ConfigStatus::kOperational);
    config.operationalStatus = chip::BitMask<OperationalStatus>();
    config.endProductType    = EndProductType::kRollerShade;
    config.mode              = chip::BitMask<Mode>();
    return config;
}

struct TestWindowCoveringCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    TestServerClusterContext testContext;
};

TEST_F(TestWindowCoveringCluster, MandatoryAttributesOnly)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    BitFlags<Feature> features;

    WindowCoveringCluster cluster(kRootEndpointId, features, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> attributes;
    ASSERT_EQ(cluster.Attributes(ConcreteClusterPath(kRootEndpointId, WindowCovering::Id), attributes), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expected;
    AttributeListBuilder listBuilder(expected);
    ASSERT_EQ(listBuilder.Append(Span(Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);
    ASSERT_TRUE(chip::Testing::EqualAttributeSets(attributes.TakeBuffer(), expected.TakeBuffer()));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AttributesWithLiftOnly)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesLiftOnly, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t numActuationsLift{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::NumberOfActuationsLift::Id, numActuationsLift), CHIP_NO_ERROR);

    uint16_t numActuationsTilt{};
    EXPECT_NE(tester.ReadAttribute(Attributes::NumberOfActuationsTilt::Id, numActuationsTilt), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AttributesWithTiltOnly)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesTiltOnly, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t numActuationsTilt{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::NumberOfActuationsTilt::Id, numActuationsTilt), CHIP_NO_ERROR);

    uint16_t numActuationsLift{};
    EXPECT_NE(tester.ReadAttribute(Attributes::NumberOfActuationsLift::Id, numActuationsLift), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AttributesWithPaLift)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<Percent> liftPct{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercentage::Id, liftPct), CHIP_NO_ERROR);

    DataModel::Nullable<Percent100ths> liftTarget{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::TargetPositionLiftPercent100ths::Id, liftTarget), CHIP_NO_ERROR);

    DataModel::Nullable<Percent100ths> liftPct100{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, liftPct100), CHIP_NO_ERROR);

    DataModel::Nullable<Percent> tiltPct{};
    EXPECT_NE(tester.ReadAttribute(Attributes::CurrentPositionTiltPercentage::Id, tiltPct), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AttributesWithPaTilt)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaTilt, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<Percent> tiltPct{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionTiltPercentage::Id, tiltPct), CHIP_NO_ERROR);

    DataModel::Nullable<Percent100ths> tiltTarget{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::TargetPositionTiltPercent100ths::Id, tiltTarget), CHIP_NO_ERROR);

    DataModel::Nullable<Percent100ths> tiltPct100{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionTiltPercent100ths::Id, tiltPct100), CHIP_NO_ERROR);

    DataModel::Nullable<Percent> liftPct{};
    EXPECT_NE(tester.ReadAttribute(Attributes::CurrentPositionLiftPercentage::Id, liftPct), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AttributesWithSafetyStatus)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    optAttrs.Set<Attributes::SafetyStatus::Id>();
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    chip::BitMask<SafetyStatus> safety{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::SafetyStatus::Id, safety), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SafetyStatusNotPresentWhenNotEnabled)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ClusterTester tester(cluster);

    chip::BitMask<SafetyStatus> safety{};
    EXPECT_NE(tester.ReadAttribute(Attributes::SafetyStatus::Id, safety), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// AcceptedCommands

TEST_F(TestWindowCoveringCluster, AcceptedCommandsMandatoryOnly)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    BitFlags<Feature> features;

    WindowCoveringCluster cluster(kRootEndpointId, features, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commands;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, WindowCovering::Id), commands), CHIP_NO_ERROR);

    auto buf = commands.TakeBuffer();
    EXPECT_TRUE(std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::UpOrOpen::Id; }));
    EXPECT_TRUE(std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::DownOrClose::Id; }));
    EXPECT_TRUE(std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::StopMotion::Id; }));
    EXPECT_FALSE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToLiftPercentage::Id; }));
    EXPECT_FALSE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToTiltPercentage::Id; }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AcceptedCommandsWithPaLift)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commands;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, WindowCovering::Id), commands), CHIP_NO_ERROR);

    auto buf = commands.TakeBuffer();
    EXPECT_TRUE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToLiftPercentage::Id; }));
    EXPECT_FALSE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToTiltPercentage::Id; }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AcceptedCommandsWithPaTilt)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaTilt, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commands;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, WindowCovering::Id), commands), CHIP_NO_ERROR);

    auto buf = commands.TakeBuffer();
    EXPECT_TRUE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToTiltPercentage::Id; }));
    EXPECT_FALSE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToLiftPercentage::Id; }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, AcceptedCommandsWithAllFeatures)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> commands;
    ASSERT_EQ(cluster.AcceptedCommands(ConcreteClusterPath(kRootEndpointId, WindowCovering::Id), commands), CHIP_NO_ERROR);

    auto buf = commands.TakeBuffer();
    EXPECT_TRUE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToLiftPercentage::Id; }));
    EXPECT_TRUE(
        std::any_of(buf.begin(), buf.end(), [](const auto & e) { return e.commandId == Commands::GoToTiltPercentage::Id; }));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Read Attribute Tests

TEST_F(TestWindowCoveringCluster, ReadMandatoryAttributes)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, WindowCovering::kRevision);

    uint32_t featureMap{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, kFeaturesAll.Raw());

    Type type{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::Type::Id, type), CHIP_NO_ERROR);
    EXPECT_EQ(type, Type::kRollerShade);

    chip::BitMask<ConfigStatus> configStatus{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::ConfigStatus::Id, configStatus), CHIP_NO_ERROR);
    EXPECT_TRUE(configStatus.Has(ConfigStatus::kOperational));

    chip::BitMask<OperationalStatus> opStatus{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::OperationalStatus::Id, opStatus), CHIP_NO_ERROR);

    EndProductType endProduct{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::EndProductType::Id, endProduct), CHIP_NO_ERROR);
    EXPECT_EQ(endProduct, EndProductType::kRollerShade);

    chip::BitMask<Mode> mode{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::Mode::Id, mode), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, ReadUnsupportedAttribute)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    BitFlags<Feature> features;

    WindowCoveringCluster cluster(kRootEndpointId, features, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    uint16_t dummy{};
    EXPECT_NE(tester.ReadAttribute(0xFFF0, dummy), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, ReadNullablePositionAttributes)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    DataModel::Nullable<Percent> liftPct{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercentage::Id, liftPct), CHIP_NO_ERROR);
    EXPECT_TRUE(liftPct.IsNull());

    DataModel::Nullable<Percent> tiltPct{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionTiltPercentage::Id, tiltPct), CHIP_NO_ERROR);
    EXPECT_TRUE(tiltPct.IsNull());

    DataModel::Nullable<Percent100ths> liftPct100{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionLiftPercent100ths::Id, liftPct100), CHIP_NO_ERROR);
    EXPECT_TRUE(liftPct100.IsNull());

    DataModel::Nullable<Percent100ths> tiltPct100{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentPositionTiltPercent100ths::Id, tiltPct100), CHIP_NO_ERROR);
    EXPECT_TRUE(tiltPct100.IsNull());

    DataModel::Nullable<Percent100ths> liftTarget{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::TargetPositionLiftPercent100ths::Id, liftTarget), CHIP_NO_ERROR);

    DataModel::Nullable<Percent100ths> tiltTarget{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::TargetPositionTiltPercent100ths::Id, tiltTarget), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Setter / No-Op / Dirty Tests

TEST_F(TestWindowCoveringCluster, SetNumberOfActuationsLift)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetNumberOfActuationsLift(), 0);

    cluster.SetNumberOfActuationsLift(42);
    EXPECT_EQ(cluster.GetNumberOfActuationsLift(), 42);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::NumberOfActuationsLift::Id));

    tester.GetDirtyList().clear();
    cluster.SetNumberOfActuationsLift(42);
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetNumberOfActuationsTilt)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.GetNumberOfActuationsTilt(), 0);

    cluster.SetNumberOfActuationsTilt(99);
    EXPECT_EQ(cluster.GetNumberOfActuationsTilt(), 99);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::NumberOfActuationsTilt::Id));

    tester.GetDirtyList().clear();
    cluster.SetNumberOfActuationsTilt(99);
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetCurrentPositionLiftPercentage)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.GetCurrentPositionLiftPercentage().IsNull());

    cluster.SetCurrentPositionLiftPercentage(NPercent(50));
    EXPECT_FALSE(cluster.GetCurrentPositionLiftPercentage().IsNull());
    EXPECT_EQ(cluster.GetCurrentPositionLiftPercentage().Value(), 50);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::CurrentPositionLiftPercentage::Id));

    tester.GetDirtyList().clear();
    cluster.SetCurrentPositionLiftPercentage(NPercent(50));
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.SetCurrentPositionLiftPercentage(DataModel::NullNullable);
    EXPECT_TRUE(cluster.GetCurrentPositionLiftPercentage().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetCurrentPositionTiltPercentage)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaTilt, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.GetCurrentPositionTiltPercentage().IsNull());

    cluster.SetCurrentPositionTiltPercentage(NPercent(75));
    EXPECT_EQ(cluster.GetCurrentPositionTiltPercentage().Value(), 75);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::CurrentPositionTiltPercentage::Id));

    tester.GetDirtyList().clear();
    cluster.SetCurrentPositionTiltPercentage(NPercent(75));
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.SetCurrentPositionTiltPercentage(DataModel::NullNullable);
    EXPECT_TRUE(cluster.GetCurrentPositionTiltPercentage().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetCurrentPositionLiftPercentage100ths)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.GetCurrentPositionLiftPercentage100ths().IsNull());

    cluster.SetCurrentPositionLiftPercentage100ths(NPercent100ths(5000));
    EXPECT_EQ(cluster.GetCurrentPositionLiftPercentage100ths().Value(), 5000);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::CurrentPositionLiftPercent100ths::Id));

    tester.GetDirtyList().clear();
    cluster.SetCurrentPositionLiftPercentage100ths(NPercent100ths(5000));
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.SetCurrentPositionLiftPercentage100ths(DataModel::NullNullable);
    EXPECT_TRUE(cluster.GetCurrentPositionLiftPercentage100ths().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetCurrentPositionTiltPercentage100ths)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaTilt, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.GetCurrentPositionTiltPercentage100ths().IsNull());

    cluster.SetCurrentPositionTiltPercentage100ths(NPercent100ths(7500));
    EXPECT_EQ(cluster.GetCurrentPositionTiltPercentage100ths().Value(), 7500);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::CurrentPositionTiltPercent100ths::Id));

    tester.GetDirtyList().clear();
    cluster.SetCurrentPositionTiltPercentage100ths(NPercent100ths(7500));
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.SetCurrentPositionTiltPercentage100ths(DataModel::NullNullable);
    EXPECT_TRUE(cluster.GetCurrentPositionTiltPercentage100ths().IsNull());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetConfigStatusNoOp)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();

    cluster.SetConfigStatus(config.configStatus);
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetOperationalStatusNoOp)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();

    cluster.SetOperationalStatus(config.operationalStatus);
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, SetSafetyStatusNoOp)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    optAttrs.Set<Attributes::SafetyStatus::Id>();
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    tester.GetDirtyList().clear();

    cluster.SetSafetyStatus(chip::BitMask<SafetyStatus>());
    EXPECT_TRUE(tester.GetDirtyList().empty());

    cluster.SetSafetyStatus(chip::BitMask<SafetyStatus>(SafetyStatus::kRemoteLockout));
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::SafetyStatus::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// WriteAttribute (Mode)

TEST_F(TestWindowCoveringCluster, WriteModeAttribute)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::BitMask<Mode> newMode(Mode::kLedFeedback);
    ASSERT_EQ(tester.WriteAttribute(Attributes::Mode::Id, newMode), CHIP_NO_ERROR);
    EXPECT_EQ(cluster.GetMode(), newMode);
    EXPECT_TRUE(tester.IsAttributeDirty(Attributes::Mode::Id));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, WriteModeConstraintError)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    chip::BitMask<Mode> invalidMode;
    invalidMode.SetRaw(0x10);
    DataModel::ActionReturnStatus status = tester.WriteAttribute(Attributes::Mode::Id, invalidMode);
    EXPECT_TRUE(status.IsError());
    EXPECT_EQ(status, Status::ConstraintError);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, WriteUnsupportedAttribute)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ClusterTester tester(cluster);
    ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR);

    Type typeVal                         = Type::kRollerShade;
    DataModel::ActionReturnStatus status = tester.WriteAttribute(Attributes::Type::Id, typeVal);
    EXPECT_TRUE(status.IsError());
    EXPECT_EQ(status, Status::UnsupportedWrite);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Mode + ConfigStatus Interaction

TEST_F(TestWindowCoveringCluster, MaintenanceModeClearsCalibration)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    chip::BitMask<Mode> both(Mode::kMaintenanceMode, Mode::kCalibrationMode);
    cluster.SetMode(both);
    EXPECT_TRUE(cluster.GetMode().Has(Mode::kMaintenanceMode));
    EXPECT_FALSE(cluster.GetMode().Has(Mode::kCalibrationMode));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, ModeUpdatesConfigStatus)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    EXPECT_TRUE(cluster.GetConfigStatus().Has(ConfigStatus::kOperational));

    chip::BitMask<Mode> maint(Mode::kMaintenanceMode);
    cluster.SetMode(maint);
    EXPECT_FALSE(cluster.GetConfigStatus().Has(ConfigStatus::kOperational));

    chip::BitMask<Mode> reversed(Mode::kMotorDirectionReversed);
    cluster.SetMode(reversed);
    EXPECT_TRUE(cluster.GetConfigStatus().Has(ConfigStatus::kLiftMovementReversed));
    EXPECT_TRUE(cluster.GetConfigStatus().Has(ConfigStatus::kOperational));

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Persistence Round-Trip

TEST_F(TestWindowCoveringCluster, PersistenceRoundTrip)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    {
        WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        cluster.SetNumberOfActuationsLift(100);
        cluster.SetNumberOfActuationsTilt(200);
        cluster.SetCurrentPositionLiftPercentage(NPercent(42));
        cluster.SetCurrentPositionTiltPercentage(NPercent(73));
        cluster.SetCurrentPositionLiftPercentage100ths(NPercent100ths(4200));
        cluster.SetCurrentPositionTiltPercentage100ths(NPercent100ths(7300));

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }

    {
        WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);
        ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

        EXPECT_EQ(cluster.GetNumberOfActuationsLift(), 100);
        EXPECT_EQ(cluster.GetNumberOfActuationsTilt(), 200);
        EXPECT_FALSE(cluster.GetCurrentPositionLiftPercentage().IsNull());
        EXPECT_EQ(cluster.GetCurrentPositionLiftPercentage().Value(), 42);
        EXPECT_FALSE(cluster.GetCurrentPositionTiltPercentage().IsNull());
        EXPECT_EQ(cluster.GetCurrentPositionTiltPercentage().Value(), 73);
        EXPECT_FALSE(cluster.GetCurrentPositionLiftPercentage100ths().IsNull());
        EXPECT_EQ(cluster.GetCurrentPositionLiftPercentage100ths().Value(), 4200);
        EXPECT_FALSE(cluster.GetCurrentPositionTiltPercentage100ths().IsNull());
        EXPECT_EQ(cluster.GetCurrentPositionTiltPercentage100ths().Value(), 7300);

        EXPECT_EQ(cluster.GetType(), config.type);

        cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
    }
}

// Delegate Interaction Tests

TEST_F(TestWindowCoveringCluster, UpOrOpenCallsDelegate)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    Status status = cluster.HandleUpOrOpen();
    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mHandleMovementCalled);
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), WC_PERCENT100THS_MIN_OPEN);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, DownOrCloseCallsDelegate)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    Status status = cluster.HandleDownOrClose();
    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mHandleMovementCalled);
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), WC_PERCENT100THS_MAX_CLOSED);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, StopMotionCallsDelegate)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    cluster.SetCurrentPositionLiftPercentage100ths(NPercent100ths(3000));

    Commands::StopMotion::DecodableType fields;
    Status status = cluster.HandleStopMotion(fields);
    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mHandleStopMotionCalled);
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 3000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, StopMotionInProgressDoesNotLatchTarget)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;
    delegate.mStopMotionError = CHIP_ERROR_IN_PROGRESS;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    cluster.SetTargetPositionLiftPercent100ths(NPercent100ths(10000));
    cluster.SetCurrentPositionLiftPercentage100ths(NPercent100ths(5000));

    ClusterTester tester(cluster);
    tester.GetDirtyList().clear();

    Commands::StopMotion::DecodableType fields;
    Status status = cluster.HandleStopMotion(fields);
    EXPECT_EQ(status, Status::Success);
    EXPECT_FALSE(tester.IsAttributeDirty(Attributes::TargetPositionLiftPercent100ths::Id));
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 10000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, GoToLiftPercentageCallsDelegate)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    Commands::GoToLiftPercentage::DecodableType fields;
    fields.liftPercent100thsValue = 5000;
    Status status                 = cluster.HandleGoToLiftPercentage(fields);
    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mHandleMovementCalled);
    EXPECT_EQ(delegate.mLastMovementType, WindowCoveringType::Lift);
    EXPECT_EQ(cluster.GetTargetPositionLiftPercent100ths().Value(), 5000);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, GoToLiftPercentageConstraintError)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    Commands::GoToLiftPercentage::DecodableType fields;
    fields.liftPercent100thsValue = 10001;
    Status status                 = cluster.HandleGoToLiftPercentage(fields);
    EXPECT_EQ(status, Status::ConstraintError);
    EXPECT_FALSE(delegate.mHandleMovementCalled);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, GoToTiltPercentageCallsDelegate)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaTilt, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    Commands::GoToTiltPercentage::DecodableType fields;
    fields.tiltPercent100thsValue = 7500;
    Status status                 = cluster.HandleGoToTiltPercentage(fields);
    EXPECT_EQ(status, Status::Success);
    EXPECT_TRUE(delegate.mHandleMovementCalled);
    EXPECT_EQ(delegate.mLastMovementType, WindowCoveringType::Tilt);
    EXPECT_EQ(cluster.GetTargetPositionTiltPercent100ths().Value(), 7500);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, GoToTiltPercentageConstraintError)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaTilt, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    Commands::GoToTiltPercentage::DecodableType fields;
    fields.tiltPercent100thsValue = 10001;
    Status status                 = cluster.HandleGoToTiltPercentage(fields);
    EXPECT_EQ(status, Status::ConstraintError);
    EXPECT_FALSE(delegate.mHandleMovementCalled);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, GoToLiftPercentageFailsWithoutPaLift)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesLiftOnly, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    Commands::GoToLiftPercentage::DecodableType fields;
    fields.liftPercent100thsValue = 5000;
    Status status                 = cluster.HandleGoToLiftPercentage(fields);
    EXPECT_EQ(status, Status::Failure);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, GoToTiltPercentageFailsWithoutPaTilt)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesTiltOnly, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    Commands::GoToTiltPercentage::DecodableType fields;
    fields.tiltPercent100thsValue = 5000;
    Status status                 = cluster.HandleGoToTiltPercentage(fields);
    EXPECT_EQ(status, Status::Failure);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Motion Lock (Maintenance / Calibration)

TEST_F(TestWindowCoveringCluster, MotionLockedInMaintenanceMode)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    cluster.SetMode(chip::BitMask<Mode>(Mode::kMaintenanceMode));

    EXPECT_EQ(cluster.HandleUpOrOpen(), Status::Busy);
    EXPECT_FALSE(delegate.mHandleMovementCalled);

    EXPECT_EQ(cluster.HandleDownOrClose(), Status::Busy);
    EXPECT_FALSE(delegate.mHandleMovementCalled);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestWindowCoveringCluster, MotionLockedInCalibrationMode)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();
    MockDelegate delegate;

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesPaLift, optAttrs, config);
    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.SetDelegate(&delegate);

    cluster.SetMode(chip::BitMask<Mode>(Mode::kCalibrationMode));

    EXPECT_EQ(cluster.HandleUpOrOpen(), Status::Failure);
    EXPECT_FALSE(delegate.mHandleMovementCalled);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

// Utility Functions

TEST_F(TestWindowCoveringCluster, IsPercent100thsValidTests)
{
    EXPECT_TRUE(IsPercent100thsValid(static_cast<Percent100ths>(0)));
    EXPECT_TRUE(IsPercent100thsValid(static_cast<Percent100ths>(5000)));
    EXPECT_TRUE(IsPercent100thsValid(static_cast<Percent100ths>(10000)));
    EXPECT_FALSE(IsPercent100thsValid(static_cast<Percent100ths>(10001)));
    EXPECT_FALSE(IsPercent100thsValid(static_cast<Percent100ths>(65535)));

    EXPECT_TRUE(IsPercent100thsValid(NPercent100ths()));
    EXPECT_TRUE(IsPercent100thsValid(NPercent100ths(0)));
    EXPECT_TRUE(IsPercent100thsValid(NPercent100ths(10000)));
    EXPECT_FALSE(IsPercent100thsValid(NPercent100ths(10001)));
}

TEST_F(TestWindowCoveringCluster, ComputeOperationalStateTests)
{
    EXPECT_EQ(ComputeOperationalState(static_cast<uint16_t>(100), static_cast<uint16_t>(100)),
              WindowCovering::OperationalState::Stall);
    EXPECT_EQ(ComputeOperationalState(static_cast<uint16_t>(200), static_cast<uint16_t>(100)),
              WindowCovering::OperationalState::MovingDownOrClose);
    EXPECT_EQ(ComputeOperationalState(static_cast<uint16_t>(50), static_cast<uint16_t>(100)),
              WindowCovering::OperationalState::MovingUpOrOpen);

    EXPECT_EQ(ComputeOperationalState(NPercent100ths(), NPercent100ths()), WindowCovering::OperationalState::Stall);
    EXPECT_EQ(ComputeOperationalState(NPercent100ths(5000), NPercent100ths()), WindowCovering::OperationalState::Stall);
    EXPECT_EQ(ComputeOperationalState(NPercent100ths(5000), NPercent100ths(3000)),
              WindowCovering::OperationalState::MovingDownOrClose);
    EXPECT_EQ(ComputeOperationalState(NPercent100ths(3000), NPercent100ths(5000)),
              WindowCovering::OperationalState::MovingUpOrOpen);
}

TEST_F(TestWindowCoveringCluster, ComputePercent100thsStepTests)
{
    EXPECT_EQ(ComputePercent100thsStep(WindowCovering::OperationalState::MovingDownOrClose, 5000, 1000), 6000);
    EXPECT_EQ(ComputePercent100thsStep(WindowCovering::OperationalState::MovingUpOrOpen, 5000, 1000), 4000);
    EXPECT_EQ(ComputePercent100thsStep(WindowCovering::OperationalState::Stall, 5000, 1000), 5000);

    EXPECT_EQ(ComputePercent100thsStep(WindowCovering::OperationalState::MovingDownOrClose, 9500, 1000), 10000);
    EXPECT_EQ(ComputePercent100thsStep(WindowCovering::OperationalState::MovingUpOrOpen, 500, 1000), 0);
}

TEST_F(TestWindowCoveringCluster, CheckLimitStateTests)
{
    AbsoluteLimits limits = { .open = 0, .closed = 10000 };

    EXPECT_EQ(CheckLimitState(0, limits), LimitStatus::IsUpOrOpen);
    EXPECT_EQ(CheckLimitState(10000, limits), LimitStatus::IsDownOrClose);
    EXPECT_EQ(CheckLimitState(5000, limits), LimitStatus::Intermediate);

    AbsoluteLimits inverted = { .open = 10000, .closed = 0 };
    EXPECT_EQ(CheckLimitState(5000, inverted), LimitStatus::Inverted);
}

// Startup / Shutdown cycle

TEST_F(TestWindowCoveringCluster, StartupShutdownCycle)
{
    WindowCovering::OptionalAttributeSet optAttrs;
    WindowCovering::Config config = MakeDefaultConfig();

    WindowCoveringCluster cluster(kRootEndpointId, kFeaturesAll, optAttrs, config);

    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);

    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);

    ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR);
    cluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

} // namespace
