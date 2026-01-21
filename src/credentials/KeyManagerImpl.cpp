#include <credentials/KeyManagerImpl.h>
#include <lib/support/CommonPersistentData.h>

namespace chip {
namespace Credentials {

constexpr size_t kPersistentBufferMax = 128;

struct Tags
{
    static constexpr TLV::Tag KeyId() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag Policy() { return TLV::ContextTag(2); }
    static constexpr TLV::Tag NumKeys() { return TLV::ContextTag(3); }
    static constexpr TLV::Tag Credentials() { return TLV::ContextTag(10); }
    static constexpr TLV::Tag StartTime() { return TLV::ContextTag(11); }
    static constexpr TLV::Tag KeyHash() { return TLV::ContextTag(12); }
    static constexpr TLV::Tag KeyValue() { return TLV::ContextTag(13); }

};

void KeysetData::Clear()
{
    fabric_index = kUndefinedFabricIndex;
    policy = SecurityPolicy::kCacheAndSync;
    keys_count = 0;
    memset(operational_keys, 0x00, sizeof(operational_keys));
}

Crypto::GroupOperationalCredentials * KeysetData::GetCurrentGroupCredentials()
{
    // An epoch key update SHALL order the keys from oldest to newest,
    // the current epoch key having the second newest time if time
    // synchronization is not achieved or guaranteed.
    switch (this->keys_count)
    {
    case 1:
    case 2:
        return &operational_keys[0];
    case 3:
        return &operational_keys[1];
    default:
        return nullptr;
    }
}

KeysetList::KeysetList(PersistentStorageDelegate * storage, FabricIndex fabric_index) :
    PersistentArray<kMaxKeysPerFabric, MaxPersistentBuffer(), KeysetData>(storage), mFabric(fabric_index)
{}

CHIP_ERROR KeysetList::UpdateKey(StorageKeyName & key) const
{
    VerifyOrReturnError(kUndefinedFabricIndex != mFabric, CHIP_ERROR_INVALID_FABRIC_INDEX);
    key = DefaultStorageKeyAllocator::FabricKeyset(mFabric, 0); // TODO: USE A NEW KEY
    return CHIP_NO_ERROR;
}

void KeysetList::ClearEntry(KeysetData & entry)
{
    entry.Clear();
}

CHIP_ERROR KeysetList::Copy(KeysetData & dest, const KeysetData & src) const
{
    dest.fabric_index = src.fabric_index;
    dest.policy = src.policy;
    dest.keyset_id = src.keyset_id;
    dest.keys_count = src.keys_count;
    memcpy(dest.operational_keys, src.operational_keys, sizeof(src.operational_keys));
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeysetList::Serialize(TLV::TLVWriter & writer, const KeysetData & entry) const
{
    // ChipLogDetail(DeviceLayer, "~~~~ MAX: (%u * %u) %u", (unsigned)MaxPersistentBuffer(), (unsigned)mLimit, (unsigned)sizeof(mBuffer));
    // keyset_id
    ReturnErrorOnFailure(writer.Put(Tags::KeyId(), static_cast<uint16_t>(entry.keyset_id)));
    // policy
    ReturnErrorOnFailure(writer.Put(Tags::Policy(), static_cast<uint16_t>(entry.policy)));
    // keys_count
    ReturnErrorOnFailure(writer.Put(Tags::NumKeys(), static_cast<uint16_t>(entry.keys_count)));
    // operational_keys
    {
        TLV::TLVType array, item;
        ReturnErrorOnFailure(writer.StartContainer(Tags::Credentials(), TLV::kTLVType_Array, array));
        for (size_t i = 0; (i < entry.keys_count) && (i < KeySet::kEpochKeysMax); i++)
        {
            auto &key = entry.operational_keys[i];
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, item));
            ReturnErrorOnFailure(writer.Put(Tags::StartTime(), static_cast<uint64_t>(key.start_time)));
            ReturnErrorOnFailure(writer.Put(Tags::KeyHash(), key.hash));
            ReturnErrorOnFailure(writer.Put(Tags::KeyValue(), ByteSpan(key.encryption_key)));
            ReturnErrorOnFailure(writer.EndContainer(item));
        }
        ReturnErrorOnFailure(writer.EndContainer(array));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR KeysetList::Deserialize(TLV::TLVReader & reader, KeysetData & entry)
{
    // fabric_index
    entry.fabric_index = mFabric;
    // keyset_id
    ReturnErrorOnFailure(reader.Next(Tags::KeyId()));
    ReturnErrorOnFailure(reader.Get(entry.keyset_id));
    // policy
    ReturnErrorOnFailure(reader.Next(Tags::Policy()));
    ReturnErrorOnFailure(reader.Get(entry.policy));
    // keys_count
    ReturnErrorOnFailure(reader.Next(Tags::NumKeys()));
    ReturnErrorOnFailure(reader.Get(entry.keys_count));
    // TODO(#21614): Enforce maximum number of 3 keys in a keyset
    {
        // operational_keys
        ReturnErrorOnFailure(reader.Next(Tags::Credentials()));
        VerifyOrReturnError(TLV::kTLVType_Array == reader.GetType(), CHIP_ERROR_INTERNAL);

        TLV::TLVType array, item;
        ReturnErrorOnFailure(reader.EnterContainer(array));
        for (size_t i = 0; (i < entry.keys_count) && (i < KeySet::kEpochKeysMax); i++)
        {
            auto &key = entry.operational_keys[i];
            ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
            VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);

            ReturnErrorOnFailure(reader.EnterContainer(item));
            // start_time
            ReturnErrorOnFailure(reader.Next(Tags::StartTime()));
            ReturnErrorOnFailure(reader.Get(key.start_time));
            // key hash
            ReturnErrorOnFailure(reader.Next(Tags::KeyHash()));
            ReturnErrorOnFailure(reader.Get(key.hash));
            // key value
            ByteSpan encryption_key;
            ReturnErrorOnFailure(reader.Next(Tags::KeyValue()));
            ReturnErrorOnFailure(reader.Get(encryption_key));
            VerifyOrReturnError(Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES == encryption_key.size(), CHIP_ERROR_INTERNAL);
            memcpy(key.encryption_key, encryption_key.data(), encryption_key.size());
            // Re-derive privacy key from encryption key when loading from storage to save on storage size.
            MutableByteSpan privacy_key(key.privacy_key);
            ReturnErrorOnFailure(Crypto::DeriveGroupPrivacyKey(encryption_key, privacy_key));
            ReturnErrorOnFailure(reader.ExitContainer(item));
        }
        ReturnErrorOnFailure(reader.ExitContainer(array));
    }
    return CHIP_NO_ERROR;
}

//
// KeyManagerImpl
//

constexpr size_t Credentials::EpochKey::kLengthBytes;

CHIP_ERROR KeyManagerImpl::Initialize(PersistentStorageDelegate * storage, Crypto::SessionKeystore * key_store)
{
    VerifyOrReturnError(storage != nullptr, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(key_store != nullptr, CHIP_ERROR_INTERNAL);
    mStorage  = storage;
    mKeystore = key_store;
    return CHIP_NO_ERROR;
}

//
// Keysets
//

CHIP_ERROR KeyManagerImpl::SetKeySet(chip::FabricIndex fabric_index, const ByteSpan & compressed_fabric_id,
                                            const KeySet & in_keyset)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);
    
