/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <credentials/KeyManager.h>
#include <credentials/FabricTable.h>
#include <lib/support/PersistentArray.h>

namespace chip {
namespace Credentials {

static constexpr size_t kMaxKeysPerFabric = CHIP_CONFIG_MAX_GROUP_KEYS_PER_FABRIC;

static constexpr size_t MaxPersistentBuffer()
{
    return TLV::EstimateStructOverhead(sizeof(uint16_t) + sizeof(uint32_t) + sizeof(uint8_t), KeySet::kEpochKeysMax * sizeof(Crypto::GroupOperationalCredentials));
}

struct KeysetData
{
    chip::FabricIndex fabric_index = kUndefinedFabricIndex;
    SecurityPolicy policy = SecurityPolicy::kCacheAndSync;
    uint32_t keyset_id    = 0;
    uint8_t keys_count    = 0;
    Crypto::GroupOperationalCredentials operational_keys[KeySet::kEpochKeysMax];

    KeysetData() = default;
    KeysetData(chip::FabricIndex fid, uint32_t kid): fabric_index(fid), keyset_id(kid) {}
    bool operator==(const KeysetData & other) const { return (this->fabric_index == other.fabric_index) && (this->keyset_id == other.keyset_id); }
    void Clear();
    Crypto::GroupOperationalCredentials * GetCurrentGroupCredentials();
};

struct KeysetList : public PersistentArray<kMaxKeysPerFabric, MaxPersistentBuffer(), KeysetData>
{
    FabricIndex mFabric;

    KeysetList(PersistentStorageDelegate * storage, FabricIndex fabric_index);
    CHIP_ERROR UpdateKey(StorageKeyName & key) const override;
    void ClearEntry(KeysetData & entry) override;
    CHIP_ERROR Copy(KeysetData & dest, const KeysetData & src) const override;
    CHIP_ERROR Serialize(TLV::TLVWriter & writer, const KeysetData & entry) const override;
    CHIP_ERROR Deserialize(TLV::TLVReader & reader, KeysetData & entry) override;
};

class KeyManagerImpl: public KeyManager
{
    struct KeyContext : chip::Crypto::SymmetricKeyContext
    {
        // KeyContext(KeyManager & manager, const Crypto::SessionKeystore * keystore);
        KeyContext(KeyManagerImpl & manager, const Crypto::GroupOperationalCredentials &creds);

        uint16_t GetKeyHash() override;
        CHIP_ERROR MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce, MutableByteSpan & mic,
                                    MutableByteSpan & ciphertext) const override;
        CHIP_ERROR MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce, const ByteSpan & mic,
                                    MutableByteSpan & plaintext) const override;
        CHIP_ERROR PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        CHIP_ERROR PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const override;
        void Release() override;

    protected:
        void ReleaseKeys();
        KeyManagerImpl & mManager;
        uint16_t mHash;
        Crypto::Aes128KeyHandle mEncryptionKey;
        Crypto::Aes128KeyHandle mPrivacyKey;
    };

    class KeySetIteratorImpl : public KeySetIterator
    {
    public:
        KeySetIteratorImpl(KeyManagerImpl & manager, PersistentStorageDelegate & storage, FabricIndex fabric_index);
        size_t Count() override;
        bool Next(KeySet & output) override;
        void Release() override;

    protected:
        KeyManagerImpl & mManager;
        KeysetList mList;
        size_t mIndex = 0;
    };

public:
    KeyManagerImpl() = default;
    ~KeyManagerImpl() override {}

    CHIP_ERROR Initialize(PersistentStorageDelegate * storage, chip::Crypto::SessionKeystore * keystore);
    bool IsInitialized() override { return (mStorage != nullptr) && (mKeystore != nullptr); }
    void Finish() override {}

    // Keysets
    CHIP_ERROR SetKeySet(FabricIndex fabric_index, const ByteSpan & compressed_fabric_id, const KeySet & keys) override;
    CHIP_ERROR GetKeySet(FabricIndex fabric_index, KeysetId keyset_id, KeySet & keys)                          override;
    CHIP_ERROR RemoveKeySet(FabricIndex fabric_index, KeysetId keyset_id)                                      override;
    virtual KeySetIterator * IterateKeySets(FabricIndex fabric_index)                                          override;
    // IPK
    CHIP_ERROR GetIpkKeySet(FabricIndex fabric_index, KeySet & out_keyset)                                     override;
    // Groups
    Crypto::SymmetricKeyContext * CreateKeyContext(FabricIndex fabric_index, KeysetId keyset_id)               override;
    CHIP_ERROR GetGroupSession(FabricIndex fabric_index, KeysetId keyset_id, uint16_t hash, GroupSession &session) override;
    // Fabrics
    CHIP_ERROR RemoveFabric(FabricIndex fabric_index) override;

private:
    PersistentStorageDelegate * mStorage       = nullptr;
    Crypto::SessionKeystore * mKeystore = nullptr;
    ObjectPool<KeySetIteratorImpl, kIteratorsMax> mKeySetIteratorPool;
    ObjectPool<KeyContext, kIteratorsMax> mKeyContextPool;
};

} // namespace Credentials
} // namespace chip
