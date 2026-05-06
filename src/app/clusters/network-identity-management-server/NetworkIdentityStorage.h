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

#include <clusters/NetworkIdentityManagement/Enums.h>
#include <credentials/CHIPCert.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Span.h>
#include <system/SystemClock.h>

namespace chip::app::Clusters {

/**
 * @brief Read-only storage interface for the Network Identity Management cluster.
 *
 * Provides query access to Client Identities, Network Identities, and the Network Administrator
 * Shared Secret (NASS). The full mutable interface is `NetworkIdentityStorage` below.
 *
 * The primary key for each stored Network Identity and Client Identity are the Network Identity
 * Index and Client Index, respectively. These are 16 bit unsigned integers allocated by the storage
 * implementation from a spec-defined range (1-65534 and 1-2047 respectively), and are exposed
 * externally via the cluster's attributes and commands. Additionally, each Network and Client
 * Identity also has an Identifier; this is a 20 byte CertificateKeyId that is derived from the
 * public key of the underlying identity certificate. The derivation of the Identifier is handled by
 * the cluster code; in terms of the storage interface it is simply a secondary key for the identity.
 *
 * Keypair handles are stored as opaque bytes; the cluster layer handles typed handle conversions.
 */
class ReadOnlyNetworkIdentityStorage
{
public:
    virtual ~ReadOnlyNetworkIdentityStorage() = default;

    /// Maximum size of a serialized keypair handle
    static constexpr size_t kMaxKeypairHandleBytesLength = Crypto::P256KeypairHandle::Capacity();

    /// Maximum number of concurrently active Network Identities (spec-defined).
    static constexpr uint16_t kMaxNetworkIdentities = 4;

    ///// Network Administrator Shared Secret (NASS) Storage //////////////////

    struct NetworkAdministratorSecretInfo
    {
        System::Clock::Seconds32 createdTimestamp;
        Crypto::HkdfKeyHandle secretHandle;
    };

    /**
     * Retrieves meta-data about the current NASS.
     * @retval CHIP_ERROR_NOT_FOUND if no NASS is stored.
     */
    virtual CHIP_ERROR RetrieveNetworkAdministratorSecret(NetworkAdministratorSecretInfo & outSecretInfo) = 0;

    ///// Network Identity Storage ////////////////////////////////////////////

    enum class NetworkIdentityFlags : uint8_t
    {
        kPopulateIdentifier       = (1 << 0),
        kPopulateCreatedTimestamp = (1 << 1),
        kPopulateCompactIdentity  = (1 << 2),
        kPopulateKeypairHandle    = (1 << 3),
        kPopulateClientCount      = (1 << 4),

        kPopulateAll = kPopulateIdentifier | kPopulateCreatedTimestamp | kPopulateCompactIdentity | kPopulateKeypairHandle |
            kPopulateClientCount
    };

    static constexpr size_t NetworkIdentityBufferSize(BitFlags<NetworkIdentityFlags> flags = NetworkIdentityFlags::kPopulateAll)
    {
        return //
            (flags.Has(NetworkIdentityFlags::kPopulateCompactIdentity) ? Credentials::kMaxCHIPCompactNetworkIdentityLength : 0) +
            (flags.Has(NetworkIdentityFlags::kPopulateKeypairHandle) ? kMaxKeypairHandleBytesLength : 0);
    }

    /// Input data for storing a Network Identity
    struct NetworkIdentityInfo
    {
        NetworkIdentityManagement::IdentityTypeEnum type;
        Credentials::CertificateKeyIdStorage identifier;
        ByteSpan compactIdentity;
        ByteSpan keypairHandle; ///< Opaque bytes at storage layer

        CHIP_ERROR GetKeypairHandle(Crypto::P256KeypairHandle & outHandle);
    };

    /// Output data when retrieving a Network Identity.
    struct NetworkIdentityEntry : public NetworkIdentityInfo
    {
        uint16_t index;                            ///< Network Identity Index, always retrieved
        bool current;                              ///< true if this is the current identity for its type, always retrieved
        System::Clock::Seconds32 createdTimestamp; ///< Retrieved only if kPopulateCreatedTimestamp
        uint16_t clientCount; ///< Number of clients referencing this entry, retrieved only if kPopulateClientCount
    };

