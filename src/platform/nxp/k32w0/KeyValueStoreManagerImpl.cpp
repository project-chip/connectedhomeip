/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/**
 *    @file
 *          Platform-specific key value storage implementation for K32W
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMem.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/nxp/k32w0/K32W0Config.h>
#include <platform/nxp/k32w0/KeyValueStoreManagerImpl.h>
#include <platform/nxp/k32w0/RamStorage.h>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

constexpr size_t kMaxNumberOfKeys  = 200;
constexpr size_t kMaxKeyValueBytes = 255;

Internal::RamStorage KeyValueStoreManagerImpl::sKeysStorage         = { kNvmId_KvsKeys, "Keys" };
Internal::RamStorage KeyValueStoreManagerImpl::sValuesStorage       = { kNvmId_KvsValues, "Values" };
Internal::RamStorage KeyValueStoreManagerImpl::sSubscriptionStorage = { kNvmId_KvsSubscription, "Subscriptions" };
Internal::RamStorage KeyValueStoreManagerImpl::sGroupsStorage       = { kNvmId_KvsGroups, "Groups" };
Internal::RamStorage KeyValueStoreManagerImpl::sAclStorage          = { kNvmId_KvsAcl, "Acl" };

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

#if CONFIG_CHIP_K32W0_KVS_MOVE_KEYS_TO_SPECIFIC_STORAGE
static CHIP_ERROR MoveKeysAndValues();
#endif // CONFIG_CHIP_K32W0_KVS_MOVE_KEYS_TO_SPECIFIC_STORAGE

static inline bool IsKeyRelatedToGroups(const char * key)
{
    std::string _key(key);
    if (_key.find("f/") != 0)
        return false;

    return (_key.find("/g") != std::string::npos) || (_key.find("/k/") != std::string::npos);
}

static inline bool IsKeyRelatedToSubscriptions(const char * key)
{
    std::string _key(key);
    return _key.find("g/su") == 0;
}

static inline bool IsKeyRelatedToAcl(const char * key)
{
    std::string _key(key);
    return _key.find("/ac/") != std::string::npos;
}

static Internal::RamStorage * GetValStorage(const char * key)
{
    Internal::RamStorage * storage = nullptr;

    storage = IsKeyRelatedToSubscriptions(key) ? &KeyValueStoreManagerImpl::sSubscriptionStorage
                                               : &KeyValueStoreManagerImpl::sValuesStorage;
    storage = IsKeyRelatedToGroups(key) ? &KeyValueStoreManagerImpl::sGroupsStorage : storage;
    storage = IsKeyRelatedToAcl(key) ? &KeyValueStoreManagerImpl::sAclStorage : storage;

    return storage;
}

static Internal::RamStorage * GetKeyStorage(const char * key)
{
    Internal::RamStorage * storage = nullptr;

    storage = IsKeyRelatedToSubscriptions(key) ? &KeyValueStoreManagerImpl::sSubscriptionStorage
                                               : &KeyValueStoreManagerImpl::sKeysStorage;
    storage = IsKeyRelatedToGroups(key) ? &KeyValueStoreManagerImpl::sGroupsStorage : storage;
    storage = IsKeyRelatedToAcl(key) ? &KeyValueStoreManagerImpl::sAclStorage : storage;

    return storage;
}

uint16_t GetStringKeyId(const char * key, uint16_t * freeId)
{
    CHIP_ERROR err                    = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint8_t keyId                     = 0;
    bool bFreeIdxFound                = false;
    char keyString[kMaxKeyValueBytes] = { 0 };
    uint16_t pdmInternalId;

    for (keyId = 0; keyId < kMaxNumberOfKeys; keyId++)
    {
        uint16_t keyStringSize = kMaxKeyValueBytes;
        pdmInternalId          = Internal::RamStorageKey::GetInternalId(kKeyId_KvsKeys, keyId);
        err                    = GetKeyStorage(key)->Read(pdmInternalId, 0, (uint8_t *) keyString, &keyStringSize);

        if (err == CHIP_NO_ERROR)
        {
            if (strcmp(key, keyString) == 0)
            {
                // found the entry we are looking for
                break;
            }
        }
        else if ((NULL != freeId) && (false == bFreeIdxFound))
        {
            bFreeIdxFound = true;
            *freeId       = keyId;
        }
    }
    return keyId;
}

CHIP_ERROR KeyValueStoreManagerImpl::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = sKeysStorage.Init(Internal::RamStorage::kRamBufferInitialSize);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS keys storage with id: %d. Error: %s", kNvmId_KvsKeys, ErrorStr(err));
    }

    err = sValuesStorage.Init(Internal::RamStorage::kRamBufferInitialSize, true);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS values storage with id: %d. Error: %s", kNvmId_KvsValues, ErrorStr(err));
    }

    err = sSubscriptionStorage.Init(Internal::RamStorage::kRamBufferInitialSize);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS subscription storage with id: %d. Error: %s", kNvmId_KvsSubscription,
                        ErrorStr(err));
    }

    err = sGroupsStorage.Init(Internal::RamStorage::kRamBufferInitialSize, true);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS groups storage with id: %d. Error: %s", kNvmId_KvsGroups, ErrorStr(err));
    }

    err = sAclStorage.Init(Internal::RamStorage::kRamBufferInitialSize, true);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS acl storage with id: %d. Error: %s", kNvmId_KvsAcl, ErrorStr(err));
    }

#if CONFIG_CHIP_K32W0_KVS_MOVE_KEYS_TO_SPECIFIC_STORAGE
    ChipLogProgress(DeviceLayer, "Moving some keys to dedicated storage");
    MoveKeysAndValues();
#endif /* CONFIG_CHIP_K32W0_KVS_MOVE_KEYS_TO_SPECIFIC_STORAGE */

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err         = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint8_t keyId          = 0;
    uint16_t pdmInternalId = 0;
    uint16_t valueSize     = value_size;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    keyId = GetStringKeyId(key, NULL);

    if (keyId < kMaxNumberOfKeys)
    {
        /* Use kKeyId_KvsValues as base key id for all keys. */
        pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, keyId);
        ChipLogProgress(DeviceLayer, "KVS val: get [%s][%i][%s]", key, pdmInternalId, GetValStorage(key)->GetName());
        err              = GetValStorage(key)->Read(pdmInternalId, 0, (uint8_t *) value, &valueSize);
        *read_bytes_size = valueSize;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "KVS key [%s] not found in persistent storage.", key);
    }

