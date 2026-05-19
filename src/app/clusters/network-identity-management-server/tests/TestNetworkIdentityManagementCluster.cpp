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

#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/network-identity-management-server/DefaultNetworkIdentityStorage.h>
#include <app/clusters/network-identity-management-server/NetworkAdministratorSecret.h>
#include <app/clusters/network-identity-management-server/NetworkIdentityKeystore.h>
#include <app/clusters/network-identity-management-server/NetworkIdentityManagementCluster.h>
#include <app/clusters/network-identity-management-server/tests/NASS_test_vectors.h>
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/server-cluster/testing/ValidateGlobalAttributes.h>
#include <clusters/NetworkIdentityManagement/Attributes.h>
#include <clusters/NetworkIdentityManagement/Commands.h>
#include <clusters/NetworkIdentityManagement/Enums.h>
#include <clusters/NetworkIdentityManagement/Metadata.h>
#include <clusters/NetworkIdentityManagement/Structs.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/Base64.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/TestPersistentStorageDelegate.h>

#include <algorithm>
#include <variant>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkIdentityManagement;
using namespace chip::app::Clusters::NetworkIdentityManagement::Attributes;
using namespace chip::Crypto;
using namespace chip::Protocols::InteractionModel;
using namespace chip::Testing;

namespace {

// A mock NetworkIdentityKeystore that works on all platforms, including those with
// small (e.g. PSA) key handles that can't hold raw key material. Uses two fixed
// slots (the API requires at most 2 simultaneous NASS handles); the first byte of
// each HkdfKeyHandle holds the slot index.
//
// DeriveECDSANetworkIdentity generates a valid random identity for each distinct
// NASS, caching results so that re-"deriving" from the same NASS produces the
// same identity.
//
// Limitations:
// - Identities are random, not derived as required by the spec
// - The returned P256KeypairHandle can't be used for any operations
class TestNetworkIdentityKeystore : public NetworkIdentityKeystore
{
private:
    struct HkdfSlot
    {
        bool inUse = false;
        NetworkAdministratorRawSecret secret;
    };
    struct P256Slot
    {
        bool inUse = false;
    };

    HkdfSlot mHkdfSlots[2]; // minimum per NetworkIdentityKeystore API contract
    P256Slot mP256Slots[4]; // minimum per NetworkIdentityKeystore API contract

    HkdfSlot * FindSlot(const HkdfKeyHandle & handle)
    {
        auto index = static_cast<uint8_t>(handle.As<uint8_t>() ^ 0xaa);
        return (index < std::size(mHkdfSlots)) ? &mHkdfSlots[index] : nullptr;
    }

    void MakeHandle(HkdfSlot const * slot, HkdfKeyHandle & outHandle)
    {
        outHandle.AsMutable<uint8_t>() = static_cast<uint8_t>((slot - mHkdfSlots) ^ 0xaa);
    }

    P256Slot * FindSlot(const P256KeypairHandle & handle)
    {
        VerifyOrReturnValue(handle.Length() == 1, nullptr);
        auto index = static_cast<uint8_t>(*handle.ConstBytes() ^ 0xbb);
        return (index < std::size(mP256Slots)) ? &mP256Slots[index] : nullptr;
    }

    void MakeHandle(P256Slot const * slot, P256KeypairHandle & outHandle)
    {
        *outHandle.Bytes() = static_cast<uint8_t>((slot - mP256Slots) ^ 0xbb);
        SuccessOrDie(outHandle.SetLength(1));
    }

