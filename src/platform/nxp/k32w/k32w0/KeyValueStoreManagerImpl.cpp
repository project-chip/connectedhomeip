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
#include <platform/nxp/k32w/k32w0/K32W0Config.h>
#include <platform/nxp/k32w/k32w0/KeyValueStoreManagerImpl.h>
#include <platform/nxp/k32w/k32w0/RamStorage.h>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

constexpr size_t kMaxNumberOfKeys  = 200;
constexpr size_t kMaxKeyValueBytes = 255;

Internal::RamStorage KeyValueStoreManagerImpl::sKeysStorage   = { kNvmId_KvsKeys };
Internal::RamStorage KeyValueStoreManagerImpl::sValuesStorage = { kNvmId_KvsValues };

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

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
        err = KeyValueStoreManagerImpl::sKeysStorage.Read(pdmInternalId, 0, (uint8_t *) keyString, &keyStringSize);

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
    err            = sKeysStorage.Init(Internal::RamStorage::kRamBufferInitialSize);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS keys storage with id: %d. Error: %s", kNvmId_KvsKeys, ErrorStr(err));
    }
    // Set values storage to a big RAM buffer size as a temporary fix for TC-RR-1.1.
    err = sValuesStorage.Init(kKVS_RamBufferSize, true);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(DeviceLayer, "Cannot init KVS values storage with id: %d. Error: %s", kNvmId_KvsValues, ErrorStr(err));
    }

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
        // This is the ID of the actual data
        pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, keyId);
        ChipLogProgress(DeviceLayer, "KVS, get the value of Matter key [%s] with PDM id: %i", key, pdmInternalId);
        err              = KeyValueStoreManagerImpl::sValuesStorage.Read(pdmInternalId, 0, (uint8_t *) value, &valueSize);
        *read_bytes_size = valueSize;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "KVS, Matter key [%s] not found in persistent storage.", key);
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

    pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, keyId);
    ChipLogProgress(DeviceLayer, "KVS, save in flash the value of the Matter key [%s] with PDM id: %i", key, pdmInternalId);

    err = sValuesStorage.Write(pdmInternalId, (uint8_t *) value, value_size);

    /* save the 'key' in flash such that it can be retrieved later on */
    if (err == CHIP_NO_ERROR)
    {
        if (putKey)
        {
            pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsKeys, keyId);
            ChipLogProgress(DeviceLayer, "KVS, save in flash the Matter key [%s] with PDM id: %i and length %d", key, pdmInternalId,
                            strlen(key) + 1);

            err = sKeysStorage.Write(pdmInternalId, (uint8_t *) key, strlen(key) + 1);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "KVS, Error while saving in flash the Matter key [%s] with PDM id: %i", key,
                                pdmInternalId);
            }
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "KVS, Error while saving in flash the value of the Matter key [%s] with PDM id: %i", key,
                        pdmInternalId);
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

        ChipLogProgress(DeviceLayer, "KVS, delete from flash the Matter key [%s] with PDM id: %i", key, pdmInternalId);
        err = sKeysStorage.Delete(pdmInternalId, -1);

        /* also delete the 'key string' from flash */
        if (err == CHIP_NO_ERROR)
        {
            pdmInternalId = Internal::RamStorageKey::GetInternalId(kKeyId_KvsValues, keyId);
            ChipLogProgress(DeviceLayer, "KVS, delete from flash the value of the Matter key [%s] with PDM id: %i", key,
                            pdmInternalId);

            err = sValuesStorage.Delete(pdmInternalId, -1);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer,
                                "KVS, Error while deleting from flash the value of the Matter key [%s] with PDM id: %i", key,
                                pdmInternalId);
            }
        }
        else
        {
            ChipLogProgress(DeviceLayer, "KVS, Error while deleting from flash the Matter key [%s] with PDM id: %i", key,
                            pdmInternalId);
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
}

void KeyValueStoreManagerImpl::ConvertError(CHIP_ERROR & err)
{
    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