    KeysetList list(mStorage, fabric_index);
    KeysetData keyset(fabric_index, in_keyset.keyset_id);
    size_t index = 0;

    // Search existing keyset
    bool found = (CHIP_NO_ERROR == list.Find(keyset, index));

    keyset.keyset_id  = in_keyset.keyset_id;
    keyset.policy     = in_keyset.policy;
    keyset.keys_count = in_keyset.num_keys_used;
    memset(keyset.operational_keys, 0x00, sizeof(keyset.operational_keys));
    keyset.operational_keys[0].start_time = in_keyset.epoch_keys[0].start_time;
    keyset.operational_keys[1].start_time = in_keyset.epoch_keys[1].start_time;
    keyset.operational_keys[2].start_time = in_keyset.epoch_keys[2].start_time;

    // Store the operational keys and hash instead of the epoch keys
    for (size_t i = 0; i < in_keyset.num_keys_used; ++i)
    {
        ByteSpan epoch_key(in_keyset.epoch_keys[i].key, Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES);
        ReturnErrorOnFailure(
            Crypto::DeriveGroupOperationalCredentials(epoch_key, compressed_fabric_id, keyset.operational_keys[i]));
    }

    if (found)
    {
        // Update existing keyset info, keep next
        ReturnErrorOnFailure(list.Set(index, keyset));
        return list.Save(mStorage);
    }

    // New keyset
    VerifyOrReturnError(list.Count() < kMaxKeysPerFabric, CHIP_ERROR_INVALID_LIST_LENGTH);

    // Insert first
    list.Add(keyset);
    return list.Save(mStorage);
}

CHIP_ERROR KeyManagerImpl::GetKeySet(chip::FabricIndex fabric_index, uint16_t target_id, KeySet & out_keyset)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    KeysetList list(mStorage, fabric_index);
    KeysetData keyset(fabric_index, target_id);
    size_t index = 0;
    ReturnErrorOnFailure(list.Find(keyset, index), CHIP_ERROR_NOT_FOUND);