    struct CachedIdentity
    {
        NetworkAdministratorRawSecret rawSecret;
        uint8_t identityBuffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        size_t identityLen = 0;
    };
    std::vector<CachedIdentity> mIdentityCache;

public:
    CHIP_ERROR ImportNetworkAdministratorSecret(const NetworkAdministratorRawSecret & secret, HkdfKeyHandle & outHandle) override
    {
        for (auto & slot : mHkdfSlots)
        {
            if (!slot.inUse)
            {
                slot.inUse  = true;
                slot.secret = secret;
                MakeHandle(&slot, outHandle);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    CHIP_ERROR ExportNetworkAdministratorSecret(const HkdfKeyHandle & handle, NetworkAdministratorRawSecret & outRawSecret) override
    {
        auto * hkdf = FindSlot(handle);
        VerifyOrReturnError(hkdf != nullptr && hkdf->inUse, CHIP_ERROR_NOT_FOUND);
        memcpy(outRawSecret.Bytes(), hkdf->secret.ConstBytes(), outRawSecret.Capacity());
        return CHIP_NO_ERROR;
    }

    void DestroyNetworkAdministratorSecret(HkdfKeyHandle & handle) override
    {
        auto * hkdf = FindSlot(handle);
        ASSERT_TRUE(hkdf != nullptr && hkdf->inUse);
        hkdf->inUse = false;
    }

    CHIP_ERROR DeriveECDSANetworkIdentity(const HkdfKeyHandle & nassHandle, P256KeypairHandle & outKeypairHandle,
                                          MutableByteSpan & outIdentity) override
    {
        auto * hkdf = FindSlot(nassHandle);
        VerifyOrReturnError(hkdf != nullptr && hkdf->inUse, CHIP_ERROR_NOT_FOUND);

        // Look up cached identities by raw secret bytes (not slot index) so that re-importing
        // the same secret produces the same identity, matching real deterministic derivation.
        CachedIdentity * entry = nullptr;
        for (auto & cached : mIdentityCache)
        {
            if (memcmp(cached.rawSecret.ConstBytes(), hkdf->secret.ConstBytes(), NetworkAdministratorRawSecret::Length()) == 0)
            {
                entry = &cached;
                break;
            }
        }

        if (entry == nullptr)
        {
            // Generate a new random keypair and derive a valid compact identity from it
            entry            = &mIdentityCache.emplace_back();
            entry->rawSecret = hkdf->secret;
            P256Keypair keypair;
            MutableByteSpan certSpan(entry->identityBuffer);
            SuccessOrDie(keypair.Initialize(ECPKeyTarget::ECDSA));
            SuccessOrDie(Credentials::NewChipNetworkIdentity(keypair, certSpan));
            entry->identityLen = certSpan.size();
        }

        ReturnErrorOnFailure(CopySpanToMutableSpan(ByteSpan(entry->identityBuffer, entry->identityLen), outIdentity));

        for (auto & slot : mP256Slots)
        {
            if (!slot.inUse)
            {
                slot.inUse = true;
                MakeHandle(&slot, outKeypairHandle);
                return CHIP_NO_ERROR;
            }
        }
        return CHIP_ERROR_NO_MEMORY;
    }

    void DestroyNetworkIdentityKeypair(P256KeypairHandle & handle) override
    {
        auto * p256 = FindSlot(handle);
        ASSERT_TRUE(p256 != nullptr && p256->inUse);
        p256->inUse = false;
    }

    void ValidateHandles(int expectedNass, int expectedKeypairs)
    {
        EXPECT_EQ(std::count_if(std::begin(mHkdfSlots), std::end(mHkdfSlots), [](auto & s) { return s.inUse; }), expectedNass);
        EXPECT_EQ(std::count_if(std::begin(mP256Slots), std::end(mP256Slots), [](auto & s) { return s.inUse; }), expectedKeypairs);
    }
};

// A test AuthenticatorDriver that records all calls in a combined event log.
class TestAuthenticatorDriver : public NetworkIdentityManagement::AuthenticatorDriver
{
public:
    struct NIAddedEvent
    {
        uint16_t index;
        NetworkIdentityManagement::IdentityTypeEnum type;
        Credentials::CertificateKeyIdStorage identifier;
    };

    struct NIRemovedEvent
    {
        uint16_t index;
        NetworkIdentityManagement::IdentityTypeEnum type;
        Credentials::CertificateKeyIdStorage identifier;
    };

    struct ClientAddedEvent
    {
        uint16_t clientIndex;
    };

    struct ClientRemovedEvent
    {
        uint16_t clientIndex;
        Credentials::CertificateKeyIdStorage identifier;
    };

    using Event = std::variant<NIAddedEvent, NIRemovedEvent, ClientAddedEvent, ClientRemovedEvent>;

    void OnStartup(NetworkIdentityManagement::AuthenticatorDriverCallback & callback,
                   ReadOnlyNetworkIdentityStorage & storage) override
    {
        mCallback = &callback;
        mStorage  = &storage;
    }

    void OnShutdown() override
    {
        mCallback = nullptr;
        mStorage  = nullptr;
    }

    CHIP_ERROR PrepareNetworkIdentityAddition() override { return mPrepareNIAdditionResult; }
    CHIP_ERROR PrepareClientAddition() override { return mPrepareClientAdditionResult; }

    void OnNetworkIdentityAdded(const ReadOnlyNetworkIdentityStorage::NetworkIdentityEntry & entry) override
    {
        mEvents.push_back(NIAddedEvent{ entry.index, entry.type, entry.identifier });
    }

    void OnNetworkIdentityRemoved(const ReadOnlyNetworkIdentityStorage::NetworkIdentityEntry & entry) override
    {
        mEvents.push_back(NIRemovedEvent{ entry.index, entry.type, entry.identifier });
    }

    void OnClientAdded(const ReadOnlyNetworkIdentityStorage::ClientEntry & entry) override
    {
        EXPECT_NE(entry.index, 0u);
        EXPECT_NE(entry.identifier, Credentials::CertificateKeyIdStorage{});
        EXPECT_FALSE(entry.compactIdentity.empty());
        mEvents.push_back(ClientAddedEvent{ entry.index });
    }

    void OnClientRemoved(const ReadOnlyNetworkIdentityStorage::ClientEntry & entry) override
    {
        EXPECT_NE(entry.index, 0u);
        EXPECT_NE(entry.identifier, Credentials::CertificateKeyIdStorage{});
        EXPECT_FALSE(entry.compactIdentity.empty());
        ClientRemovedEvent event;
        event.clientIndex = entry.index;
        event.identifier  = entry.identifier;
        mEvents.push_back(event);
    }

    // Count events of a specific type
    template <typename T>
    size_t Count() const
    {
        return static_cast<size_t>(
            std::count_if(mEvents.begin(), mEvents.end(), [](auto e) { return std::holds_alternative<T>(e); }));
    }

    // Get the Nth event of a specific type (0-indexed)
    template <typename T>
    const T & Get(size_t n = 0) const
    {
        for (const auto & e : mEvents)
        {
            if (std::holds_alternative<T>(e) && (n-- == 0))
            {
                return std::get<T>(e);
            }
        }
        VerifyOrDie(false); // not found
    }

    NetworkIdentityManagement::AuthenticatorDriverCallback * mCallback = nullptr;
    ReadOnlyNetworkIdentityStorage * mStorage                          = nullptr;
    std::vector<Event> mEvents;

    CHIP_ERROR mPrepareNIAdditionResult     = CHIP_NO_ERROR;
    CHIP_ERROR mPrepareClientAdditionResult = CHIP_NO_ERROR;
};

// Helper: encode a NASS TLV payload from a raw secret and timestamp.
static CHIP_ERROR EncodeTestNASS(const NetworkAdministratorRawSecret & rawSecret, System::Clock::Seconds32 created,
                                 MutableByteSpan & outEncoded)
{
    NetworkAdministratorSecretData secretData;
    secretData.created   = created;
    secretData.rawSecret = rawSecret;
    return EncodeNetworkAdministratorSecret(secretData, outEncoded);
}

// Pre-defined raw secrets for tests. Three distinct secrets are sufficient for all current tests.
static const NetworkAdministratorRawSecret kRawSecretA((const uint8_t[]){
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
});
static const NetworkAdministratorRawSecret kRawSecretB((const uint8_t[]){
    0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAa, 0xAb, 0xAc, 0xAd, 0xAe, 0xAf, 0xB0,
    0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBa, 0xBb, 0xBc, 0xBd, 0xBe, 0xBf, 0xC0,
});
static const NetworkAdministratorRawSecret kRawSecretC((const uint8_t[]){
    0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
    0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0,
});

struct TestNetworkIdentityManagementCluster : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    void SetUp() override { ASSERT_EQ(cluster.Startup(tester.GetServerClusterContext()), CHIP_NO_ERROR); }
    void TearDown() override { cluster.Shutdown(ClusterShutdownType::kClusterShutdown); }

    TestNetworkIdentityManagementCluster() : cluster(kTestEndpointId, storage, keystore, authenticatorDriver), tester(cluster) {}

    static constexpr EndpointId kTestEndpointId = 1;

    chip::TestPersistentStorageDelegate persistentStorage;
    DefaultNetworkIdentityStorage storage{ persistentStorage };
    TestNetworkIdentityKeystore keystore;
    TestAuthenticatorDriver authenticatorDriver;
    NetworkIdentityManagementCluster cluster;
    ClusterTester tester;

    // Helper: invoke ImportAdminSecret with a given raw secret and timestamp.
    ClusterTester::InvokeResult<DataModel::NullObjectType> ImportNASS(const NetworkAdministratorRawSecret & rawSecret,
                                                                      System::Clock::Seconds32 created)
    {
        uint8_t nassEncoded[kNetworkAdministratorSecretDataMaxEncodedLength];
        MutableByteSpan nassSpan(nassEncoded);
        SuccessOrDie(EncodeTestNASS(rawSecret, created, nassSpan));

        Commands::ImportAdminSecret::Type request;
        request.networkAdministratorSharedSecret = nassSpan;
        return tester.Invoke(request);
    }

    // Helper: generate a random valid client identity (self-signed compact certificate).
    struct GeneratedClientIdentity
    {
        uint8_t certBuffer[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        size_t certLen = 0;
        Credentials::CertificateKeyIdStorage identifier;

        ByteSpan Cert() const { return ByteSpan(certBuffer, certLen); }
    };

    static GeneratedClientIdentity GenerateClientIdentity()
    {
        GeneratedClientIdentity result;
        P256Keypair keypair;
        MutableByteSpan certSpan(result.certBuffer);
        SuccessOrDie(keypair.Initialize(ECPKeyTarget::ECDSA));
        SuccessOrDie(Credentials::NewChipNetworkIdentity(keypair, certSpan));
        result.certLen = certSpan.size();
        SuccessOrDie(Credentials::ExtractIdentifierFromChipNetworkIdentity(certSpan, result.identifier));
        return result;
    }
};

} // namespace

TEST_F(TestNetworkIdentityManagementCluster, AttributeListTest)
{
    ASSERT_TRUE(IsAttributesListEqualTo(cluster,
                                        {
                                            ActiveNetworkIdentities::kMetadataEntry,
                                            Clients::kMetadataEntry,
                                            ClientTableSize::kMetadataEntry,
                                        }));
}

TEST_F(TestNetworkIdentityManagementCluster, ReadGlobalAttributes)
{
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);
    EXPECT_EQ(revision, NetworkIdentityManagement::kRevision);

    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);
    EXPECT_EQ(features, 0u);
}

TEST_F(TestNetworkIdentityManagementCluster, InitialState)
{
    // ActiveNetworkIdentities is empty
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);

