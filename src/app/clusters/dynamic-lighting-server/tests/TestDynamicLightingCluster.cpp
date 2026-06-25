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
#include <pw_unit_test/framework.h>

#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/DynamicLighting/Attributes.h>
#include <clusters/DynamicLighting/Commands.h>
#include <clusters/DynamicLighting/Metadata.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DynamicLighting;
using namespace chip::Testing;

namespace {

using Protocols::InteractionModel::ClusterStatusCode;
using Protocols::InteractionModel::Status;

struct TestDynamicLightingCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    TestDynamicLightingCluster() : cluster(kRootEndpointId) {}

    void SetUp() override { ASSERT_EQ(cluster.Startup(testContext.Get()), CHIP_NO_ERROR); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestServerClusterContext testContext;
    DynamicLightingCluster cluster;
};

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

TEST_F(TestDynamicLightingCluster, ReadPlaceholderAttributes)
{
    ClusterTester tester(cluster);

    uint16_t clusterRevision{};
    ASSERT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);

    uint32_t featureMap = 1;
    ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);

    Attributes::AvailableEffects::TypeInfo::DecodableType availableEffects;
    ASSERT_EQ(tester.ReadAttribute(Attributes::AvailableEffects::Id, availableEffects), CHIP_NO_ERROR);
    auto effectIterator = availableEffects.begin();
    ASSERT_FALSE(effectIterator.Next());

    Attributes::CurrentEffectID::TypeInfo::DecodableType currentEffectId;
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentEffectID::Id, currentEffectId), CHIP_NO_ERROR);
    EXPECT_TRUE(currentEffectId.IsNull());

    Attributes::CurrentSpeed::TypeInfo::DecodableType currentSpeed;
    ASSERT_EQ(tester.ReadAttribute(Attributes::CurrentSpeed::Id, currentSpeed), CHIP_NO_ERROR);
    EXPECT_TRUE(currentSpeed.IsNull());
}

TEST_F(TestDynamicLightingCluster, WriteCurrentSpeedWithNoRunningEffect)
{
    ClusterTester tester(cluster);

    EXPECT_EQ(tester.WriteAttribute(Attributes::CurrentSpeed::Id, static_cast<uint16_t>(1)),
              Protocols::InteractionModel::Status::InvalidInState);
}

TEST_F(TestDynamicLightingCluster, ReadUnsupportedAttribute)
{
    constexpr AttributeId kUnsupportedAttributeId = 0xFFF0;

    ReadOperation readOperation(kRootEndpointId, DynamicLighting::Id, kUnsupportedAttributeId);
    std::unique_ptr<AttributeValueEncoder> encoder = readOperation.StartEncoding();

    EXPECT_EQ(cluster.ReadAttribute(readOperation.GetRequest(), *encoder),
              Protocols::InteractionModel::Status::UnsupportedAttribute);
}

TEST_F(TestDynamicLightingCluster, WriteUnsupportedAttribute)
{
    constexpr AttributeId kUnsupportedAttributeId = 0xFFF0;

    WriteOperation writeOperation(kRootEndpointId, DynamicLighting::Id, kUnsupportedAttributeId);
    AttributeValueDecoder decoder = writeOperation.DecoderFor(static_cast<uint16_t>(1));

    EXPECT_EQ(cluster.WriteAttribute(writeOperation.GetRequest(), decoder), Protocols::InteractionModel::Status::UnsupportedWrite);
}

TEST_F(TestDynamicLightingCluster, InvokeStartEffectWithNoAvailableEffects)
{
    ClusterTester tester(cluster);

    Commands::StartEffect::Type request;
    request.effectID = 1;
    request.speed    = 1;

    auto result = tester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), std::make_optional(ClusterStatusCode(Status::InvalidCommand)));
}

TEST_F(TestDynamicLightingCluster, InvokeUnsupportedCommand)
{
    constexpr CommandId kUnsupportedCommandId = 0xFFF0;

    MockCommandHandler handler;
    DataModel::InvokeRequest request({ kRootEndpointId, DynamicLighting::Id, kUnsupportedCommandId },
                                     handler.GetSubjectDescriptor());

    uint8_t buffer[16];
    TLV::TLVWriter writer;
    writer.Init(buffer);

    TLV::TLVType outerContainerType;
    ASSERT_EQ(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, outerContainerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.EndContainer(outerContainerType), CHIP_NO_ERROR);
    ASSERT_EQ(writer.Finalize(), CHIP_NO_ERROR);

    TLV::TLVReader reader;
    reader.Init(buffer, writer.GetLengthWritten());
    ASSERT_EQ(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()), CHIP_NO_ERROR);

    EXPECT_EQ(cluster.InvokeCommand(request, reader, &handler), Protocols::InteractionModel::Status::UnsupportedCommand);
}

TEST_F(TestDynamicLightingCluster, InvokeStopEffectWithNoRunningEffect)
{
    ClusterTester tester(cluster);

    Commands::StopEffect::Type request;

    auto result = tester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), std::make_optional(ClusterStatusCode(Status::InvalidCommand)));
}
