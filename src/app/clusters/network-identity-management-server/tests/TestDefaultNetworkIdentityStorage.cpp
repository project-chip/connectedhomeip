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

#include <cstdint>
#include <lib/core/CHIPError.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <pw_unit_test/framework.h>

#include <app/clusters/network-identity-management-server/DefaultNetworkIdentityStorage.h>
#include <clusters/NetworkIdentityManagement/Enums.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/TestPersistentStorageDelegate.h>

using namespace chip;
using namespace chip::Crypto;
using namespace chip::Credentials;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::NetworkIdentityManagement;
using namespace chip::System;

using NetworkIdentityFlags = NetworkIdentityStorage::NetworkIdentityFlags;
using ClientFlags          = NetworkIdentityStorage::ClientFlags;

// Opaque handle helpers: store a uint8_t tag in the first byte of the handle
// buffer to identify handles across store/retrieve round-trips.

static void MakeNassHandle(uint8_t tag, HkdfKeyHandle & handle)
{
    handle.AsMutable<uint8_t>() = tag;
}

static uint8_t NassHandleTag(const HkdfKeyHandle & handle)
{
    return handle.As<uint8_t>();
}

static CertificateKeyIdStorage MakeKeyId(uint8_t fill)
{
    CertificateKeyIdStorage id;
    id.fill(fill);
    return id;
}

// Test Network Identity data. Each identity has distinct values for all fields
// so that round-trip verification is unambiguous.

static const NetworkIdentityStorage::NetworkIdentityInfo kTestIdentityA = {
    .type            = IdentityTypeEnum::kEcdsa,
    .identifier      = { 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A,
                         0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A, 0x1A },
    .compactIdentity = ByteSpan((const uint8_t[]){ 0xCA, 0xFE }),
    .keypairHandle   = ByteSpan((const uint8_t[]){ 0xBA }),
};

static const NetworkIdentityStorage::NetworkIdentityInfo kTestIdentityB = {
    .type            = IdentityTypeEnum::kEcdsa,
    .identifier      = { 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,
                         0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B },
    .compactIdentity = ByteSpan((const uint8_t[]){ 0xCB, 0xFE }),
    .keypairHandle   = ByteSpan((const uint8_t[]){ 0xBB }),
};

static const NetworkIdentityStorage::NetworkIdentityInfo kTestIdentityC = {
    .type            = IdentityTypeEnum::kEcdsa,
    .identifier      = { 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C,
                         0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C, 0x3C },
    .compactIdentity = ByteSpan((const uint8_t[]){ 0xCC, 0xFE }),
    .keypairHandle   = ByteSpan((const uint8_t[]){ 0xBC }),
};

// Helper: store a NASS (no identities).
static CHIP_ERROR StoreTestNASS(DefaultNetworkIdentityStorage & storage, uint8_t nassHandleTag, uint32_t timestamp)
{
    NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
    MakeNassHandle(nassHandleTag, nassInfo.secretHandle);
    nassInfo.createdTimestamp = Clock::Seconds32(timestamp);
    return storage.StoreNetworkAdministratorSecretAndDerivedIdentities(
        nassInfo, Span<NetworkIdentityStorage::NetworkIdentityInfo const * const>(), Span<uint16_t>());
}

// Helper: store a NASS together with a single derived identity.
static CHIP_ERROR StoreTestNASSAndIdentity(DefaultNetworkIdentityStorage & storage, uint8_t nassHandleTag, uint32_t nassTimestamp,
                                           const NetworkIdentityStorage::NetworkIdentityInfo & identity, uint16_t & outIndex)
{
    auto const * identityPtr = &identity;
    NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
    MakeNassHandle(nassHandleTag, nassInfo.secretHandle);
    nassInfo.createdTimestamp = Clock::Seconds32(nassTimestamp);
    return storage.StoreNetworkAdministratorSecretAndDerivedIdentities(nassInfo, Span(&identityPtr, 1), Span(&outIndex, 1));
}

struct TestDefaultNetworkIdentityStorage : public ::testing::Test
{
    static void SetUpTestSuite() { ASSERT_SUCCESS(chip::Platform::MemoryInit()); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }

    // Scratch variable for UpdateClientNetworkIdentityIndex calls where the old index isn't checked
    uint16_t unusedOldNI = 0;
};

///// NASS Storage Tests //////////////////////////////////////////////////////////////

TEST_F(TestDefaultNetworkIdentityStorage, RetrieveNASSNotFound)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    NetworkIdentityStorage::NetworkAdministratorSecretInfo info;
    EXPECT_EQ(storage.RetrieveNetworkAdministratorSecret(info), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportAndRetrieveNASS)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    ASSERT_SUCCESS(StoreTestNASS(storage, 0xA1, 12345));

    NetworkIdentityStorage::NetworkAdministratorSecretInfo retrieved;
    ASSERT_SUCCESS(storage.RetrieveNetworkAdministratorSecret(retrieved));
    EXPECT_EQ(NassHandleTag(retrieved.secretHandle), 0xA1);
    EXPECT_EQ(retrieved.createdTimestamp, Clock::Seconds32(12345));
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportReplacesNASS)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    ASSERT_SUCCESS(StoreTestNASS(storage, 0xA1, 100));
    ASSERT_SUCCESS(StoreTestNASS(storage, 0xA2, 200));

    // NASS should be the second one
    NetworkIdentityStorage::NetworkAdministratorSecretInfo retrieved;
    ASSERT_SUCCESS(storage.RetrieveNetworkAdministratorSecret(retrieved));
    EXPECT_EQ(NassHandleTag(retrieved.secretHandle), 0xA2);
    EXPECT_EQ(retrieved.createdTimestamp, Clock::Seconds32(200));
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveNASS)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    ASSERT_SUCCESS(StoreTestNASS(storage, 0xA1, 1000));

    EXPECT_SUCCESS(storage.RemoveNetworkAdministratorSecret());
    EXPECT_EQ(storage.RemoveNetworkAdministratorSecret(), CHIP_ERROR_NOT_FOUND);

    NetworkIdentityStorage::NetworkAdministratorSecretInfo retrieved;
    EXPECT_EQ(storage.RetrieveNetworkAdministratorSecret(retrieved), CHIP_ERROR_NOT_FOUND);
}

///// Network Identity Storage Tests //////////////////////////////////////////////////

TEST_F(TestDefaultNetworkIdentityStorage, NoNetworkIdentities)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);
    NetworkIdentityStorage::NetworkIdentityEntry entry;

    {
        NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(
            storage.IterateNetworkIdentities({}, MutableByteSpan()));
        EXPECT_EQ(iter.Count(), 0u);
        EXPECT_FALSE(iter.Next(entry));
    }

    EXPECT_EQ(storage.FindCurrentNetworkIdentity(IdentityTypeEnum::kEcdsa, entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(storage.FindNetworkIdentity(uint16_t(42), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);

    CertificateKeyIdStorage keyId = MakeKeyId(0x55);
    EXPECT_EQ(storage.FindNetworkIdentity(CertificateKeyId(keyId), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, StoreAndFindNetworkIdentity)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t assignedIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, assignedIndex));
    EXPECT_NE(assignedIndex, 0u);

    // Find by index and check all fields round-tripped correctly.
    constexpr BitFlags<NetworkIdentityFlags> allFlags(NetworkIdentityFlags::kPopulateAll);
    uint8_t findBuffer[NetworkIdentityStorage::NetworkIdentityBufferSize(allFlags)];

    NetworkIdentityStorage::NetworkIdentityEntry found;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(assignedIndex, found, allFlags, Span(findBuffer)));

    EXPECT_EQ(found.index, assignedIndex);
    EXPECT_EQ(found.type, kTestIdentityA.type);
    EXPECT_TRUE(found.current);
    EXPECT_EQ(found.createdTimestamp, Clock::Seconds32(100)); // matches NASS timestamp
    EXPECT_EQ(found.identifier, kTestIdentityA.identifier);
    EXPECT_TRUE(found.compactIdentity.data_equal(kTestIdentityA.compactIdentity));
    EXPECT_TRUE(found.keypairHandle.data_equal(kTestIdentityA.keypairHandle));
    EXPECT_EQ(found.clientCount, 0u);

    // Find by identifier returns the same entry.
    NetworkIdentityStorage::NetworkIdentityEntry foundById;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(CertificateKeyId(kTestIdentityA.identifier), foundById, {}, MutableByteSpan()));
    EXPECT_EQ(foundById.index, assignedIndex);
    EXPECT_TRUE(foundById.current);
    EXPECT_EQ(foundById.type, kTestIdentityA.type);

    // Find current returns the same entry (it's the only one, so it must be current).
    NetworkIdentityStorage::NetworkIdentityEntry foundCurrent;
    ASSERT_SUCCESS(storage.FindCurrentNetworkIdentity(IdentityTypeEnum::kEcdsa, foundCurrent, {}, MutableByteSpan()));
    EXPECT_EQ(foundCurrent.index, assignedIndex);
    EXPECT_TRUE(foundCurrent.current);
}