    // Clients is empty
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);

    // ClientTableSize
    uint16_t clientTableSize{};
    ASSERT_EQ(tester.ReadAttribute(ClientTableSize::Id, clientTableSize), CHIP_NO_ERROR);
    EXPECT_EQ(clientTableSize, 500u);
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAndExportAdminSecret)
{
    // Base64-decode the test vector to get the NASS TLV payload
    uint8_t nassBuf[kNetworkAdministratorSecretDataMaxEncodedLength];
    uint16_t nassLen = Base64Decode(kNASSTestVector1_PEM, static_cast<uint16_t>(strlen(kNASSTestVector1_PEM)), nassBuf);
    ASSERT_NE(nassLen, /* decode error */ UINT16_MAX);
    ByteSpan nassTlv(nassBuf, nassLen);

    // Decode the test vector to get expected attribute values
    NetworkAdministratorSecretData expectedSecret;
    ASSERT_SUCCESS(DecodeNetworkAdministratorSecret(nassTlv, expectedSecret));

    // Import the test vector NASS TLV directly
    Commands::ImportAdminSecret::Type importRequest;
    importRequest.networkAdministratorSharedSecret = nassTlv;
    auto importResult                              = tester.Invoke(importRequest);
    ASSERT_TRUE(importResult.IsSuccess());

    // Verify ActiveNetworkIdentities now has one entry
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);

    size_t count = 0;
    ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);

    auto iter = activeNIs.begin();
    ASSERT_TRUE(iter.Next());
    const auto & entry = iter.GetValue();
    EXPECT_EQ(entry.type, IdentityTypeEnum::kEcdsa);
    EXPECT_TRUE(entry.current);
    EXPECT_EQ(entry.createdTimestamp, expectedSecret.created.count());
    EXPECT_TRUE(entry.remainingClients.IsNull()); // must be null for current entry

    // Export and verify the round-tripped TLV matches the original
    Commands::ExportAdminSecret::Type exportRequest;
    auto exportResult = tester.Invoke(exportRequest);
    ASSERT_TRUE(exportResult.IsSuccess());
    ASSERT_TRUE(exportResult.response.has_value());
    EXPECT_TRUE(exportResult.response->networkAdministratorSharedSecret.data_equal(nassTlv));

    keystore.ValidateHandles(1, 1); // One NASS handle + one ECDSA keypair handle outstanding
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretRequiresCASE)
{
    // Set auth mode to PASE — should return UNSUPPORTED_ACCESS
    Access::SubjectDescriptor paseDescriptor{
        .fabricIndex = 1,
        .authMode    = Access::AuthMode::kPase,
        .subject     = 0x1234,
    };
    tester.SetSubjectDescriptor(paseDescriptor);

    auto result = ImportNASS(kRawSecretA, System::Clock::Seconds32(100));
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedAccess));
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretTimestampValidation)
{
    // First import at timestamp 2000
    auto result1 = ImportNASS(kRawSecretA, System::Clock::Seconds32(2000));
    ASSERT_TRUE(result1.IsSuccess());

    // Older timestamp — should fail
    auto result2 = ImportNASS(kRawSecretA, System::Clock::Seconds32(1000));
    EXPECT_EQ(result2.GetStatusCode(), ClusterStatusCode(Status::DynamicConstraintError));

    // Exact same NASS (same secret + same timestamp) — idempotent success
    auto result3 = ImportNASS(kRawSecretA, System::Clock::Seconds32(2000));
    ASSERT_TRUE(result3.IsSuccess());

    // Same secret with newer timestamp — should fail (same secret repackaged)
    auto result4 = ImportNASS(kRawSecretA, System::Clock::Seconds32(3000));
    EXPECT_EQ(result4.GetStatusCode(), ClusterStatusCode(Status::DynamicConstraintError));

    // Genuinely new secret with newer timestamp — should succeed
    auto result5 = ImportNASS(kRawSecretB, System::Clock::Seconds32(4000));
    ASSERT_TRUE(result5.IsSuccess());

    // Only the latest NASS + keypair should remain; old ones destroyed.
    // The old NI is still present (non-current, not yet retired), so 2 keypairs.
    keystore.ValidateHandles(1, 2);
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretInvalidNASS)
{
    // Import garbage bytes as NASS — should return INVALID_COMMAND
    Commands::ImportAdminSecret::Type importRequest;
    const uint8_t garbage[]                        = { 0xFF, 0xFF, 0xFF };
    importRequest.networkAdministratorSharedSecret = ByteSpan(garbage);
    auto result                                    = tester.Invoke(importRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretRetiringAndCapacity)
{
    // We need to fill the NI table to kMaxNetworkIdentities (4) by adding clients
    // to each NI as it becomes non-current, preventing retirement during subsequent imports.
    NetworkAdministratorRawSecret secrets[NetworkIdentityStorage::kMaxNetworkIdentities + 1];
    for (size_t i = 0; i < std::size(secrets); i++)
    {
        memset(secrets[i].Bytes(), static_cast<int>(0x10 + i), secrets[i].Capacity());
    }

    // Import first 4 secrets, adding a client to each NI before the next import
    // so the non-current NIs can't be retired.
    std::vector<uint16_t> clientIndices;
    for (size_t i = 0; i < NetworkIdentityStorage::kMaxNetworkIdentities; i++)
    {
        auto result = ImportNASS(secrets[i], System::Clock::Seconds32(static_cast<uint32_t>(100 * (i + 1))));
        ASSERT_TRUE(result.IsSuccess());

        // After import, the NI that just became non-current (from the previous import) needs a client
        // to prevent retirement on the next import. Read the current NI index so we can reference it.
        // The NI from the previous import is now non-current — give it a client
        ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
        auto iter = activeNIs.begin();
        // Find a non-current NI without clients
        while (iter.Next())
        {
            auto & ni = iter.GetValue();
            if (!ni.current && !ni.remainingClients.IsNull() && ni.remainingClients.Value() == 0)
            {
                auto identity = GenerateClientIdentity();
                Commands::AddClient::Type addRequest;
                addRequest.clientIdentity = identity.Cert();
                auto addResult            = tester.Invoke(addRequest);
                ASSERT_TRUE(addResult.IsSuccess());
                clientIndices.push_back(addResult.response->clientIndex);
                authenticatorDriver.mCallback->OnClientAuthenticated(addResult.response->clientIndex, ni.index);
                break;
            }
        }
    }

    // Verify table is full; we should have created 3 clients
    {
        ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
        size_t count = 0;
        ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
        EXPECT_EQ(count, NetworkIdentityStorage::kMaxNetworkIdentities);
        EXPECT_EQ(clientIndices.size(), NetworkIdentityStorage::kMaxNetworkIdentities - 1u);
    }

    // Next Import should fail — no retirable NIs (3 non-current have clients, 1 is current)
    auto failResult = ImportNASS(secrets[4], System::Clock::Seconds32(500));
    EXPECT_EQ(failResult.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));

    // Verify table is unchanged — still 4 NIs
    {
        ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
        size_t count = 0;
        ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
        EXPECT_EQ(count, NetworkIdentityStorage::kMaxNetworkIdentities);
    }

    // Remove all clients — non-current NIs become retirable
    for (uint16_t clientIndex : clientIndices)
    {
        Commands::RemoveClient::Type removeRequest;
        removeRequest.clientIndex.SetValue(clientIndex);
        auto removeResult = tester.Invoke(removeRequest);
        ASSERT_TRUE(removeResult.IsSuccess());
    }

    // Import should now succeed — retires 3 stale NIs, stores the new one
    auto successResult = ImportNASS(secrets[NetworkIdentityStorage::kMaxNetworkIdentities], System::Clock::Seconds32(999999));
    ASSERT_TRUE(successResult.IsSuccess());

    // Verify table has 2 NIs: the previous current (now non-current) + the new current
    {
        ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
        ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
        size_t count = 0;
        ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
        EXPECT_EQ(count, 2u);
    }
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretHandleCleanupOnRetirementFailure)
{
    // Import 2 NIs so the first becomes non-current and retirable
    ASSERT_TRUE(ImportNASS(kRawSecretA, System::Clock::Seconds32(1000)).IsSuccess());
    ASSERT_TRUE(ImportNASS(kRawSecretB, System::Clock::Seconds32(2000)).IsSuccess());
    keystore.ValidateHandles(1, 2);

    // NI1 is non-current with 0 clients — will be retired on next import
    // Poison the NI table index so that RetireNetworkIdentity's RemoveNetworkIdentity fails.
    // The third import should fail, but the NASS handle and keypair handle it allocated
    // must be cleaned up (not leaked).
    persistentStorage.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex().KeyName(), -1, 0);
    auto result = ImportNASS(kRawSecretC, System::Clock::Seconds32(3000));
    persistentStorage.ClearPoisonKeys();
    EXPECT_FALSE(result.IsSuccess());
    keystore.ValidateHandles(1, 2); // unchanged
}

