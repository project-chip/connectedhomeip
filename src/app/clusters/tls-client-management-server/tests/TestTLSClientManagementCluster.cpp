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

#include <app/clusters/tls-client-management-server/TLSClientManagementCluster.h>
#include <clusters/TlsClientManagement/Attributes.h>
#include <clusters/TlsClientManagement/Metadata.h>
#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/data-model-provider/tests/TestConstants.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <credentials/FabricTable.h>
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
    uint32_t rootLookupCount   = 0;
    uint32_t clientLookupCount = 0;

    /** @brief Leave certificate state in memory; this mock does not load or persist certificates. */
    CHIP_ERROR Init(PersistentStorageDelegate & storage) override { return CHIP_NO_ERROR; }

    void Finish() override {}

    CHIP_ERROR UpsertRootCertificateEntry(FabricIndex fabric_index, Optional<TLSCAID> & id, RootBuffer & buffer,
                                          const ByteSpan & certificate) override
    {
        return CHIP_NO_ERROR;
    }

    /** @brief Stub unused certificate retrieval; command tests consult HasRootCertificateEntry instead. */
    CHIP_ERROR GetRootCertificateEntry(FabricIndex fabric_index, TLSCAID id, BufferedRootCert & entry) override
    {
        return CHIP_NO_ERROR;
    }

    /** @brief Look up a root certificate and count accesses to verify request validation precedes lookup. */
    CHIP_ERROR HasRootCertificateEntry(FabricIndex fabric_index, TLSCAID id) override
    {
        ++rootLookupCount;
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

    /** @brief Stub unused certificate retrieval; command tests consult HasClientCertificateEntry instead. */
    CHIP_ERROR GetClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id, BufferedClientCert & entry) override
    {
        return CHIP_NO_ERROR;
    }

    /** @brief Look up a client certificate and count accesses to verify request validation precedes lookup. */
    CHIP_ERROR HasClientCertificateEntry(FabricIndex fabric_index, TLSCCDID id) override
    {
        ++clientLookupCount;
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

class MockTLSClientManagementDelegate : public TLSClientManagementDelegate
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
        uint16_t port;
    };

    std::vector<MockEndpoint> endpoints;
    uint16_t nextEndpointID = 1;
    uint32_t provisionCount = 0;

    CHIP_ERROR provisionError = CHIP_NO_ERROR;
    Protocols::InteractionModel::ClusterStatusCode provisionStatus =
        Protocols::InteractionModel::ClusterStatusCode(Protocols::InteractionModel::Status::Success);
    Protocols::InteractionModel::Status removeStatus = Protocols::InteractionModel::Status::Success;
    CHIP_ERROR mutateReferenceCountError             = CHIP_NO_ERROR;

    /** @brief Keep mock endpoints in memory without loading or writing persistent storage. */
    CHIP_ERROR Init(PersistentStorageDelegate & storage) override { return CHIP_NO_ERROR; }

    /** @brief Encode the retained endpoint fields, including their port, for attribute-read controls. */
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
                decodable.port  = endpoint.port;
                ReturnErrorOnFailure(callback(decodable));
            }
        }
        return CHIP_NO_ERROR;
    }

    /** @brief Return a matching endpoint with its original hostname, certificate references, and port. */
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
                decodable.port  = endpoint.port;
                return callback(decodable);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    /** @brief Count provision calls and retain endpoint fields to expose allocation before invalid-port rejection. */
    Protocols::InteractionModel::ClusterStatusCode
    ProvisionEndpoint(EndpointId matterEndpoint, FabricIndex fabric,
                      const Commands::ProvisionEndpoint::DecodableType & provisionReq, uint16_t & endpointID) override
    {
        ++provisionCount;
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
        endpoints.push_back(
            { matterEndpoint, fabric, endpointID, hostname, provisionReq.caid, provisionReq.ccdid, provisionReq.port });

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

struct TestTLSClientManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);

        // Add some test root certificates for the test fabric index.
        constexpr FabricIndex kMockTestFabric = Testing::kTestFabricIndex;
        mMockCertTable.rootCerts.push_back({ kMockTestFabric, 1 });
        mMockCertTable.rootCerts.push_back({ kMockTestFabric, 2 });

        EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        app::SetSafeAttributePersistenceProvider(nullptr);
    }

    MockTLSClientManagementDelegate mMockDelegate;
    MockCertificateTable mMockCertTable;
    FabricTable mFabricTable;

    TLSClientManagementCluster mCluster{ { mFabricTable }, kTestEndpointId, mMockDelegate, mMockCertTable, kMaxProvisioned };

    ClusterTester mClusterTester{ mCluster };

    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
};

