/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          Provides an implementation of the key-value storage manager class
 *          for Open IOT SDK platform.
 */

#include <cstring>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMem.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/openiotsdk/KVPsaPsStore.h>

using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

CHIP_ERROR KeyValueStoreManagerImpl::ToKeyValueStoreManagerError(CHIP_ERROR err)
{
    return err == CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND ? CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND : err;
}

CHIP_ERROR KeyValueStoreManagerImpl::Init(void)
{
    return KVStoreConfig::Init();
}

CHIP_ERROR KeyValueStoreManagerImpl::Shutdown(void)
{
    return KVStoreConfig::Shutdown();
}

CHIP_ERROR
KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size, size_t offset)
{
    KVStoreKeyBuilder key_builder(key);
    if (!key_builder.GetKey())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    size_t outLen;
    CHIP_ERROR err = ToKeyValueStoreManagerError(
        KVStoreConfig::ReadConfigValueBin(key_builder.GetKey(), reinterpret_cast<uint8_t *>(value), value_size, outLen, offset));
    if (read_bytes_size)
    {
        *read_bytes_size = outLen;
    }
    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    KVStoreKeyBuilder key_builder(key);
    if (!key_builder.GetKey())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    CHIP_ERROR err = ToKeyValueStoreManagerError(KVStoreConfig::ClearConfigValue(key_builder.GetKey()));
    if (err == CHIP_NO_ERROR)
    {
        key_builder.RemoveKey();
    }

    return err;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    KVStoreKeyBuilder key_builder(key);
    if (!key_builder.GetKey())
    {
        return CHIP_ERROR_PERSISTED_STORAGE_FAILED;
    }

    CHIP_ERROR err = ToKeyValueStoreManagerError(
        KVStoreConfig::WriteConfigValueBin(key_builder.GetKey(), reinterpret_cast<const uint8_t *>(value), value_size));
    if (err == CHIP_NO_ERROR)
    {
        key_builder.AddKey();
    }

    return err;
}

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