TEST_F(TestNetworkIdentityManagementCluster, ExportAdminSecretRequiresCASE)
{
    // Set auth mode to PASE — should return UNSUPPORTED_ACCESS
    Access::SubjectDescriptor paseDescriptor{
        .fabricIndex = 1,
        .authMode    = Access::AuthMode::kPase,
        .subject     = 0x1234,
    };
    tester.SetSubjectDescriptor(paseDescriptor);

    Commands::ExportAdminSecret::Type exportRequest;
    auto result = tester.Invoke(exportRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::UnsupportedAccess));
}

TEST_F(TestNetworkIdentityManagementCluster, ExportAdminSecretNoSecret)
{
    // Export without having imported — should return NOT_FOUND
    Commands::ExportAdminSecret::Type exportRequest;
    auto exportResult = tester.Invoke(exportRequest);
    EXPECT_EQ(exportResult.GetStatusCode(), ClusterStatusCode(Status::NotFound));
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityByIndex)
{
    auto importResult = ImportNASS(kRawSecretA, System::Clock::Seconds32(100));
    ASSERT_TRUE(importResult.IsSuccess());

    // Read ActiveNetworkIdentities to get the index
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    auto iter = activeNIs.begin();
    ASSERT_TRUE(iter.Next());
    uint16_t niIndex = iter.GetValue().index;

    // Query by index
    Commands::QueryIdentity::Type queryRequest;
    queryRequest.networkIdentityIndex.SetValue(niIndex);
    auto queryResult = tester.Invoke(queryRequest);
    ASSERT_TRUE(queryResult.IsSuccess());
    ASSERT_TRUE(queryResult.response.has_value());
    EXPECT_FALSE(queryResult.response->identity.empty());

    // Verify identity is a valid network identity
    EXPECT_SUCCESS(Credentials::ValidateChipNetworkIdentity(queryResult.response->identity));
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityByIndexNotFound)
{
    // Query by index with no identities stored — should return NOT_FOUND
    Commands::QueryIdentity::Type queryRequest;
    queryRequest.networkIdentityIndex.SetValue(999);
    auto queryResult = tester.Invoke(queryRequest);
    EXPECT_EQ(queryResult.GetStatusCode(), ClusterStatusCode(Status::NotFound));
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityByType)
{
    auto importResult = ImportNASS(kRawSecretA, System::Clock::Seconds32(200));
    ASSERT_TRUE(importResult.IsSuccess());

    // Query by type (ECDSA)
    Commands::QueryIdentity::Type queryRequest;
    queryRequest.networkIdentityType.SetValue(IdentityTypeEnum::kEcdsa);
    auto queryResult = tester.Invoke(queryRequest);
    ASSERT_TRUE(queryResult.IsSuccess());
    ASSERT_TRUE(queryResult.response.has_value());
    EXPECT_FALSE(queryResult.response->identity.empty());
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityClientByIdentifier)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identity.Cert();
    auto addResult            = tester.Invoke(addRequest);
    ASSERT_TRUE(addResult.IsSuccess());

    // Query by identifier should find the client identity
    Commands::QueryIdentity::Type queryRequest;
    queryRequest.identifier.SetValue(ByteSpan(identity.identifier));
    auto queryResult = tester.Invoke(queryRequest);
    ASSERT_TRUE(queryResult.IsSuccess());
    ASSERT_TRUE(queryResult.response.has_value());
    EXPECT_TRUE(queryResult.response->identity.data_equal(identity.Cert()));
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityByNIIdentifier)
{
    auto importResult = ImportNASS(kRawSecretA, System::Clock::Seconds32(100));
    ASSERT_TRUE(importResult.IsSuccess());

    // Read the NI identifier from ActiveNetworkIdentities
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    auto iter = activeNIs.begin();
    ASSERT_TRUE(iter.Next());
    auto niIdentifier = iter.GetValue().identifier;

    // Query by the NI's identifier — should resolve to the NI's identity
    Commands::QueryIdentity::Type queryRequest;
    queryRequest.identifier.SetValue(niIdentifier);
    auto queryResult = tester.Invoke(queryRequest);
    ASSERT_TRUE(queryResult.IsSuccess());
    ASSERT_TRUE(queryResult.response.has_value());
    EXPECT_FALSE(queryResult.response->identity.empty());
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityMultipleFieldsRejected)
{
    // Passing all three optional fields should return InvalidCommand
    Commands::QueryIdentity::Type queryRequest;
    queryRequest.networkIdentityIndex.SetValue(1);
    queryRequest.networkIdentityType.SetValue(IdentityTypeEnum::kEcdsa);
    queryRequest.identifier.SetValue(ByteSpan());
    auto queryResult = tester.Invoke(queryRequest);
    EXPECT_EQ(queryResult.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestNetworkIdentityManagementCluster, QueryIdentityNoFieldsRejected)
{
    // No optional fields present — should return InvalidCommand
    Commands::QueryIdentity::Type queryRequest;
    auto queryResult = tester.Invoke(queryRequest);
    EXPECT_EQ(queryResult.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestNetworkIdentityManagementCluster, AddClientBasic)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type request;
    request.clientIdentity = identity.Cert();
    auto result            = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());
    ASSERT_TRUE(result.response.has_value());
    EXPECT_GE(result.response->clientIndex, 1u);
    EXPECT_LE(result.response->clientIndex, 2047u);

    // Verify Clients attribute has one entry
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);

    auto iter = clients.begin();
    ASSERT_TRUE(iter.Next());
    const auto & entry = iter.GetValue();
    EXPECT_EQ(entry.clientIndex, result.response->clientIndex);
    EXPECT_TRUE(entry.clientIdentifier.data_equal(ByteSpan(identity.identifier)));
    EXPECT_TRUE(entry.networkIdentityIndex.IsNull());
}

