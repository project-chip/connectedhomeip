/*
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

#include <app/clusters/dynamic-lighting-server/DynamicLightingCluster.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/DynamicLighting/Attributes.h>
#include <clusters/DynamicLighting/Commands.h>
#include <clusters/DynamicLighting/Metadata.h>
#include <lib/support/Span.h>

#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DynamicLighting;
using namespace chip::Testing;

namespace {

using Protocols::InteractionModel::ClusterStatusCode;
using Protocols::InteractionModel::Status;

constexpr uint16_t kFireplaceId = 1;
constexpr uint16_t kCycleId   = 2;
constexpr uint16_t kFireplaceMaxSpeed = 10;
constexpr uint16_t kCycleMaxSpeed   = 100;
constexpr uint16_t kCycleDefaultSpeed = 50;

Structs::EffectStruct::Type MakeEffect(uint16_t id, CharSpan label, uint16_t maxSpeed, uint16_t defaultSpeed, bool supportsPalette)
{
    Structs::EffectStruct::Type effect;
    effect.effectID             = id;
    effect.source               = EffectSourceEnum::kInternal;
    effect.label                = label;
    effect.maxSpeed             = maxSpeed;
    effect.defaultSpeed         = defaultSpeed;
    effect.supportsColorPalette = supportsPalette;
    return effect;
}

class TestDelegate : public DynamicLightingDelegate
{
public:
    Span<const Structs::EffectStruct::Type> GetAvailableEffects() override { return Span<const Structs::EffectStruct::Type>(effects.data(), effects.size()); }

    CHIP_ERROR StartEffect(const Structs::EffectStruct::Type & effect, uint16_t speed) override
    {
        startCalls++;
        lastStartedId    = effect.effectID;
        lastStartedSpeed = speed;
        return startResult;
    }

    void StopEffect() override { stopCalls++; }

    CHIP_ERROR SetEffectSpeed(uint16_t speed) override
    {
        lastSetSpeed = speed;
        return setSpeedResult;
    }

    std::vector<Structs::EffectStruct::Type> effects = {
        MakeEffect(kFireplaceId, "Fireplace"_span, kFireplaceMaxSpeed, 5, false),
        MakeEffect(kCycleId, "Cycle"_span, kCycleMaxSpeed, kCycleDefaultSpeed, false),
    };
    CHIP_ERROR startResult    = CHIP_NO_ERROR;
    CHIP_ERROR setSpeedResult = CHIP_NO_ERROR;
    int startCalls            = 0;
    int stopCalls             = 0;
    uint16_t lastStartedId    = 0;
    uint16_t lastStartedSpeed = 0;
    uint16_t lastSetSpeed     = 0;
};

struct TestDynamicLightingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_SUCCESS(Platform::MemoryInit()); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TestDynamicLightingCluster() : cluster(kRootEndpointId, delegate) {}

    void SetUp() override { ASSERT_SUCCESS(cluster.Startup(testContext.Get())); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    Status Start(ClusterTester & tester, const Commands::StartEffect::Type & request)
    {
        auto result = tester.Invoke(request);
        return result.GetStatusCode().value_or(ClusterStatusCode(Status::Failure)).GetStatus();
    }

    Status Stop(ClusterTester & tester)
    {
        Commands::StopEffect::Type request;
        auto result = tester.Invoke(request);
        return result.GetStatusCode().value_or(ClusterStatusCode(Status::Failure)).GetStatus();
    }

    DataModel::Nullable<uint16_t> ReadEffectID(ClusterTester & tester)
    {
        DataModel::Nullable<uint16_t> value;
        EXPECT_SUCCESS(tester.ReadAttribute(Attributes::CurrentEffectID::Id, value));
        return value;
    }

    DataModel::Nullable<uint16_t> ReadSpeed(ClusterTester & tester)
    {
        DataModel::Nullable<uint16_t> value;
        EXPECT_SUCCESS(tester.ReadAttribute(Attributes::CurrentSpeed::Id, value));
        return value;
    }

    TestServerClusterContext testContext;
    TestDelegate delegate;
    DynamicLightingCluster cluster;
};

Commands::StartEffect::Type MakeStart(uint16_t effectId, uint16_t speed)
{
    Commands::StartEffect::Type request;
    request.effectID = effectId;
    request.speed    = speed;
    return request;
}

} // namespace

TEST_F(TestDynamicLightingCluster, AttributeList)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            Attributes::AvailableEffects::kMetadataEntry,
                                            Attributes::CurrentEffectID::kMetadataEntry,
                                            Attributes::CurrentSpeed::kMetadataEntry,
                                        }));
}

TEST_F(TestDynamicLightingCluster, AcceptedCommands)
{
    ASSERT_TRUE(IsAcceptedCommandsListEqualTo(cluster,
                                              {
                                                  Commands::StartEffect::kMetadataEntry,
                                                  Commands::StopEffect::kMetadataEntry,
                                              }));
}

TEST_F(TestDynamicLightingCluster, ReadInitialState)
{
    ClusterTester tester(cluster);

    uint16_t clusterRevision{};
    ASSERT_SUCCESS(tester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision));
    EXPECT_EQ(clusterRevision, kRevision);

    uint32_t featureMap = 1;
    ASSERT_SUCCESS(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap));
    EXPECT_EQ(featureMap, 0u);

    Attributes::AvailableEffects::TypeInfo::DecodableType availableEffects;
    ASSERT_SUCCESS(tester.ReadAttribute(Attributes::AvailableEffects::Id, availableEffects));
    size_t count = 0;
    ASSERT_SUCCESS(availableEffects.ComputeSize(&count));
    EXPECT_EQ(count, 2u);
    auto effectIterator = availableEffects.begin();
    ASSERT_TRUE(effectIterator.Next());
    EXPECT_EQ(effectIterator.GetValue().effectID, kFireplaceId);
    EXPECT_TRUE(effectIterator.GetValue().label.data_equal("Fireplace"_span));
    EXPECT_EQ(effectIterator.GetValue().maxSpeed, kFireplaceMaxSpeed);

    EXPECT_TRUE(ReadEffectID(tester).IsNull());
    EXPECT_TRUE(ReadSpeed(tester).IsNull());
}

TEST_F(TestDynamicLightingCluster, StartupRejectsInvalidEffects)
{
    struct Case
    {
        const char * name;
        void (*mutate)(std::vector<Structs::EffectStruct::Type> &);
    };
    const Case cases[] = {
        { "duplicate id", [](auto & e) { e[1].effectID = e[0].effectID; } },
        { "duplicate label", [](auto & e) { e[1].label = e[0].label; } },
        { "empty label", [](auto & e) { e[0].label = CharSpan(); } },
        { "label too long", [](auto & e) { e[0].label = "123456789012345678901234567890123"_span; } },
        { "zero max speed", [](auto & e) { e[0].maxSpeed = 0; } },
        { "zero default speed", [](auto & e) { e[0].defaultSpeed = 0; } },
        { "default above max", [](auto & e) { e[0].defaultSpeed = e[0].maxSpeed + 1; } },
        { "more than 128 effects",
          [](auto & e) {
              e.clear();
              for (uint16_t i = 0; i < 129; i++)
              {
                  e.push_back(MakeEffect(i, "x"_span, 1, 1, false));
              }
          } },
    };

    for (const auto & testCase : cases)
    {
        TestDelegate badDelegate;
        testCase.mutate(badDelegate.effects);
        DynamicLightingCluster badCluster(kRootEndpointId, badDelegate);
        EXPECT_EQ(badCluster.Startup(testContext.Get()), CHIP_ERROR_INVALID_ARGUMENT) << testCase.name;
    }
}

TEST_F(TestDynamicLightingCluster, StartupAcceptsEmptyEffectList)
{
    TestDelegate emptyDelegate;
    emptyDelegate.effects.clear();
    DynamicLightingCluster emptyCluster(kRootEndpointId, emptyDelegate);
    EXPECT_SUCCESS(emptyCluster.Startup(testContext.Get()));
    emptyCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestDynamicLightingCluster, StartEffectRejectsUnknownEffect)
{
    ClusterTester tester(cluster);

    EXPECT_EQ(Start(tester, MakeStart(99, 1)), Status::InvalidCommand);
    EXPECT_EQ(delegate.startCalls, 0);
    EXPECT_TRUE(ReadEffectID(tester).IsNull());
}

TEST_F(TestDynamicLightingCluster, StartEffectRejectsSpeedOutOfRange)
{
    ClusterTester tester(cluster);

    EXPECT_EQ(Start(tester, MakeStart(kFireplaceId, 0)), Status::InvalidCommand);
    EXPECT_EQ(Start(tester, MakeStart(kFireplaceId, kFireplaceMaxSpeed + 1)), Status::InvalidCommand);
    EXPECT_EQ(delegate.startCalls, 0);
}

TEST_F(TestDynamicLightingCluster, StartEffectUpdatesState)
{
    ClusterTester tester(cluster);

    EXPECT_EQ(Start(tester, MakeStart(kFireplaceId, kFireplaceMaxSpeed)), Status::Success);

    EXPECT_EQ(delegate.startCalls, 1);
    EXPECT_EQ(delegate.lastStartedId, kFireplaceId);
    EXPECT_EQ(delegate.lastStartedSpeed, kFireplaceMaxSpeed);
    EXPECT_EQ(ReadEffectID(tester), DataModel::MakeNullable<uint16_t>(kFireplaceId));
    EXPECT_EQ(ReadSpeed(tester), DataModel::MakeNullable<uint16_t>(kFireplaceMaxSpeed));
}

TEST_F(TestDynamicLightingCluster, StartEffectReplacesRunningEffect)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(Start(tester, MakeStart(kFireplaceId, 1)), Status::Success);
    EXPECT_EQ(delegate.stopCalls, 0);

    ASSERT_EQ(Start(tester, MakeStart(kCycleId, 20)), Status::Success);
    EXPECT_EQ(delegate.stopCalls, 1);
    EXPECT_EQ(delegate.startCalls, 2);
    EXPECT_EQ(ReadEffectID(tester), DataModel::MakeNullable<uint16_t>(kCycleId));
    EXPECT_EQ(ReadSpeed(tester), DataModel::MakeNullable<uint16_t>(20));
}

TEST_F(TestDynamicLightingCluster, StartEffectDelegateFailureLeavesNoEffectRunning)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(Start(tester, MakeStart(kFireplaceId, 1)), Status::Success);

    delegate.startResult = CHIP_ERROR_INTERNAL;
    EXPECT_EQ(Start(tester, MakeStart(kCycleId, 20)), Status::Failure);
    EXPECT_TRUE(ReadEffectID(tester).IsNull());
    EXPECT_TRUE(ReadSpeed(tester).IsNull());
}

TEST_F(TestDynamicLightingCluster, StopEffectWithNoRunningEffectSucceeds)
{
    ClusterTester tester(cluster);

    EXPECT_EQ(Stop(tester), Status::Success);
    EXPECT_EQ(delegate.stopCalls, 0);
}

TEST_F(TestDynamicLightingCluster, StopEffectClearsState)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(Start(tester, MakeStart(kFireplaceId, 3)), Status::Success);
    EXPECT_EQ(Stop(tester), Status::Success);

    EXPECT_EQ(delegate.stopCalls, 1);
    EXPECT_TRUE(ReadEffectID(tester).IsNull());
    EXPECT_TRUE(ReadSpeed(tester).IsNull());

    EXPECT_EQ(Stop(tester), Status::Success);
    EXPECT_EQ(delegate.stopCalls, 1);
}

TEST_F(TestDynamicLightingCluster, WriteCurrentSpeedWithNoRunningEffect)
{
    ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, static_cast<uint16_t>(1)), Status::InvalidInState);
}

TEST_F(TestDynamicLightingCluster, WriteCurrentSpeedUpdatesRunningEffect)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(Start(tester, MakeStart(kCycleId, 20)), Status::Success);

    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, kCycleMaxSpeed), Status::Success);
    EXPECT_EQ(delegate.lastSetSpeed, kCycleMaxSpeed);
    EXPECT_EQ(ReadSpeed(tester), DataModel::MakeNullable<uint16_t>(kCycleMaxSpeed));
}

TEST_F(TestDynamicLightingCluster, WriteCurrentSpeedRejectsOutOfRangeValues)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(Start(tester, MakeStart(kFireplaceId, 3)), Status::Success);

    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, static_cast<uint16_t>(0)), Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, static_cast<uint16_t>(kFireplaceMaxSpeed + 1)),
              Status::ConstraintError);
    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, DataModel::Nullable<uint16_t>()), Status::ConstraintError);
    EXPECT_EQ(ReadSpeed(tester), DataModel::MakeNullable<uint16_t>(3));
}

TEST_F(TestDynamicLightingCluster, WriteCurrentSpeedDelegateFailureKeepsOldSpeed)
{
    ClusterTester tester(cluster);

    ASSERT_EQ(Start(tester, MakeStart(kFireplaceId, 3)), Status::Success);

    delegate.setSpeedResult = CHIP_ERROR_INTERNAL;
    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, static_cast<uint16_t>(4)), Status::Failure);
    EXPECT_EQ(ReadSpeed(tester), DataModel::MakeNullable<uint16_t>(3));
}
