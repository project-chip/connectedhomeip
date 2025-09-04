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

#include "GroupcastDataProvider.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/PersistentArray.h>
#include <lib/support/logging/CHIPLogging.h>
#include <system/SystemClock.h>

using namespace chip;

namespace chip {
namespace Groupcast {

constexpr size_t kPersistentBufferMax = 256;

static constexpr TLV::Tag TargetIdTag()
{
    return TLV::ContextTag(1);
}
static constexpr TLV::Tag EndpointCountTag()
{
    return TLV::ContextTag(2);
}
static constexpr TLV::Tag EndpointListTag()
{
    return TLV::ContextTag(3);
}
static constexpr TLV::Tag EndpointEntryTag()
{
    return TLV::ContextTag(4);
}
static constexpr TLV::Tag KeyListTag()
{
    return TLV::ContextTag(5);
}
static constexpr TLV::Tag EncryptKeyTag()
{
    return TLV::ContextTag(6);
}
static constexpr TLV::Tag PrivacyKeyTag()
{
    return TLV::ContextTag(7);
}
static constexpr TLV::Tag ExpirationTag()
{
    return TLV::ContextTag(8);
}

bool GroupKey::IsActive()
{
    uint64_t now = System::SystemClock().GetMonotonicTimestamp().count();
    return (0 == this->expiration) || (now < this->expiration);
}

bool GroupEntry::operator==(const GroupEntry & other) const
{
    return this->group_id == other.group_id;
}

GroupEntry & GroupEntry::operator=(const GroupEntry & t)
{
    this->group_id       = t.group_id;
    this->endpoint_count = t.endpoint_count;
    memcpy(this->endpoints, t.endpoints, this->endpoint_count * sizeof(EndpointId));
    for (size_t i = 0; i < kGroupKeyCount; ++i)
    {
        this->keys[i] = t.keys[i];
    }
    return *this;
}

void GroupEntry::InvalidateKeys()
{
    memset((void *) this->keys, 0x00, kGroupKeyCount * sizeof(GroupKey));
    keys[0].expiration = keys[1].expiration = 1;
}

void GroupEntry::SetActiveKey(const GroupKey & key, uint32_t period)
{
    size_t active_index = (0 == keys[1].expiration) ? 1 : 0;
    size_t new_index    = active_index ? 0 : 1;

    // Set active key (new value, no expiration)
    keys[new_index]            = key;
    keys[new_index].expiration = 0;
    // Set previous key (keep value, set expiration)
    keys[active_index].expiration = 1;
    if (period > 0)
    {
        uint64_t now                  = System::SystemClock().GetMonotonicTimestamp().count();
        keys[active_index].expiration = (period * 1000) + now;
    }
}

CHIP_ERROR GroupEntry::GetActiveKey(GroupKey & key)
{
    for (size_t i = 0; i < kGroupKeyCount; ++i)
    {
        if (keys[i].IsActive())
        {
            key = keys[i];
            return CHIP_NO_ERROR;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

//
// GroupList
//

static constexpr size_t kMaxMembershipCount = CHIP_CONFIG_MAX_FABRICS; // TODO

struct GroupList : public PersistentArray<kMaxMembershipCount, kPersistentBufferMax, GroupEntry>
{
    GroupList(FabricIndex fabric, PersistentStorageDelegate * storage) :
        PersistentArray<CHIP_CONFIG_MAX_FABRICS, kPersistentBufferMax, GroupEntry>(storage), mFabric(fabric)
    {}

    CHIP_ERROR UpdateKey(StorageKeyName & key) const override
    {
        VerifyOrReturnError(kUndefinedFabricIndex != mFabric, CHIP_ERROR_INVALID_FABRIC_INDEX);
        key = DefaultStorageKeyAllocator::GroupcastMembership(mFabric);
        return CHIP_NO_ERROR;
    }

    void ClearEntry(GroupEntry & entry) override
    {
        entry.group_id       = 0;
        entry.endpoint_count = 0;
        memset(entry.endpoints, 0x00, sizeof(entry.endpoints));
    }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer, const GroupEntry & entry) const override
    {
        ReturnErrorOnFailure(writer.Put(TargetIdTag(), static_cast<uint16_t>(entry.group_id)));
        ReturnErrorOnFailure(writer.Put(EndpointCountTag(), static_cast<uint16_t>(entry.endpoint_count)));
        // Endpoints
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(EndpointListTag(), TLV::kTLVType_Array, array));
            for (size_t i = 0; i < kEndpointsMax; ++i)
            {
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
                // Endpoint
                ReturnErrorOnFailure(writer.Put(EndpointEntryTag(), entry.endpoints[i]));
                ReturnErrorOnFailure(writer.EndContainer(item));
            }
            ReturnErrorOnFailure(writer.EndContainer(array));
        }
        // Keys
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(writer.StartContainer(KeyListTag(), TLV::kTLVType_Array, array));
            for (size_t i = 0; i < kGroupKeyCount; ++i)
            {
                ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
                // Encryption Key
                ByteSpan encrypt_key(entry.keys[i].encryption.As<Crypto::Symmetric128BitsKeyByteArray>());
                ReturnErrorOnFailure(writer.Put(EncryptKeyTag(), encrypt_key));
                // Privacy Key
                ByteSpan privacy_key(entry.keys[i].privacy.As<Crypto::Symmetric128BitsKeyByteArray>());
                ReturnErrorOnFailure(writer.Put(PrivacyKeyTag(), privacy_key));
                // Expiration
                ReturnErrorOnFailure(writer.Put(ExpirationTag(), static_cast<uint64_t>(entry.keys[i].expiration)));
                ReturnErrorOnFailure(writer.EndContainer(item));
            }
            ReturnErrorOnFailure(writer.EndContainer(array));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader, GroupEntry & entry) override
    {
        // Group Id
        ReturnErrorOnFailure(reader.Next(TargetIdTag()));
        ReturnErrorOnFailure(reader.Get(entry.group_id));
        // Endpoint Count
        ReturnErrorOnFailure(reader.Next(EndpointCountTag()));
        ReturnErrorOnFailure(reader.Get(entry.endpoint_count));
        // Endpoints
        ReturnErrorOnFailure(reader.Next(EndpointListTag()));
        VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (size_t i = 0; i < kEndpointsMax; ++i)
            {
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
                ReturnErrorOnFailure(reader.EnterContainer(item));
                // Endpoint
                ReturnErrorOnFailure(reader.Next(EndpointEntryTag()));
                ReturnErrorOnFailure(reader.Get(entry.endpoints[i]));
                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }
        // Keys
        ReturnErrorOnFailure(reader.Next(KeyListTag()));
        VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);
        {
            TLV::TLVType array, item;
            ReturnErrorOnFailure(reader.EnterContainer(array));
            for (size_t i = 0; i < kGroupKeyCount; ++i)
            {
                ByteSpan buf;
                ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
                VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
                ReturnErrorOnFailure(reader.EnterContainer(item));
                // Encryption Key
                ReturnErrorOnFailure(reader.Next(EncryptKeyTag()));
                ReturnErrorOnFailure(reader.Get(buf));
                memcpy(entry.keys[i].encryption.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(), buf.data(),
                       sizeof(Crypto::Symmetric128BitsKeyByteArray));
                // Privacy Key
                ReturnErrorOnFailure(reader.Next(PrivacyKeyTag()));
                ReturnErrorOnFailure(reader.Get(buf));
                memcpy(entry.keys[i].privacy.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(), buf.data(),
                       sizeof(Crypto::Symmetric128BitsKeyByteArray));
                // Endpoint Count
                ReturnErrorOnFailure(reader.Next(ExpirationTag()));
                ReturnErrorOnFailure(reader.Get(entry.keys[i].expiration));
                ReturnErrorOnFailure(reader.ExitContainer(item));
            }
            ReturnErrorOnFailure(reader.ExitContainer(array));
        }

        return CHIP_NO_ERROR;
    }