TEST_F(TestNetworkIdentityManagementCluster, AddClientIdempotent)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type request;
    request.clientIdentity = identity.Cert();

    auto result1 = tester.Invoke(request);
    ASSERT_TRUE(result1.IsSuccess());
    ASSERT_TRUE(result1.response.has_value());

    // Add the same identity again — should succeed with the same clientIndex
    auto result2 = tester.Invoke(request);
    ASSERT_TRUE(result2.IsSuccess());
    ASSERT_TRUE(result2.response.has_value());
    EXPECT_EQ(result2.response->clientIndex, result1.response->clientIndex);

    // Should still be only 1 entry
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
}

TEST_F(TestNetworkIdentityManagementCluster, AddMultipleClients)
{
    auto identity1 = GenerateClientIdentity();
    auto identity2 = GenerateClientIdentity();

    Commands::AddClient::Type request1;
    request1.clientIdentity = identity1.Cert();
    auto result1            = tester.Invoke(request1);
    ASSERT_TRUE(result1.IsSuccess());

    Commands::AddClient::Type request2;
    request2.clientIdentity = identity2.Cert();
    auto result2            = tester.Invoke(request2);
    ASSERT_TRUE(result2.IsSuccess());
    EXPECT_NE(result2.response->clientIndex, result1.response->clientIndex);

    // Verify Clients attribute has both entries
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 2u);
}