TEST_F(TestDefaultNetworkIdentityStorage, StoreMultipleNetworkIdentities)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t index1, index2, index3;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, index1));

    NetworkIdentityStorage::NetworkIdentityEntry current;
    ASSERT_SUCCESS(storage.FindCurrentNetworkIdentity(IdentityTypeEnum::kEcdsa, current, {}, MutableByteSpan()));
    EXPECT_EQ(current.index, index1);
    EXPECT_TRUE(current.current);

    // Store second identity — should become current, first should no longer be current.
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA2, 200, kTestIdentityB, index2));
    EXPECT_NE(index1, index2);
    EXPECT_GT(index2, index1);

    ASSERT_SUCCESS(storage.FindCurrentNetworkIdentity(IdentityTypeEnum::kEcdsa, current, {}, MutableByteSpan()));
    EXPECT_EQ(current.index, index2);
    EXPECT_TRUE(current.current);

    // Store third — indices should keep increasing.
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA3, 300, kTestIdentityC, index3));
    EXPECT_GT(index3, index2);

    // Iterator should return all three, with only the last being current.
    NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(storage.IterateNetworkIdentities({}, MutableByteSpan()));
    EXPECT_TRUE(iter.IsValid());
    EXPECT_EQ(iter.Count(), 3u);

    NetworkIdentityStorage::NetworkIdentityEntry e;
    EXPECT_TRUE(iter.Next(e));
    EXPECT_FALSE(e.current);
    EXPECT_TRUE(iter.Next(e));
    EXPECT_FALSE(e.current);
    EXPECT_TRUE(iter.Next(e));
    EXPECT_TRUE(e.current);
    EXPECT_FALSE(iter.Next(e)); // exhausted

    // Find by identifier distinguishes between entries.
    NetworkIdentityStorage::NetworkIdentityEntry found;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(CertificateKeyId(kTestIdentityA.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, index1);
    ASSERT_SUCCESS(storage.FindNetworkIdentity(CertificateKeyId(kTestIdentityB.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, index2);
    ASSERT_SUCCESS(storage.FindNetworkIdentity(CertificateKeyId(kTestIdentityC.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, index3);

    // Non-existent identifier returns NOT_FOUND.
    CertificateKeyIdStorage keyIdMissing = MakeKeyId(0xFF);
    EXPECT_EQ(storage.FindNetworkIdentity(CertificateKeyId(keyIdMissing), found, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, StoreMultipleDifferentIdentitiesAtomically)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Store a NASS with two identities of different types (using a cast to create a
    // second type value, since the storage layer doesn't restrict type values).
    auto secondType                                       = static_cast<IdentityTypeEnum>(1);
    NetworkIdentityStorage::NetworkIdentityInfo identityB = kTestIdentityB;
    identityB.type                                        = secondType;

    const NetworkIdentityStorage::NetworkIdentityInfo * identities[] = { &kTestIdentityA, &identityB };
    uint16_t indices[2];

    NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
    MakeNassHandle(0xA1, nassInfo.secretHandle);
    nassInfo.createdTimestamp = Clock::Seconds32(100);
    ASSERT_SUCCESS(
        storage.StoreNetworkAdministratorSecretAndDerivedIdentities(nassInfo, Span(identities), Span<uint16_t>(indices, 2)));

    // Both should be stored and current (each is the only identity of its type)
    NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(
        storage.IterateNetworkIdentities(NetworkIdentityStorage::NetworkIdentityFlags::kPopulateIdentifier, MutableByteSpan()));
    ASSERT_TRUE(iter.IsValid());
    EXPECT_EQ(iter.Count(), 2u);

    NetworkIdentityStorage::NetworkIdentityEntry entry;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(indices[0], entry, {}, MutableByteSpan()));
    EXPECT_EQ(entry.type, IdentityTypeEnum::kEcdsa);
    EXPECT_TRUE(entry.current);

    ASSERT_SUCCESS(storage.FindNetworkIdentity(indices[1], entry, {}, MutableByteSpan()));
    EXPECT_EQ(entry.type, secondType);
    EXPECT_TRUE(entry.current);

    // Verify persistence
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing freshIter(
        freshStorage.IterateNetworkIdentities({}, MutableByteSpan()));
    ASSERT_TRUE(freshIter.IsValid());
    EXPECT_EQ(freshIter.Count(), 2u);
}

TEST_F(TestDefaultNetworkIdentityStorage, StoreNetworkIdentityViaEntryOverload)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    constexpr uint32_t kNassTimestamp = 500;

    // Store using the Entry-based convenience overload
    NetworkIdentityStorage::NetworkIdentityEntry entry;
    entry.type            = kTestIdentityA.type;
    entry.identifier      = kTestIdentityA.identifier;
    entry.compactIdentity = kTestIdentityA.compactIdentity;
    entry.keypairHandle   = kTestIdentityA.keypairHandle;

    NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
    MakeNassHandle(0xA1, nassInfo.secretHandle);
    nassInfo.createdTimestamp = Clock::Seconds32(kNassTimestamp);

    auto * entryPtr = &entry;
    ASSERT_SUCCESS(storage.StoreNetworkAdministratorSecretAndDerivedIdentities(
        nassInfo, Span<NetworkIdentityStorage::NetworkIdentityEntry * const>(&entryPtr, 1)));

    // Verify the entry was populated by the overload
    EXPECT_NE(entry.index, 0u);
    EXPECT_TRUE(entry.current);
    EXPECT_EQ(entry.createdTimestamp, Clock::Seconds32(kNassTimestamp));
    EXPECT_EQ(entry.clientCount, 0u);

    // Fetch via FindNetworkIdentity and verify all fields match
    constexpr BitFlags<NetworkIdentityFlags> allFlags(NetworkIdentityFlags::kPopulateAll);
    uint8_t findBuffer[NetworkIdentityStorage::NetworkIdentityBufferSize(allFlags)];
    NetworkIdentityStorage::NetworkIdentityEntry found;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(entry.index, found, allFlags, Span(findBuffer)));

    EXPECT_EQ(found.index, entry.index);
    EXPECT_EQ(found.type, entry.type);
    EXPECT_EQ(found.current, entry.current);
    EXPECT_EQ(found.createdTimestamp, entry.createdTimestamp);
    EXPECT_EQ(found.clientCount, entry.clientCount);
    EXPECT_EQ(found.identifier, entry.identifier);
    EXPECT_TRUE(found.compactIdentity.data_equal(kTestIdentityA.compactIdentity));
    EXPECT_TRUE(found.keypairHandle.data_equal(kTestIdentityA.keypairHandle));
}

TEST_F(TestDefaultNetworkIdentityStorage, NetworkIdentityCapacityLimit)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);
    NetworkIdentityStorage::NetworkIdentityInfo identity = kTestIdentityA;

    for (size_t i = 0; i <= NetworkIdentityStorage::kMaxNetworkIdentities; i++)
    {
        identity.identifier[0] = static_cast<uint8_t>(i);
        uint16_t idx;
        EXPECT_EQ(StoreTestNASSAndIdentity(storage, 0xA1, static_cast<uint32_t>(i + 1), identity, idx),
                  (i < NetworkIdentityStorage::kMaxNetworkIdentities ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY));
    }
}

TEST_F(TestDefaultNetworkIdentityStorage, NetworkIdentityIndexWraparound)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Import two identities, remove the first. Index 1 is now free, index 2 is allocated
    // (current), and mNINextIndex is 3.
    uint32_t timestamp = 1;
    uint16_t indexA, indexB;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, timestamp++, kTestIdentityA, indexA));
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA2, timestamp++, kTestIdentityB, indexB));
    EXPECT_EQ(indexA, 1u);
    EXPECT_EQ(indexB, 2u);
    ASSERT_SUCCESS(storage.RemoveNetworkIdentity(indexA));

    // Keep importing and removing to advance mNINextIndex all the way to the end
    // of the range (0xFFFE). Each iteration imports a new identity (which becomes current,
    // pushing the previous to non-current) and then removes the now non-current identity.
    NetworkIdentityStorage::NetworkIdentityInfo dummy = kTestIdentityA;
    uint16_t prevIndex                                = indexB;
    for (uint16_t expected = 3; expected <= 0xFFFE; expected++)
    {
        dummy.identifier.fill(static_cast<uint8_t>(expected));
        uint16_t idx;
        ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA0, timestamp++, dummy, idx));
        EXPECT_EQ(idx, expected);
        ASSERT_SUCCESS(storage.RemoveNetworkIdentity(prevIndex));
        prevIndex = idx;
    }

    // Next allocation should wrap to 1 (the slot we freed at the start)
    dummy.identifier.fill(0xAA);
    uint16_t wrappedIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA0, timestamp++, dummy, wrappedIndex));
    EXPECT_EQ(wrappedIndex, 1u);
    ASSERT_SUCCESS(storage.RemoveNetworkIdentity(prevIndex));

    // Next should get 2 (freed during cycling, mNINextIndex continues from 1)
    dummy.identifier.fill(0xBB);
    uint16_t nextIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA0, timestamp++, dummy, nextIndex));
    EXPECT_EQ(nextIndex, 2u);
}