    FabricIndex mFabric;
};

DataProvider::KeyContext::KeyContext(DataProvider & provider, const GroupKey & key) : mProvider(provider)
{
    memcpy(mEncryptionKey.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
           key.encryption.As<Crypto::Symmetric128BitsKeyByteArray>(), sizeof(Crypto::Symmetric128BitsKeyByteArray));
    memcpy(mPrivacyKey.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(), key.privacy.As<Crypto::Symmetric128BitsKeyByteArray>(),
           sizeof(Crypto::Symmetric128BitsKeyByteArray));
}

uint16_t DataProvider::KeyContext::GetKeyHash()
{
    return 0;
}

CHIP_ERROR DataProvider::KeyContext::MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce,
                                                    MutableByteSpan & mic, MutableByteSpan & ciphertext) const
{
    uint8_t * output = ciphertext.data();
    return Crypto::AES_CCM_encrypt(plaintext.data(), plaintext.size(), aad.data(), aad.size(), mEncryptionKey, nonce.data(),
                                   nonce.size(), output, mic.data(), mic.size());
}

CHIP_ERROR DataProvider::KeyContext::MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce,
                                                    const ByteSpan & mic, MutableByteSpan & plaintext) const
{
    uint8_t * output = plaintext.data();
    return Crypto::AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), mic.data(), mic.size(),
                                   mEncryptionKey, nonce.data(), nonce.size(), output);
}

