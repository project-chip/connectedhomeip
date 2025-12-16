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

#include <app/clusters/tls-client-management-server/TlsClientManagementCluster.h>
#include <clusters/TlsClientManagement/Attributes.h>
#include <clusters/TlsClientManagement/Metadata.h>
#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <lib/support/ReadOnlyBuffer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsClientManagement;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId = 1;
constexpr uint8_t kMaxProvisioned    = 10;

// Mock implementations
class MockCertificateTable : public CertificateTable
{
public:
    struct MockRootCert
    {
        FabricIndex fabric;
        TLSCAID caid;
    };

    struct MockClientCert
    {
        FabricIndex fabric;
        TLSCCDID ccdid;
    };

    std::vector<MockRootCert> rootCerts;
    std::vector<MockClientCert> clientCerts;

    CHIP_ERROR Init(PersistentStorageDelegate & storage) override { return CHIP_NO_ERROR; }

    void Finish() override {}

    CHIP_ERROR UpsertRootCertificateEntry(FabricIndex fabric_index, Optional<TLSCAID> & id, RootBuffer & buffer,
                                          const ByteSpan & certificate) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID id, BufferedRootCert & entry) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID id) override
    {
        for (const auto & cert : rootCerts)
        {
            if (cert.fabric == fabric_index && cert.caid == id)
            {
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR IterateRootCertificates(FabricIndex fabric, BufferedRootCert & store, IterateRootCertFnType iterateFn) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, BufferedClientCert & entry) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id) override
    {
        for (const auto & cert : clientCerts)
        {
            if (cert.fabric == fabric_index && cert.ccdid == id)
            {
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR IterateClientCertificates(FabricIndex fabric, BufferedClientCert & store, IterateClientCertFnType iterateFn) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveRootCertificate(FabricIndex fabric_index, TLSCAID id) override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetRootCertificateCount(FabricIndex fabric, uint8_t & outCount) override
    {
        outCount = 0;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR PrepareClientCertificate(FabricIndex fabric, const ByteSpan & nonce, ClientBuffer & buffer, Optional<TLSCCDID> & id,
                                        MutableByteSpan & csr, MutableByteSpan & nonceSignature) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR UpdateClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, ClientBuffer & buffer,
                                            const ClientCertStruct & entry) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveClientCertificate(FabricIndex fabric_index, TLSCCDID id) override { return CHIP_NO_ERROR; }

    CHIP_ERROR GetClientCertificateCount(FabricIndex fabric, uint8_t & outCount) override
    {
        outCount = 0;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RemoveFabric(FabricIndex fabricIndex) override { return CHIP_NO_ERROR; }
};

class MockTlsClientManagementDelegate : public TlsClientManagementDelegate
{
public:
    struct MockEndpoint
    {
        EndpointId matterEndpoint;
        FabricIndex fabric;
        uint16_t endpointID;
        std::string hostname;
        TLSCAID caid;
        DataModel::Nullable<TLSCCDID> ccdid;
    };

    std::vector<MockEndpoint> endpoints;
    uint16_t nextEndpointID = 1;

    CHIP_ERROR provisionError = CHIP_NO_ERROR;
    Protocols::InteractionModel::ClusterStatusCode provisionStatus =
        Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::Success);
    Protocols::InteractionModel::Status removeStatus = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR mutateReferenceCountError             = CHIP_NO_ERROR;

    CHIP_ERROR Init(PersistentStorageDelegate & storage) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ForEachEndpoint(EndpointId matterEndpoint, FabricIndex fabric, LoadedEndpointCallback callback) override
    {
        for (auto & endpoint : endpoints)
        {
            if (endpoint.matterEndpoint == matterEndpoint && endpoint.fabric == fabric)
            {
                Structs::TLSEndpointStruct::Type decodable;
                decodable.endpointID = endpoint.endpointID;
                decodable.hostname =
                    ByteSpan(reinterpret_cast<const uint8_t *>(endpoint.hostname.c_str()), endpoint.hostname.size());
                decodable.caid  = endpoint.caid;
                decodable.ccdid = endpoint.ccdid;
                ReturnErrorOnFailure(callback(decodable));
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FindProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                           LoadedEndpointCallback callback) override
    {
        for (auto & endpoint : endpoints)
        {
            if (endpoint.matterEndpoint == matterEndpoint && endpoint.fabric == fabric && endpoint.endpointID == endpointID)
            {
                Structs::TLSEndpointStruct::Type decodable;
                decodable.endpointID = endpoint.endpointID;
                decodable.hostname =
                    ByteSpan(reinterpret_cast<const uint8_t *>(endpoint.hostname.c_str()), endpoint.hostname.size());
                decodable.caid  = endpoint.caid;
                decodable.ccdid = endpoint.ccdid;
                return callback(decodable);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    Protocols::InteractionModel::ClusterStatusCode
    ProvisionEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                      const Commands::ProvisionEndpoint::DecodableType & provisionReq, uint16_t & endpointID) override
    {
        if (provisionError != CHIP_NO_ERROR)
        {
            return Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(StatusCodeEnum::kRootCertificateNotFound);
        }

        if (!provisionStatus.IsSuccess())
        {
            return provisionStatus;
        }

        // Store hostname data
        std::string hostname(reinterpret_cast<const char *>(provisionReq.hostname.data()), provisionReq.hostname.size());

        endpointID = nextEndpointID++;
        endpoints.push_back({ matterEndpoint, fabric, endpointID, hostname, provisionReq.caid, provisionReq.ccdid });

        return Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::Success);
    }

    Protocols::InteractionModel::Status RemoveProvisionedEndpointByID(EndpointId matterEndpoint, FabricIndex fabric,
                                                                      uint16_t endpointID) override
    {
        for (auto it = endpoints.begin(); it != endpoints.end(); ++it)
        {
            if (it->matterEndpoint == matterEndpoint && it->fabric == fabric && it->endpointID == endpointID)
            {
                endpoints.erase(it);
                return removeStatus;
            }
        }
        return Protocols::InteractionModel::Status::NotFound;
    }

    void RemoveFabric(FabricIndex fabricIndex) override
    {
        endpoints.erase(std::remove_if(endpoints.begin(), endpoints.end(),
                                       [fabricIndex](const MockEndpoint & ep) { return ep.fabric == fabricIndex; }),
                        endpoints.end());
    }

    CHIP_ERROR MutateEndpointReferenceCount(EndpointId matterEndpoint, FabricIndex fabric, uint16_t endpointID,
                                            int8_t delta) override
    {
        if (mutateReferenceCountError != CHIP_NO_ERROR)
        {
            return mutateReferenceCountError;
        }

        for (auto & endpoint : endpoints)
        {
            if (endpoint.matterEndpoint == matterEndpoint && endpoint.fabric == fabric && endpoint.endpointID == endpointID)
            {
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCAID id) override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, Tls::TLSCCDID id) override
    {
        return CHIP_NO_ERROR;
    }
};

struct TestTlsClientManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);

        // Add some test root certificates for the test fabric index (151)
        constexpr FabricIndex kMockTestFabric = static_cast<FabricIndex>(151);
        mMockCertTable.rootCerts.push_back({ kMockTestFabric, 1 });
        mMockCertTable.rootCerts.push_back({ kMockTestFabric, 2 });

        EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override { app::SetSafeAttributePersistenceProvider(nullptr); }

    MockTlsClientManagementDelegate mMockDelegate;
    MockCertificateTable mMockCertTable;

    TlsClientManagementCluster mCluster{ kTestEndpointId, mMockDelegate, mMockCertTable, kMaxProvisioned };

    ClusterTester mClusterTester{ mCluster };

    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
};

TEST_F(TestTlsClientManagementCluster, TestAttributesList)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> listBuilder;
    EXPECT_EQ(mCluster.Attributes(ConcreteClusterPath(kTestEndpointId, TlsClientManagement::Id), listBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedListBuilder;
    AttributeListBuilder attributeListBuilder(expectedListBuilder);
    EXPECT_EQ(attributeListBuilder.Append(Span(Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);

    EXPECT_TRUE(EqualAttributeSets(listBuilder.TakeBuffer(), expectedListBuilder.TakeBuffer()));
}

TEST_F(TestTlsClientManagementCluster, TestAcceptedCommands)
{
    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> listBuilder;
    EXPECT_EQ(mCluster.AcceptedCommands(ConcreteClusterPath(kTestEndpointId, TlsClientManagement::Id), listBuilder), CHIP_NO_ERROR);

    static constexpr DataModel::AcceptedCommandEntry kExpectedCommands[] = {
        Commands::ProvisionEndpoint::kMetadataEntry,
        Commands::FindEndpoint::kMetadataEntry,
        Commands::RemoveEndpoint::kMetadataEntry,
    };

    ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> expectedListBuilder;
    EXPECT_EQ(expectedListBuilder.ReferenceExisting(kExpectedCommands), CHIP_NO_ERROR);

    EXPECT_TRUE(EqualAcceptedCommandSets(listBuilder.TakeBuffer(), expectedListBuilder.TakeBuffer()));
}

TEST_F(TestTlsClientManagementCluster, TestReadMaxProvisioned)
{
    uint8_t maxProvisioned = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxProvisioned::Id, maxProvisioned), CHIP_NO_ERROR);
    EXPECT_EQ(maxProvisioned, kMaxProvisioned);
}

TEST_F(TestTlsClientManagementCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestTlsClientManagementCluster, TestReadFeatureMap)
{
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestTlsClientManagementCluster, TestReadProvisionedEndpointsEmpty)
{
    Attributes::ProvisionedEndpoints::TypeInfo::DecodableType list;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ProvisionedEndpoints::Id, list), CHIP_NO_ERROR);

    auto it = list.begin();
    EXPECT_FALSE(it.Next());
}

TEST_F(TestTlsClientManagementCluster, TestProvisionEndpointSuccess)
{
    Commands::ProvisionEndpoint::Type request;
    const char * hostnameStr = "example.com";
    request.hostname         = ByteSpan(reinterpret_cast<const uint8_t *>(hostnameStr), 11);
    request.caid             = 1;
    request.ccdid.SetNull();

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().endpointID, 1);
    }

    // Verify endpoint was added
    EXPECT_EQ(mMockDelegate.endpoints.size(), 1u);
    EXPECT_EQ(mMockDelegate.endpoints[0].endpointID, 1);
}

TEST_F(TestTlsClientManagementCluster, TestProvisionEndpointConstraintErrors)
{
    // Test hostname too short
    Commands::ProvisionEndpoint::Type request;
    const char * shortHostname = "abc";
    request.hostname           = ByteSpan(reinterpret_cast<const uint8_t *>(shortHostname), 3);
    request.caid               = 1;
    request.ccdid.SetNull();

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    }

    // Test invalid CAID
    const char * validHostname = "example.com";
    request.hostname           = ByteSpan(reinterpret_cast<const uint8_t *>(validHostname), 11);
    request.caid               = 256; // exceeds kMaxRootCertId
    result                     = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(TestTlsClientManagementCluster, TestProvisionEndpointRootCertNotFound)
{
    Commands::ProvisionEndpoint::Type request;
    const char * exampleHostname = "example.com";
    request.hostname             = ByteSpan(reinterpret_cast<const uint8_t *>(exampleHostname), 11);
    request.caid                 = 99; // Not in mock cert table
    request.ccdid.SetNull();

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        auto statusCode = result.status.value().GetStatusCode();
        EXPECT_TRUE(statusCode.GetClusterSpecificCode().has_value());
    }
}

TEST_F(TestTlsClientManagementCluster, TestProvisionEndpointClientCertNotFound)
{
    Commands::ProvisionEndpoint::Type request;
    const char * hostnameForClientTest = "example.com";
    request.hostname                   = ByteSpan(reinterpret_cast<const uint8_t *>(hostnameForClientTest), 11);
    request.caid                       = 1;
    request.ccdid.SetNonNull(99); // Not in mock cert table

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
}

TEST_F(TestTlsClientManagementCluster, TestFindEndpointSuccess)
{
    // First provision an endpoint
    Commands::ProvisionEndpoint::Type provisionReq;
    const char * hostnameForFind = "example.com";
    provisionReq.hostname        = ByteSpan(reinterpret_cast<const uint8_t *>(hostnameForFind), 11);
    provisionReq.caid            = 1;
    provisionReq.ccdid.SetNull();

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());
    ASSERT_TRUE(provisionResult.response.has_value());

    uint16_t provisionedID = provisionResult.response.value().endpointID;

    // Now find it
    Commands::FindEndpoint::Type findReq;
    findReq.endpointID = provisionedID;

    auto findResult = mClusterTester.Invoke<Commands::FindEndpoint::Type>(findReq);
    EXPECT_TRUE(findResult.IsSuccess());

    if (findResult.response.has_value())
    {
        EXPECT_EQ(findResult.response.value().endpoint.endpointID, provisionedID);
        const char * expectedHostname = "example.com";
        EXPECT_TRUE(findResult.response.value().endpoint.hostname.data_equal(
            ByteSpan(reinterpret_cast<const uint8_t *>(expectedHostname), 11)));
    }
}

