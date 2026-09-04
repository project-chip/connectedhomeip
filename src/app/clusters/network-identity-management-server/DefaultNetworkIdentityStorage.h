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

#pragma once

#include <app/clusters/network-identity-management-server/NetworkIdentityStorage.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Pool.h>

namespace chip::app::Clusters {

/**
 * NetworkIdentityStorage implementation that persists data via a PersistentStorageDelegate.
 * Supports a single cluster instance (no per-endpoint key namespacing).
 */
class DefaultNetworkIdentityStorage : public NetworkIdentityStorage
{
public:
    /**
     * @param storage              Persistent storage backend for all records.
     * @param clientTableCapacity  Maximum number of client table entries (spec range: 500..2047).
     *                             Clamped to the ClientIndex range capacity (2047).
     */
    DefaultNetworkIdentityStorage(PersistentStorageDelegate & storage, uint16_t maxClients = 500);

    CHIP_ERROR RetrieveNetworkAdministratorSecret(NetworkAdministratorSecretInfo & outSecretInfo) override;
    CHIP_ERROR RemoveNetworkAdministratorSecret() override;

    NetworkIdentityIterator * IterateNetworkIdentities(BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer) override;

    CHIP_ERROR FindNetworkIdentity(uint16_t index, NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                   MutableByteSpan buffer) override;
    CHIP_ERROR FindNetworkIdentity(const Credentials::CertificateKeyId & identifier, NetworkIdentityEntry & outEntry,
                                   BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer) override;
    CHIP_ERROR FindCurrentNetworkIdentity(NetworkIdentityManagement::IdentityTypeEnum type, NetworkIdentityEntry & outEntry,
                                          BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer) override;

    CHIP_ERROR StoreNetworkAdministratorSecretAndDerivedIdentities(const NetworkAdministratorSecretInfo & nassInfo,
                                                                   Span<const NetworkIdentityInfo * const> identities,
                                                                   Span<uint16_t> outIndices) override;
    using NetworkIdentityStorage::StoreNetworkAdministratorSecretAndDerivedIdentities; // unhide convenience overloads

    CHIP_ERROR RemoveNetworkIdentity(uint16_t index) override;

    uint16_t GetClientTableCapacity() const override { return mMaxClients; }
    ClientIterator * IterateClients(BitFlags<ClientFlags> flags, MutableByteSpan buffer) override;
    CHIP_ERROR FindClient(uint16_t index, ClientEntry & outEntry, BitFlags<ClientFlags> flags, MutableByteSpan buffer) override;
    CHIP_ERROR FindClient(const Credentials::CertificateKeyId & identifier, ClientEntry & outEntry, BitFlags<ClientFlags> flags,
                          MutableByteSpan buffer) override;
    CHIP_ERROR AddClient(const ClientInfo & info, uint16_t & outClientIndex) override;
    using NetworkIdentityStorage::AddClient; // unhide convenience overload
    CHIP_ERROR RemoveClient(uint16_t clientIndex, uint16_t & outNetworkIdentityIndex) override;
    CHIP_ERROR UpdateClientNetworkIdentityIndex(uint16_t clientIndex, uint16_t newNetworkIdentityIndex,
                                                uint16_t & outOldNetworkIdentityIndex) override;

private:
    struct TLVConstants; // defined in .cpp, can access private members of this type

    class NetworkIdentityIteratorImpl : public NetworkIdentityIterator
    {
    public:
        NetworkIdentityIteratorImpl(DefaultNetworkIdentityStorage & storage, BitFlags<NetworkIdentityFlags> flags,
                                    MutableByteSpan buffer);
        size_t Count() override;
        bool Next(NetworkIdentityEntry & outEntry) override;
        void Release() override;

    private:
        DefaultNetworkIdentityStorage & mStorage;
        BitFlags<NetworkIdentityFlags> mFlags;
        MutableByteSpan mBuffer;
        size_t mNext = 0; // offset into mStorage.mNITableIndex
    };

    struct NetworkIdentityIndexEntry
    {
        uint16_t index;
        NetworkIdentityManagement::IdentityTypeEnum type;
        uint16_t clientCount;
    };

    // Sentinel value indicating NetworkIdentityIndexEntry.clientCount needs to be recalculated
    static constexpr uint16_t kClientCountDirty = UINT16_MAX;
    static_assert(kClientCountDirty > ClientIndexRange::kCardinality);

