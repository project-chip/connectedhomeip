/*
 *
 *    Copyright (c) 2023-2025 Project CHIP Authors
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

#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/silabs/multi-ota/OTATlvProcessor.h>
#include <psa/crypto.h>
#include <stddef.h>
#include <stdint.h>

#define AU8IV_INIT_VALUE 0x00, 0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x00, 0x00, 0x00, 0x00

namespace chip {
namespace DeviceLayer {
namespace Silabs {
class OtaTlvEncryptionKey
{
public:
    static constexpr uint32_t kAES_KeyId_Default    = (PSA_KEY_ID_USER_MIN + 2);
    static constexpr size_t kOTAEncryptionKeyLength = 128u / 8u; // 128 bits KeyLength expressed in bytes.

    OtaTlvEncryptionKey(uint32_t id = 0) { mId = (id > 0) ? id : kAES_KeyId_Default; }
    ~OtaTlvEncryptionKey() = default;

#if defined(SL_MBEDTLS_USE_TINYCRYPT)
    static CHIP_ERROR Decrypt(const ByteSpan & key, MutableByteSpan & block, uint32_t & mIVOffset);
#else  // SL_MBEDTLS_USE_PSA_CRYPTO
    uint32_t GetId() { return mId; }
    CHIP_ERROR Import(const uint8_t * key, size_t key_len);
    CHIP_ERROR Decrypt(MutableByteSpan & block, uint32_t & mIVOffset);
#endif // SL_MBEDTLS_USE_TINYCRYPT

protected:
    uint32_t mId = 0;
};

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
