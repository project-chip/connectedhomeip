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
#include <app/clusters/operational-credentials-server/operational-credentials-cluster.h>
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

/* class MockOperationalCertificateStore : public chip::Credentials::OperationalCertificateStore
{
public:
    struct CertChain
    {
        ByteSpan rcac;
        ByteSpan icac;
        ByteSpan noc;
    };

    MockOperationalCertificateStore()           = default;
    ~MockOperationalCertificateStore() override = default;

    bool hasPendingRoot      = false;
    bool hasPendingNoc       = false;
    bool committed           = false;
    FabricIndex pendingIndex = kUndefinedFabricIndex;

    std::map<FabricIndex, CertChain> mCommitted;
    std::map<FabricIndex, CertChain> mPending;

    bool HasPendingRootCert() const override { return hasPendingRoot; }
    bool HasPendingNocChain() const override { return hasPendingNoc; }

    bool HasCertificateForFabric(FabricIndex fabricIndex, CertChainElement element) const override
    {
        auto it = mCommitted.find(fabricIndex);
        if (it == mCommitted.end())
            return false;

        switch (element)
        {
        case CertChainElement::kRcac:
            return it->second.rcac.data() != nullptr;
        case CertChainElement::kIcac:
            return it->second.icac.data() != nullptr;
        case CertChainElement::kNoc:
            return it->second.noc.data() != nullptr;
        default:
            return false;
        }
    }

    CHIP_ERROR AddNewTrustedRootCertForFabric(FabricIndex fabricIndex, const ByteSpan & rcac) override
    {
        if (rcac.empty())
            return CHIP_ERROR_INVALID_ARGUMENT;

        hasPendingRoot             = true;
        pendingIndex               = fabricIndex;
        mPending[fabricIndex].rcac = rcac;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR AddNewOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac) override
    {
        if (!hasPendingRoot || pendingIndex != fabricIndex)
            return CHIP_ERROR_INCORRECT_STATE;

        hasPendingNoc              = true;
        mPending[fabricIndex].noc  = noc;
        mPending[fabricIndex].icac = icac;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UpdateOpCertsForFabric(FabricIndex fabricIndex, const ByteSpan & noc, const ByteSpan & icac) override
    {
        auto it = mCommitted.find(fabricIndex);
        if (it == mCommitted.end())
            return CHIP_ERROR_INCORRECT_STATE;

        mPending[fabricIndex]      = it->second;
        mPending[fabricIndex].noc  = noc;
        mPending[fabricIndex].icac = icac;
        hasPendingNoc              = true;
        pendingIndex               = fabricIndex;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR CommitOpCertsForFabric(FabricIndex fabricIndex) override
    {
        auto it = mPending.find(fabricIndex);
        if (it == mPending.end())
            return CHIP_ERROR_INCORRECT_STATE;

        mCommitted[fabricIndex] = it->second;
        mPending.erase(it);
        hasPendingNoc  = false;
        hasPendingRoot = false;
        committed      = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveOpCertsForFabric(FabricIndex fabricIndex) override
    {
        mCommitted.erase(fabricIndex);
        mPending.erase(fabricIndex);
        if (pendingIndex == fabricIndex)
        {
            hasPendingNoc  = false;
            hasPendingRoot = false;
        }
        return CHIP_NO_ERROR;
    }

    void RevertPendingOpCerts() override
    {
        mPending.clear();
        hasPendingNoc  = false;
        hasPendingRoot = false;
    }

    void RevertPendingOpCertsExceptRoot() override
    {
        for (auto & [index, chain] : mPending)
        {
            chain.icac = ByteSpan();
            chain.noc  = ByteSpan();
        }
        hasPendingNoc = false;
    }

    CHIP_ERROR GetCertificate(FabricIndex fabricIndex, CertChainElement element, MutableByteSpan & outCertificate) const override
    {
        const CertChain * chain = nullptr;

        auto pendingIt = mPending.find(fabricIndex);
        if (pendingIt != mPending.end())
            chain = &pendingIt->second;
        else
        {
            auto committedIt = mCommitted.find(fabricIndex);
            if (committedIt == mCommitted.end())
                return CHIP_ERROR_NOT_FOUND;
            chain = &committedIt->second;
        }

        const ByteSpan * src = nullptr;
        switch (element)
        {
        case CertChainElement::kRcac:
            src = &chain->rcac;
            break;
        case CertChainElement::kIcac:
            src = &chain->icac;
            break;
        case CertChainElement::kNoc:
            src = &chain->noc;
            break;
        }

        if (src == nullptr || src->empty())
            return CHIP_ERROR_NOT_FOUND;

        if (outCertificate.size() < src->size())
            return CHIP_ERROR_BUFFER_TOO_SMALL;

        memcpy(outCertificate.data(), src->data(), src->size());
        outCertificate.reduce_size(src->size());
        return CHIP_NO_ERROR;
    }
}; */

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

    // FabricTable & fabricTable = Server::GetInstance().GetFabricTable();
    // FabricTable::InitParams initParams;
    // initParams.opCertStore         = &mMockOpCertStore;
    // initParams.storage             = &mTestContext.StorageDelegate();
    // initParams.operationalKeystore = nullptr;
    // fabricTable.Init(initParams);

    auto result = tester.Invoke(Commands::OpenCommissioningWindow::Id, request);
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
