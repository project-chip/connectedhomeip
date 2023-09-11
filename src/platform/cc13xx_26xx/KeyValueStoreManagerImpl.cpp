/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Platform-specific key value storage implementation for CC1352
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <string.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/cc13xx_26xx/CC13XX_26XXConfig.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

using namespace ::chip::DeviceLayer::Internal;

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    CHIP_ERROR err;

    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    if (0U != value_size)
    {
        VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);
    }

    err = CC13XX_26XXConfig::ReadKVS(key, value, value_size, read_bytes_size, offset_bytes);

    if (err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND)
    {
        err = CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);

    return CC13XX_26XXConfig::WriteKVS(key, value, value_size);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    return CC13XX_26XXConfig::ClearKVS(key);
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