TEST_F(TestDefaultNetworkIdentityStorage, NetworkIdentityPopulateFlags)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t index;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, index));

    // With no flags: all detail-record fields should be empty/zero, clientCount should be 0.
    // The detail record should not be loaded at all.
    {
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(storage.FindNetworkIdentity(index, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.identifier, CertificateKeyIdStorage{});
        EXPECT_EQ(entry.createdTimestamp, Clock::Seconds32(0));
        EXPECT_TRUE(entry.compactIdentity.empty());
        EXPECT_TRUE(entry.keypairHandle.empty());
        EXPECT_EQ(entry.clientCount, 0u);
        // index, type, current are always populated (from the table index)
        EXPECT_EQ(entry.index, index);
        EXPECT_EQ(entry.type, kTestIdentityA.type);
        EXPECT_TRUE(entry.current);
    }

    // With only kPopulateIdentifier
    {
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(storage.FindNetworkIdentity(index, entry, NetworkIdentityFlags::kPopulateIdentifier, MutableByteSpan()));
        EXPECT_EQ(entry.identifier, kTestIdentityA.identifier);
        EXPECT_EQ(entry.createdTimestamp, Clock::Seconds32(0));
    }

    // With only kPopulateCreatedTimestamp
    {
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(
            storage.FindNetworkIdentity(index, entry, NetworkIdentityFlags::kPopulateCreatedTimestamp, MutableByteSpan()));
        EXPECT_EQ(entry.identifier, CertificateKeyIdStorage{});
        EXPECT_EQ(entry.createdTimestamp, Clock::Seconds32(100)); // matches NASS timestamp
    }

    // With only kPopulateCompactIdentity
    {
        constexpr BitFlags<NetworkIdentityFlags> flags(NetworkIdentityFlags::kPopulateCompactIdentity);
        uint8_t buf[NetworkIdentityStorage::NetworkIdentityBufferSize(flags)];
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(storage.FindNetworkIdentity(index, entry, flags, MutableByteSpan(buf)));
        EXPECT_TRUE(entry.compactIdentity.data_equal(kTestIdentityA.compactIdentity));
        EXPECT_TRUE(entry.keypairHandle.empty());
    }

    // With only kPopulateKeypairHandle
    {
        constexpr BitFlags<NetworkIdentityFlags> flags(NetworkIdentityFlags::kPopulateKeypairHandle);
        uint8_t buf[NetworkIdentityStorage::NetworkIdentityBufferSize(flags)];
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(storage.FindNetworkIdentity(index, entry, flags, MutableByteSpan(buf)));
        EXPECT_TRUE(entry.compactIdentity.empty());
        EXPECT_TRUE(entry.keypairHandle.data_equal(kTestIdentityA.keypairHandle));
    }

    // With all flags: all optional fields populated.
    {
        uint8_t buf[NetworkIdentityStorage::NetworkIdentityBufferSize()];
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(storage.FindNetworkIdentity(index, entry, NetworkIdentityFlags::kPopulateAll, MutableByteSpan(buf)));
        EXPECT_EQ(entry.identifier, kTestIdentityA.identifier);
        EXPECT_EQ(entry.createdTimestamp, Clock::Seconds32(100)); // matches NASS timestamp
        EXPECT_TRUE(entry.compactIdentity.data_equal(kTestIdentityA.compactIdentity));
        EXPECT_TRUE(entry.keypairHandle.data_equal(kTestIdentityA.keypairHandle));
        EXPECT_EQ(entry.clientCount, 0u);
    }

    // Buffer too small: direct path (FindNetworkIdentity by index).
    {
        constexpr BitFlags<NetworkIdentityFlags> flags(NetworkIdentityFlags::kPopulateCompactIdentity);
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        EXPECT_EQ(storage.FindNetworkIdentity(index, entry, flags, MutableByteSpan()), CHIP_ERROR_BUFFER_TOO_SMALL);
    }

    // Buffer too small: iteration-based path (FindNetworkIdentity by identifier).
    {
        constexpr BitFlags<NetworkIdentityFlags> flags(NetworkIdentityFlags::kPopulateCompactIdentity);
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        EXPECT_EQ(storage.FindNetworkIdentity(CertificateKeyId(kTestIdentityA.identifier), entry, flags, MutableByteSpan()),
                  CHIP_ERROR_BUFFER_TOO_SMALL);
    }
}

