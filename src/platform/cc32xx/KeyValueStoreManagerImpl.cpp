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
 *          Platform-specific key value storage implementation for CC32XX
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <string.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/cc32xx/CC32XXConfig.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

using namespace ::chip::DeviceLayer::Internal;

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    if (0U != value_size)
    {
        VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);
    }

    return CC32XXConfig::ReadKVS(key, value, value_size, read_bytes_size, offset_bytes);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);

    return CC32XXConfig::WriteKVS(key, value, value_size);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    return CC32XXConfig::ClearKVS(key);
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
