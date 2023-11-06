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
 *          Platform-specific key value storage implementation for Tizen.
 */

/**
 * Note: Use public include for KeyValueStoreManager which includes our local
 *       platform/<PLATFORM>/KeyValueStoreManagerImpl.h after defining interface
 *       class. */
#include <platform/KeyValueStoreManager.h>

#include <cstddef>

#include <lib/core/CHIPError.h>

#include "AppPreference.h"

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Check(const char * key)
{
    return Internal::AppPreference::CheckData(key);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    return Internal::AppPreference::GetData(key, value, value_size, read_bytes_size, offset_bytes);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    return Internal::AppPreference::SaveData(key, value, value_size);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    return Internal::AppPreference::RemoveData(key);
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
