/*
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
#include <pw_unit_test/framework.h>

#include <app/clusters/administrator-commissioning-server/AdministratorCommissioningCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <clusters/AdministratorCommissioning/Enums.h>
#include <clusters/AdministratorCommissioning/Metadata.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestAdministratorCommissioningCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

static chip::DeviceLayer::TestOnlyCommissionableDataProvider sTestCommissionableDataProvider;

TEST_F(TestAdministratorCommissioningCluster, TestAttributes)
{
    {
        AdministratorCommissioningCluster cluster(kRootEndpointId, {});

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kRootEndpointId, AdministratorCommissioning::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      AdministratorCommissioning::Attributes::WindowStatus::kMetadataEntry,
                      AdministratorCommissioning::Attributes::AdminFabricIndex::kMetadataEntry,
                      AdministratorCommissioning::Attributes::AdminVendorId::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        AdministratorCommissioningWithBasicCommissioningWindowCluster cluster(kRootEndpointId,
                                                                              BitFlags<Feature>{ Feature::kBasic });

        ReadOnlyBufferBuilder<AttributeEntry> builder;
        ASSERT_EQ(cluster.Attributes({ kRootEndpointId, AdministratorCommissioning::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AttributeEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      AdministratorCommissioning::Attributes::WindowStatus::kMetadataEntry,
                      AdministratorCommissioning::Attributes::AdminFabricIndex::kMetadataEntry,
                      AdministratorCommissioning::Attributes::AdminVendorId::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);
        ASSERT_EQ(expectedBuilder.ReferenceExisting(app::DefaultServerCluster::GlobalAttributes()), CHIP_NO_ERROR);

        ASSERT_TRUE(Testing::EqualAttributeSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

TEST_F(TestAdministratorCommissioningCluster, TestCommands)
{
    {
        AdministratorCommissioningCluster cluster(kRootEndpointId, {});

        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kRootEndpointId, AdministratorCommissioning::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
                      AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        EXPECT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        AdministratorCommissioningWithBasicCommissioningWindowCluster cluster(kRootEndpointId, BitFlags<Feature>{});

        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kRootEndpointId, AdministratorCommissioning::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
                      AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        EXPECT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }

    {
        AdministratorCommissioningWithBasicCommissioningWindowCluster cluster(kRootEndpointId,
                                                                              BitFlags<Feature>{ Feature::kBasic });

        ReadOnlyBufferBuilder<AcceptedCommandEntry> builder;
        ASSERT_EQ(cluster.AcceptedCommands({ kRootEndpointId, AdministratorCommissioning::Id }, builder), CHIP_NO_ERROR);

        ReadOnlyBufferBuilder<AcceptedCommandEntry> expectedBuilder;
        ASSERT_EQ(expectedBuilder.AppendElements({
                      AdministratorCommissioning::Commands::OpenCommissioningWindow::kMetadataEntry,
                      AdministratorCommissioning::Commands::RevokeCommissioning::kMetadataEntry,
                      AdministratorCommissioning::Commands::OpenBasicCommissioningWindow::kMetadataEntry,
                  }),
                  CHIP_NO_ERROR);

        EXPECT_TRUE(Testing::EqualAcceptedCommandSets(builder.TakeBuffer(), expectedBuilder.TakeBuffer()));
    }
}

} // namespace

TEST_F(TestAdministratorCommissioningCluster, TestReadAttributesDefaultValues)
{
    AdministratorCommissioningCluster cluster(kRootEndpointId, {});
    chip::Test::ClusterTester tester(cluster);

    {
        Attributes::FeatureMap::TypeInfo::Type feature{};
        ASSERT_EQ(tester.ReadAttribute(Attributes::FeatureMap::Id, feature), CHIP_NO_ERROR);
        ASSERT_EQ(feature, 0u);
    }

    {
        uint16_t revision;
        ASSERT_EQ(tester.ReadAttribute(Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
        ASSERT_EQ(revision, 1u);
    }

    {
        Attributes::WindowStatus::TypeInfo::Type winStatus;
        auto status = tester.ReadAttribute(Attributes::WindowStatus::Id, winStatus);
        ASSERT_TRUE(status.IsSuccess());
        EXPECT_EQ(winStatus, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);
    }

    {
        Attributes::AdminFabricIndex::TypeInfo::Type adminFabric;
        ASSERT_EQ(tester.ReadAttribute(Attributes::AdminFabricIndex::Id, adminFabric), CHIP_NO_ERROR);
        ASSERT_TRUE(adminFabric.IsNull());
    }

    {
        Attributes::AdminVendorId::TypeInfo::Type adminVendor;
        ASSERT_EQ(tester.ReadAttribute(Attributes::AdminVendorId::Id, adminVendor), CHIP_NO_ERROR);
        ASSERT_TRUE(adminVendor.IsNull());
    }
}

TEST_F(TestAdministratorCommissioningCluster, TestAttributeSpecComplianceAfterOpeningWindow)
{
    AdministratorCommissioningCluster cluster(kRootEndpointId, {});
    chip::Test::ClusterTester tester(cluster);

    Attributes::WindowStatus::TypeInfo::DecodableType winStatus;
    auto status = tester.ReadAttribute(Attributes::WindowStatus::Id, winStatus);
    ASSERT_TRUE(status.IsSuccess());
    EXPECT_EQ(winStatus, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kWindowNotOpen);

    Commands::OpenCommissioningWindow::Type request;
    request.commissioningTimeout = 900;
    uint16_t originDiscriminator;
    EXPECT_EQ(sTestCommissionableDataProvider.GetSetupDiscriminator(originDiscriminator), CHIP_NO_ERROR);
    request.discriminator = static_cast<uint16_t>(originDiscriminator + 1);
    chip::Crypto::Spake2pVerifier verifier{};
    request.PAKEPasscodeVerifier = chip::ByteSpan(reinterpret_cast<const uint8_t *>(&verifier), sizeof(verifier));
    request.iterations           = chip::Crypto::kSpake2p_Min_PBKDF_Iterations;
    auto result                  = tester.Invoke(Commands::OpenCommissioningWindow::Id, request);
    ASSERT_TRUE(result.status.has_value());

    if (result.status->IsSuccess()) // NOLINT(bugprone-unchecked-optional-access)
    {
        status = tester.ReadAttribute(Attributes::WindowStatus::Id, winStatus);
        ASSERT_TRUE(status.IsSuccess());
        EXPECT_EQ(winStatus, chip::app::Clusters::AdministratorCommissioning::CommissioningWindowStatusEnum::kEnhancedWindowOpen);

        Attributes::AdminFabricIndex::TypeInfo::Type adminFabric;
        status = tester.ReadAttribute(Attributes::AdminFabricIndex::Id, adminFabric);
        ASSERT_TRUE(status.IsSuccess());
        ASSERT_FALSE(adminFabric.IsNull());

        Attributes::AdminVendorId::TypeInfo::Type adminVendor;
        status = tester.ReadAttribute(Attributes::AdminVendorId::Id, adminVendor);
        ASSERT_TRUE(status.IsSuccess());
        ASSERT_FALSE(adminVendor.IsNull());
    }
}