TEST_F(TestNetworkIdentityManagementCluster, AddClientInvalidIdentity)
{
    const uint8_t garbage[] = { 0xFF, 0xFF, 0xFF };
    Commands::AddClient::Type request;
    request.clientIdentity = ByteSpan(garbage);
    auto result            = tester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::DynamicConstraintError));
}

TEST_F(TestNetworkIdentityManagementCluster, AddClientResourceExhausted)
{
    class FullClientStorage : public DefaultNetworkIdentityStorage
    {
    public:
        using DefaultNetworkIdentityStorage::DefaultNetworkIdentityStorage;
        CHIP_ERROR AddClient(const ClientInfo & info, uint16_t & outClientIndex) override { return CHIP_ERROR_NO_MEMORY; }
    };

    FullClientStorage fullStorage(persistentStorage);
    NetworkIdentityManagementCluster fullCluster(kTestEndpointId, fullStorage, keystore, authenticatorDriver);
    ClusterTester fullTester(fullCluster);
    ASSERT_EQ(fullCluster.Startup(fullTester.GetServerClusterContext()), CHIP_NO_ERROR);

    auto identity = GenerateClientIdentity();
    Commands::AddClient::Type request;
    request.clientIdentity = identity.Cert();
    auto result            = fullTester.Invoke(request);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ResourceExhausted));

    fullCluster.Shutdown(ClusterShutdownType::kClusterShutdown);
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientByIndex)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identity.Cert();
    auto addResult            = tester.Invoke(addRequest);
    ASSERT_TRUE(addResult.IsSuccess());

    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIndex.SetValue(addResult.response->clientIndex);
    auto removeResult = tester.Invoke(removeRequest);
    ASSERT_TRUE(removeResult.IsSuccess());

    // Verify Clients attribute is now empty
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientByIdentifier)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identity.Cert();
    auto addResult            = tester.Invoke(addRequest);
    ASSERT_TRUE(addResult.IsSuccess());

    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIdentifier.SetValue(ByteSpan(identity.identifier));
    auto removeResult = tester.Invoke(removeRequest);
    ASSERT_TRUE(removeResult.IsSuccess());

    // Verify empty
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientNotFound)
{
    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIndex.SetValue(999);
    auto result = tester.Invoke(removeRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::NotFound));
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientByIdentifierNotFound)
{
    // Valid 20-byte identifier that doesn't match any stored client
    const uint8_t unknownId[20] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10 };
    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIdentifier.SetValue(ByteSpan(unknownId));
    auto result = tester.Invoke(removeRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::NotFound));
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientInvalidCommand)
{
    // Neither field present
    Commands::RemoveClient::Type removeRequest;
    auto result = tester.Invoke(removeRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientBothFieldsPresent)
{
    // Both fields present — should return InvalidCommand
    const uint8_t id[20] = { 1, 2, 3 };
    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIndex.SetValue(1);
    removeRequest.clientIdentifier.SetValue(ByteSpan(id));
    auto result = tester.Invoke(removeRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::InvalidCommand));
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientIdentifierWrongSize)
{
    // Identifier with wrong size — should return ConstraintError
    const uint8_t shortId[] = { 0x01, 0x02, 0x03 };
    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIdentifier.SetValue(ByteSpan(shortId));
    auto result = tester.Invoke(removeRequest);
    EXPECT_EQ(result.GetStatusCode(), ClusterStatusCode(Status::ConstraintError));
}

TEST_F(TestNetworkIdentityManagementCluster, ClientNIReferenceAndNotifications)
{
    // 1. Import NASS1 → NI1 (current)
    auto import1 = ImportNASS(kRawSecretA, System::Clock::Seconds32(1000));
    ASSERT_TRUE(import1.IsSuccess());

    // Read the NI1 index
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    auto niIter = activeNIs.begin();
    ASSERT_TRUE(niIter.Next());
    uint16_t ni1Index = niIter.GetValue().index;
    EXPECT_TRUE(niIter.GetValue().current);

    // 2. Add clients A and B
    auto identityA = GenerateClientIdentity();
    auto identityB = GenerateClientIdentity();

    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identityA.Cert();
    auto addResultA           = tester.Invoke(addRequest);
    ASSERT_TRUE(addResultA.IsSuccess());

    addRequest.clientIdentity = identityB.Cert();
    auto addResultB           = tester.Invoke(addRequest);
    ASSERT_TRUE(addResultB.IsSuccess());

    // 3. Both clients authenticate against NI1 (current).
    // NI1 is current, so RemainingClients is null — no notification expected for ActiveNetworkIdentities.
    tester.GetDirtyList().clear();
    authenticatorDriver.mCallback->OnClientAuthenticated(addResultA.response->clientIndex, ni1Index);
    authenticatorDriver.mCallback->OnClientAuthenticated(addResultB.response->clientIndex, ni1Index);
    EXPECT_FALSE(tester.IsAttributeDirty(ActiveNetworkIdentities::Id));

    // 4. Import NASS2 → NI2 (current), NI1 becomes non-current
    auto import2 = ImportNASS(kRawSecretB, System::Clock::Seconds32(2000));
    ASSERT_TRUE(import2.IsSuccess());

    // Read the NI2 index
    uint16_t ni2Index = 0;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    for (niIter = activeNIs.begin(); niIter.Next(); /**/)
    {
        if (niIter.GetValue().current)
        {
            ni2Index = niIter.GetValue().index;
        }
    }
    ASSERT_NE(ni2Index, 0u);

    // 5. Verify: NI1 has RemainingClients=2, NI2 (current) has RemainingClients=null
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    size_t niCount = 0;
    ASSERT_EQ(activeNIs.ComputeSize(&niCount), CHIP_NO_ERROR);
    EXPECT_EQ(niCount, 2u);

    for (niIter = activeNIs.begin(); niIter.Next(); /**/)
    {
        if (niIter.GetValue().index == ni1Index)
        {
            EXPECT_FALSE(niIter.GetValue().current);
            ASSERT_FALSE(niIter.GetValue().remainingClients.IsNull());
            EXPECT_EQ(niIter.GetValue().remainingClients.Value(), 2u);
        }
        else
        {
            EXPECT_TRUE(niIter.GetValue().current);
            EXPECT_TRUE(niIter.GetValue().remainingClients.IsNull());
        }
    }

    // 6. Client B authenticates against NI2 (current), moving from NI1→NI2.
    // NI1 drops to RemainingClients=1 (notifies), NI2 is current so no notification for it.
    tester.GetDirtyList().clear();
    authenticatorDriver.mCallback->OnClientAuthenticated(addResultB.response->clientIndex, ni2Index);
    EXPECT_TRUE(tester.IsAttributeDirty(ActiveNetworkIdentities::Id));

    // 7. Remove client B (references current NI2) — should NOT notify ActiveNetworkIdentities
    tester.GetDirtyList().clear();

    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIndex.SetValue(addResultB.response->clientIndex);
    auto removeResult = tester.Invoke(removeRequest);
    ASSERT_TRUE(removeResult.IsSuccess());

    EXPECT_TRUE(tester.IsAttributeDirty(Clients::Id));
    EXPECT_FALSE(tester.IsAttributeDirty(ActiveNetworkIdentities::Id));

    // 8. Remove client A (references non-current NI1) — SHOULD notify ActiveNetworkIdentities
    tester.GetDirtyList().clear();

    removeRequest = {};
    removeRequest.clientIndex.SetValue(addResultA.response->clientIndex);
    removeResult = tester.Invoke(removeRequest);
    ASSERT_TRUE(removeResult.IsSuccess());

    EXPECT_TRUE(tester.IsAttributeDirty(Clients::Id));
    EXPECT_TRUE(tester.IsAttributeDirty(ActiveNetworkIdentities::Id));

    // Verify NI1 now has RemainingClients=0
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    niIter = activeNIs.begin();
    while (niIter.Next())
    {
        if (niIter.GetValue().index == ni1Index)
        {
            ASSERT_FALSE(niIter.GetValue().remainingClients.IsNull());
            EXPECT_EQ(niIter.GetValue().remainingClients.Value(), 0u);
        }
    }
}

