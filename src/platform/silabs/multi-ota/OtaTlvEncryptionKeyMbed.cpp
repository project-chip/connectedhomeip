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

#include "OtaTlvEncryptionKey.h"

#include "mbedtls/aes.h"

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

CHIP_ERROR OtaTlvEncryptionKey::Decrypt(const ByteSpan & key, MutableByteSpan & block, uint32_t & mIVOffset)
{
    uint8_t iv[16]           = { AU8IV_INIT_VALUE };
    uint8_t stream_block[16] = { 0 };
    size_t nc_off            = 0;

    // Set IV based on mIVOffset
    uint32_t counter = ((uint32_t) iv[12] << 24) | ((uint32_t) iv[13] << 16) | ((uint32_t) iv[14] << 8) | (uint32_t) iv[15];

    counter += (mIVOffset / 16);
    iv[12] = (counter >> 24) & 0xFF;
    iv[13] = (counter >> 16) & 0xFF;
    iv[14] = (counter >> 8) & 0xFF;
    iv[15] = counter & 0xFF;

    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);

    if (mbedtls_aes_setkey_enc(&aes_ctx, key.data(), (key.size() * 8)) != 0)
    {
        ChipLogError(DeviceLayer, "Failed to set AES key");
        mbedtls_aes_free(&aes_ctx);
        return CHIP_ERROR_INTERNAL;
    }

    // Decrypt
    if (mbedtls_aes_crypt_ctr(&aes_ctx, block.size(), &nc_off, iv, stream_block, block.data(), block.data()) != 0)
    {
        ChipLogError(DeviceLayer, "AES-CTR decryption failed");
        mbedtls_aes_free(&aes_ctx);
        return CHIP_ERROR_INTERNAL;
    }

    mIVOffset += block.size();

    ChipLogProgress(DeviceLayer, "Decryption complete");
    mbedtls_aes_free(&aes_ctx);
    return CHIP_NO_ERROR;
}
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