// StoreNetworkAdministratorSecretAndDerivedIdentities writes in order:
//   1. NI detail entries                   g/nim/n/<index>
//   2. NI table index with pending marker  g/nim/ni
//   3. NASS                                g/nim/nass
//   4. NI table index                      g/nim/ni
static void TestStoreNASSAndIdentitiesPersistenceFaults(void (*poison)(TestPersistentStorageDelegate &), bool success)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t index1 = 0;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, index1));

    poison(backingStore);
    uint16_t index2;
    EXPECT_EQ((StoreTestNASSAndIdentity(storage, 0xA2, 200, kTestIdentityB, index2) == CHIP_NO_ERROR), success);
    backingStore.ClearPoisonKeys();

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        // Check the NASS
        NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
        ASSERT_SUCCESS(s.RetrieveNetworkAdministratorSecret(nassInfo));
        EXPECT_EQ(NassHandleTag(nassInfo.secretHandle), success ? 0xA2 : 0xA1);

        // Check the Network Identities
        NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(s.IterateNetworkIdentities({}, MutableByteSpan()));
        EXPECT_TRUE(iter.IsValid());
        EXPECT_EQ(iter.Count(), success ? 2u : 1u);

        // NI 1 should always exist
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        EXPECT_SUCCESS(s.FindNetworkIdentity(index1, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.current, !success);

        EXPECT_EQ(s.FindNetworkIdentity(index2, entry, {}, MutableByteSpan()), success ? CHIP_NO_ERROR : CHIP_ERROR_NOT_FOUND);
        EXPECT_TRUE(!success || entry.current);
    };

    // Verify in-memory state and persisted state
    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportNASSAndIdentitiesRollbackOnDetailWriteFailure)
{
    // Poison the NI entry key that would be allocated next (index 2, since 1 is taken).
    // This causes failure at step 1 (StoreNetworkIdentityDetail).
    TestStoreNASSAndIdentitiesPersistenceFaults(
        [](TestPersistentStorageDelegate & store) {
            store.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentity(2).KeyName());
        },
        /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportNASSAndIdentitiesRollbackOnIndexWriteFailure)
{
    // Poison the NI table index key. NI detail write succeeds but index write fails (step 2).
    TestStoreNASSAndIdentitiesPersistenceFaults(
        [](TestPersistentStorageDelegate & store) {
            store.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex().KeyName());
        },
        /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportNASSAndIdentitiesRollbackOnNASSWriteFailure)
{
    // Poison the NASS key. NI detail and index succeed but NASS write fails (step 3).
    // The in-memory rollback undoes the index write, so this is a clean rollback.
    TestStoreNASSAndIdentitiesPersistenceFaults(
        [](TestPersistentStorageDelegate & store) {
            store.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementAdministratorSecret().KeyName());
        },
        /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportNASSAndIdentitiesRollbackOnNASSWriteFailureWithRecovery)
{
    // NASS write fails (step 3) AND the rollback index write also fails, leaving the
    // pending marker in the persisted index. The fresh instance detects the marker,
    // compares against the NASS timestamp, and prunes the orphaned entries.
    TestStoreNASSAndIdentitiesPersistenceFaults(
        [](TestPersistentStorageDelegate & store) {
            store.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementAdministratorSecret().KeyName(), -1, 0);
            // NI index: first write (step 2) succeeds, second write (rollback) fails, recovery cleanup succeeds
            store.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex().KeyName(), -1, 0b101);
        },
        /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, ImportNASSAndIdentitiesSuccessWithStaleMarker)
{
    // NASS write succeeds (step 3) but the cleanup index write (step 4) fails, leaving
    // the pending marker. The import is still durable; the marker cleared by the fresh
    // instance on load.
    TestStoreNASSAndIdentitiesPersistenceFaults(
        [](TestPersistentStorageDelegate & store) {
            // NI index: first write (step 2) succeeds, second (step 4, cleanup) fails, recovery cleanup succeeds
            store.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex().KeyName(), -1, 0b101);
        },
        /* success */ true);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveNetworkIdentity)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t indexA, indexB, indexC;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, indexA));
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xB2, 200, kTestIdentityB, indexB));
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xC3, 300, kTestIdentityC, indexC));

    // Can't remove current
    EXPECT_EQ(storage.RemoveNetworkIdentity(indexC), CHIP_ERROR_INCORRECT_STATE);

    // Remove B
    ASSERT_SUCCESS(storage.RemoveNetworkIdentity(indexB));

    // Should have 2 remaining
    NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(storage.IterateNetworkIdentities({}, MutableByteSpan()));
    EXPECT_TRUE(iter.IsValid());
    EXPECT_EQ(iter.Count(), 2u);

    // B is gone
    NetworkIdentityStorage::NetworkIdentityEntry entry;
    EXPECT_EQ(storage.FindNetworkIdentity(indexB, entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);

    // A and C should still be findable, C is still current
    ASSERT_SUCCESS(storage.FindNetworkIdentity(indexA, entry, {}, MutableByteSpan()));
    EXPECT_EQ(entry.index, indexA);
    EXPECT_FALSE(entry.current);

    ASSERT_SUCCESS(storage.FindNetworkIdentity(indexC, entry, {}, MutableByteSpan()));
    EXPECT_EQ(entry.index, indexC);
    EXPECT_TRUE(entry.current);

    ASSERT_SUCCESS(storage.FindCurrentNetworkIdentity(IdentityTypeEnum::kEcdsa, entry, {}, MutableByteSpan()));
    EXPECT_EQ(entry.index, indexC);

    // Remove again should fail
    EXPECT_EQ(storage.RemoveNetworkIdentity(indexB), CHIP_ERROR_NOT_FOUND);
}

// RemoveNetworkIdentity writes in order:
//   1. NI table index omitting the entry  g/nim/ni   — commit point
//   2. Delete NI detail                   g/nim/n/<index>   — best-effort
static void TestRemoveNetworkIdentityPersistenceFaults(StorageKeyName poisonKeyName, bool success)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Store two NIs so the first becomes non-current
    uint16_t indexA, indexB;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, indexA));
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xB2, 200, kTestIdentityB, indexB));

    backingStore.AddPoisonKey(poisonKeyName.KeyName());
    ASSERT_EQ((storage.RemoveNetworkIdentity(indexA) == CHIP_NO_ERROR), success);
    backingStore.ClearPoisonKeys();

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        NetworkIdentityStorage::NetworkIdentityEntry entry;

        // B should always be present and current
        ASSERT_SUCCESS(s.FindNetworkIdentity(indexB, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.index, indexB);
        EXPECT_TRUE(entry.current);

        // A should be gone if we succeeded
        EXPECT_EQ(s.FindNetworkIdentity(indexA, entry, {}, MutableByteSpan()), success ? CHIP_ERROR_NOT_FOUND : CHIP_NO_ERROR);

        NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(s.IterateNetworkIdentities({}, MutableByteSpan()));
        EXPECT_TRUE(iter.IsValid());
        EXPECT_EQ(iter.Count(), success ? 1u : 2u);
    };

    // Verify in-memory state and persisted state
    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveNetworkIdentityRollbackOnIndexWriteFailure)
{
    // Poison the NI table index key — removal should fail and both NIs remain.
    TestRemoveNetworkIdentityPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex(), false);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveNetworkIdentitySuccessWithStaleDetailRecord)
{
    // Poison the NI detail key — removal succeeds (detail deletion is best-effort),
    // but a stale detail record is left behind.
    TestRemoveNetworkIdentityPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentity(1), true);
}