TEST_F(TestNetworkIdentityManagementCluster, ReadClientsMultiple)
{
    auto identity1 = GenerateClientIdentity();
    auto identity2 = GenerateClientIdentity();
    auto identity3 = GenerateClientIdentity();

    Commands::AddClient::Type request;
    request.clientIdentity = identity1.Cert();
    ASSERT_TRUE(tester.Invoke(request).IsSuccess());

    request.clientIdentity = identity2.Cert();
    ASSERT_TRUE(tester.Invoke(request).IsSuccess());

    request.clientIdentity = identity3.Cert();
    ASSERT_TRUE(tester.Invoke(request).IsSuccess());

    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 3u);
}

///// AuthenticatorDriver Integration Tests //////////////////////////////////////////////

TEST_F(TestNetworkIdentityManagementCluster, AuthenticatorDriverStartupAndShutdown)
{
    // OnStartup is called during SetUp() which calls cluster.Startup()
    EXPECT_NE(authenticatorDriver.mCallback, nullptr);
    EXPECT_EQ(authenticatorDriver.mStorage, &storage);
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretNotifiesAD)
{
    authenticatorDriver.mEvents.clear();
    auto result = ImportNASS(kRawSecretA, System::Clock::Seconds32(1000));
    ASSERT_TRUE(result.IsSuccess());

    // Should have called OnNetworkIdentityAdded exactly once
    ASSERT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIAddedEvent>(), 1u);
    EXPECT_EQ(authenticatorDriver.Get<TestAuthenticatorDriver::NIAddedEvent>().type, IdentityTypeEnum::kEcdsa);
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIRemovedEvent>(), 0u);
}

TEST_F(TestNetworkIdentityManagementCluster, ImportAdminSecretRetirementNotifiesAD)
{
    // Import 2 NIs so the first becomes non-current and retirable
    ASSERT_TRUE(ImportNASS(kRawSecretA, System::Clock::Seconds32(1000)).IsSuccess());
    ASSERT_TRUE(ImportNASS(kRawSecretB, System::Clock::Seconds32(2000)).IsSuccess());

    // NI1 is non-current with 0 clients — retirable
    authenticatorDriver.mEvents.clear();
    ASSERT_TRUE(ImportNASS(kRawSecretC, System::Clock::Seconds32(3000)).IsSuccess());

    // Should have removed NI1 (retired) then added NI3, in that order
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIRemovedEvent>(), 1u);
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIAddedEvent>(), 1u);
    ASSERT_GE(authenticatorDriver.mEvents.size(), 2u);
    EXPECT_TRUE(std::holds_alternative<TestAuthenticatorDriver::NIRemovedEvent>(authenticatorDriver.mEvents[0]));
    EXPECT_TRUE(std::holds_alternative<TestAuthenticatorDriver::NIAddedEvent>(authenticatorDriver.mEvents.back()));
    keystore.ValidateHandles(1, 2); // NASS, NI2 + NI3 (NI1 was retired)
}

TEST_F(TestNetworkIdentityManagementCluster, AddClientNotifiesAD)
{
    auto identity = GenerateClientIdentity();

    authenticatorDriver.mEvents.clear();
    Commands::AddClient::Type request;
    request.clientIdentity = identity.Cert();
    auto result            = tester.Invoke(request);
    ASSERT_TRUE(result.IsSuccess());

    ASSERT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::ClientAddedEvent>(), 1u);
    EXPECT_EQ(authenticatorDriver.Get<TestAuthenticatorDriver::ClientAddedEvent>().clientIndex, result.response->clientIndex);
}

TEST_F(TestNetworkIdentityManagementCluster, AddClientIdempotentDoesNotNotifyAD)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type request;
    request.clientIdentity = identity.Cert();
    ASSERT_TRUE(tester.Invoke(request).IsSuccess());

    // Second add with same identity — idempotent, should NOT notify again
    authenticatorDriver.mEvents.clear();
    ASSERT_TRUE(tester.Invoke(request).IsSuccess());
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::ClientAddedEvent>(), 0u);
}

