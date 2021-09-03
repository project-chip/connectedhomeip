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

/* TODO: adjust this value */
#define MAX_NO_OF_KEYS 20

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

/* hashmap having:
 * 	- the matter key string as key;
 * 	- internal PDM identifier as value;
 */
std::unordered_map<std::string, uint8_t> g_kvs_map;

/* list containing used PDM identifiers */
std::list<uint8_t> g_key_ids_list;

/* max no of bytes for a key */
#define MAX_KEY_VALUE 255

/* used to check if we need to restore values from flash (e.g.: reset) */
static bool g_restored_from_flash = FALSE;

CHIP_ERROR RestoreFromFlash()
{
    CHIP_ERROR err                    = CHIP_NO_ERROR;
    uint8_t key_id                    = 0;
    char key_string_id[MAX_KEY_VALUE] = { 0 };
    size_t key_string_id_size         = 0;
    uint8_t pdm_id_kvs                = chip::DeviceLayer::Internal::K32WConfig::kPDMId_KVS;

    if (g_restored_from_flash)
    {
        /* already restored from flash, nothing to do */
        return err;
    }

    for (key_id = 0; key_id < MAX_NO_OF_KEYS; key_id++)
    {
        /* key was saved as string in flash (key_string_id) using (key_id + MAX_NO_OF_KEYS) as PDM key
         * value corresponding to key_string_id was saved in flash using key_id as PDM key
         */

        err = chip::DeviceLayer::Internal::K32WConfig::ReadConfigValueStr(
            chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id + MAX_NO_OF_KEYS), key_string_id, MAX_KEY_VALUE,
            key_string_id_size);

        if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
        {
            continue;
        }

        if (key_string_id_size)
        {
            g_key_ids_list.push_back(key_id);
            g_kvs_map.insert(std::make_pair(std::string(key_string_id), key_id));
            key_string_id_size = 0;

            ChipLogProgress(DeviceLayer, "KVS, restored key [%s] from flash with PDM key: %i", key_string_id, key_id);
        }
    }

    g_restored_from_flash = TRUE;

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
    bool_t put_key     = FALSE;
    uint8_t pdm_id_kvs = chip::DeviceLayer::Internal::K32WConfig::kPDMId_KVS;
    std::unordered_map<std::string, uint8_t>::const_iterator it;

    VerifyOrExit((key != NULL) && (value != NULL) && (RestoreFromFlash() == CHIP_NO_ERROR), err = CHIP_ERROR_INVALID_ARGUMENT);

    if ((it = g_kvs_map.find(key)) == g_kvs_map.end()) /* new key */
    {
        for (key_id = 0; key_id < MAX_NO_OF_KEYS; key_id++)
        {
            std::list<uint8_t>::iterator iter = std::find(g_key_ids_list.begin(), g_key_ids_list.end(), key_id);

            if (iter == g_key_ids_list.end())
            {
                g_key_ids_list.push_back(key_id);

                put_key = TRUE;
                break;
            }
        }
    }
    else /* overwrite key */
    {
        put_key = TRUE;
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
            ChipLogProgress(DeviceLayer, "KVS, save in flash key [%s] with PDM key: %i", key, key_id + MAX_NO_OF_KEYS);
            err = chip::DeviceLayer::Internal::K32WConfig::WriteConfigValueStr(
                chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id + MAX_NO_OF_KEYS), key, strlen(key));
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
        g_key_ids_list.remove(key_id);
        g_kvs_map.erase(it);

        ChipLogProgress(DeviceLayer, "KVS, delete key [%s] with PDM key: %i", key, key_id);
        err = chip::DeviceLayer::Internal::K32WConfig::ClearConfigValue(
            chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id));

        /* also delete the 'key string' from flash */
        if (err == CHIP_NO_ERROR)
        {
            ChipLogProgress(DeviceLayer, "KVS, delete key [%s] with PDM key: %i", key, key_id + MAX_NO_OF_KEYS);
            err = chip::DeviceLayer::Internal::K32WConfig::ClearConfigValue(
                chip::DeviceLayer::Internal::K32WConfigKey(pdm_id_kvs, key_id + MAX_NO_OF_KEYS));
        }
    }

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