TEST_F(TestTLSClientManagementCluster, TestAttributesList)
{
    ReadOnlyBufferBuilder<DataModel::AttributeEntry> listBuilder;
    EXPECT_EQ(mCluster.Attributes(ConcreteClusterPath(kTestEndpointId, TlsClientManagement::Id), listBuilder), CHIP_NO_ERROR);

    ReadOnlyBufferBuilder<DataModel::AttributeEntry> expectedListBuilder;
    AttributeListBuilder attributeListBuilder(expectedListBuilder);
    EXPECT_EQ(attributeListBuilder.Append(Span(Attributes::kMandatoryMetadata), {}), CHIP_NO_ERROR);

    EXPECT_TRUE(EqualAttributeSets(listBuilder.TakeBuffer(), expectedListBuilder.TakeBuffer()));
}

TEST_F(TestTLSClientManagementCluster, TestAcceptedCommands)
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

TEST_F(TestTLSClientManagementCluster, TestReadMaxProvisioned)
{
    uint8_t maxProvisioned = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxProvisioned::Id, maxProvisioned), CHIP_NO_ERROR);
    EXPECT_EQ(maxProvisioned, kMaxProvisioned);
}

TEST_F(TestTLSClientManagementCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestTLSClientManagementCluster, TestReadFeatureMap)
{
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

/** @brief Decode an empty endpoint list as the attribute-read control for rejected provisioning. */
TEST_F(TestTLSClientManagementCluster, TestReadProvisionedEndpointsEmpty)
{
    Attributes::ProvisionedEndpoints::TypeInfo::DecodableType list;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ProvisionedEndpoints::Id, list), CHIP_NO_ERROR);

    auto it = list.begin();
    EXPECT_FALSE(it.Next());
}

/** @brief Retain successful provisioning with a valid port and root certificate as a command-response control. */
TEST_F(TestTLSClientManagementCluster, TestProvisionEndpointSuccess)
{
    Commands::ProvisionEndpoint::Type request;
    request.hostname = ByteSpan::fromCharSpan("example.com"_span);
    request.caid     = 1;
    request.port     = 443;
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

/**
 * @brief Reject port zero with valid certificates before allocating an endpoint or notifying attributes.
 * Exercise both a null and a present client certificate through command TLV encoding and decoding.
 */
TEST_F(TestTLSClientManagementCluster, TestProvisionEndpointRejectsZeroPort)
{
    mMockCertTable.clientCerts.push_back({ Testing::kTestFabricIndex, 1 });
    auto & listener = mClusterTester.GetTestContext().ChangeListener();

    for (bool withClientCertificate : { false, true })
    {
        Commands::ProvisionEndpoint::Type request;
        request.hostname = ByteSpan::fromCharSpan("example.com"_span);
        request.port     = 0;
        request.caid     = 1;
        request.endpointID.SetNull();
        if (withClientCertificate)
        {
            request.ccdid.SetNonNull(1);
        }
        else
        {
            request.ccdid.SetNull();
        }

        auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
        EXPECT_FALSE(result.IsSuccess());
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
        }
        EXPECT_FALSE(result.response.has_value());
        EXPECT_EQ(mMockDelegate.provisionCount, 0u);
        EXPECT_EQ(mMockDelegate.nextEndpointID, 1u);
        EXPECT_TRUE(mMockDelegate.endpoints.empty());
        EXPECT_EQ(mMockCertTable.rootLookupCount, 0u);
        EXPECT_EQ(mMockCertTable.clientLookupCount, 0u);
        EXPECT_TRUE(listener.DirtyList().empty());
        EXPECT_TRUE(listener.ChangedList().empty());

        Attributes::ProvisionedEndpoints::TypeInfo::DecodableType list;
        ASSERT_TRUE(mClusterTester.ReadAttribute(Attributes::ProvisionedEndpoints::Id, list).IsSuccess());
        auto iterator = list.begin();
        EXPECT_FALSE(iterator.Next());
        EXPECT_EQ(iterator.GetStatus(), CHIP_NO_ERROR);
    }
}

