/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <credentials/GroupDataProvider.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Pool.h>

namespace chip {
namespace Credentials {

class GroupDataProviderImpl : public GroupDataProvider
{
public:
    static constexpr size_t kIteratorsMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

    GroupDataProviderImpl() = default;
    GroupDataProviderImpl(uint16_t maxGroupsPerFabric, uint16_t maxGroupKeysPerFabric) :
        GroupDataProvider(maxGroupsPerFabric, maxGroupKeysPerFabric)
    {}
    ~GroupDataProviderImpl() override {}

    /**
     * @brief Set the storage implementation used for non-volatile storage of configuration data.
     *        This method MUST be called before Init().
     *
     * @param storage Pointer to storage instance to set. Cannot be nullptr, will assert.
     */
    void SetStorageDelegate(PersistentStorageDelegate * storage);

    void SetSessionKeystore(Crypto::SessionKeystore * keystore) { mSessionKeystore = keystore; }
    Crypto::SessionKeystore * GetSessionKeystore() const { return mSessionKeystore; }

    CHIP_ERROR Init() override;
    void Finish() override;

    //
    // Group Info
    //

    // By id
    CHIP_ERROR SetGroupInfo(FabricIndex fabric_index, const GroupInfo & info) override;
    CHIP_ERROR GetGroupInfo(FabricIndex fabric_index, GroupId group_id, GroupInfo & info) override;
    CHIP_ERROR RemoveGroupInfo(FabricIndex fabric_index, GroupId group_id) override;
    // By index
    CHIP_ERROR SetGroupInfoAt(FabricIndex fabric_index, size_t index, const GroupInfo & info) override;
    CHIP_ERROR GetGroupInfoAt(FabricIndex fabric_index, size_t index, GroupInfo & info) override;
    CHIP_ERROR RemoveGroupInfoAt(FabricIndex fabric_index, size_t index) override;
    // Endpoints
    bool HasEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id) override;
    CHIP_ERROR AddEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id) override;
    CHIP_ERROR RemoveEndpoint(FabricIndex fabric_index, GroupId group_id, EndpointId endpoint_id) override;
    CHIP_ERROR RemoveEndpoint(FabricIndex fabric_index, EndpointId endpoint_id) override;
    // Iterators
    GroupInfoIterator * IterateGroupInfo(FabricIndex fabric_index) override;
    EndpointIterator * IterateEndpoints(FabricIndex fabric_index, Optional<GroupId> group_id = NullOptional) override;

    //
    // Group-Key map
    //

    CHIP_ERROR SetGroupKeyAt(FabricIndex fabric_index, size_t index, const GroupKey & info) override;
    CHIP_ERROR GetGroupKeyAt(FabricIndex fabric_index, size_t index, GroupKey & info) override;
    CHIP_ERROR RemoveGroupKeyAt(FabricIndex fabric_index, size_t index) override;
    CHIP_ERROR RemoveGroupKeys(FabricIndex fabric_index) override;
    GroupKeyIterator * IterateGroupKeys(FabricIndex fabric_index) override;

    //
    // Key Sets
    //

    CHIP_ERROR SetKeySet(FabricIndex fabric_index, const ByteSpan & compressed_fabric_id, const KeySet & keys) override;
    CHIP_ERROR GetKeySet(FabricIndex fabric_index, chip::KeysetId keyset_id, KeySet & keys) override;
    CHIP_ERROR RemoveKeySet(FabricIndex fabric_index, chip::KeysetId keyset_id) override;
    CHIP_ERROR GetIpkKeySet(FabricIndex fabric_index, KeySet & out_keyset) override;
    KeySetIterator * IterateKeySets(FabricIndex fabric_index) override;

    // Fabrics
    CHIP_ERROR RemoveFabric(FabricIndex fabric_index) override;

    // Decryption
    Crypto::SymmetricKeyContext * GetKeyContext(FabricIndex fabric_index, GroupId group_id) override;
    GroupSessionIterator * IterateGroupSessions(uint16_t session_id) override;

protected:
    class GroupInfoIteratorImpl : public GroupInfoIterator
    {
    public:
        GroupInfoIteratorImpl(GroupDataProviderImpl & provider, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(GroupInfo & output) override;
        void Release() override;

    protected:
        GroupDataProviderImpl & mProvider;
        FabricIndex mFabric = kUndefinedFabricIndex;
        uint16_t mNextId    = 0;
        size_t mCount       = 0;
        size_t mTotal       = 0;
    };

    class GroupKeyIteratorImpl : public GroupKeyIterator
    {
    public:
        GroupKeyIteratorImpl(GroupDataProviderImpl & provider, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(GroupKey & output) override;
        void Release() override;

    protected:
        GroupDataProviderImpl & mProvider;
        FabricIndex mFabric = kUndefinedFabricIndex;
        uint16_t mNextId    = 0;
        size_t mCount       = 0;
        size_t mTotal       = 0;
    };