CHIP_ERROR DataProvider::KeyContext::PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const
{
    return Crypto::AES_CTR_crypt(input.data(), input.size(), mPrivacyKey, nonce.data(), nonce.size(), output.data());
}

CHIP_ERROR DataProvider::KeyContext::PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const
{
    return Crypto::AES_CTR_crypt(input.data(), input.size(), mPrivacyKey, nonce.data(), nonce.size(), output.data());
}

void DataProvider::KeyContext::Release()
{
    mProvider.mKeyContextPool.ReleaseObject(this);
}

//
// KeyContext Iterator
//

DataProvider::KeyContextIterator::KeyContextIterator(DataProvider & provider, FabricIndex fabric, GroupId group_id) :
    mProvider(provider), mEntry(group_id)
{
    size_t index = 0;
    chip::Groupcast::GroupList list(fabric, provider.mStorage);
    list.Find(mEntry, index);
}

size_t DataProvider::KeyContextIterator::Count()
{
    size_t count = 0;
    for (size_t i = 0; i < kGroupKeyCount; ++i)
    {
        if (mEntry.keys[0].IsActive())
        {
            count++;
        }
    }
    return count;
}

bool DataProvider::KeyContextIterator::Next(Crypto::SymmetricKeyContext *& output)
{
    while (mIndex < kGroupKeyCount)
    {
        GroupKey & keyset = mEntry.keys[mIndex++];

        if (keyset.IsActive())
        {
            output = mProvider.mKeyContextPool.CreateObject(mProvider, keyset);
            return true;
        }
    }
    return false;
}

void DataProvider::KeyContextIterator::Release()
{
    mProvider.mKeyContextIteratorPool.ReleaseObject(this);
}

//
// Group Iterator
//

DataProvider::GroupIterator::GroupIterator(DataProvider & provider, FabricIndex fabric) : mProvider(provider), mFabric(fabric)
{
    chip::Groupcast::GroupList list(fabric, provider.mStorage);
    list.Load();
    mCount = list.Count();
}

size_t DataProvider::GroupIterator::Count()
{
    return mCount;
}

bool DataProvider::GroupIterator::Next(Group & out)
{
    VerifyOrReturnValue(mIndex < mCount, false);
    GroupEntry entry;
    chip::Groupcast::GroupList list(mFabric, mProvider.mStorage);
    list.Load();
    VerifyOrReturnValue(CHIP_NO_ERROR == list.Get(mIndex++, entry), false);
    out = entry;
    return true;
}

