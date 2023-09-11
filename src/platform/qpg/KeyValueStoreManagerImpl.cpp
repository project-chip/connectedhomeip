/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific key value storage implementation
 *          for Qorvo QPG platforms
 */

#include <lib/support/CodeUtils.h>
#include <platform/KeyValueStoreManager.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qvStatus_t result;
    size_t actual_read_bytes;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    result = qvCHIP_KvsGet(key, value, value_size, &actual_read_bytes, offset_bytes);
    if (result == QV_STATUS_BUFFER_TOO_SMALL)
    {
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    else if (result != QV_STATUS_NO_ERROR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    if (read_bytes_size)
    {
        *read_bytes_size = actual_read_bytes;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qvStatus_t result;

    VerifyOrExit((key != NULL) && (value != NULL), err = CHIP_ERROR_INVALID_ARGUMENT);

    result = qvCHIP_KvsPut(key, value, value_size);
    if (result != QV_STATUS_NO_ERROR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

exit:
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    qvStatus_t result;

    VerifyOrExit(key != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    result = qvCHIP_KvsDelete(key);
    if (result != QV_STATUS_NO_ERROR)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

exit:
    return err;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