    using NetworkIdentityIterator = CommonIterator<NetworkIdentityEntry>;

    /**
     * Creates an iterator over all stored Network Identities.
     *
     * Release() must be called on the iterator after iteration is complete;
     * the use of the nested AutoReleasing wrapper is recommended.
     *
     * Adding or removing Network Identities during iteration is not supported.
     *
     * The provided flags control what data is populated in each NetworkIdentityEntry;
     * fields that have no corresponding flag are populated unconditionally.
     *
     * Retrieving the compactIdentity or keypairHandle fields requires the caller to provide a
     * buffer, which can be statically sized using NetworkIdentityBufferSize(). ByteSpans in
     * the loaded entry will point into this buffer, which will be re-used on each Next() call.
     *
     * Note: If the kPopulateKeypairHandle flag is specified, callers should wipe the buffer
     * after use, as (depending on the platform) keypair handles may contain raw key material.
     *
     * @param flags Controls which optional fields are populated.
     * @param buffer Persistence buffer for variable-length / ByteSpan fields.
     *
     * @retval A NetworkIdentityIterator instance on success.
     * @retval nullptr if no iterator instances are available or buffer is too small.
     */
    virtual NetworkIdentityIterator * IterateNetworkIdentities(BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer) = 0;

    /**
     * Retrieves the Network Identity with the specified index.
     * @retval CHIP_ERROR_NOT_FOUND if not found.
     * @see IterateNetworkIdentities() for flags / buffer semantics.
     */
    virtual CHIP_ERROR FindNetworkIdentity(uint16_t index, NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                           MutableByteSpan buffer) = 0;

    /**
     * Retrieves the Network Identity with the specified identifier.
     * @retval CHIP_ERROR_NOT_FOUND if not found.
     * @see IterateNetworkIdentities() for flags / buffer semantics.
     */
    virtual CHIP_ERROR FindNetworkIdentity(const Credentials::CertificateKeyId & identifier, NetworkIdentityEntry & outEntry,
                                           BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer) = 0;

    /**
     * Retrieves the current (i.e. most recently added) Network Identity for the specified type.
     * @retval CHIP_ERROR_NOT_FOUND if no Network Identities of the given type are stored.
     * @see IterateNetworkIdentities() for flags / buffer semantics.
     */
    virtual CHIP_ERROR FindCurrentNetworkIdentity(NetworkIdentityManagement::IdentityTypeEnum type, NetworkIdentityEntry & outEntry,
                                                  BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer) = 0;

    ///// Client Storage /////////////////////////////////////////////////////////

    /// Value of networkIdentityIndex when the client has never authenticated.
    /// Distinct from 0, which is the default for unpopulated fields (when kPopulateNetworkIdentityIndex is not set).
    static constexpr uint16_t kNullNetworkIdentityIndex = UINT16_MAX;

    enum class ClientFlags : uint8_t
    {
        kPopulateIdentifier           = (1 << 0),
        kPopulateCompactIdentity      = (1 << 1),
        kPopulateNetworkIdentityIndex = (1 << 2),

        kPopulateAll = kPopulateIdentifier | kPopulateCompactIdentity | kPopulateNetworkIdentityIndex
    };

    static constexpr size_t ClientBufferSize(BitFlags<ClientFlags> flags = ClientFlags::kPopulateAll)
    {
        return //
            (flags.Has(ClientFlags::kPopulateCompactIdentity) ? Credentials::kMaxCHIPCompactNetworkIdentityLength : 0);
    }

    struct ClientInfo
    {
        Credentials::CertificateKeyIdStorage identifier; ///< 20-byte client identifier, retrieved only if kPopulateIdentifier
        ByteSpan compactIdentity; ///< Client Identity certificate, retrieved only if kPopulateCompactIdentity
    };

    struct ClientEntry : public ClientInfo
    {
        uint16_t index;                ///< always populated
        uint16_t networkIdentityIndex; ///< retrieved only if kPopulateNetworkIdentityIndex
    };