TEST_F(TestDefaultNetworkIdentityStorage, PersistenceAcrossInstances)
{
    TestPersistentStorageDelegate backingStore;

    uint16_t index1, index2;
    {
        DefaultNetworkIdentityStorage storage(backingStore);
        ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, index1));
        ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA2, 200, kTestIdentityB, index2));
    }

    // Create a fresh instance on the same backing store — data should survive.
    {
        DefaultNetworkIdentityStorage storage(backingStore);

        // NASS should be the second one.
        NetworkIdentityStorage::NetworkAdministratorSecretInfo nassInfo;
        ASSERT_SUCCESS(storage.RetrieveNetworkAdministratorSecret(nassInfo));
        EXPECT_EQ(NassHandleTag(nassInfo.secretHandle), 0xA2);
        EXPECT_EQ(nassInfo.createdTimestamp, Clock::Seconds32(200));

        // Both NI entries should be present with correct metadata.
        NetworkIdentityStorage::NetworkIdentityIterator::AutoReleasing iter(
            storage.IterateNetworkIdentities({}, MutableByteSpan()));
        EXPECT_TRUE(iter.IsValid());
        EXPECT_EQ(iter.Count(), 2u);

        // Find by index should still work.
        NetworkIdentityStorage::NetworkIdentityEntry entry;
        ASSERT_SUCCESS(storage.FindNetworkIdentity(index1, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.index, index1);
        EXPECT_FALSE(entry.current);

        ASSERT_SUCCESS(storage.FindNetworkIdentity(index2, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.index, index2);
        EXPECT_TRUE(entry.current);

        // Index allocation should continue past previously used indices.
        uint16_t index3;
        ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA3, 300, kTestIdentityC, index3));
        EXPECT_GT(index3, index2);
    }
}

///// Client Storage Tests /////////////////////////////////////////////////////////////

// Test client data. Each client has distinct identifier and compact identity values.
static const NetworkIdentityStorage::ClientInfo kTestClientX = {
    .identifier      = { 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D,
                         0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D, 0x4D },
    .compactIdentity = ByteSpan((const uint8_t[]){ 0xD1, 0xD2, 0xD3 }),
};

static const NetworkIdentityStorage::ClientInfo kTestClientY = {
    .identifier      = { 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E,
                         0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E, 0x5E },
    .compactIdentity = ByteSpan((const uint8_t[]){ 0xE1, 0xE2, 0xE3 }),
};

static const NetworkIdentityStorage::ClientInfo kTestClientZ = {
    .identifier      = { 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F,
                         0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F },
    .compactIdentity = ByteSpan((const uint8_t[]){ 0xF1, 0xF2, 0xF3 }),
};

TEST_F(TestDefaultNetworkIdentityStorage, NoClients)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);
    NetworkIdentityStorage::ClientEntry entry;

    {
        NetworkIdentityStorage::ClientIterator::AutoReleasing iter(storage.IterateClients({}, MutableByteSpan()));
        EXPECT_TRUE(iter.IsValid());
        EXPECT_EQ(iter.Count(), 0u);
        EXPECT_FALSE(iter.Next(entry));
    }

    EXPECT_EQ(storage.FindClient(uint16_t(1), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);

    CertificateKeyIdStorage keyId = MakeKeyId(0x55);
    EXPECT_EQ(storage.FindClient(CertificateKeyId(keyId), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, AddAndFindClient)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));
    EXPECT_GE(clientIndex, 1u);
    EXPECT_LE(clientIndex, 2047u);

    // Find by index with all flags
    constexpr BitFlags<ClientFlags> allFlags(ClientFlags::kPopulateAll);
    uint8_t buf[NetworkIdentityStorage::ClientBufferSize(allFlags)];
    NetworkIdentityStorage::ClientEntry found;
    ASSERT_SUCCESS(storage.FindClient(clientIndex, found, allFlags, MutableByteSpan(buf)));

    EXPECT_EQ(found.index, clientIndex);
    EXPECT_EQ(found.identifier, kTestClientX.identifier);
    EXPECT_TRUE(found.compactIdentity.data_equal(kTestClientX.compactIdentity));
    EXPECT_EQ(found.networkIdentityIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex);

    // Find by identifier
    NetworkIdentityStorage::ClientEntry foundById;
    ASSERT_SUCCESS(storage.FindClient(CertificateKeyId(kTestClientX.identifier), foundById, {}, MutableByteSpan()));
    EXPECT_EQ(foundById.index, clientIndex);
}