/** @brief Return the port constraint error before consulting absent root or client certificate entries. */
TEST_F(TestTLSClientManagementCluster, TestZeroPortValidationPrecedesCertificateLookup)
{
    for (bool missingRoot : { true, false })
    {
        Commands::ProvisionEndpoint::Type request;
        request.hostname = ByteSpan::fromCharSpan("example.com"_span);
        request.port     = 0;
        request.caid     = missingRoot ? 99 : 1;
        request.endpointID.SetNull();
        if (missingRoot)
        {
            request.ccdid.SetNull();
        }
        else
        {
            request.ccdid.SetNonNull(99);
        }

        auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
        EXPECT_FALSE(result.IsSuccess());
        ASSERT_TRUE(result.status.has_value());
        if (result.status.has_value())
        {
            EXPECT_EQ(result.status->GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
            EXPECT_FALSE(result.status->GetStatusCode().GetClusterSpecificCode().has_value());
        }
        EXPECT_FALSE(result.response.has_value());
        EXPECT_EQ(mMockCertTable.rootLookupCount, 0u);
        EXPECT_EQ(mMockCertTable.clientLookupCount, 0u);
        EXPECT_EQ(mMockDelegate.provisionCount, 0u);
        EXPECT_EQ(mMockDelegate.nextEndpointID, 1u);
        EXPECT_TRUE(mMockDelegate.endpoints.empty());
        EXPECT_TRUE(mClusterTester.GetTestContext().ChangeListener().DirtyList().empty());
    }
}

/** @brief Accept ports 1, 443, and 65535 without narrowing the valid range or changing certificate-reference handling. */
TEST_F(TestTLSClientManagementCluster, TestProvisionEndpointValidPortBoundaries)
{
    mMockCertTable.clientCerts.push_back({ Testing::kTestFabricIndex, 1 });
    constexpr uint16_t ports[] = { 1, 443, 65535 };
    auto & listener            = mClusterTester.GetTestContext().ChangeListener();

    for (uint16_t port : ports)
    {
        for (bool withClientCertificate : { false, true })
        {
            Commands::ProvisionEndpoint::Type request;
            request.hostname = ByteSpan::fromCharSpan("example.com"_span);
            request.port     = port;
            request.caid     = 1;
            request.endpointID.SetNull();
            if (withClientCertificate)
            {
                request.ccdid.SetNonNull(1);
            }
            else
            {
                request.ccdid.SetNull();
            }
            const uint16_t nextID = mMockDelegate.nextEndpointID;
            const size_t dirty    = listener.DirtyList().size();

            auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
            ASSERT_TRUE(result.IsSuccess());
            ASSERT_TRUE(result.response.has_value());
            if (result.response.has_value())
            {
                EXPECT_EQ(result.response->endpointID, nextID);
            }
            ASSERT_FALSE(mMockDelegate.endpoints.empty());
            EXPECT_EQ(mMockDelegate.endpoints.back().port, port);
            EXPECT_EQ(mMockDelegate.nextEndpointID, nextID + 1u);
            EXPECT_EQ(listener.DirtyList().size(), dirty + 1u);

            Commands::FindEndpoint::Type find;
            find.endpointID = nextID;
            auto found      = mClusterTester.Invoke<Commands::FindEndpoint::Type>(find);
            ASSERT_TRUE(found.IsSuccess());
            ASSERT_TRUE(found.response.has_value());
            if (found.response.has_value())
            {
                EXPECT_EQ(found.response->endpoint.port, port);
                EXPECT_TRUE(found.response->endpoint.hostname.data_equal(request.hostname));
                EXPECT_EQ(found.response->endpoint.caid, request.caid);
                EXPECT_EQ(found.response->endpoint.ccdid, request.ccdid);
            }
        }
    }
    EXPECT_EQ(mMockDelegate.provisionCount, 6u);
    EXPECT_EQ(mMockDelegate.endpoints.size(), 6u);
    EXPECT_EQ(mMockCertTable.rootLookupCount, 6u);
    EXPECT_EQ(mMockCertTable.clientLookupCount, 3u);
}

/** @brief Reject a port-zero update without consuming an ID or changing a previously provisioned endpoint. */
TEST_F(TestTLSClientManagementCluster, TestZeroPortPreservesProvisionedEndpoint)
{
    Commands::ProvisionEndpoint::Type request;
    request.hostname = ByteSpan::fromCharSpan("example.com"_span);
    request.port     = 443;
    request.caid     = 1;
    request.endpointID.SetNull();
    request.ccdid.SetNull();
    auto first = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    ASSERT_TRUE(first.IsSuccess());
    ASSERT_TRUE(first.response.has_value());
    if (!first.response.has_value())
    {
        return;
    }
    const uint16_t id     = first.response->endpointID;
    const uint16_t nextID = mMockDelegate.nextEndpointID;
    const auto calls      = mMockDelegate.provisionCount;
    auto & listener       = mClusterTester.GetTestContext().ChangeListener();
    const auto dirty      = listener.DirtyList().size();
    const auto changed    = listener.ChangedList().size();
    const auto lookups    = mMockCertTable.rootLookupCount;

    request.hostname = ByteSpan::fromCharSpan("other.example"_span);
    request.port     = 0;
    request.caid     = 2;
    request.endpointID.SetNonNull(id);
    auto rejected = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(rejected.IsSuccess());
    ASSERT_TRUE(rejected.status.has_value());
    if (rejected.status.has_value())
    {
        EXPECT_EQ(rejected.status->GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    }
    EXPECT_FALSE(rejected.response.has_value());
    EXPECT_EQ(mMockDelegate.provisionCount, calls);
    EXPECT_EQ(mMockDelegate.nextEndpointID, nextID);
    ASSERT_EQ(mMockDelegate.endpoints.size(), 1u);
    EXPECT_EQ(mMockDelegate.endpoints[0].hostname, "example.com");
    EXPECT_EQ(mMockDelegate.endpoints[0].port, 443u);
    EXPECT_EQ(mMockDelegate.endpoints[0].caid, 1u);
    EXPECT_EQ(mMockCertTable.rootLookupCount, lookups);
    EXPECT_EQ(listener.DirtyList().size(), dirty);
    EXPECT_EQ(listener.ChangedList().size(), changed);

    Commands::FindEndpoint::Type find;
    find.endpointID = id;
    auto found      = mClusterTester.Invoke<Commands::FindEndpoint::Type>(find);
    ASSERT_TRUE(found.IsSuccess());
    ASSERT_TRUE(found.response.has_value());
    if (found.response.has_value())
    {
        EXPECT_EQ(found.response->endpoint.port, 443u);
        EXPECT_TRUE(found.response->endpoint.hostname.data_equal(ByteSpan::fromCharSpan("example.com"_span)));
    }
}

/** @brief Keep hostname and CA-ID constraint checks isolated from port validation by supplying a valid port. */
TEST_F(TestTLSClientManagementCluster, TestProvisionEndpointConstraintErrors)
{
    // Test hostname too short
    Commands::ProvisionEndpoint::Type request;
    request.hostname = ByteSpan::fromCharSpan("abc"_span);
    request.caid     = 1;
    request.port     = 443;
    request.ccdid.SetNull();

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::ConstraintError);
    }

    // Test invalid CAID
    request.hostname = ByteSpan::fromCharSpan("example.com"_span);
    request.caid     = 65535; // exceeds kMaxRootCertId
    result           = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
}

