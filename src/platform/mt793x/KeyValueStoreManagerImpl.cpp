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
 *          Platform-specific key value storage implementation for Genio
 */

#include <platform/KeyValueStoreManager.h>

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
 * MT793x KVS Implemetation
 */
CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    CHIP_ERROR err;
    nvdm_status_t nvdm_status;

    nvdm_status = nvdm_read_data_item(kNamespace, key, (uint8_t *) value, (uint32_t *) &value_size);
    if (read_bytes_size)
    {
        *read_bytes_size = value_size;
    }
    err = MapNvdmStatus(nvdm_status);

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err;
    nvdm_status_t nvdm_status;

    nvdm_status = nvdm_write_data_item(kNamespace, key, NVDM_DATA_ITEM_TYPE_RAW_DATA, (uint8_t *) value, value_size);
    err         = MapNvdmStatus(nvdm_status);

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err;
    nvdm_status_t nvdm_status;

    nvdm_status = nvdm_delete_data_item(kNamespace, key);
    err         = MapNvdmStatus(nvdm_status);

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::ErasePartition()
{
    CHIP_ERROR err;
    nvdm_status_t nvdm_status;

    nvdm_status = nvdm_delete_group(kNamespace);
    err         = MapNvdmStatus(nvdm_status);

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::MapNvdmStatus(nvdm_status_t nvdm_status)
{
    CHIP_ERROR err;

    //    NVDM_STATUS_INVALID_PARAMETER = -5,  /**< The user parameter is invalid. */
    //    NVDM_STATUS_ITEM_NOT_FOUND = -4,     /**< The data item wasn't found by the NVDM. */
    //    NVDM_STATUS_INSUFFICIENT_SPACE = -3, /**< No space is available in the flash. */
    //    NVDM_STATUS_INCORRECT_CHECKSUM = -2, /**< The NVDM found a checksum error when reading the data item. */
    //    NVDM_STATUS_ERROR = -1,              /**< An unknown error occurred. */
    //    NVDM_STATUS_OK = 0,                  /**< The operation was successful. */

    switch (nvdm_status)
    {
    case NVDM_STATUS_OK:
        err = CHIP_NO_ERROR;
        break;
    case NVDM_STATUS_ITEM_NOT_FOUND:
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
        break;
    case NVDM_STATUS_INCORRECT_CHECKSUM:
        err = CHIP_ERROR_INTEGRITY_CHECK_FAILED;
        break;
    case NVDM_STATUS_INSUFFICIENT_SPACE:
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
        break;
    case NVDM_STATUS_INVALID_PARAMETER:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    case NVDM_STATUS_ERROR:
        err = CHIP_ERROR_INTERNAL;
        break;
    default:
        err = CHIP_ERROR_INTERNAL;
        break;
    }

    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