    using ClientIterator = CommonIterator<ClientEntry>;

    /**
     * Returns the maximum number of clients the table can hold.
     */
    virtual uint16_t GetClientTableCapacity() const = 0;

    /**
     * Creates an iterator over all stored clients.
     *
     * Release() must be called on the iterator after iteration is complete;
     * the use of the nested AutoReleasing wrapper is recommended.
     *
     * @param flags  Controls which optional fields are populated in each ClientEntry.
     * @param buffer Buffer for variable-length fields; can be sized via ClientBufferSize().
     *
     * @retval A ClientIterator instance on success.
     * @retval nullptr if no iterator instances are available or buffer is too small.
     */
    virtual ClientIterator * IterateClients(BitFlags<ClientFlags> flags, MutableByteSpan buffer) = 0;

    /**
     * Retrieves the client with the specified index.
     * @retval CHIP_ERROR_NOT_FOUND if not found.
     * @see IterateClients() for flags / buffer semantics.
     */
    virtual CHIP_ERROR FindClient(uint16_t index, ClientEntry & outEntry, BitFlags<ClientFlags> flags, MutableByteSpan buffer) = 0;

    /**
     * Retrieves the client with the specified identifier.
     * @retval CHIP_ERROR_NOT_FOUND if not found.
     * @see IterateClients() for flags / buffer semantics.
     */
    virtual CHIP_ERROR FindClient(const Credentials::CertificateKeyId & identifier, ClientEntry & outEntry,
                                  BitFlags<ClientFlags> flags, MutableByteSpan buffer) = 0;
};

/**
 * @brief Full (mutable) storage interface for the Network Identity Management cluster.
 */
class NetworkIdentityStorage : public ReadOnlyNetworkIdentityStorage
{
public:
    /**
     * Removes any stored NASS.
     * @retval CHIP_ERROR_NOT_FOUND if no NASS is stored.
     */
    virtual CHIP_ERROR RemoveNetworkAdministratorSecret() = 0;

    /**
     * Atomically stores a new NASS and the Network Identities derived from it.
     *
     * This composite operation replaces any existing NASS and adds one or more new
     * Network Identities, which must have distinct types. Each of the new identities
     * will become the current identity for its type. The NASS createdTimestamp is
     * used as the createdTimestamp for the derived identities.
     *
     * @param nassInfo      The NASS to store (replaces any existing NASS).
     * @param identities    The derived Network Identities to store.
     * @param outIndices    Output span for the assigned indices (must be same size as identities).
     *
     * On failure, the storage state is unchanged (internal rollback).
     * @retval CHIP_ERROR_NO_MEMORY if the identity table does not have enough room.
     */
    virtual CHIP_ERROR StoreNetworkAdministratorSecretAndDerivedIdentities(const NetworkAdministratorSecretInfo & nassInfo,
                                                                           Span<const NetworkIdentityInfo * const> identities,
                                                                           Span<uint16_t> outIndices) = 0;

    /**
     * Convenience overload that populates a full NetworkIdentityEntry for each identity on output.
     */
    CHIP_ERROR StoreNetworkAdministratorSecretAndDerivedIdentities(const NetworkAdministratorSecretInfo & nassInfo,
                                                                   Span<NetworkIdentityEntry * const> identities);

    /**
     * Removes a non-current Network Identity that has no clients referencing it.
     *
     * @param index The index of the Network Identity to remove.
     *
     * @retval CHIP_ERROR_NOT_FOUND if no Network Identity with the given index exists.
     * @retval CHIP_ERROR_INCORRECT_STATE if the entry has clientCount > 0.
     */
    virtual CHIP_ERROR RemoveNetworkIdentity(uint16_t index) = 0;

    /**
     * Adds a new client.
     *
     * The caller is responsible for checking for identifier collisions before
     * calling this method (i.e. ensuring no existing client has the same identifier).
     *
     * @param clientInfo     The client identifier and identity to store.
     * @param outClientIndex The index assigned to the client.
     *
     * @retval CHIP_ERROR_NO_MEMORY if the client table is full.
     */
    virtual CHIP_ERROR AddClient(const ClientInfo & clientInfo, uint16_t & outClientIndex) = 0;