TEST_F(TestDefaultNetworkIdentityStorage, AddClientViaEntryOverload)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Add using the Entry-based convenience overload
    NetworkIdentityStorage::ClientEntry entry;
    entry.identifier      = kTestClientX.identifier;
    entry.compactIdentity = kTestClientX.compactIdentity;
    ASSERT_SUCCESS(storage.AddClient(entry));

    // Verify the entry was populated by the overload
    EXPECT_GE(entry.index, 1u);
    EXPECT_LE(entry.index, 2047u);
    EXPECT_EQ(entry.networkIdentityIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex);

    // Verify we can find the client by index and identifier
    NetworkIdentityStorage::ClientEntry found;
    ASSERT_SUCCESS(
        storage.FindClient(entry.index, found, NetworkIdentityStorage::ClientFlags::kPopulateIdentifier, MutableByteSpan()));
    EXPECT_EQ(found.identifier, kTestClientX.identifier);

    ASSERT_SUCCESS(storage.FindClient(CertificateKeyId(kTestClientX.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, entry.index);
}

TEST_F(TestDefaultNetworkIdentityStorage, AddMultipleClients)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t ix, iy, iz;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, ix));
    ASSERT_SUCCESS(storage.AddClient(kTestClientY, iy));
    ASSERT_SUCCESS(storage.AddClient(kTestClientZ, iz));
    EXPECT_NE(ix, iy);
    EXPECT_NE(iy, iz);

    // Iterator should return all three
    constexpr BitFlags<ClientFlags> flags(ClientFlags::kPopulateIdentifier);
    NetworkIdentityStorage::ClientIterator::AutoReleasing iter(storage.IterateClients(flags, MutableByteSpan()));
    EXPECT_TRUE(iter.IsValid());
    EXPECT_EQ(iter.Count(), 3u);

    NetworkIdentityStorage::ClientEntry entry;
    size_t count = 0;
    while (iter.Next(entry))
    {
        count++;
    }
    EXPECT_EQ(count, 3u);

    // Find by identifier distinguishes entries
    NetworkIdentityStorage::ClientEntry found;
    ASSERT_SUCCESS(storage.FindClient(CertificateKeyId(kTestClientX.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, ix);
    ASSERT_SUCCESS(storage.FindClient(CertificateKeyId(kTestClientY.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, iy);
    ASSERT_SUCCESS(storage.FindClient(CertificateKeyId(kTestClientZ.identifier), found, {}, MutableByteSpan()));
    EXPECT_EQ(found.index, iz);

    // Non-existent identifier returns NOT_FOUND
    CertificateKeyIdStorage missing = MakeKeyId(0xFF);
    EXPECT_EQ(storage.FindClient(CertificateKeyId(missing), found, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, ClientCapacityLimit)
{
    TestPersistentStorageDelegate backingStore;
    // Use a small capacity to keep the test fast
    DefaultNetworkIdentityStorage storage(backingStore, /* maxClients */ 3);
    EXPECT_EQ(storage.GetClientTableCapacity(), 3u);

    NetworkIdentityStorage::ClientInfo client = kTestClientX;
    for (uint16_t i = 0; i < 4; i++)
    {
        client.identifier[0] = static_cast<uint8_t>(i);
        uint16_t idx;
        EXPECT_EQ(storage.AddClient(client, idx), (i < 3 ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY));
    }
}

TEST_F(TestDefaultNetworkIdentityStorage, ClientIndexWraparound)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Add two clients, remove the first. Index 1 is now free, index 2 is allocated,
    // and mClientNextIndex is 3.
    uint16_t ix, iy;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, ix));
    ASSERT_SUCCESS(storage.AddClient(kTestClientY, iy));
    EXPECT_EQ(ix, 1u);
    EXPECT_EQ(iy, 2u);
    uint16_t removedNI;
    ASSERT_SUCCESS(storage.RemoveClient(ix, removedNI));

    // Keep adding and immediately removing clients to advance mClientNextIndex
    // all the way to the end of the range (2047), without exceeding maxClients.
    NetworkIdentityStorage::ClientInfo dummy = kTestClientX;
    for (uint16_t expected = 3; expected <= 2047; expected++)
    {
        dummy.identifier.fill(static_cast<uint8_t>(expected));
        uint16_t idx;
        ASSERT_SUCCESS(storage.AddClient(dummy, idx));
        EXPECT_EQ(idx, expected);
        ASSERT_SUCCESS(storage.RemoveClient(idx, removedNI));
    }

    // Next allocation should wrap to 1 (the slot we freed at the start)
    dummy.identifier.fill(0xAA);
    uint16_t wrappedIndex;
    ASSERT_SUCCESS(storage.AddClient(dummy, wrappedIndex));
    EXPECT_EQ(wrappedIndex, 1u);

    // Next should skip 2 (still allocated) and land on 3
    dummy.identifier.fill(0xBB);
    uint16_t skippedIndex;
    ASSERT_SUCCESS(storage.AddClient(dummy, skippedIndex));
    EXPECT_EQ(skippedIndex, 3u);
}

TEST_F(TestDefaultNetworkIdentityStorage, ClientPopulateFlags)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));

    // No flags: only index is populated, all other fields are zero
    {
        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.index, clientIndex);
        EXPECT_TRUE(entry.compactIdentity.empty());
        EXPECT_EQ(entry.networkIdentityIndex, 0u); // unpopulated sentinel, distinct from kNullNetworkIdentityIndex
    }

    // kPopulateIdentifier only
    {
        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateIdentifier, MutableByteSpan()));
        EXPECT_EQ(entry.identifier, kTestClientX.identifier);
        EXPECT_TRUE(entry.compactIdentity.empty());
    }

    // kPopulateCompactIdentity only
    {
        uint8_t buf[Credentials::kMaxCHIPCompactNetworkIdentityLength];
        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateCompactIdentity, MutableByteSpan(buf)));
        EXPECT_TRUE(entry.compactIdentity.data_equal(kTestClientX.compactIdentity));
    }

    // kPopulateNetworkIdentityIndex only
    {
        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateNetworkIdentityIndex, MutableByteSpan()));
        EXPECT_EQ(entry.networkIdentityIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex);
    }

    // All flags
    {
        uint8_t buf[NetworkIdentityStorage::ClientBufferSize()];
        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateAll, MutableByteSpan(buf)));
        EXPECT_EQ(entry.identifier, kTestClientX.identifier);
        EXPECT_TRUE(entry.compactIdentity.data_equal(kTestClientX.compactIdentity));
        EXPECT_EQ(entry.networkIdentityIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex);
    }
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClient)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));

    uint16_t removedNIIndex;
    ASSERT_SUCCESS(storage.RemoveClient(clientIndex, removedNIIndex));
    EXPECT_EQ(removedNIIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex);

    // Should be gone
    NetworkIdentityStorage::ClientEntry entry;
    EXPECT_EQ(storage.FindClient(clientIndex, entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
    EXPECT_EQ(storage.FindClient(CertificateKeyId(kTestClientX.identifier), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);

    // Iterator should be empty
    NetworkIdentityStorage::ClientIterator::AutoReleasing iter(storage.IterateClients({}, MutableByteSpan()));
    EXPECT_TRUE(iter.IsValid());
    EXPECT_EQ(iter.Count(), 0u);

    // Remove again should fail
    EXPECT_EQ(storage.RemoveClient(clientIndex, removedNIIndex), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClientNotFound)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);
    uint16_t removedNIIndex;
    EXPECT_EQ(storage.RemoveClient(999, removedNIIndex), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, UpdateClientNetworkIdentityIndex)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Store a NASS + NI so we have an NI to reference
    uint16_t niIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, niIndex));

    // Add a client
    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));

    // Update to reference the NI
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndex, niIndex, unusedOldNI));

    // Verify the client now references the NI
    NetworkIdentityStorage::ClientEntry entry;
    ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateNetworkIdentityIndex, MutableByteSpan()));
    EXPECT_EQ(entry.networkIdentityIndex, niIndex);

    // Verify NI client count is incremented
    NetworkIdentityStorage::NetworkIdentityEntry niEntry;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(niIndex, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
    EXPECT_EQ(niEntry.clientCount, 1u);

    // Update to null — client count should go back to 0
    ASSERT_SUCCESS(
        storage.UpdateClientNetworkIdentityIndex(clientIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex, unusedOldNI));

    ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateNetworkIdentityIndex, MutableByteSpan()));
    EXPECT_EQ(entry.networkIdentityIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex);

    ASSERT_SUCCESS(storage.FindNetworkIdentity(niIndex, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
    EXPECT_EQ(niEntry.clientCount, 0u);

    // No-op update (already null) should succeed
    ASSERT_SUCCESS(
        storage.UpdateClientNetworkIdentityIndex(clientIndex, NetworkIdentityStorage::kNullNetworkIdentityIndex, unusedOldNI));
}

TEST_F(TestDefaultNetworkIdentityStorage, UpdateClientNetworkIdentityIndexNotFound)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Store an NI and add a client referencing it
    uint16_t niIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, niIndex));
    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndex, niIndex, unusedOldNI));

    constexpr uint16_t kBogusClientIndex = 999;
    ASSERT_NE(kBogusClientIndex, clientIndex);
    constexpr uint16_t kBogusNIIndex = 9999;
    ASSERT_NE(kBogusNIIndex, niIndex);

    // UpdateClientNetworkIdentityIndex with non-existent client index
    EXPECT_EQ(storage.UpdateClientNetworkIdentityIndex(kBogusClientIndex, niIndex, unusedOldNI), CHIP_ERROR_NOT_FOUND);

    // UpdateClientNetworkIdentityIndex with non-existent NI index — client's NI reference should be unchanged
    EXPECT_EQ(storage.UpdateClientNetworkIdentityIndex(clientIndex, kBogusNIIndex, unusedOldNI), CHIP_ERROR_NOT_FOUND);

    NetworkIdentityStorage::ClientEntry entry;
    ASSERT_SUCCESS(storage.FindClient(clientIndex, entry, ClientFlags::kPopulateNetworkIdentityIndex, MutableByteSpan()));
    EXPECT_EQ(entry.networkIdentityIndex, niIndex);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClientUpdatesNIClientCount)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    uint16_t niIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, niIndex));

    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndex, niIndex, unusedOldNI));

    // Verify count is 1
    NetworkIdentityStorage::NetworkIdentityEntry niEntry;
    ASSERT_SUCCESS(storage.FindNetworkIdentity(niIndex, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
    EXPECT_EQ(niEntry.clientCount, 1u);

    // Remove the client — NI count should go back to 0
    uint16_t removedNIIndex;
    ASSERT_SUCCESS(storage.RemoveClient(clientIndex, removedNIIndex));
    EXPECT_EQ(removedNIIndex, niIndex);

    ASSERT_SUCCESS(storage.FindNetworkIdentity(niIndex, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
    EXPECT_EQ(niEntry.clientCount, 0u);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveNetworkIdentityWithClients)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Store two NIs so the first becomes non-current
    uint16_t niIndex, niIndex2;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, niIndex));
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA2, 200, kTestIdentityB, niIndex2));

    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndex, niIndex, unusedOldNI));

    // Should fail — has clients
    EXPECT_EQ(storage.RemoveNetworkIdentity(niIndex), CHIP_ERROR_INCORRECT_STATE);

    // Remove the client, then removal should succeed
    uint16_t removedNIIndex;
    ASSERT_SUCCESS(storage.RemoveClient(clientIndex, removedNIIndex));
    ASSERT_SUCCESS(storage.RemoveNetworkIdentity(niIndex));

    NetworkIdentityStorage::NetworkIdentityEntry entry;
    EXPECT_EQ(storage.FindNetworkIdentity(niIndex, entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
}

TEST_F(TestDefaultNetworkIdentityStorage, ClientPersistenceAcrossInstances)
{
    TestPersistentStorageDelegate backingStore;

    uint16_t ix, iy;
    {
        DefaultNetworkIdentityStorage storage(backingStore);
        ASSERT_SUCCESS(storage.AddClient(kTestClientX, ix));
        ASSERT_SUCCESS(storage.AddClient(kTestClientY, iy));
    }

    // Fresh instance on the same backing store
    {
        DefaultNetworkIdentityStorage storage(backingStore);

        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(storage.FindClient(ix, entry, ClientFlags::kPopulateIdentifier, MutableByteSpan()));
        EXPECT_EQ(entry.identifier, kTestClientX.identifier);

        ASSERT_SUCCESS(storage.FindClient(iy, entry, ClientFlags::kPopulateIdentifier, MutableByteSpan()));
        EXPECT_EQ(entry.identifier, kTestClientY.identifier);

        // Find by identifier should still work
        ASSERT_SUCCESS(storage.FindClient(CertificateKeyId(kTestClientX.identifier), entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.index, ix);

        // Index allocation should continue past previously used indices
        uint16_t iz;
        ASSERT_SUCCESS(storage.AddClient(kTestClientZ, iz));
        EXPECT_GT(iz, iy);
    }
}

// AddClient writes three things in order:
//   1. Client detail record (g/nim/c/<index>)
//   2. Identifier mapping   (g/nim/@<base85>)
//   3. Client table index   (g/nim/ci)
// Each test below poisons one of these keys to trigger a failure at that step
// and verifies that the storage state is rolled back to the pre-call state.
static void TestAddClientRollback(StorageKeyName poisonKeyName)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Add one client successfully
    uint16_t firstIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, firstIndex));

    // Poison the specified key to trigger a failure during the second add
    backingStore.AddPoisonKey(poisonKeyName.KeyName());

    uint16_t secondIndex;
    EXPECT_NE(storage.AddClient(kTestClientY, secondIndex), CHIP_NO_ERROR);

    // Verify rollback: storage should look like the second call never happened
    backingStore.ClearPoisonKeys();

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        // Only the first client should exist
        NetworkIdentityStorage::ClientIterator::AutoReleasing iter(s.IterateClients({}, MutableByteSpan()));
        EXPECT_TRUE(iter.IsValid());
        EXPECT_EQ(iter.Count(), 1u);

        // First client is findable
        NetworkIdentityStorage::ClientEntry entry;
        ASSERT_SUCCESS(s.FindClient(firstIndex, entry, {}, MutableByteSpan()));
        EXPECT_EQ(entry.index, firstIndex);

        // Second client is not findable by index or identifier
        EXPECT_EQ(s.FindClient(static_cast<uint16_t>(firstIndex + 1), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
        EXPECT_EQ(s.FindClient(CertificateKeyId(kTestClientY.identifier), entry, {}, MutableByteSpan()), CHIP_ERROR_NOT_FOUND);
    };

    // Verify in-memory state and persisted state
    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}