    class ClientIteratorImpl : public ClientIterator
    {
    public:
        ClientIteratorImpl(DefaultNetworkIdentityStorage & storage, BitFlags<ClientFlags> flags, MutableByteSpan buffer);
        size_t Count() override;
        bool Next(ClientEntry & outEntry) override;
        void Release() override;

    private:
        DefaultNetworkIdentityStorage & mStorage;
        BitFlags<ClientFlags> mFlags;
        MutableByteSpan mBuffer;
        uint16_t mNextIndex = ClientIndexRange::kMin; // next candidate Client Index
    };

    struct StoreNITableIndexOptions
    {
        uint16_t indexToSkip        = 0;
        uint16_t pendingImportCount = 0;
        System::Clock::Seconds32 pendingImportTimestamp{};
        bool dirtyClientCounts = false;
    };

    CHIP_ERROR StoreNASS(const NetworkAdministratorSecretInfo & secretInfo);
    CHIP_ERROR LoadNASS(NetworkAdministratorSecretInfo & outSecretInfo);

    uint16_t AllocateNIIndex();
    CHIP_ERROR StoreNITableIndex(const StoreNITableIndexOptions & options);
    void LoadNITableIndexIfNeeded();
    CHIP_ERROR RebuildNIClientCounts();
    NetworkIdentityIndexEntry * FindNIIndexEntry(uint16_t index);
    NetworkIdentityIndexEntry * FindCurrentNIIndexEntry(NetworkIdentityManagement::IdentityTypeEnum type);
    NetworkIdentityIndexEntry * FindClientNIIndexEntry(ClientEntry const & client);

    CHIP_ERROR StoreNetworkIdentityDetail(uint16_t index, const NetworkIdentityInfo & info,
                                          System::Clock::Seconds32 createdTimestamp);
    CHIP_ERROR LoadNetworkIdentity(const NetworkIdentityIndexEntry & meta, NetworkIdentityEntry & outEntry,
                                   BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer);
    void DeleteNetworkIdentityDetail(uint16_t index, bool ignoreNotFound = false);

    bool IsClientIndexAllocated(uint16_t index) const;
    void SetClientIndexAllocated(uint16_t index, bool allocated);
    uint16_t AllocateClientIndex();

    CHIP_ERROR StoreClientTableIndex();
    void LoadClientTableIndexIfNeeded();

    CHIP_ERROR StoreClientDetail(uint16_t index, const ClientInfo & info, uint16_t networkIdentityIndex);
    CHIP_ERROR LoadClient(uint16_t index, ClientEntry & outEntry, BitFlags<ClientFlags> flags, MutableByteSpan buffer);
    void DeleteClientDetail(uint16_t index);

    CHIP_ERROR StoreClientIdentifierMapping(const Credentials::CertificateKeyId & identifier, uint16_t clientIndex);
    CHIP_ERROR LookupClientIdentifierMapping(const Credentials::CertificateKeyId & identifier, uint16_t & outClientIndex);
    void DeleteClientIdentifierMapping(const Credentials::CertificateKeyId & identifier);

    // Sentinel value for mNINextIndex / mClientNextIndex
    static constexpr uint16_t kTableIndexNotLoaded = 0;
    static_assert(kTableIndexNotLoaded < NetworkIdentityIndexRange::kMin);
    static_assert(kTableIndexNotLoaded < ClientIndexRange::kMin);

    PersistentStorageDelegate & mStorage;
    const uint16_t mMaxClients;

    ObjectPool<NetworkIdentityIteratorImpl, 2> mNIIterators; // 1 for the cluster plus one for other clients
    ObjectPool<ClientIteratorImpl, 2> mClientIterators;      // ditto

    // Network Identity table metadata

    uint16_t mNINextIndex = kTableIndexNotLoaded;
    uint16_t mNICount;
    NetworkIdentityIndexEntry mNITableIndex[kMaxNetworkIdentities];

    // Client Identity table metadata

    uint16_t mClientNextIndex = kTableIndexNotLoaded;
    uint16_t mClientCount;

    using BitmapWord                        = uint32_t;
    static constexpr size_t kBitmapWordBits = std::numeric_limits<BitmapWord>::digits;
    // Little endian bitmap, directly indexed by client index (bit 0 is unused).
    // The bit corresponding to kMax lives in the word at offset (kMax / WordBits).
    BitmapWord mClientAllocBitmap[(ClientIndexRange::kMax / kBitmapWordBits) + 1];
};

} // namespace chip::app::Clusters