TEST_F(TestNetworkIdentityManagementCluster, RemoveClientNotifiesAD)
{
    auto identity = GenerateClientIdentity();

    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identity.Cert();
    auto addResult            = tester.Invoke(addRequest);
    ASSERT_TRUE(addResult.IsSuccess());

    authenticatorDriver.mEvents.clear();
    Commands::RemoveClient::Type removeRequest;
    removeRequest.clientIndex.SetValue(addResult.response->clientIndex);
    auto removeResult = tester.Invoke(removeRequest);
    ASSERT_TRUE(removeResult.IsSuccess());

    // Should have called OnClientRemoved
    ASSERT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::ClientRemovedEvent>(), 1u);
    EXPECT_EQ(authenticatorDriver.Get<TestAuthenticatorDriver::ClientRemovedEvent>().clientIndex, addResult.response->clientIndex);
    EXPECT_EQ(authenticatorDriver.Get<TestAuthenticatorDriver::ClientRemovedEvent>().identifier, identity.identifier);
}

TEST_F(TestNetworkIdentityManagementCluster, OnClientAuthenticatedUpdatesNIReference)
{
    // Import a NASS to create an NI
    ASSERT_TRUE(ImportNASS(kRawSecretA, System::Clock::Seconds32(1000)).IsSuccess());

    // Get the NI index
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    auto niIter = activeNIs.begin();
    ASSERT_TRUE(niIter.Next());
    uint16_t niIndex = niIter.GetValue().index;

    // Add a client
    auto identity = GenerateClientIdentity();
    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identity.Cert();
    auto addResult            = tester.Invoke(addRequest);
    ASSERT_TRUE(addResult.IsSuccess());

    // Simulate authentication via the AD callback
    ASSERT_NE(authenticatorDriver.mCallback, nullptr);
    authenticatorDriver.mCallback->OnClientAuthenticated(addResult.response->clientIndex, niIndex);

    // Verify the client now references the NI
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    auto clientIter = clients.begin();
    ASSERT_TRUE(clientIter.Next());
    EXPECT_FALSE(clientIter.GetValue().networkIdentityIndex.IsNull());
    EXPECT_EQ(clientIter.GetValue().networkIdentityIndex.Value(), niIndex);
}

TEST_F(TestNetworkIdentityManagementCluster, OnClientAuthenticatedNonCurrentNINotifies)
{
    // Import two NIs so the first becomes non-current
    ASSERT_TRUE(ImportNASS(kRawSecretA, System::Clock::Seconds32(1000)).IsSuccess());

    // Get NI1 index (current at this point)
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    auto niIter = activeNIs.begin();
    ASSERT_TRUE(niIter.Next());
    uint16_t ni1Index = niIter.GetValue().index;

    // Import second — NI1 becomes non-current
    ASSERT_TRUE(ImportNASS(kRawSecretB, System::Clock::Seconds32(2000)).IsSuccess());

    // Add a client
    auto identity = GenerateClientIdentity();
    Commands::AddClient::Type addRequest;
    addRequest.clientIdentity = identity.Cert();
    auto addResult            = tester.Invoke(addRequest);
    ASSERT_TRUE(addResult.IsSuccess());

    // Client authenticates against non-current NI1 — should notify ActiveNetworkIdentities
    tester.GetDirtyList().clear();
    authenticatorDriver.mCallback->OnClientAuthenticated(addResult.response->clientIndex, ni1Index);
    EXPECT_TRUE(tester.IsAttributeDirty(ActiveNetworkIdentities::Id));

    // Second auth with same NI — no change, should NOT notify
    tester.GetDirtyList().clear();
    authenticatorDriver.mCallback->OnClientAuthenticated(addResult.response->clientIndex, ni1Index);
    EXPECT_FALSE(tester.IsAttributeDirty(ActiveNetworkIdentities::Id));
}

TEST_F(TestNetworkIdentityManagementCluster, PrepareNetworkIdentityAdditionRejectsImport)
{
    // Make the driver reject NI additions
    authenticatorDriver.mPrepareNIAdditionResult = CHIP_ERROR_INTERNAL;

    authenticatorDriver.mEvents.clear();
    auto result = ImportNASS(kRawSecretA, System::Clock::Seconds32(1000));
    EXPECT_FALSE(result.IsSuccess());

    // No NI should have been added, no notifications should have fired
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIAddedEvent>(), 0u);

    // No keystore handles should be outstanding
    keystore.ValidateHandles(0, 0);

    // ActiveNetworkIdentities should still be empty
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}

TEST_F(TestNetworkIdentityManagementCluster, PrepareNetworkIdentityAdditionRejectsImportAfterExistingNASS)
{
    // Successfully import a first NASS
    ASSERT_TRUE(ImportNASS(kRawSecretA, System::Clock::Seconds32(1000)).IsSuccess());
    keystore.ValidateHandles(1, 1); // 1 NASS + 1 keypair

    // Now make the driver reject NI additions
    authenticatorDriver.mPrepareNIAdditionResult = CHIP_ERROR_INTERNAL;

    authenticatorDriver.mEvents.clear();
    auto result = ImportNASS(kRawSecretB, System::Clock::Seconds32(2000));
    EXPECT_FALSE(result.IsSuccess());

    // No notifications for the rejected import
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIAddedEvent>(), 0u);
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::NIRemovedEvent>(), 0u);

    // Original NASS + keypair still intact, no new handles leaked
    keystore.ValidateHandles(1, 1);

    // Only the original NI should remain
    ActiveNetworkIdentities::TypeInfo::DecodableType activeNIs;
    ASSERT_EQ(tester.ReadAttribute(ActiveNetworkIdentities::Id, activeNIs), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(activeNIs.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 1u);
}

TEST_F(TestNetworkIdentityManagementCluster, PrepareClientAdditionRejectsAddClient)
{
    // Make the driver reject client additions
    authenticatorDriver.mPrepareClientAdditionResult = CHIP_ERROR_INTERNAL;

    auto identity = GenerateClientIdentity();
    authenticatorDriver.mEvents.clear();
    Commands::AddClient::Type request;
    request.clientIdentity = identity.Cert();
    auto result            = tester.Invoke(request);
    EXPECT_FALSE(result.IsSuccess());

    // No client should have been added, no notification
    EXPECT_EQ(authenticatorDriver.Count<TestAuthenticatorDriver::ClientAddedEvent>(), 0u);

    // Clients attribute should be empty
    Clients::TypeInfo::DecodableType clients;
    ASSERT_EQ(tester.ReadAttribute(Clients::Id, clients), CHIP_NO_ERROR);
    size_t count = 0;
    ASSERT_EQ(clients.ComputeSize(&count), CHIP_NO_ERROR);
    EXPECT_EQ(count, 0u);
}
