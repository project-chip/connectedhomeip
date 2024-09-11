/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          Platform-specific key value storage implementation for NXP platforms
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "NXPConfig.h"
#include "lib/support/CHIPMem.h"
#include <platform/KeyValueStoreManager.h>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    size_t read_bytes;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, "KVS, get key id:: %s", key);

    err = chip::DeviceLayer::Internal::NXPConfig::ReadConfigValueBin(key, (uint8_t *) value, value_size, read_bytes);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        ChipLogError(DeviceLayer, "KVS, key not found!");
    }
    else if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "KVS, failed to read key!");
    }

    if (read_bytes_size)
    {
        *read_bytes_size = read_bytes;
    }

exit:
    ConvertError(err);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, "KVS, put key id:: %s ", key);

    err = chip::DeviceLayer::Internal::NXPConfig::WriteConfigValueBin(key, (uint8_t *) value, value_size);

    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "KVS, failed to save key!");

exit:
    ConvertError(err);
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(key != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogProgress(DeviceLayer, "KVS, deleting key id:: %s", key);

    err = chip::DeviceLayer::Internal::NXPConfig::ClearConfigValue(key);

    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "KVS, failed to delete key!");

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