TEST_F(TestDefaultNetworkIdentityStorage, AddClientRollbackOnDetailWriteFailure)
{
    // Poison the client detail key for index 2 (index 1 is taken by the first add).
    TestAddClientRollback(DefaultStorageKeyAllocator::NetworkIdentityManagementClient(2));
}

TEST_F(TestDefaultNetworkIdentityStorage, AddClientRollbackOnMappingWriteFailure)
{
    // Poison the identifier mapping key for kTestClientY.
    TestAddClientRollback(DefaultStorageKeyAllocator::NetworkIdentityManagementClientIdentifierMapping(kTestClientY.identifier));
}

TEST_F(TestDefaultNetworkIdentityStorage, AddClientRollbackOnTableIndexWriteFailure)
{
    // Poison the table index key. Detail and mapping succeed but table index persist fails.
    TestAddClientRollback(DefaultStorageKeyAllocator::NetworkIdentityManagementClientIndex());
}

// RemoveClient (when the client references an NI) writes in order:
//   1. NI table index with dirty counts  g/nim/ni
//   2. Client table index                g/nim/ci   — commit point
//   3. NI table index with clean counts  g/nim/ni   — best-effort
//   4. Delete client detail              g/nim/c/<index>   — best-effort
//   5. Delete identifier mapping         g/nim/@<base85>   — best-effort
static void TestRemoveClientPersistenceFaults(StorageKeyName poisonKeyName, int writePattern, bool success)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Store an NI and a client that references it
    uint16_t niIndex;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, niIndex));
    uint16_t clientIndex;
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndex));
    uint16_t unusedOldNI;
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndex, niIndex, unusedOldNI));

    // Poison and attempt removal
    backingStore.AddPoisonKey(poisonKeyName.KeyName(), /* read */ -1, writePattern);
    uint16_t removedNIIndex;
    ASSERT_EQ((storage.RemoveClient(clientIndex, removedNIIndex) == CHIP_NO_ERROR), success);
    backingStore.ClearPoisonKeys();

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        // Client should be gone if removal succeeded
        NetworkIdentityStorage::ClientEntry entry;
        EXPECT_EQ(s.FindClient(clientIndex, entry, {}, MutableByteSpan()), success ? CHIP_ERROR_NOT_FOUND : CHIP_NO_ERROR);

        NetworkIdentityStorage::ClientIterator::AutoReleasing iter(s.IterateClients({}, MutableByteSpan()));
        EXPECT_TRUE(iter.IsValid());
        EXPECT_EQ(iter.Count(), success ? 0u : 1u);

        // NI client count should be correct (may require rebuild from dirty state)
        NetworkIdentityStorage::NetworkIdentityEntry niEntry;
        ASSERT_SUCCESS(s.FindNetworkIdentity(niIndex, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
        EXPECT_EQ(niEntry.clientCount, success ? 0 : 1);
    };

    // Verify in-memory state and persisted state
    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClientRollbackOnDirtyCountsWriteFailure)
{
    // Poison NI table index — dirty counts write (step 1) fails, nothing changes.
    TestRemoveClientPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex(),
                                      /* writePattern */ 0, /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClientRollbackOnTableIndexWriteFailure)
{
    // Poison client table index — commit-point write (step 2) fails, bitmap rolled back.
    // NI dirty counts from step 1 are left behind but safely rebuilt on next load.
    TestRemoveClientPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementClientIndex(),
                                      /* writePattern */ 0, /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClientSuccessWithDirtyCounts)
{
    // Dirty counts write succeeds (bit 0), client removal succeeds,
    // but clean counts write (bit 1) fails. Counts rebuilt on next load.
    TestRemoveClientPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex(),
                                      /* writePattern */ 0b01, /* success */ true);
}