exit:
    ConvertError(err);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err         = CHIP_ERROR_INVALID_ARGUMENT;
    bool_t putKey          = false;
    uint16_t pdmInternalId = 0;
    uint16_t freeKeyId;
    uint8_t keyId;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    keyId = GetStringKeyId(key, &freeKeyId);

    // Key does not exist. Write both key and value in persistent storage.
    if (kMaxNumberOfKeys == keyId)
    {
        putKey = true;
        keyId  = freeKeyId;
    }

    /* Use kKeyId_KvsValues as base key id for all keys. */
    pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, keyId);
    ChipLogProgress(DeviceLayer, "KVS val: set [%s][%i][%s]", key, pdmInternalId, GetValStorage(key)->GetName());
    err = GetValStorage(key)->Write(pdmInternalId, (uint8_t *) value, value_size);
    /* save the 'key' in flash such that it can be retrieved later on */
    if (err == CHIP_NO_ERROR)
    {
        if (putKey)
        {
            pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsKeys, keyId);
            ChipLogProgress(DeviceLayer, "KVS key: set [%s][%i][%s]", key, pdmInternalId, GetKeyStorage(key)->GetName());

            err = GetKeyStorage(key)->Write(pdmInternalId, (uint8_t *) key, strlen(key) + 1);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "KVS key: error when setting [%s][%i]", key, pdmInternalId);
            }
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "KVS val: error when setting [%s][%i]", key, pdmInternalId);
    }

