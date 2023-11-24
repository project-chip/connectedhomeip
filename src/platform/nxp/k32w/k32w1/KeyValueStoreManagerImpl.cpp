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
#include <platform/nxp/k32w/k32w1/K32W1Config.h>
#include <set>
#include <string>

#include <unordered_map>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

/* TODO: adjust these values */
constexpr size_t kMaxNumberOfKeys  = 200;
constexpr size_t kMaxKeyValueBytes = 255;

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

uint16_t GetStringKeyId(const char * key, uint16_t * freeId)
{
    CHIP_ERROR err                    = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint8_t keyId                     = 0;
    uint8_t nvmIdKvsKey               = chip::DeviceLayer::Internal::K32WConfig::kFileId_KVSKey;
    bool bFreeIdxFound                = false;
    char keyString[kMaxKeyValueBytes] = { 0 };
    size_t keyStringSize              = 0;
    uint16_t nvmInternalId;

    for (keyId = 0; keyId < kMaxNumberOfKeys; keyId++)
    {
        nvmInternalId = chip::DeviceLayer::Internal::K32WConfigKey(nvmIdKvsKey, keyId);
        err =
            chip::DeviceLayer::Internal::K32WConfig::ReadConfigValueStr(nvmInternalId, keyString, kMaxKeyValueBytes, keyStringSize);

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

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err         = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint8_t nvmIdKvsValue  = chip::DeviceLayer::Internal::K32WConfig::kFileId_KVSValue;
    size_t read_bytes      = 0;
    uint8_t keyId          = 0;
    uint16_t nvmInternalId = 0;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    keyId = GetStringKeyId(key, NULL);

    if (keyId < kMaxNumberOfKeys)
    {
        // This is the ID of the actual data
        nvmInternalId = chip::DeviceLayer::Internal::K32WConfigKey(nvmIdKvsValue, keyId);
        ChipLogProgress(DeviceLayer, "KVS, get the value of Matter key [%s] with NVM id: %i", key, nvmInternalId);
        err = chip::DeviceLayer::Internal::K32WConfig::ReadConfigValueBin(nvmInternalId, (uint8_t *) value, value_size, read_bytes);

        // According to Get api read_bytes_size can be null
        if (read_bytes_size)
        {
            *read_bytes_size = read_bytes;
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "KVS, error in getting the value of Matter key [%s]. Key not found in persistent storage.",
                        key);
    }

exit:
    ConvertError(err);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err         = CHIP_ERROR_INVALID_ARGUMENT;
    bool_t putKey          = false;
    uint8_t nvmIdKvsKey    = chip::DeviceLayer::Internal::K32WConfig::kFileId_KVSKey;
    uint8_t nvmIdKvsValue  = chip::DeviceLayer::Internal::K32WConfig::kFileId_KVSValue;
    uint16_t nvmInternalId = 0;
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

    nvmInternalId = chip::DeviceLayer::Internal::K32WConfigKey(nvmIdKvsValue, keyId);
    ChipLogProgress(DeviceLayer, "KVS, save in flash the value of the Matter key [%s] with NVM id: %i", key, nvmInternalId);

    err = chip::DeviceLayer::Internal::K32WConfig::WriteConfigValueBin(nvmInternalId, (uint8_t *) value, value_size);

    /* save the 'key' in flash such that it can be retrieved later on */
    if (err == CHIP_NO_ERROR)
    {
        if (true == putKey)
        {
            nvmInternalId = chip::DeviceLayer::Internal::K32WConfigKey(nvmIdKvsKey, keyId);
            ChipLogProgress(DeviceLayer, "KVS, save in flash the Matter key [%s] with NVM id: %i", key, nvmInternalId);

            err = chip::DeviceLayer::Internal::K32WConfig::WriteConfigValueStr(nvmInternalId, key, strlen(key) + 1);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer, "KVS, Error while saving in flash the Matter key [%s] with NVM id: %i", key,
                                nvmInternalId);
            }
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "KVS, Error while saving in flash the value of the Matter key [%s] with NVM id: %i", key,
                        nvmInternalId);
    }

exit:
    ConvertError(err);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err         = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    uint8_t nvmIdKvsKey    = chip::DeviceLayer::Internal::K32WConfig::kFileId_KVSKey;
    uint8_t nvmIdKvsValue  = chip::DeviceLayer::Internal::K32WConfig::kFileId_KVSValue;
    uint8_t keyId          = 0;
    uint16_t nvmInternalId = 0;

    VerifyOrExit((key != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    keyId = GetStringKeyId(key, NULL);

    if (keyId < kMaxNumberOfKeys)
    {
        // entry exists so we can remove it
        nvmInternalId = chip::DeviceLayer::Internal::K32WConfigKey(nvmIdKvsKey, keyId);

        ChipLogProgress(DeviceLayer, "KVS, delete from flash the Matter key [%s] with NVM id: %i", key, nvmInternalId);
        err = chip::DeviceLayer::Internal::K32WConfig::ClearConfigValue(nvmInternalId);

        /* also delete the 'key string' from flash */
        if (err == CHIP_NO_ERROR)
        {
            nvmInternalId = chip::DeviceLayer::Internal::K32WConfigKey(nvmIdKvsValue, keyId);
            ChipLogProgress(DeviceLayer, "KVS, delete from flash the value of the Matter key [%s] with NVM id: %i", key,
                            nvmInternalId);

            err = chip::DeviceLayer::Internal::K32WConfig::ClearConfigValue(nvmInternalId);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogProgress(DeviceLayer,
                                "KVS, Error while deleting from flash the value of the Matter key [%s] with NVM id: %i", key,
                                nvmInternalId);
            }
        }
        else
        {
            ChipLogProgress(DeviceLayer, "KVS, Error while deleting from flash the Matter key [%s] with NVM id: %i", key,
                            nvmInternalId);
        }
    }
exit:
    ConvertError(err);
    return err;
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