    // Target keyset found
    out_keyset.ClearKeys();
    out_keyset.keyset_id     = static_cast<KeysetId>(keyset.keyset_id);
    out_keyset.policy        = keyset.policy;
    out_keyset.num_keys_used = keyset.keys_count;
    // Epoch keys are not read back, only start times
    out_keyset.epoch_keys[0].start_time = keyset.operational_keys[0].start_time;
    out_keyset.epoch_keys[1].start_time = keyset.operational_keys[1].start_time;
    out_keyset.epoch_keys[2].start_time = keyset.operational_keys[2].start_time;

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyManagerImpl::RemoveKeySet(chip::FabricIndex fabric_index, uint16_t target_id)
{
    VerifyOrReturnError(IsInitialized(), CHIP_ERROR_INTERNAL);

    KeysetList list(mStorage, fabric_index);
    KeysetData keyset(fabric_index, target_id);
    size_t index = 0;
    ReturnErrorOnFailure(list.Find(keyset, index), CHIP_ERROR_NOT_FOUND);
    ReturnErrorOnFailure(list.Remove(index));
    return list.Save();
}

//
// IPK
//

CHIP_ERROR KeyManagerImpl::GetIpkKeySet(FabricIndex fabric_index, KeySet & out_keyset)
{
    KeysetList list(mStorage, fabric_index);
    KeysetData keyset(fabric_index, kIdentityProtectionKeySetId);
    size_t index = 0;
    VerifyOrReturnValue(CHIP_NO_ERROR == list.Find(keyset, index), CHIP_ERROR_NOT_FOUND);

    // // If the keyset ID doesn't match, we have a ... problem.
    VerifyOrReturnError(keyset.keyset_id == kIdentityProtectionKeySetId, CHIP_ERROR_INTERNAL);
    out_keyset.keyset_id     = static_cast<uint16_t>(keyset.keyset_id);
    out_keyset.num_keys_used = keyset.keys_count;
    out_keyset.policy        = keyset.policy;

    for (size_t key_idx = 0; key_idx < MATTER_ARRAY_SIZE(out_keyset.epoch_keys); ++key_idx)
    {
        out_keyset.epoch_keys[key_idx].Clear();
        if (key_idx < keyset.keys_count)
        {
            out_keyset.epoch_keys[key_idx].start_time = keyset.operational_keys[key_idx].start_time;
            memcpy(&out_keyset.epoch_keys[key_idx].key[0], keyset.operational_keys[key_idx].encryption_key, EpochKey::kLengthBytes);
        }
    }
    return CHIP_NO_ERROR;
}

//
// Groups
//

Crypto::SymmetricKeyContext * KeyManagerImpl::CreateKeyContext(FabricIndex fabric_index, KeysetId keyset_id)
{
    KeysetList list(mStorage, fabric_index);
    KeysetData keyset(fabric_index, keyset_id);
    size_t index = 0;
    VerifyOrReturnValue(CHIP_NO_ERROR == list.Find(keyset, index), nullptr);
    Crypto::GroupOperationalCredentials *creds = keyset.GetCurrentGroupCredentials();
    VerifyOrReturnValue(nullptr != creds, nullptr);
    return mKeyContextPool.CreateObject(*this, *creds);
}

CHIP_ERROR KeyManagerImpl::GetGroupSession(FabricIndex fabric_index, KeysetId keyset_id, uint16_t hash, GroupSession &session)
{
    KeysetList list(mStorage, fabric_index);
    KeysetData keyset(fabric_index, keyset_id);
    size_t index = 0;
    ReturnErrorOnFailure(list.Find(keyset, index));

    Crypto::GroupOperationalCredentials * creds = keyset.GetCurrentGroupCredentials();
    VerifyOrReturnError((nullptr != creds) && (hash == creds->hash), CHIP_ERROR_NOT_FOUND);

    session.fabric_index    = fabric_index;
    session.group_id        = kUndefinedGroupId; // Key manager doesn't associate keys to any specific group
    session.security_policy = keyset.policy;
    session.keyContext = mKeyContextPool.CreateObject(*this, *creds);
    return CHIP_NO_ERROR;
}

//
// KeyContext
//

// KeyManagerImpl::KeyContext::KeyContext(KeyManager & manager, const Crypto::SessionKeystore * keystore):
//     mManager(manager)
// {
//     ReleaseKeys();
//     // TODO: Load group keys to the session keystore upon loading from persistent storage
//     //
//     // Group keys should be transformed into a key handle as soon as possible or even
//     // the key storage should be taken over by SessionKeystore interface, but this looks
//     // like more work, so let's use the transitional code below for now.
//     keystore->CreateKey(encryptionKey, mEncryptionKey);
//     keystore->CreateKey(privacyKey, mPrivacyKey);
// }

KeyManagerImpl::KeyContext::KeyContext(KeyManagerImpl & manager, const Crypto::GroupOperationalCredentials &creds):
    mManager(manager)
{
    mHash = creds.hash;
    memcpy(mEncryptionKey.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
           creds.encryption_key, sizeof(Crypto::Symmetric128BitsKeyByteArray));
    memcpy(mPrivacyKey.AsMutable<Crypto::Symmetric128BitsKeyByteArray>(),
           creds.privacy_key, sizeof(Crypto::Symmetric128BitsKeyByteArray));
}

uint16_t KeyManagerImpl::KeyContext::GetKeyHash() { return mHash; }

CHIP_ERROR KeyManagerImpl::KeyContext::MessageEncrypt(const ByteSpan & plaintext, const ByteSpan & aad, const ByteSpan & nonce, MutableByteSpan & mic,
                        MutableByteSpan & ciphertext) const 
{
    uint8_t * output = ciphertext.data();
    return Crypto::AES_CCM_encrypt(plaintext.data(), plaintext.size(), aad.data(), aad.size(), mEncryptionKey, nonce.data(),
                                   nonce.size(), output, mic.data(), mic.size());
}

CHIP_ERROR KeyManagerImpl::KeyContext::MessageDecrypt(const ByteSpan & ciphertext, const ByteSpan & aad, const ByteSpan & nonce, const ByteSpan & mic,
                        MutableByteSpan & plaintext) const 
{
    uint8_t * output = plaintext.data();
    return Crypto::AES_CCM_decrypt(ciphertext.data(), ciphertext.size(), aad.data(), aad.size(), mic.data(), mic.size(),
                                   mEncryptionKey, nonce.data(), nonce.size(), output);
}

CHIP_ERROR KeyManagerImpl::KeyContext::PrivacyEncrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const 
{
    return Crypto::AES_CTR_crypt(input.data(), input.size(), mPrivacyKey, nonce.data(), nonce.size(), output.data());
}

CHIP_ERROR KeyManagerImpl::KeyContext::PrivacyDecrypt(const ByteSpan & input, const ByteSpan & nonce, MutableByteSpan & output) const 
{
    return Crypto::AES_CTR_crypt(input.data(), input.size(), mPrivacyKey, nonce.data(), nonce.size(), output.data());
}

void KeyManagerImpl::KeyContext::Release() 
{
    ReleaseKeys();
    mManager.mKeyContextPool.ReleaseObject(this);
}

void KeyManagerImpl::KeyContext::ReleaseKeys()
{
    // if(mManager.mKeystore) {
    //     mManager.mKeystore->DestroyKey(mEncryptionKey);
    //     mManager.mKeystore->DestroyKey(mPrivacyKey);
    // }
}

//
// Iterators
//

KeySetIterator * KeyManagerImpl::IterateKeySets(chip::FabricIndex fabric_index)
{
    VerifyOrReturnValue(IsInitialized(), nullptr);
    return mKeySetIteratorPool.CreateObject(*this, *mStorage, fabric_index);
}

KeyManagerImpl::KeySetIteratorImpl::KeySetIteratorImpl(KeyManagerImpl & manager, PersistentStorageDelegate & storage, FabricIndex fabric_index) :
    mManager(manager), mList(&storage, fabric_index)
{
    mList.Load();
}


size_t KeyManagerImpl::KeySetIteratorImpl::Count()
{
    return mList.Count();
}

bool KeyManagerImpl::KeySetIteratorImpl::Next(KeySet & output)
{
    KeysetData keyset;
    VerifyOrReturnError(mIndex < mList.Count(), false);
    VerifyOrReturnValue(CHIP_NO_ERROR == mList.At(mIndex++, keyset), false);

    output.ClearKeys();
    output.keyset_id     =  static_cast<KeysetId>(keyset.keyset_id);
    output.policy        = keyset.policy;
    output.num_keys_used = keyset.keys_count;
    // Epoch keys are not read back, only start times
    output.epoch_keys[0].start_time = keyset.operational_keys[0].start_time;
    output.epoch_keys[1].start_time = keyset.operational_keys[1].start_time;
    output.epoch_keys[2].start_time = keyset.operational_keys[2].start_time;
    return true;
}

void KeyManagerImpl::KeySetIteratorImpl::Release()
{
    mManager.mKeySetIteratorPool.ReleaseObject(this);
}

//
// Fabrics
//

CHIP_ERROR KeyManagerImpl::RemoveFabric(FabricIndex fabric_index)
{
    KeysetList list(mStorage, fabric_index);
    ReturnErrorOnFailure(list.RemoveAll());
    return list.Save();
}


namespace {

KeyManager * gKeyManager = nullptr;

} // namespace

KeyManager * GetKeyManager()
{
    return gKeyManager;
}

void SetKeyManager(KeyManager * manager)
{
    gKeyManager = manager;
}

} // namespace Credentials
} // namespace chip
