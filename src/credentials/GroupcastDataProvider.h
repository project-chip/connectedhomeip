/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <credentials/FabricTable.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/SessionKeystore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Pool.h>
#include <lib/support/Span.h>
#include <string.h>

namespace chip {
namespace Groupcast {

constexpr size_t kEndpointsMax         = 8;
constexpr size_t kGroupKeyCount       = 2;
constexpr size_t kIteratorMax          = 3;
static constexpr size_t kKeyContextMax = CHIP_CONFIG_MAX_GROUP_CONCURRENT_ITERATORS;

struct Group
{
    Group() = default;
    Group(GroupId id) : group_id(id) {}

    Group & operator=(const Group & t)
    {
        this->group_id       = t.group_id;
        this->endpoint_count = t.endpoint_count;
        memcpy(this->endpoints, t.endpoints, this->endpoint_count * sizeof(EndpointId));
        return *this;
    }

    GroupId group_id        = kUndefinedGroupId;
    uint16_t endpoint_count = 0;
    EndpointId endpoints[kEndpointsMax];
};

struct GroupKey
{
    Crypto::Aes128KeyHandle encryption;
    Crypto::Aes128KeyHandle privacy;
    uint64_t expiration = 0;

    GroupKey & operator=(const GroupKey & t)
    {
        memcpy(this->encryption.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
               t.encryption.As<Crypto::Symmetric128BitsKeyByteArray>(), sizeof(Crypto::Symmetric128BitsKeyByteArray));
        memcpy(this->privacy.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
               t.encryption.As<Crypto::Symmetric128BitsKeyByteArray>(), sizeof(Crypto::Symmetric128BitsKeyByteArray));
        this->expiration = t.expiration;
        return *this;
    }

    bool IsActive();
};

struct GroupEntry : Group
{
    GroupEntry() = default;
    GroupEntry(GroupId id) : Group(id) {}

    bool operator==(const GroupEntry & other) const;
    GroupEntry & operator=(const GroupEntry & t);
    void InvalidateKeys();
    void SetActiveKey(const GroupKey & key, uint32_t period);
    CHIP_ERROR GetActiveKey(GroupKey & key);
    GroupKey keys[kGroupKeyCount];
};

struct DataProvider
{
    struct KeyContext : chip::Crypto::SymmetricKeyContext
    {
        KeyContext(DataProvider & provider, const GroupKey & key);

        uint16_t GetKeyHash() override;
        CHIP_ERROR MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce, MutableByteSpan & mic,
                                  MutableByteSpan & ciphertext) const override;
        CHIP_ERROR MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce, const ByteSpan & mic,
                                  MutableByteSpan & plaintext) const override;
        CHIP_ERROR PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        CHIP_ERROR PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        void Release() override;

    private:
        DataProvider & mProvider;
        Crypto::Aes128KeyHandle mEncryptionKey;
        Crypto::Aes128KeyHandle mPrivacyKey;
    };

    struct KeyContextIterator : CommonIterator<Crypto::SymmetricKeyContext *&>
    {
        KeyContextIterator(DataProvider & provider, FabricIndex fabric, GroupId group_id);
        size_t Count() override;
        bool Next(Crypto::SymmetricKeyContext *& output) override;
        void Release() override;

    private:
        DataProvider & mProvider;
        GroupEntry mEntry;
        size_t mIndex = 0;
    };

    struct GroupIterator : CommonIterator<Group &>
    {
        GroupIterator(DataProvider & provider, FabricIndex fabric);
        size_t Count() override;
        bool Next(Group &) override;
        void Release() override;

    private:
        DataProvider & mProvider;
        FabricIndex mFabric;
        size_t mIndex = 0;
        size_t mCount = 0;
    };

    DataProvider() = default;

    static DataProvider & Instance();
    CHIP_ERROR Initialize(PersistentStorageDelegate * storage, chip::Crypto::SessionKeystore * keystore);
    uint8_t GetMaxMembershipCount();
    CHIP_ERROR JoinGroup(chip::FabricIndex fabric_idx, const ByteSpan & compressed_fabric_id,
                                   Group & target, const ByteSpan & key, uint32_t period);
    CHIP_ERROR GetGroup(FabricIndex fabric_idx, Group & target);
    chip::Crypto::SymmetricKeyContext * CreateKeyContext(FabricIndex fabric, GroupId groupId);
    KeyContextIterator * IterateKeyContexts(FabricIndex fabric, GroupId group_id);
    GroupIterator * IterateGroups(FabricIndex fabric);

private:
    PersistentStorageDelegate * mStorage      = nullptr;
    chip::Crypto::SessionKeystore * mKeystore = nullptr;
    ObjectPool<KeyContext, kKeyContextMax> mKeyContextPool;
    ObjectPool<KeyContextIterator, kIteratorMax> mKeyContextIteratorPool;
    ObjectPool<GroupIterator, kIteratorMax> mGroupIteratorPool;
};

} // namespace Groupcast
} // namespace chip