/** @brief Preserve the missing-root-certificate response for an otherwise valid endpoint request. */
TEST_F(TestTLSClientManagementCluster, TestProvisionEndpointRootCertNotFound)
{
    Commands::ProvisionEndpoint::Type request;
    request.hostname = ByteSpan::fromCharSpan("example.com"_span);
    request.caid     = 99; // Not in mock cert table
    request.port     = 443;
    request.ccdid.SetNull();

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        auto statusCode = result.status.value().GetStatusCode();
        EXPECT_TRUE(statusCode.GetClusterSpecificCode().has_value());
    }
}

/** @brief Preserve the missing-client-certificate response for an otherwise valid endpoint request. */
TEST_F(TestTLSClientManagementCluster, TestProvisionEndpointClientCertNotFound)
{
    Commands::ProvisionEndpoint::Type request;
    request.hostname = ByteSpan::fromCharSpan("example.com"_span);
    request.caid     = 1;
    request.port     = 443;
    request.ccdid.SetNonNull(99); // Not in mock cert table

    auto result = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
}

/** @brief Provision with a valid port and retain successful lookup of the resulting endpoint and hostname. */
TEST_F(TestTLSClientManagementCluster, TestFindEndpointSuccess)
{
    // First provision an endpoint
    Commands::ProvisionEndpoint::Type provisionReq;
    provisionReq.hostname = ByteSpan::fromCharSpan("example.com"_span);
    provisionReq.caid     = 1;
    provisionReq.port     = 443;
    provisionReq.ccdid.SetNull();

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());
    ASSERT_TRUE(provisionResult.response.has_value());

    uint16_t provisionedID = 0;
    if (provisionResult.response.has_value())
    {
        provisionedID = provisionResult.response.value().endpointID;
    }

    // Now find it
    Commands::FindEndpoint::Type findReq;
    findReq.endpointID = provisionedID;

    auto findResult = mClusterTester.Invoke<Commands::FindEndpoint::Type>(findReq);
    EXPECT_TRUE(findResult.IsSuccess());

    if (findResult.response.has_value())
    {
        EXPECT_EQ(findResult.response.value().endpoint.endpointID, provisionedID);
        EXPECT_TRUE(findResult.response.value().endpoint.hostname.data_equal(provisionReq.hostname));
    }
}

