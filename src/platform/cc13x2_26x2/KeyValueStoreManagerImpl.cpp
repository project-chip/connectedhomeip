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
 *          Platform-specific key value storage implementation for CC1352
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/KeyValueStoreManager.h>

#include <platform/cc13x2_26x2/CC13X2_26X2Config.h>

namespace chip {
namespace DeviceLayer {
namespace PersistedStorage {

using namespace ::chip::DeviceLayer::Internal;

KeyValueStoreManagerImpl KeyValueStoreManagerImpl::sInstance;

/**
 * Calculate crc16 CCITT from the c string key.
 *
 * This could be optimized because key is probably a printable string and not a
 * binary array. The length is limited to 10 bits because the maximum value of
 * an NV sub ID is 10 bits long. Consult
 * `<simplelink_sdk>/source/ti/common/nv/nvocmp.c:NVOCMP_MAXSUBID`.
 */
static uint16_t calc_key_10(const char * key)
{
    uint16_t ret = 0xFFFF;

    while (*key != '\0')
    {
        char x;
        x = ret >> 0x8 ^ *key++;
        x ^= x >> 0x4;
        ret = (ret << 0x8) ^ ((uint16_t)(x << 0xC)) ^ ((uint16_t)(x << 0x5)) ^ ((uint16_t) x);
    }

    ret &= 0x03FF;
    return ret;
}

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);

    return CC13X2_26X2Config::ReadKVS(calc_key_10(key), value, value_size, read_bytes_size, offset_bytes);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value_size > 0, CHIP_ERROR_INVALID_ARGUMENT);

    return CC13X2_26X2Config::WriteKVS(calc_key_10(key), value, value_size);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    return CC13X2_26X2Config::ClearKVS(calc_key_10(key));
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
