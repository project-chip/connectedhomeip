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

#include <app/clusters/tls-certificate-management-server/TLSCertificateManagementCluster.h>
#include <clusters/TlsCertificateManagement/Attributes.h>
#include <clusters/TlsCertificateManagement/Metadata.h>
#include <pw_unit_test/framework.h>

#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/server-cluster/testing/AttributeTesting.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/TestServerClusterContext.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/ReadOnlyBuffer.h>
#include <lib/support/TimeUtils.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Tls;
using namespace chip::app::Clusters::TlsCertificateManagement;
using namespace chip::Testing;

namespace {

constexpr EndpointId kTestEndpointId     = 1;
constexpr uint8_t kMaxRootCertificates   = 10;
constexpr uint8_t kMaxClientCertificates = 10;

// Helper function to generate a valid test certificate
CHIP_ERROR GenerateTestCertificate(MutableByteSpan & certSpan)
{
    using namespace Credentials;
    using namespace Crypto;

    // Generate a keypair for the certificate
    P256Keypair keypair;
    ReturnErrorOnFailure(keypair.Initialize(Crypto::ECPKeyTarget::ECDSA));

    // Set up certificate parameters with valid dates
    // validityStart = 1: Just after CHIP epoch (2000-01-01 00:00:01 UTC) - always in the past
    //                    Note: 0 is special (kNullCertTime) and maps to 9999-12-31, which would fail
    // validityEnd = 0:   kNullCertTime maps to 9999-12-31 23:59:59 UTC - always in the future
    // This ensures the certificate passes IsCertificateValidAtCurrentTime regardless of system time
    uint32_t validityStart = 1;
    uint32_t validityEnd   = kNullCertTime;

    ChipDN subjectDN;
    ReturnErrorOnFailure(subjectDN.AddAttribute_MatterRCACId(0x1234ABCD));

    X509CertRequestParams certParams = { 1, validityStart, validityEnd, subjectDN, subjectDN };

    // Generate the certificate
    return NewRootX509Cert(certParams, keypair, certSpan);
}

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

class MockCertificateDependencyChecker : public CertificateDependencyChecker
{
public:
    CHIP_ERROR RootCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, TLSCAID id) override { return CHIP_NO_ERROR; }

    CHIP_ERROR ClientCertCanBeRemoved(EndpointId matterEndpoint, FabricIndex fabric, TLSCCDID id) override { return CHIP_NO_ERROR; }
};

class MockTLSCertificateManagementDelegate : public TLSCertificateManagementDelegate
{
public:
    struct MockRootCert
    {
        EndpointId matterEndpoint;
        FabricIndex fabric;
        TLSCAID caid;
        std::vector<uint8_t> certificate;
        std::vector<uint8_t> fingerprint;
    };

    struct MockClientCert
    {
        EndpointId matterEndpoint;
        FabricIndex fabric;
        TLSCCDID ccdid;
        std::vector<uint8_t> certificate;
        std::vector<uint8_t> fingerprint;
        TLSCAID caid;
    };

    std::vector<MockRootCert> rootCerts;
    std::vector<MockClientCert> clientCerts;

    Protocols::InteractionModel::Status provisionRootStatus   = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status provisionClientStatus = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status removeRootStatus      = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status removeClientStatus    = Protocols::InteractionModel::Status::Success;
    Protocols::InteractionModel::Status generateCsrStatus     = Protocols::InteractionModel::Status::Success;

    TLSCAID nextRootCertId            = 1;
    mutable TLSCCDID nextClientCertId = 1;

    // Reference to mock certificate table for coordinating GenerateClientCsr with HasClientCertificateEntry
    MockCertificateTable * mCertTable = nullptr;

    void SetCertificateTable(MockCertificateTable * certTable) { mCertTable = certTable; }