TEST_F(TestTLSClientManagementCluster, TestFindEndpointNotFound)
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

/** @brief Reject endpoint IDs outside the declared range before invoking the lookup delegate. */
TEST_F(TestTLSClientManagementCluster, TestFindEndpointConstraintError)
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

/** @brief Provision with a valid port and retain removal of the resulting endpoint. */
TEST_F(TestTLSClientManagementCluster, TestRemoveEndpointSuccess)
{
    // First provision an endpoint
    Commands::ProvisionEndpoint::Type provisionReq;
    provisionReq.hostname = ByteSpan::fromCharSpan("example.com"_span);
    provisionReq.caid     = 1;
    provisionReq.port     = 443;
    provisionReq.ccdid.SetNull();

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionEndpoint::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());
    ASSERT_TRUE(provisionResult.response.has_value());

    uint16_t provisionedID = 0;
    if (provisionResult.response.has_value())
    {
        provisionedID = provisionResult.response.value().endpointID;
    }

    EXPECT_EQ(mMockDelegate.endpoints.size(), 1u);

    // Now remove it
    Commands::RemoveEndpoint::Type removeReq;
    removeReq.endpointID = provisionedID;

    auto removeResult = mClusterTester.Invoke<Commands::RemoveEndpoint::Type>(removeReq);
    EXPECT_TRUE(removeResult.IsSuccess());

    // Verify it was removed
    EXPECT_EQ(mMockDelegate.endpoints.size(), 0u);
}

TEST_F(TestTLSClientManagementCluster, TestRemoveEndpointNotFound)
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

TEST_F(TestTLSClientManagementCluster, TestRemoveEndpointConstraintError)
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
