/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          Platform-specific key value storage implementation for ESP32
 */

#include <platform/ESP32/ScopedNvsHandle.h>
#include <platform/KeyValueStoreManager.h>

#include <string.h>

#include "nvs.h"
#include "nvs_flash.h"
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

namespace {
// Implementation Note: esp-idf nvs implementation cannot handle key length > 15,
// Below implementation tries to handle that case by hashing the key
// If key length is > 15 then take the SHA1 of the key and convert the first 7.5 bytes to hex string.
// Not sure how likely we would run into a conflict as we are only using 8 bytes out of 20
//
// key returned by below function will not collide with any existing "normal" keys because those always have a "/" in
// the first few chars and the output of this code never will.
//
// Returns true if key is hashed, false otherwise.
bool HashIfLongKey(const char * key, char * keyHash)
{
    ReturnErrorCodeIf(strlen(key) < NVS_KEY_NAME_MAX_SIZE, false);

    uint8_t hashBuffer[chip::Crypto::kSHA1_Hash_Length];
    ReturnErrorCodeIf(Crypto::Hash_SHA1(Uint8::from_const_char(key), strlen(key), hashBuffer) != CHIP_NO_ERROR, false);

    BitFlags<Encoding::HexFlags> flags(Encoding::HexFlags::kNone);
    Encoding::BytesToHex(hashBuffer, NVS_KEY_NAME_MAX_SIZE / 2, keyHash, NVS_KEY_NAME_MAX_SIZE, flags);
    keyHash[NVS_KEY_NAME_MAX_SIZE - 1] = 0;

    ChipLogDetail(DeviceLayer, "Using hash:%s for nvs key:%s", keyHash, StringOrNullMarker(key));
    return true;
}
} // namespace

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes)
{
    // value may be NULL when checking whether the key exists

    // Offset and partial reads are not supported in nvs, for now just return NOT_IMPLEMENTED. Support can be added in the
    // future if this is needed.
    VerifyOrReturnError(offset_bytes == 0, CHIP_ERROR_NOT_IMPLEMENTED);

    Internal::ScopedNvsHandle handle;
    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READONLY, CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION));

    char keyHash[NVS_KEY_NAME_MAX_SIZE];
    VerifyOrDo(HashIfLongKey(key, keyHash) == false, key = keyHash);

    ReturnMappedErrorOnFailure(nvs_get_blob(handle, key, value, &value_size));

    if (read_bytes_size)
    {
        *read_bytes_size = value_size;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);

    Internal::ScopedNvsHandle handle;
    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READWRITE, CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION));

    char keyHash[NVS_KEY_NAME_MAX_SIZE];
    VerifyOrDo(HashIfLongKey(key, keyHash) == false, key = keyHash);

    ReturnMappedErrorOnFailure(nvs_set_blob(handle, key, value, value_size));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    Internal::ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READWRITE, CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION));

    char keyHash[NVS_KEY_NAME_MAX_SIZE];
    VerifyOrDo(HashIfLongKey(key, keyHash) == false, key = keyHash);

    ReturnMappedErrorOnFailure(nvs_erase_key(handle, key));

    // Commit the value to the persistent store.
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    return CHIP_NO_ERROR;
}

CHIP_ERROR KeyValueStoreManagerImpl::EraseAll(void)
{
    Internal::ScopedNvsHandle handle;

    ReturnErrorOnFailure(handle.Open(kNamespace, NVS_READWRITE, CHIP_DEVICE_CONFIG_CHIP_KVS_NAMESPACE_PARTITION));
    ReturnMappedErrorOnFailure(nvs_erase_all(handle));
    ReturnMappedErrorOnFailure(nvs_commit(handle));

    return CHIP_NO_ERROR;
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
