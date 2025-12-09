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
#include <app/clusters/operational-credentials-server/OperationalCredentialsCluster.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <clusters/AdministratorCommissioning/Enums.h>
#include <clusters/AdministratorCommissioning/Metadata.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <platform/NetworkCommissioning.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>
#include <credentials/PersistentStorageOpCertStore.h>
#include <credentials/tests/CHIPCert_unit_test_vectors.h>
#include <crypto/PersistentStorageOperationalKeystore.h>
#include <lib/support/TestPersistentStorageDelegate.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

#include <app/clusters/testing/ClusterTester.h>
#include <app/clusters/testing/ValidateGlobalAttributes.h>

namespace {

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AdministratorCommissioning;

using chip::app::DataModel::AcceptedCommandEntry;
using chip::app::DataModel::AttributeEntry;

// initialize memory as ReadOnlyBufferBuilder may allocate
struct TestAdministratorCommissioningCluster : public ::testing::Test
{
    static chip::DeviceLayer::TestOnlyCommissionableDataProvider sTestCommissionableDataProvider;
    static chip::Credentials::PersistentStorageOpCertStore sTestOpCertStore;
    static chip::PersistentStorageOperationalKeystore sTestOpKeystore;
    static chip::TestPersistentStorageDelegate sStorageDelegate;

    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(chip::DeviceLayer::PlatformMgr().InitChipStack(), CHIP_NO_ERROR);

        ASSERT_EQ(sTestOpCertStore.Init(&sStorageDelegate), CHIP_NO_ERROR);
        ASSERT_EQ(sTestOpKeystore.Init(&sStorageDelegate), CHIP_NO_ERROR);

        auto & fabricTable = Server::GetInstance().GetFabricTable();
        FabricTable::InitParams initParams;
        initParams.storage             = &sStorageDelegate;
        initParams.operationalKeystore = &sTestOpKeystore;
        initParams.opCertStore         = &sTestOpCertStore;
        ASSERT_EQ(fabricTable.Init(initParams), CHIP_NO_ERROR);

        // Initialize CommissioningWindowManager so it has a valid Server pointer
        ASSERT_EQ(Server::GetInstance().GetCommissioningWindowManager().Init(&Server::GetInstance()), CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        chip::Server::GetInstance().GetCommissioningWindowManager().Shutdown();
        chip::Server::GetInstance().GetFabricTable().Shutdown();
        sTestOpCertStore.Finish();
        sTestOpKeystore.Finish();
        chip::DeviceLayer::PlatformMgr().Shutdown();
        chip::Platform::MemoryShutdown();
    }
};

chip::DeviceLayer::TestOnlyCommissionableDataProvider TestAdministratorCommissioningCluster::sTestCommissionableDataProvider;
chip::Credentials::PersistentStorageOpCertStore TestAdministratorCommissioningCluster::sTestOpCertStore;
chip::PersistentStorageOperationalKeystore TestAdministratorCommissioningCluster::sTestOpKeystore;
chip::TestPersistentStorageDelegate TestAdministratorCommissioningCluster::sStorageDelegate;

const chip::FabricIndex kTestFabricIndex = chip::app::Testing::kTestFabrixIndex;

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
    request.salt = { 0x53, 0x50, 0x41, 0x4B, 0x45, 0x32, 0x50, 0x20, 0x4B, 0x65, 0x79, 0x20, 0x53, 0x61, 0x6C, 0x74 };

    // auto optCertStore = chip::Server::GetInstance().GetOpCertStore();
    // chip::CommonCaseDeviceServerInitParams initParams;
    // EXPECT_EQ(initParams.opCertStore, nullptr);
    auto & fabricTable          = Server::GetInstance().GetFabricTable();
    FabricIndex testFabricIndex = kTestFabricIndex;
    ASSERT_EQ(fabricTable.AddNewFabricForTest(chip::TestCerts::GetRootACertAsset().mCert, chip::TestCerts::GetIAA1CertAsset().mCert,
                                              chip::TestCerts::GetNodeA1CertAsset().mCert,
                                              chip::TestCerts::GetNodeA1CertAsset().mKey, &testFabricIndex),
              CHIP_NO_ERROR);
    ASSERT_NE(testFabricIndex, chip::kUndefinedFabricIndex);
    tester.SetFabricIndex(testFabricIndex);
    // ASSERT_EQ(optCertStore, nullptr);

    // OperationalCredentialsCluster::Context context = { .fabricTable     = Server::GetInstance().GetFabricTable(),
    //                                                    .failSafeContext = Server::GetInstance().GetFailSafeContext(),
    //                                                    .sessionManager  = Server::GetInstance().GetSecureSessionManager(),
    //                                                    .dnssdServer     = app::DnssdServer::Instance(),
    //                                                    .commissioningWindowManager =
    //                                                        Server::GetInstance().GetCommissioningWindowManager() };
    // OperationalCredentialsCluster opCreds(kRootEndpointId, context);
    // chip::Test::ClusterTester opCredsTester(opCreds);
    // EXPECT_EQ(opCreds.Startup(opCredsTester.GetServerClusterContext()), CHIP_NO_ERROR);
    // opCredsTester.SetFabricIndex(testFabricIndex);

    // 1. Arm FailSafe because AddNOC command requires it.
    // auto & failSafe = context.failSafeContext;
    // ASSERT_EQ(failSafe.ArmFailSafe(testFabricIndex, System::Clock::Seconds16(900)), CHIP_NO_ERROR);

    // 2. Add Trusted Root Cert for the fabric.
    // OperationalCredentials::Commands::AddTrustedRootCertificate::Type addRcaRequest;
    // addRcaRequest.rootCACertificate = chip::TestCerts::GetRootACertAsset().mCert;
    // auto root = opCredsTester.Invoke(OperationalCredentials::Commands::AddTrustedRootCertificate::Id, addRcaRequest);
    // ASSERT_TRUE(root.status.has_value());
    // ASSERT_TRUE(root.status->IsSuccess());

    // Read Fabrics attribute to verify the fabric was added
    // OperationalCredentials::Attributes::Fabrics::TypeInfo::DecodableType fabrics;
    // auto stat = opCredsTester.ReadAttribute(OperationalCredentials::Attributes::Fabrics::Id, fabrics);
    // ASSERT_TRUE(stat.IsSuccess());

    auto result = tester.Invoke(Commands::OpenCommissioningWindow::Id, request);
    // ASSERT_EQ(result.status, std::nullopt);
    // ASSERT_TRUE(result.status.has_value());
    // ASSERT_TRUE(result.status->IsSuccess());
    ASSERT_TRUE(result.IsSuccess());

    // if (result.status->IsSuccess()) // NOLINT(bugprone-unchecked-optional-access)
    // {
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
    // }
}
