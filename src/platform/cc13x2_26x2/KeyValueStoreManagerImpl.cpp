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

#include <string.h>

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
 * Code generated with the following commands. Output was modified to change
 * the type `crc_t` to `uint16_t`.
 *
 * ```
 * $ git clone https://github.com/tpircher/pycrc
 * $ python3 pycrc/pycrc.py --generate c --algorithm=bbf --model=crc-16-ccitt
 * ```
 *
 * Generated on Wed Jun  9 17:09:19 2021
 * by pycrc v0.9.2, https://pycrc.org
 * using the configuration:
 *  - Width         = 16
 *  - Poly          = 0x1021
 *  - XorIn         = 0x1d0f
 *  - ReflectIn     = False
 *  - XorOut        = 0x0000
 *  - ReflectOut    = False
 *  - Algorithm     = bit-by-bit-fast
 */
uint16_t crc_update(uint16_t crc, const void * data, size_t data_len)
{
    const unsigned char * d = (const unsigned char *) data;
    unsigned int i;
    bool bit;
    unsigned char c;

    while (data_len--)
    {
        c = *d++;
        for (i = 0x80; i > 0; i >>= 1)
        {
            bit = crc & 0x8000;
            if (c & i)
            {
                bit = !bit;
            }
            crc <<= 1;
            if (bit)
            {
                crc ^= 0x1021;
            }
        }
        crc &= 0xffff;
    }
    return crc & 0xffff;
}

#define CRC_INITIAL (0xFFFF)

CHIP_ERROR KeyValueStoreManagerImpl::_Get(const char * key, void * value, size_t value_size, size_t * read_bytes_size,
                                          size_t offset_bytes) const
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);

    return CC13X2_26X2Config::ReadKVS(crc_update(CRC_INITIAL, key, strlen(key)), value, value_size, read_bytes_size, offset_bytes);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Put(const char * key, const void * value, size_t value_size)
{
    VerifyOrReturnError(key, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(value_size > 0, CHIP_ERROR_INVALID_ARGUMENT);

    return CC13X2_26X2Config::WriteKVS(crc_update(CRC_INITIAL, key, strlen(key)), value, value_size);
}

CHIP_ERROR KeyValueStoreManagerImpl::_Delete(const char * key)
{
    return CC13X2_26X2Config::ClearKVS(crc_update(CRC_INITIAL, key, strlen(key)));
}

} // namespace PersistedStorage
} // namespace DeviceLayer
} // namespace chip
