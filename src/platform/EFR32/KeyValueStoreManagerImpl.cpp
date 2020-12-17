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
 *          Platform-specific key value storage implementation for EFR32
 */

#include <platform/KeyValueStoreManager.h>

/* ignore GCC Wconversion warnings for pigweed */
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include <pw_log/log.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

#if defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    assert(CHIP_KVS_AVAILABLE);
    auto status_and_size = mKvs.Get(key, std::span<std::byte>(reinterpret_cast<std::byte *>(value), value_size), offset_bytes);
    if (read_bytes_size)
    {
        *read_bytes_size = status_and_size.size();
    }
    switch (status_and_size.status().code())
    {
    case pw::Status::OK:
        return CHIP_NO_ERROR;
    case pw::Status::NOT_FOUND:
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    case pw::Status::DATA_LOSS:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    case pw::Status::RESOURCE_EXHAUSTED:
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    case pw::Status::FAILED_PRECONDITION:
        return CHIP_ERROR_WELL_UNINITIALIZED;
    case pw::Status::INVALID_ARGUMENT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    default:
        break;
    }
    return CHIP_ERROR_INTERNAL; // Unexpected KVS status.
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    assert(CHIP_KVS_AVAILABLE);
    auto status = mKvs.Put(key, std::span<const std::byte>(reinterpret_cast<const std::byte *>(value), value_size));
    switch (status.code())
    {
    case pw::Status::OK:
        return CHIP_NO_ERROR;
    case pw::Status::DATA_LOSS:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    case pw::Status::RESOURCE_EXHAUSTED:
    case pw::Status::ALREADY_EXISTS:
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    case pw::Status::FAILED_PRECONDITION:
        return CHIP_ERROR_WELL_UNINITIALIZED;
    case pw::Status::INVALID_ARGUMENT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    default:
        break;
    }
    return CHIP_ERROR_INTERNAL; // Unexpected KVS status.
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    assert(CHIP_KVS_AVAILABLE);
    auto status = mKvs.Delete(key);
    switch (status.code())
    {
    case pw::Status::OK:
        return CHIP_NO_ERROR;
    case pw::Status::NOT_FOUND:
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    case pw::Status::DATA_LOSS:
        return CHIP_ERROR_INTEGRITY_CHECK_FAILED;
    case pw::Status::RESOURCE_EXHAUSTED:
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    case pw::Status::FAILED_PRECONDITION:
        return CHIP_ERROR_WELL_UNINITIALIZED;
    case pw::Status::INVALID_ARGUMENT:
        return CHIP_ERROR_INVALID_ARGUMENT;
    default:
        break;
    }
    return CHIP_ERROR_INTERNAL; // Unexpected KVS status.
}

CHIP_ERROR KeyValueStoreManagerImpl::ErasePartition()
{
    assert(CHIP_KVS_AVAILABLE);
    auto status = mKvsPartition.Erase();
    switch (status.code())
    {
    case pw::Status::OK:
        return CHIP_NO_ERROR;
    case pw::Status::DEADLINE_EXCEEDED:
        return CHIP_ERROR_TIMEOUT;
    case pw::Status::PERMISSION_DENIED:
        return CHIP_ERROR_ACCESS_DENIED;
    default:
        break;
    }
    return CHIP_ERROR_INTERNAL; // Unexpected KVS status.
}
#endif // defined(CHIP_KVS_AVAILABLE) && CHIP_KVS_AVAILABLE

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