    Protocols::InteractionModel::Status ProvisionRootCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                          const ProvisionRootCertificateType & provisionReq,
                                                          TLSCAID & outCaid) override
    {
        if (provisionRootStatus != Protocols::InteractionModel::Status::Success)
        {
            return provisionRootStatus;
        }

        outCaid = nextRootCertId++;
        MockRootCert cert;
        cert.matterEndpoint = matterEndpoint;
        cert.fabric         = fabric;
        cert.caid           = outCaid;
        cert.certificate    = std::vector<uint8_t>(provisionReq.certificate.data(),
                                                provisionReq.certificate.data() + provisionReq.certificate.size());
        rootCerts.push_back(cert);

        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR LoadedRootCerts(EndpointId matterEndpoint, FabricIndex fabric,
                               LoadedRootCertificateCallback loadedCallback) const override
    {
        for (const auto & cert : rootCerts)
        {
            if (cert.matterEndpoint == matterEndpoint && cert.fabric == fabric)
            {
                Structs::TLSCertStruct::Type decodable;
                decodable.caid = cert.caid;
                decodable.certificate.SetValue(ByteSpan(cert.certificate.data(), cert.certificate.size()));
                ReturnErrorOnFailure(loadedCallback(decodable));
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RootCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                  RootCertificateListCallback loadedCallback) const override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FindRootCert(EndpointId matterEndpoint, FabricIndex fabric, TLSCAID id,
                            LoadedRootCertificateCallback loadedCallback) const override
    {
        for (const auto & cert : rootCerts)
        {
            if (cert.matterEndpoint == matterEndpoint && cert.fabric == fabric && cert.caid == id)
            {
                Structs::TLSCertStruct::Type decodable;
                decodable.caid = cert.caid;
                decodable.certificate.SetValue(ByteSpan(cert.certificate.data(), cert.certificate.size()));
                return loadedCallback(decodable);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR LookupRootCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & fingerprint,
                                           LoadedRootCertificateCallback loadedCallback) const override
    {
        for (const auto & cert : rootCerts)
        {
            if (cert.matterEndpoint == matterEndpoint && cert.fabric == fabric && cert.fingerprint.size() == fingerprint.size() &&
                memcmp(cert.fingerprint.data(), fingerprint.data(), fingerprint.size()) == 0)
            {
                Structs::TLSCertStruct::Type decodable;
                decodable.caid = cert.caid;
                decodable.certificate.SetValue(ByteSpan(cert.certificate.data(), cert.certificate.size()));
                return loadedCallback(decodable);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR LookupRootCert(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & cert,
                              LoadedRootCertificateCallback loadedCallback) const override
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    Protocols::InteractionModel::Status RemoveRootCert(EndpointId matterEndpoint, FabricIndex fabric, TLSCAID id) override
    {
        if (removeRootStatus != Protocols::InteractionModel::Status::Success)
        {
            return removeRootStatus;
        }

        for (auto it = rootCerts.begin(); it != rootCerts.end(); ++it)
        {
            if (it->matterEndpoint == matterEndpoint && it->fabric == fabric && it->caid == id)
            {
                rootCerts.erase(it);
                return Protocols::InteractionModel::Status::Success;
            }
        }
        return Protocols::InteractionModel::Status::NotFound;
    }

    Protocols::InteractionModel::Status GenerateClientCsr(EndpointId matterEndpoint, FabricIndex fabric,
                                                          const ClientCsrType & request,
                                                          GeneratedCsrCallback loadedCallback) const override
    {
        if (generateCsrStatus != Protocols::InteractionModel::Status::Success)
        {
            return generateCsrStatus;
        }

        ClientCsrResponseType response;
        uint8_t dummyCsr[32]    = { 0x01, 0x02, 0x03 };
        uint8_t dummySig[32]    = { 0x04, 0x05, 0x06 };
        response.csr            = ByteSpan(dummyCsr, sizeof(dummyCsr));
        response.nonceSignature = ByteSpan(dummySig, sizeof(dummySig));

        // Allocate a ccdid and add entry to certificate table (simulates real delegate behavior)
        TLSCCDID ccdid = nextClientCertId++;
        if (mCertTable != nullptr)
        {
            mCertTable->clientCerts.push_back({ fabric, ccdid });
        }
        response.ccdid = ccdid;

        return loadedCallback(response);
    }

    Protocols::InteractionModel::Status ProvisionClientCert(EndpointId matterEndpoint, FabricIndex fabric,
                                                            const ProvisionClientCertificateType & provisionReq) override
    {
        if (provisionClientStatus != Protocols::InteractionModel::Status::Success)
        {
            return provisionClientStatus;
        }

        MockClientCert cert;
        cert.matterEndpoint = matterEndpoint;
        cert.fabric         = fabric;
        cert.ccdid          = provisionReq.ccdid;
        cert.certificate    = std::vector<uint8_t>(provisionReq.clientCertificate.data(),
                                                provisionReq.clientCertificate.data() + provisionReq.clientCertificate.size());
        clientCerts.push_back(cert);

        return Protocols::InteractionModel::Status::Success;
    }

    CHIP_ERROR LoadedClientCerts(EndpointId matterEndpoint, FabricIndex fabric,
                                 LoadedClientCertificateCallback loadedCallback) const override
    {
        for (const auto & cert : clientCerts)
        {
            if (cert.matterEndpoint == matterEndpoint && cert.fabric == fabric)
            {
                Structs::TLSClientCertificateDetailStruct::Type decodable;
                decodable.ccdid = cert.ccdid;
                decodable.clientCertificate.SetValue(
                    DataModel::MakeNullable(ByteSpan(cert.certificate.data(), cert.certificate.size())));
                ReturnErrorOnFailure(loadedCallback(decodable));
            }
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ClientCertsForFabric(EndpointId matterEndpoint, FabricIndex fabric,
                                    ClientCertificateListCallback loadedCallback) const override
    {
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR FindClientCert(EndpointId matterEndpoint, FabricIndex fabric, TLSCCDID id,
                              LoadedClientCertificateCallback loadedCallback) const override
    {
        for (const auto & cert : clientCerts)
        {
            if (cert.matterEndpoint == matterEndpoint && cert.fabric == fabric && cert.ccdid == id)
            {
                Structs::TLSClientCertificateDetailStruct::Type decodable;
                decodable.ccdid = cert.ccdid;
                decodable.clientCertificate.SetValue(
                    DataModel::MakeNullable(ByteSpan(cert.certificate.data(), cert.certificate.size())));
                return loadedCallback(decodable);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR LookupClientCertByFingerprint(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & fingerprint,
                                             LoadedClientCertificateCallback loadedCallback) const override
    {
        for (const auto & cert : clientCerts)
        {
            if (cert.matterEndpoint == matterEndpoint && cert.fabric == fabric && cert.fingerprint.size() == fingerprint.size() &&
                memcmp(cert.fingerprint.data(), fingerprint.data(), fingerprint.size()) == 0)
            {
                Structs::TLSClientCertificateDetailStruct::Type decodable;
                decodable.ccdid = cert.ccdid;
                decodable.clientCertificate.SetValue(
                    DataModel::MakeNullable(ByteSpan(cert.certificate.data(), cert.certificate.size())));
                return loadedCallback(decodable);
            }
        }
        return CHIP_ERROR_NOT_FOUND;
    }

    CHIP_ERROR LookupClientCert(EndpointId matterEndpoint, FabricIndex fabric, const ByteSpan & certificate,
                                LoadedClientCertificateCallback loadedCallback) const override
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    Protocols::InteractionModel::Status RemoveClientCert(EndpointId matterEndpoint, FabricIndex fabric, TLSCCDID id) override
    {
        if (removeClientStatus != Protocols::InteractionModel::Status::Success)
        {
            return removeClientStatus;
        }

        for (auto it = clientCerts.begin(); it != clientCerts.end(); ++it)
        {
            if (it->matterEndpoint == matterEndpoint && it->fabric == fabric && it->ccdid == id)
            {
                clientCerts.erase(it);
                return Protocols::InteractionModel::Status::Success;
            }
        }
        return Protocols::InteractionModel::Status::NotFound;
    }
};

struct TestTLSCertificateManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { Platform::MemoryShutdown(); }

    void SetUp() override
    {
        VerifyOrDie(mPersistenceProvider.Init(&mClusterTester.GetServerClusterContext().storage) == CHIP_NO_ERROR);
        app::SetSafeAttributePersistenceProvider(&mPersistenceProvider);

        // Wire up mock delegate to mock certificate table so GenerateClientCsr adds entries
        mMockDelegate.SetCertificateTable(&mMockCertTable);

        // Add some test root certificates for the test fabric index (151)
        constexpr FabricIndex kMockTestFabric = static_cast<FabricIndex>(151);
        mMockCertTable.rootCerts.push_back({ kMockTestFabric, 1 });
        mMockCertTable.rootCerts.push_back({ kMockTestFabric, 2 });

        EXPECT_EQ(mCluster.Startup(mClusterTester.GetServerClusterContext()), CHIP_NO_ERROR);
    }

    void TearDown() override
    {
        mCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
        app::SetSafeAttributePersistenceProvider(nullptr);
    }

    MockTLSCertificateManagementDelegate mMockDelegate;
    MockCertificateTable mMockCertTable;
    MockCertificateDependencyChecker mMockDependencyChecker;
    FabricTable mFabricTable;

    TLSCertificateManagementCluster mCluster{ { mFabricTable }, kTestEndpointId,      mMockDelegate,         mMockDependencyChecker,
                                              mMockCertTable,   kMaxRootCertificates, kMaxClientCertificates };

    ClusterTester mClusterTester{ mCluster };

    app::DefaultSafeAttributePersistenceProvider mPersistenceProvider;
};

TEST_F(TestTLSCertificateManagementCluster, TestReadMaxRootCertificates)
{
    uint8_t maxRootCertificates = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxRootCertificates::Id, maxRootCertificates), CHIP_NO_ERROR);
    EXPECT_EQ(maxRootCertificates, kMaxRootCertificates);
}

TEST_F(TestTLSCertificateManagementCluster, TestReadMaxClientCertificates)
{
    uint8_t maxClientCertificates = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::MaxClientCertificates::Id, maxClientCertificates), CHIP_NO_ERROR);
    EXPECT_EQ(maxClientCertificates, kMaxClientCertificates);
}

TEST_F(TestTLSCertificateManagementCluster, TestReadClusterRevision)
{
    uint16_t clusterRevision = 0;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ClusterRevision::Id, clusterRevision), CHIP_NO_ERROR);
    EXPECT_EQ(clusterRevision, kRevision);
}

TEST_F(TestTLSCertificateManagementCluster, TestReadFeatureMap)
{
    uint32_t featureMap = 1;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::FeatureMap::Id, featureMap), CHIP_NO_ERROR);
    EXPECT_EQ(featureMap, 0u);
}

TEST_F(TestTLSCertificateManagementCluster, TestReadProvisionedRootCertificatesEmpty)
{
    Attributes::ProvisionedRootCertificates::TypeInfo::DecodableType list;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ProvisionedRootCertificates::Id, list), CHIP_NO_ERROR);

    auto it = list.begin();
    EXPECT_FALSE(it.Next());
}

TEST_F(TestTLSCertificateManagementCluster, TestReadProvisionedClientCertificatesEmpty)
{
    Attributes::ProvisionedClientCertificates::TypeInfo::DecodableType list;
    EXPECT_EQ(mClusterTester.ReadAttribute(Attributes::ProvisionedClientCertificates::Id, list), CHIP_NO_ERROR);

    auto it = list.begin();
    EXPECT_FALSE(it.Next());
}

TEST_F(TestTLSCertificateManagementCluster, TestProvisionRootCertificateSuccess)
{
    // Generate a valid test certificate
    uint8_t certBuffer[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuffer);
    CHIP_ERROR certGenErr = GenerateTestCertificate(certSpan);
    ASSERT_EQ(certGenErr, CHIP_NO_ERROR);
    EXPECT_GT(certSpan.size(), 0u);
    ChipLogProgress(Zcl, "Generated certificate of size %u", static_cast<unsigned>(certSpan.size()));

    Commands::ProvisionRootCertificate::Type request;
    request.certificate = certSpan;

    auto result = mClusterTester.Invoke<Commands::ProvisionRootCertificate::Type>(request);
    EXPECT_TRUE(result.IsSuccess());
    if (!result.IsSuccess())
    {
        if (result.status.has_value())
        {
            ChipLogError(Zcl, "ProvisionRootCertificate failed with status: 0x%x",
                         to_underlying(result.status.value().GetStatusCode().GetStatus()));
        }
        else
        {
            ChipLogError(Zcl, "ProvisionRootCertificate failed with no status code");
        }
    }

    if (result.response.has_value())
    {
        EXPECT_EQ(result.response.value().caid, 1);
    }

    // Verify certificate was added
    EXPECT_EQ(mMockDelegate.rootCerts.size(), 1u);
    if (mMockDelegate.rootCerts.size() > 0)
    {
        EXPECT_EQ(mMockDelegate.rootCerts[0].caid, 1);
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestFindRootCertificateSuccess)
{
    // Generate a valid test certificate
    uint8_t certBuffer[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuffer);
    EXPECT_EQ(GenerateTestCertificate(certSpan), CHIP_NO_ERROR);

    // First provision a certificate
    Commands::ProvisionRootCertificate::Type provisionReq;
    provisionReq.certificate = certSpan;

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionRootCertificate::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());
    ASSERT_TRUE(provisionResult.response.has_value());

    TLSCAID provisionedId = 0;
    if (provisionResult.response.has_value())
    {
        provisionedId = provisionResult.response.value().caid;
    }

    // Now find it
    Commands::FindRootCertificate::Type findReq;
    findReq.caid = provisionedId;

    auto findResult = mClusterTester.Invoke<Commands::FindRootCertificate::Type>(findReq);
    EXPECT_TRUE(findResult.IsSuccess());

    if (findResult.response.has_value())
    {
        auto iter = findResult.response.value().certificateDetails.begin();
        ASSERT_TRUE(iter.Next());
        EXPECT_EQ(iter.GetValue().caid, provisionedId);
        EXPECT_TRUE(iter.GetValue().certificate.Value().data_equal(certSpan));
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestFindRootCertificateNotFound)
{
    Commands::FindRootCertificate::Type request;
    request.caid = 999; // Non-existent certificate

    auto result = mClusterTester.Invoke<Commands::FindRootCertificate::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestRemoveRootCertificateSuccess)
{
    // Generate a valid test certificate
    uint8_t certBuffer[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuffer);
    EXPECT_EQ(GenerateTestCertificate(certSpan), CHIP_NO_ERROR);

    // First provision a certificate
    Commands::ProvisionRootCertificate::Type provisionReq;
    provisionReq.certificate = certSpan;

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionRootCertificate::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());
    ASSERT_TRUE(provisionResult.response.has_value());

    TLSCAID provisionedId = 0;
    if (provisionResult.response.has_value())
    {
        provisionedId = provisionResult.response.value().caid;
    }

    EXPECT_EQ(mMockDelegate.rootCerts.size(), 1u);

    // Now remove it
    Commands::RemoveRootCertificate::Type removeReq;
    removeReq.caid = provisionedId;

    auto removeResult = mClusterTester.Invoke<Commands::RemoveRootCertificate::Type>(removeReq);
    EXPECT_TRUE(removeResult.IsSuccess());

    // Verify it was removed
    EXPECT_EQ(mMockDelegate.rootCerts.size(), 0u);
}

TEST_F(TestTLSCertificateManagementCluster, TestRemoveRootCertificateNotFound)
{
    Commands::RemoveRootCertificate::Type request;
    request.caid = 999; // Non-existent certificate

    auto result = mClusterTester.Invoke<Commands::RemoveRootCertificate::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestGenerateClientCsrSuccess)
{
    Commands::ClientCSR::Type request;
    uint8_t dummyNonce[32] = { 0x01, 0x02, 0x03 };
    request.nonce          = ByteSpan(dummyNonce, sizeof(dummyNonce));

    auto result = mClusterTester.Invoke<Commands::ClientCSR::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    if (result.response.has_value())
    {
        EXPECT_GT(result.response.value().csr.size(), 0u);
        EXPECT_GT(result.response.value().nonceSignature.size(), 0u);
        EXPECT_EQ(result.response.value().ccdid, 1);
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestProvisionClientCertificateSuccess)
{
    // Step 1: Generate a CSR to create the key pair and get the ccdid
    Commands::ClientCSR::Type csrRequest;
    uint8_t dummyNonce[32] = { 0x01, 0x02, 0x03 };
    csrRequest.nonce       = ByteSpan(dummyNonce, sizeof(dummyNonce));

    auto csrResult = mClusterTester.Invoke<Commands::ClientCSR::Type>(csrRequest);
    EXPECT_TRUE(csrResult.IsSuccess());
    ASSERT_TRUE(csrResult.response.has_value());

    TLSCCDID ccdid = 0;
    if (csrResult.response.has_value())
    {
        ccdid = csrResult.response.value().ccdid;
    }

    // Step 2: Generate a valid test certificate
    uint8_t certBuffer[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuffer);
    EXPECT_EQ(GenerateTestCertificate(certSpan), CHIP_NO_ERROR);

    // Step 3: Provision the client certificate using the ccdid from the CSR
    Commands::ProvisionClientCertificate::Type request;
    request.clientCertificate = certSpan;
    request.ccdid             = ccdid;

    auto result = mClusterTester.Invoke<Commands::ProvisionClientCertificate::Type>(request);
    EXPECT_TRUE(result.IsSuccess());

    // Verify certificate was added
    EXPECT_EQ(mMockDelegate.clientCerts.size(), 1u);
    if (mMockDelegate.clientCerts.size() > 0)
    {
        EXPECT_EQ(mMockDelegate.clientCerts[0].ccdid, ccdid);
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestFindClientCertificateSuccess)
{
    // Step 1: Generate a CSR to create the key pair and get the ccdid
    Commands::ClientCSR::Type csrRequest;
    uint8_t dummyNonce[32] = { 0x01, 0x02, 0x03 };
    csrRequest.nonce       = ByteSpan(dummyNonce, sizeof(dummyNonce));

    auto csrResult = mClusterTester.Invoke<Commands::ClientCSR::Type>(csrRequest);
    EXPECT_TRUE(csrResult.IsSuccess());
    ASSERT_TRUE(csrResult.response.has_value());

    TLSCCDID ccdid = 0;
    if (csrResult.response.has_value())
    {
        ccdid = csrResult.response.value().ccdid;
    }

    // Step 2: Generate a valid test client certificate
    uint8_t certBuffer[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuffer);
    EXPECT_EQ(GenerateTestCertificate(certSpan), CHIP_NO_ERROR);

    // Step 3: Provision the client certificate
    Commands::ProvisionClientCertificate::Type provisionReq;
    provisionReq.clientCertificate = certSpan;
    provisionReq.ccdid             = ccdid;

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionClientCertificate::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());

    // Now find it
    Commands::FindClientCertificate::Type findReq;
    findReq.ccdid = ccdid;

    auto findResult = mClusterTester.Invoke<Commands::FindClientCertificate::Type>(findReq);
    EXPECT_TRUE(findResult.IsSuccess());

    if (findResult.response.has_value())
    {
        auto iter = findResult.response.value().certificateDetails.begin();
        ASSERT_TRUE(iter.Next());
        EXPECT_EQ(iter.GetValue().ccdid, ccdid);
        EXPECT_TRUE(iter.GetValue().clientCertificate.Value().Value().data_equal(certSpan));
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestFindClientCertificateNotFound)
{
    Commands::FindClientCertificate::Type request;
    request.ccdid = 999; // Non-existent certificate

    auto result = mClusterTester.Invoke<Commands::FindClientCertificate::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
    }
}

TEST_F(TestTLSCertificateManagementCluster, TestRemoveClientCertificateSuccess)
{
    // Step 1: Generate a CSR to create the key pair and get the ccdid
    Commands::ClientCSR::Type csrRequest;
    uint8_t dummyNonce[32] = { 0x01, 0x02, 0x03 };
    csrRequest.nonce       = ByteSpan(dummyNonce, sizeof(dummyNonce));

    auto csrResult = mClusterTester.Invoke<Commands::ClientCSR::Type>(csrRequest);
    EXPECT_TRUE(csrResult.IsSuccess());
    ASSERT_TRUE(csrResult.response.has_value());

    TLSCCDID ccdid = 0;
    if (csrResult.response.has_value())
    {
        ccdid = csrResult.response.value().ccdid;
    }

    // Step 2: Generate a valid test client certificate
    uint8_t certBuffer[Credentials::kMaxDERCertLength];
    MutableByteSpan certSpan(certBuffer);
    EXPECT_EQ(GenerateTestCertificate(certSpan), CHIP_NO_ERROR);

    // Step 3: Provision the client certificate
    Commands::ProvisionClientCertificate::Type provisionReq;
    provisionReq.clientCertificate = certSpan;
    provisionReq.ccdid             = ccdid;

    auto provisionResult = mClusterTester.Invoke<Commands::ProvisionClientCertificate::Type>(provisionReq);
    EXPECT_TRUE(provisionResult.IsSuccess());

    EXPECT_EQ(mMockDelegate.clientCerts.size(), 1u);

    // Now remove it
    Commands::RemoveClientCertificate::Type removeReq;
    removeReq.ccdid = ccdid;

    auto removeResult = mClusterTester.Invoke<Commands::RemoveClientCertificate::Type>(removeReq);
    EXPECT_TRUE(removeResult.IsSuccess());

    // Verify it was removed
    EXPECT_EQ(mMockDelegate.clientCerts.size(), 0u);
}

TEST_F(TestTLSCertificateManagementCluster, TestRemoveClientCertificateNotFound)
{
    Commands::RemoveClientCertificate::Type request;
    request.ccdid = 999; // Non-existent certificate

    auto result = mClusterTester.Invoke<Commands::RemoveClientCertificate::Type>(request);
    EXPECT_FALSE(result.IsSuccess());
    if (result.status.has_value())
    {
        EXPECT_EQ(result.status.value().GetStatusCode().GetStatus(), Protocols::InteractionModel::Status::NotFound);
    }
}

} // namespace
