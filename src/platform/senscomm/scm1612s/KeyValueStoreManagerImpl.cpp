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
 *          Platform-specific key value storage implementation for scm1612s
 */

#include <platform/KeyValueStoreManager.h>

#include "wise_event_loop.h"
#include "wise_event.h"
#include "scm_fs.h"
#include "scm_wifi.h"
#include "wise_wifi_types.h"

/* ignore GCC Wconversion warnings for pigweed */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

// #include <pw_log/log.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

/**
 * SCM1612S KVS Implemetation
 */
CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    if (key == nullptr || value == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    int ret = scm_fs_read_config_value(kNamespace, key, static_cast<char *>(value), value_size);
    if (ret < 0)
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    if (read_bytes_size != nullptr)
        *read_bytes_size = static_cast<size_t>(ret);

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    if (key == nullptr || value == nullptr || value_size == 0)
        return CHIP_ERROR_INVALID_ARGUMENT;

    int ret = scm_fs_write_config_value(kNamespace, key, static_cast<const char *>(value), value_size);
    if (ret < 0)
        return CHIP_ERROR_WRITE_FAILED;

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    if (key == nullptr)
        return CHIP_ERROR_INVALID_ARGUMENT;

    int ret = scm_fs_remove_config_value(kNamespace, key);
    if (ret < 0)
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;

    return CHIP_NO_ERROR;
}

#define CONFIG_DIR "/config"

CHIP_ERROR KeyValueStoreManagerImpl::ErasePartition()
{
    int ret = scm_fs_format(CONFIG_DIR);
    if (ret < 0)
        return CHIP_ERROR_INTERNAL;

    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