    class EndpointIteratorImpl : public EndpointIterator
    {
    public:
        EndpointIteratorImpl(GroupDataProviderImpl & provider, FabricIndex fabric_index, Optional<GroupId> group_id);
        size_t Count() override;
        bool Next(GroupEndpoint & output) override;
        void Release() override;

    protected:
        GroupDataProviderImpl & mProvider;
        FabricIndex mFabric   = kUndefinedFabricIndex;
        GroupId mFirstGroup   = kUndefinedGroupId;
        uint16_t mGroup       = 0;
        size_t mGroupIndex    = 0;
        size_t mGroupCount    = 0;
        uint16_t mEndpoint    = 0;
        size_t mEndpointIndex = 0;
        size_t mEndpointCount = 0;
        bool mFirstEndpoint   = true;
    };

    class GroupKeyContext : public Crypto::SymmetricKeyContext
    {
    public:
        GroupKeyContext(GroupDataProviderImpl & provider) : mProvider(provider) {}

        GroupKeyContext(GroupDataProviderImpl & provider, const Crypto::Aes128KeyByteArray & encryptionKey, uint16_t hash,
                        const Crypto::Aes128KeyByteArray & privacyKey) :
            mProvider(provider)

        {
            Initialize(encryptionKey, hash, privacyKey);
        }

        void Initialize(const Crypto::Aes128KeyByteArray & encryptionKey, uint16_t hash,
                        const Crypto::Aes128KeyByteArray & privacyKey)
        {
            ReleaseKeys();
            mKeyHash = hash;
            // TODO: Load group keys to the session keystore upon loading from persistent storage
            //
            // Group keys should be transformed into a key handle as soon as possible or even
            // the key storage should be taken over by SessionKeystore interface, but this looks
            // like more work, so let's use the transitional code below for now.

            Crypto::SessionKeystore * keystore = mProvider.GetSessionKeystore();
            keystore->CreateKey(encryptionKey, mEncryptionKey);
            keystore->CreateKey(privacyKey, mPrivacyKey);
        }

        void ReleaseKeys()
        {
            Crypto::SessionKeystore * keystore = mProvider.GetSessionKeystore();
            keystore->DestroyKey(mEncryptionKey);
            keystore->DestroyKey(mPrivacyKey);
        }

        uint16_t GetKeyHash() override { return mKeyHash; }

        CHIP_ERROR MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce, MutableByteSpan & mic,
                                  MutableByteSpan & ciphertext) const override;
        CHIP_ERROR MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce, const ByteSpan & mic,
                                  MutableByteSpan & plaintext) const override;
        CHIP_ERROR PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        CHIP_ERROR PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;

        void Release() override;

    protected:
        GroupDataProviderImpl & mProvider;
        uint16_t mKeyHash = 0;
        Crypto::Aes128KeyHandle mEncryptionKey;
        Crypto::Aes128KeyHandle mPrivacyKey;
    };

    class KeySetIteratorImpl : public KeySetIterator
    {
    public:
        KeySetIteratorImpl(GroupDataProviderImpl & provider, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(KeySet & output) override;
        void Release() override;

    protected:
        GroupDataProviderImpl & mProvider;
        FabricIndex mFabric = kUndefinedFabricIndex;
        uint16_t mNextId    = 0;
        size_t mCount       = 0;
        size_t mTotal       = 0;
    };

    class GroupSessionIteratorImpl : public GroupSessionIterator
    {
    public:
        GroupSessionIteratorImpl(GroupDataProviderImpl & provider, uint16_t session_id);
        size_t Count() override;
        bool Next(GroupSession & output) override;
        void Release() override;

    protected:
        GroupDataProviderImpl & mProvider;
        uint16_t mSessionId      = 0;
        FabricIndex mFirstFabric = kUndefinedFabricIndex;
        FabricIndex mFabric      = kUndefinedFabricIndex;
        uint16_t mFabricCount    = 0;
        uint16_t mFabricTotal    = 0;
        uint16_t mMapping        = 0;
        uint16_t mMapCount       = 0;
        uint16_t mKeyIndex       = 0;
        uint16_t mKeyCount       = 0;
        bool mFirstMap           = true;
        GroupKeyContext mGroupKeyContext;
    };
    bool IsInitialized() { return (mStorage != nullptr); }
    CHIP_ERROR RemoveEndpoints(FabricIndex fabric_index, GroupId group_id);

    PersistentStorageDelegate * mStorage       = nullptr;
    Crypto::SessionKeystore * mSessionKeystore = nullptr;
    ObjectPool<GroupInfoIteratorImpl, kIteratorsMax> mGroupInfoIterators;
    ObjectPool<GroupKeyIteratorImpl, kIteratorsMax> mGroupKeyIterators;
    ObjectPool<EndpointIteratorImpl, kIteratorsMax> mEndpointIterators;
    ObjectPool<KeySetIteratorImpl, kIteratorsMax> mKeySetIterators;
    ObjectPool<GroupSessionIteratorImpl, kIteratorsMax> mGroupSessionsIterator;
    ObjectPool<GroupKeyContext, kIteratorsMax> mGroupKeyContexPool;
};

} // namespace Credentials
} // namespace chip