TEST_F(TestTlsClientManagementCluster, TestFindEndpointNotFound)
{
    Commands::FindEndpoint::Type request;
    request.endpointID = 999; // Non-existent endpoint

    auto result = mClusterTester.Invoke<Commands::FindEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
    }
}

TEST_F(TestTlsClientManagementCluster, TestFindEndpointConstraintError)
{
    Commands::FindEndpoint::Type request;
    request.endpointID = 65535; // Exceeds kMaxTlsEndpointId

    auto result = mClusterTester.Invoke<Commands::FindEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    }
}

TEST_F(TestTlsClientManagementCluster, TestRemoveEndpointSuccess)
{
    // First provision an endpoint
    Commands::ProvisionEndpoint::Type provisionReq;
    const char * hostnameForRemove = "example.com";
    provisionReq.hostname          = ByteSpan(reinterpret_cast<const uint8_t *>(hostnameForRemove), 11);
    provisionReq.caid              = 1;
    provisionReq.ccdid.SetNull();

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());
    ASSERT_TRUE(provisionResult.response.has_value());

    uint16_t provisionedID = provisionResult.response.value().endpointID;

    EXPECT_EQ(mMockDelegate.endpoints.size(), 1u);

    // Now remove it
    Commands::RemoveEndpoint::Type removeReq;
    removeReq.endpointID = provisionedID;

    auto removeResult = mClusterTester.Invoke<Commands::RemoveEndpoint::Type>(removeReq);
    EXPECT_TRUE(removeResult.IsSuccess());

    // Verify it was removed
    EXPECT_EQ(mMockDelegate.endpoints.size(), 0u);
}

TEST_F(TestTlsClientManagementCluster, TestRemoveEndpointNotFound)
{
    Commands::RemoveEndpoint::Type request;
    request.endpointID = 999; // Non-existent endpoint

    auto result = mClusterTester.Invoke<Commands::RemoveEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
    }
}

TEST_F(TestTlsClientManagementCluster, TestRemoveEndpointConstraintError)
{
    Commands::RemoveEndpoint::Type request;
    request.endpointID = 65535; // Exceeds kMaxTlsEndpointId

    auto result = mClusterTester.Invoke<Commands::RemoveEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    }
}

} // namespace