    /**
     * Convenience overload that populates a full ClientEntry on output.
     */
    CHIP_ERROR AddClient(ClientEntry & entry);

    /**
     * Removes a client.
     *
     * If the client references a Network Identity, the client count of that Network Identity is decremented.
     *
     * @param clientIndex             The index of the client to remove.
     * @param outNetworkIdentityIndex On success, set to the NetworkIdentityIndex the removed client
     *                                referenced (or kNullNetworkIdentityIndex if none).
     *
     * @retval CHIP_ERROR_NOT_FOUND if not found.
     */
    virtual CHIP_ERROR RemoveClient(uint16_t clientIndex, uint16_t & outNetworkIdentityIndex) = 0;

    /**
     * Updates the Network Identity referenced by a client.
     *
     * The client counts of the old and new Network Identities are updated as needed.
     * Pass kNullNetworkIdentityIndex to clear the reference.
     *
     * @param clientIndex                 The index of the client to update.
     * @param newNetworkIdentityIndex     The index of the Network Identity the client authenticated against.
     * @param outOldNetworkIdentityIndex  On success, set to the previous Network Identity Index,
     *                                    or kNullNetworkIdentityIndex if none.
     *
     * @retval CHIP_ERROR_NOT_FOUND if the client or Network Identity were not found.
     */
    virtual CHIP_ERROR UpdateClientNetworkIdentityIndex(uint16_t clientIndex, uint16_t newNetworkIdentityIndex,
                                                        uint16_t & outOldNetworkIdentityIndex) = 0;

protected:
    // Helpers for wrap-around logic on Index values. The concrete ranges are
    // defined in the Matter specification and avoid the value 0 and 0xFFFF
    // because the former looks like a "null" value and the latter is reserved
    // in the data model for nullable uint16 values.

    template <typename T, T Min, T Max>
    struct IndexRange
    {
        using UnderlyingType = T;

        static constexpr T kMin = Min;
        static constexpr T kMax = Max;
        static_assert(kMin <= kMax);
        static constexpr T kCardinality = kMax - kMin + 1;

        static constexpr T Clamp(T value) { return (kMin <= value && value <= kMax) ? value : kMin; }
        static constexpr T Increment(T value) { return Clamp(static_cast<T>(value + 1)); }
    };

    using NetworkIdentityIndexRange = IndexRange<uint16_t, 1, 0xFFFE>;
    using ClientIndexRange          = IndexRange<uint16_t, 1, 2047>;
    static_assert(kNullNetworkIdentityIndex > NetworkIdentityIndexRange::kMax);
    static_assert(kMaxNetworkIdentities <= NetworkIdentityIndexRange::kCardinality);

    // Iteration-based default implementations of Find* methods for Network Identities.
    // Sub-classes can delegate to these as needed if they do not have more efficient implementations.
    // Due to its potentially large size (min capacity 500), iteration-based find implementations are
    // generally inappropriate for the Client Table, and no such default methods are provided.

    CHIP_ERROR FindNetworkIdentityByIterating(uint16_t index, NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                              MutableByteSpan buffer);

    CHIP_ERROR FindNetworkIdentityByIterating(const Credentials::CertificateKeyId & identifier, NetworkIdentityEntry & outEntry,
                                              BitFlags<NetworkIdentityFlags> flags, MutableByteSpan buffer);

    CHIP_ERROR FindCurrentNetworkIdentityByIterating(NetworkIdentityManagement::IdentityTypeEnum type,
                                                     NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                                     MutableByteSpan buffer);

private:
    CHIP_ERROR FindNetworkIdentityByIterating(NetworkIdentityEntry & outEntry, BitFlags<NetworkIdentityFlags> flags,
                                              MutableByteSpan buffer, bool (*predicate)(const NetworkIdentityEntry &, const void *),
                                              const void * context);
};

} // namespace chip::app::Clusters
