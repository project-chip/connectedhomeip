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
#include <platform/nxp/k32w/k32w0/K32W0Config.h>
#include <platform/KeyValueStoreManager.h>
#include <set>
#include <string>

#include "PDM.h"

#include <unordered_map>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

/* TODO: adjust these values */
constexpr size_t kMaxNumberOfKeys  = 20;
constexpr size_t kMaxKeyValueBytes = 255;

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

/* hashmap having:
 * 	- the matter key string as key;
 * 	- internal PDM identifier as value;
 */
std::unordered_map<std::string, uint8_t> g_kvs_map;

/* set containing used PDM identifiers */
std::set<uint8_t> g_key_ids_set;

/* used to check if we need to restore values from flash (e.g.: reset) */
static bool g_restored_from_flash = false;

CHIP_ERROR RestoreFromFlash()
{
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    uint8_t key_id                        = 0;
    char key_string_id[kMaxKeyValueBytes] = { 0 };
    size_t key_string_id_size             = 0;
    uint8_t pdm_id_kvs                    = chip::DeviceLayer::Internal::K32WConfig::kPDMId_KVS;

    if (g_restored_from_flash)
    {
        /* already restored from flash, nothing to do */
        return err;
    }

    for (key_id = 0; key_id < kMaxNumberOfKeys; key_id++)
    {
        /* key was saved as string in flash (key_string_id) using (key_id + kMaxNumberOfKeys) as PDM key
         * value corresponding to key_string_id was saved in flash using key_id as PDM key
         */

        err = chip::DeviceLayer::Internal::K32WConfig::ReadConfigValueStr(
            chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id + kMaxNumberOfKeys), key_string_id, kMaxKeyValueBytes,
            key_string_id_size);

        if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
        {
            err = CHIP_NO_ERROR;
            continue;
        }
        else if (err != CHIP_NO_ERROR)
        {
            break;
        }

        if (key_string_id_size)
        {
            g_key_ids_set.insert(key_id);
            g_kvs_map.insert(std::make_pair(std::string(key_string_id), key_id));
            key_string_id_size = 0;

            ChipLogProgress(DeviceLayer, "KVS, restored key [%s] from flash with PDM key: %i", key_string_id, key_id);
        }
    }

    g_restored_from_flash = true;

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err     = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    uint8_t pdm_id_kvs = chip::DeviceLayer::Internal::K32WConfig::kPDMId_KVS;
    std::unordered_map<std::string, uint8_t>::const_iterator it;
    size_t read_bytes = 0;
    uint8_t key_id    = 0;

    VerifyOrExit((key != NULL) && (value != NULL) && (RestoreFromFlash() == CHIP_NO_ERROR), err = CHIP_ERROR_INVALID_ARGUMENT);

    if ((it = g_kvs_map.find(key)) != g_kvs_map.end())
    {
        key_id = it->second;

        err = chip::DeviceLayer::Internal::K32WConfig::ReadConfigValueBin(
            chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id), (uint8_t *) value, value_size, read_bytes);
        *read_bytes_size = read_bytes;

        ChipLogProgress(DeviceLayer, "KVS, get key [%s] with PDM key: %i", key, key_id);
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
    uint8_t key_id;
    bool_t put_key     = false;
    uint8_t pdm_id_kvs = chip::DeviceLayer::Internal::K32WConfig::kPDMId_KVS;
    std::unordered_map<std::string, uint8_t>::const_iterator it;

    VerifyOrExit((key != NULL) && (value != NULL) && (RestoreFromFlash() == CHIP_NO_ERROR), err = CHIP_ERROR_INVALID_ARGUMENT);

    if ((it = g_kvs_map.find(key)) == g_kvs_map.end()) /* new key */
    {
        for (key_id = 0; key_id < kMaxNumberOfKeys; key_id++)
        {
            std::set<uint8_t>::iterator iter = std::find(g_key_ids_set.begin(), g_key_ids_set.end(), key_id);

            if (iter == g_key_ids_set.end())
            {
                assert(g_key_ids_set.size() < kMaxNumberOfKeys);
                g_key_ids_set.insert(key_id);

                put_key = true;
                break;
            }
        }
    }
    else /* overwrite key */
    {
        put_key = true;
        key_id  = it->second;
    }

    if (put_key)
    {
        ChipLogProgress(DeviceLayer, "KVS, put key [%s] with PDM key: %i", key, key_id);

        g_kvs_map.insert(std::make_pair(std::string(key), key_id));
        err = chip::DeviceLayer::Internal::K32WConfig::WriteConfigValueBin(
            chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id), (uint8_t *) value, value_size);

        /* save the 'key' in flash such that it can be retrieved later on */
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "KVS, save in flash key [%s] with PDM key: %i", key, key_id + kMaxNumberOfKeys);
            err = chip::DeviceLayer::Internal::K32WConfig::WriteConfigValueStr(
                chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id + kMaxNumberOfKeys), key, strlen(key));
        }
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND;
    std::unordered_map<std::string, uint8_t>::const_iterator it;
    uint8_t pdm_id_kvs = chip::DeviceLayer::Internal::K32WConfig::kPDMId_KVS;
    uint8_t key_id     = 0;

    VerifyOrExit((key != NULL) && (RestoreFromFlash() == CHIP_NO_ERROR), err = CHIP_ERROR_INVALID_ARGUMENT);

    if ((it = g_kvs_map.find(key)) != g_kvs_map.end())
    {
        key_id = it->second;
        g_key_ids_set.erase(key_id);
        g_kvs_map.erase(it);

        ChipLogProgress(DeviceLayer, "KVS, delete key [%s] with PDM key: %i", key, key_id);
        err = chip::DeviceLayer::Internal::K32WConfig::ClearConfigValue(
            chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id));

        /* also delete the 'key string' from flash */
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "KVS, delete key [%s] with PDM key: %i", key, key_id + kMaxNumberOfKeys);
            err = chip::DeviceLayer::Internal::K32WConfig::ClearConfigValue(
                chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id + kMaxNumberOfKeys));
        }
    }

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