// UpdateClientNetworkIdentityIndex writes in order:
//   1. NI table index with dirty counts  g/nim/ni
//   2. Client detail with new NI index   g/nim/c/<index>
//   3. NI table index with clean counts  g/nim/ni   — best-effort
static void TestUpdateClientNIIndexPersistenceFaults(StorageKeyName poisonKeyName, int writePattern, bool success)
{
    TestPersistentStorageDelegate backingStore;
    DefaultNetworkIdentityStorage storage(backingStore);

    // Create an NI and ClientY that references it, and ClientX that doesn't.
    uint16_t niIndex, clientIndexX, clientIndexY;
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, niIndex));
    uint16_t unusedOldNI;
    ASSERT_SUCCESS(storage.AddClient(kTestClientY, clientIndexY));
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndexY, niIndex, unusedOldNI));
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, clientIndexX));

    // Attempt to update ClientX's NI index with the poisoned key
    backingStore.AddPoisonKey(poisonKeyName.KeyName(), /* read */ -1, writePattern);
    ASSERT_EQ((storage.UpdateClientNetworkIdentityIndex(clientIndexX, niIndex, unusedOldNI) == CHIP_NO_ERROR), success);

    backingStore.ClearPoisonKeys();
    uint16_t expectedNIClientCount = success ? 2 : 1;

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        // ClientX's NI reference should be updated only if the operation succeeded
        NetworkIdentityStorage::ClientEntry clientEntry;
        ASSERT_SUCCESS(s.FindClient(clientIndexX, clientEntry, ClientFlags::kPopulateNetworkIdentityIndex, MutableByteSpan()));
        EXPECT_EQ(clientEntry.networkIdentityIndex, success ? niIndex : NetworkIdentityStorage::kNullNetworkIdentityIndex);

        // NI client count should reflect whether the update took effect
        NetworkIdentityStorage::NetworkIdentityEntry niEntry;
        ASSERT_SUCCESS(s.FindNetworkIdentity(niIndex, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
        EXPECT_EQ(niEntry.clientCount, expectedNIClientCount);
    };

    // Verify in-memory state and persisted state
    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}

TEST_F(TestDefaultNetworkIdentityStorage, UpdateClientNIIndexRollbackOnDirtyCountsWriteFailure)
{
    // Dirty write itself fails → nothing happens, count stays at 1.
    TestUpdateClientNIIndexPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex(),
                                             /* writePattern */ 0, /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, UpdateClientNIIndexRollbackOnClientDetailWriteFailure)
{
    // Client detail write fails → client record NOT updated, count stays at 1.
    TestUpdateClientNIIndexPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementClient(2),
                                             /* writePattern */ 0, /* success */ false);
}

TEST_F(TestDefaultNetworkIdentityStorage, UpdateClientNIIndexSuccessWithDirtyCounts)
{
    // Dirty write succeeds (bit 0 = 1), client detail succeeds (different key),
    // clean count write fails (bit 1 = 0). Client record IS updated, count = 2.
    TestUpdateClientNIIndexPersistenceFaults(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex(),
                                             /* writePattern */ 0b01, /* success */ true);
}

// Helper: Sets up two NIs and two clients both referencing NI-A, then dirties the
// persisted client counts by poisoning the clean-counts write in
// UpdateClientNetworkIdentityIndex. The in-memory counts in the caller's instance remain
// correct; a fresh instance on the same backing store will load the dirty sentinel.
// Actual state: NI-A has clientX (count=1), NI-B has clientY (count=1).
static void SetUpDirtyClientCounts(TestPersistentStorageDelegate & backingStore, DefaultNetworkIdentityStorage & storage,
                                   uint16_t & outNIIndexA, uint16_t & outNIIndexB, uint16_t & outClientIndexX,
                                   uint16_t & outClientIndexY)
{
    // Two NIs so NI-A is non-current (needed by the tests that check counts on non-current NIs)
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xA1, 100, kTestIdentityA, outNIIndexA));
    ASSERT_SUCCESS(StoreTestNASSAndIdentity(storage, 0xB2, 200, kTestIdentityB, outNIIndexB));

    // Two clients, both referencing NI-A
    ASSERT_SUCCESS(storage.AddClient(kTestClientX, outClientIndexX));
    ASSERT_SUCCESS(storage.AddClient(kTestClientY, outClientIndexY));
    uint16_t unusedOldNI;
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(outClientIndexX, outNIIndexA, unusedOldNI));
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(outClientIndexY, outNIIndexA, unusedOldNI));

    // Poison NI index: dirty-counts write (bit 0) succeeds, clean-counts write (bit 1) fails.
    // This leaves the persisted counts dirty.
    backingStore.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementNetworkIdentityIndex().KeyName(), -1, 0b01);
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(outClientIndexY, outNIIndexB, unusedOldNI));
    backingStore.ClearPoisonKeys();

    // Validate the setup: the in-memory counts should still be correct...
    NetworkIdentityStorage::NetworkIdentityEntry niEntry;
    ASSERT_SUCCESS(
        storage.FindNetworkIdentity(outNIIndexA, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
    ASSERT_EQ(niEntry.clientCount, 1u);

    // ...but a fresh instance should have dirty counts. Verify by poisoning the client
    // detail reads so the rebuild fails — requesting counts should then return an error.
    for (uint16_t ci : { outClientIndexX, outClientIndexY })
    {
        backingStore.AddPoisonKey(DefaultStorageKeyAllocator::NetworkIdentityManagementClient(ci).KeyName());
    }
    {
        DefaultNetworkIdentityStorage dirtyProbe(backingStore);
        ASSERT_NE(
            dirtyProbe.FindNetworkIdentity(outNIIndexA, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()),
            CHIP_NO_ERROR);
    }
    backingStore.ClearPoisonKeys();
}

TEST_F(TestDefaultNetworkIdentityStorage, RemoveClientSuccessWithAlreadyDirtyClientCounts)
{
    TestPersistentStorageDelegate backingStore;
    uint16_t niIndexA, niIndexB, clientIndexX, clientIndexY;

    // Set up dirty counts, then destroy the setup instance so we get a fresh one
    // that loads the dirty sentinel from storage.
    {
        DefaultNetworkIdentityStorage setupStorage(backingStore);
        SetUpDirtyClientCounts(backingStore, setupStorage, niIndexA, niIndexB, clientIndexX, clientIndexY);
    }

    DefaultNetworkIdentityStorage storage(backingStore);

    // Remove clientX (which references NI-A) while counts are dirty
    uint16_t removedNIIndex;
    ASSERT_SUCCESS(storage.RemoveClient(clientIndexX, removedNIIndex));
    EXPECT_EQ(removedNIIndex, niIndexA);

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        NetworkIdentityStorage::NetworkIdentityEntry niEntry;
        ASSERT_SUCCESS(s.FindNetworkIdentity(niIndexA, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
        EXPECT_EQ(niEntry.clientCount, 0u);

        ASSERT_SUCCESS(s.FindNetworkIdentity(niIndexB, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
        EXPECT_EQ(niEntry.clientCount, 1u);
    };

    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}

TEST_F(TestDefaultNetworkIdentityStorage, UpdateClientNIIndexSuccessWithAlreadyDirtyClientCounts)
{
    TestPersistentStorageDelegate backingStore;
    uint16_t niIndexA, niIndexB, clientIndexX, clientIndexY;

    {
        DefaultNetworkIdentityStorage setupStorage(backingStore);
        SetUpDirtyClientCounts(backingStore, setupStorage, niIndexA, niIndexB, clientIndexX, clientIndexY);
    }

    DefaultNetworkIdentityStorage storage(backingStore);

    // Move clientX from NI-A to NI-B while counts are dirty.
    // Expected result: NI-A count=0, NI-B count=2.
    // This is distinct from the pre-dirty state (NI-A=2, NI-B=0), so correct
    // results can't happen by accident from stale data.
    ASSERT_SUCCESS(storage.UpdateClientNetworkIdentityIndex(clientIndexX, niIndexB, unusedOldNI));

    auto verifyState = [&](DefaultNetworkIdentityStorage & s) {
        NetworkIdentityStorage::NetworkIdentityEntry niEntry;
        ASSERT_SUCCESS(s.FindNetworkIdentity(niIndexA, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
        EXPECT_EQ(niEntry.clientCount, 0u);

        ASSERT_SUCCESS(s.FindNetworkIdentity(niIndexB, niEntry, NetworkIdentityFlags::kPopulateClientCount, MutableByteSpan()));
        EXPECT_EQ(niEntry.clientCount, 2u);
    };

    verifyState(storage);
    DefaultNetworkIdentityStorage freshStorage(backingStore);
    verifyState(freshStorage);
}