void DataProvider::GroupIterator::Release()
{
    mProvider.mGroupIteratorPool.ReleaseObject(this);
}

//
// DataProvider
//

namespace {
DataProvider sInstance;
}

DataProvider & DataProvider::DataProvider::Instance()
{
    return sInstance;
}

CHIP_ERROR DataProvider::Initialize(PersistentStorageDelegate * storage, chip::Crypto::SessionKeystore * keystore)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(keystore != nullptr, CHIP_ERROR_INTERNAL);
    mStorage  = storage;
    mKeystore = keystore;
    return CHIP_NO_ERROR;
}

uint8_t DataProvider::GetMaxMembershipCount()
{
    return kMaxMembershipCount;
}

CHIP_ERROR DataProvider::JoinGroup(chip::FabricIndex fabric_idx,
                                   const ByteSpan & compressed_fabric_id,
                                   Group & target, const ByteSpan & key, uint32_t period)
{
    VerifyOrReturnError(mStorage != nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mKeystore != nullptr, CHIP_ERROR_INCORRECT_STATE);
    Crypto::GroupOperationalCredentials credentials;

    // Generate credentials
    ReturnErrorOnFailure(Crypto::DeriveGroupOperationalCredentials(key, compressed_fabric_id, credentials));

    // Insert entry
    {
        chip::Groupcast::GroupList list(fabric_idx, mStorage);
        GroupEntry entry(target.group_id);
        size_t index = 0;

        // Load current entry, if any
        list.Find(entry, index);

        // Endpoints
        entry.endpoint_count = target.endpoint_count;
        memcpy(entry.endpoints, target.endpoints, entry.endpoint_count * sizeof(EndpointId));

        // Keys
        if (0 == key.size())
        {
            // Invalidate all keys
            entry.InvalidateKeys();
        }
        else
        {
            // Set new key
            GroupKey target_key;
            ReturnErrorOnFailure(mKeystore->CreateKey(credentials.encryption_key, target_key.encryption));
            ReturnErrorOnFailure(mKeystore->CreateKey(credentials.privacy_key, target_key.privacy));
            entry.SetActiveKey(target_key, period);
        }

        return list.Add(entry, true);
    }
}

CHIP_ERROR DataProvider::GetGroup(FabricIndex fabric_idx, Group & target)
{
    chip::Groupcast::GroupList list(fabric_idx, mStorage);
    GroupEntry entry(target.group_id);
    size_t index = 0;
    // Find target in NVM
    ReturnErrorOnFailure(list.Find(entry, index));
    // Copy to output
    target = entry;
    return CHIP_NO_ERROR;
}

chip::Crypto::SymmetricKeyContext * DataProvider::CreateKeyContext(FabricIndex fabric, GroupId group_id)
{
    // Find target in NVM
    GroupEntry entry(group_id);
    size_t index = 0;
    chip::Groupcast::GroupList list(fabric, mStorage);
    VerifyOrReturnValue(CHIP_NO_ERROR == list.Find(entry, index), nullptr);
    // Get active key
    GroupKey key;
    VerifyOrReturnValue(CHIP_NO_ERROR == entry.GetActiveKey(key), nullptr);
    // Create context with loaded keys
    chip::Crypto::SymmetricKeyContext * context = mKeyContextPool.CreateObject(*this, entry.keys[0]);
    return context;
}

DataProvider::KeyContextIterator * DataProvider::IterateKeyContexts(FabricIndex fabric, GroupId group_id)
{
    return mKeyContextIteratorPool.CreateObject(*this, fabric, group_id);
}

DataProvider::GroupIterator * DataProvider::IterateGroups(FabricIndex fabric)
{
    return mGroupIteratorPool.CreateObject(*this, fabric);
}

} // namespace Groupcast
} // namespace chip