exit:
    ConvertError(err);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err         = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint8_t keyId          = 0;
    uint16_t pdmInternalId = 0;

    VerifyOrExit((key != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    keyId = GetStringKeyId(key, NULL);
    if (keyId < kMaxNumberOfKeys)
    {
        // entry exists so we can remove it
        pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsKeys, keyId);

        ChipLogProgress(DeviceLayer, "KVS key: del [%s][%i][%s]", key, pdmInternalId, GetKeyStorage(key)->GetName());
        err = GetKeyStorage(key)->Delete(pdmInternalId, -1);

        /* also delete the 'key string' from flash */
        if (err == CHIP_NO_ERROR)
        {
            /* Use kKeyId_KvsValues as base key id for all keys. */
            pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, keyId);
            ChipLogProgress(DeviceLayer, "KVS val: del [%s][%i][%s]", key, pdmInternalId, GetValStorage(key)->GetName());

            err = GetValStorage(key)->Delete(pdmInternalId, -1);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "KVS val: error when deleting [%s][%i]", key, pdmInternalId);
            }
        }
        else
        {
            ChipLogProgress(DeviceLayer, "KVS key: error when deleting [%s][%i]", key, pdmInternalId);
        }
    }
exit:
    ConvertError(err);
    return err;
}

void KeyValueStoreManagerImpl::FactoryResetStorage(void)
{
    sKeysStorage.OnFactoryReset();
    sValuesStorage.OnFactoryReset();
    sSubscriptionStorage.OnFactoryReset();
    sGroupsStorage.OnFactoryReset();
}

void KeyValueStoreManagerImpl::ConvertError(CHIP_ERROR & err)
{
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
}

#if CONFIG_CHIP_K32W0_KVS_MOVE_KEYS_TO_SPECIFIC_STORAGE
static CHIP_ERROR MoveToDedicatedStorage(const char * key, uint8_t * buffer, uint16_t len, uint16_t keyId, uint16_t valId)
{
    ReturnErrorOnFailure(GetKeyStorage(key)->Write(keyId, (uint8_t *) key, strlen(key) + 1));

    ReturnErrorOnFailure(KeyValueStoreManagerImpl::sValuesStorage.Read(valId, 0, buffer, &len));
    ReturnErrorOnFailure(GetValStorage(key)->Write(valId, buffer, len));

    ReturnErrorOnFailure(KeyValueStoreManagerImpl::sKeysStorage.Delete(keyId, -1));
    ReturnErrorOnFailure(KeyValueStoreManagerImpl::sValuesStorage.Delete(valId, -1));

    return CHIP_NO_ERROR;
}

static CHIP_ERROR MoveKeysAndValues()
{
    uint16_t len                = 1024; // Should be enough for Matter keys
    uint16_t keyId              = 0;
    uint16_t valId              = 0;
    uint16_t keySize            = kMaxKeyValueBytes;
    char key[kMaxKeyValueBytes] = { 0 };
    Platform::ScopedMemoryBuffer<uint8_t> buffer;

    buffer.Calloc(len);
    ReturnErrorCodeIf(buffer.Get() == nullptr, CHIP_ERROR_NO_MEMORY);

    for (uint8_t id = 0; id < kMaxNumberOfKeys; id++)
    {
        keySize = kMaxKeyValueBytes;
        keyId   = Internal::RamStorageKey::GetInternalId(kKeyId_KvsKeys, id);
        valId   = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, id);

        auto err = KeyValueStoreManagerImpl::sKeysStorage.Read(keyId, 0, (uint8_t *) key, &keySize);
        if (err == CHIP_NO_ERROR)
        {
            if (!IsKeyRelatedToGroups(key) && !IsKeyRelatedToSubscriptions(key))
                continue;

            err = MoveToDedicatedStorage(key, buffer.Get(), len, keyId, valId);
            VerifyOrDo(err != CHIP_NO_ERROR, ChipLogProgress(DeviceLayer, "Key [%s] was moved successfully", key));
            VerifyOrDo(err == CHIP_NO_ERROR, ChipLogProgress(DeviceLayer, "Error in moving key [%s] to dedicated storage", key));
        }
    }

    return CHIP_NO_ERROR;
}
#endif // CONFIG_CHIP_K32W0_KVS_MOVE_KEYS_TO_SPECIFIC_STORAGE

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
