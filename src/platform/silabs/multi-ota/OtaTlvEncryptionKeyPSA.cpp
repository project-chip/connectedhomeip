/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <lib/support/CodeUtils.h>
#include <sl_psa_crypto.h>
#include <stdio.h>
#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

int destroyAESKey(uint32_t kid)
{
    psa_key_handle_t key_handle;

    int err = psa_open_key(kid, &key_handle);
    if (err)
    {
        psa_close_key(kid);
    }
    else
    {
        err = psa_destroy_key(kid);
    }
    return err;
}

CHIP_ERROR OtaTlvEncryptionKey::Import(const uint8_t * key, size_t key_len)
{
    destroyAESKey(mId);

    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_status_t status;

    psa_key_id_t key_id;
    psa_set_key_id(&attributes, mId);
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attributes, (kOTAEncryptionKeyLength * 8u));
    psa_set_key_algorithm(&attributes, PSA_ALG_CTR);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DECRYPT);

    status = psa_import_key(&attributes, key, key_len, &key_id);
    if (status != PSA_SUCCESS)
    {
        printf("Failed to import a key error:%ld\n", status);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaTlvEncryptionKey::Decrypt(MutableByteSpan & block, uint32_t & mIVOffset)
{
    uint8_t iv[16]                   = { AU8IV_INIT_VALUE };
    psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;
    psa_status_t status;
    uint8_t output[PSA_BLOCK_CIPHER_BLOCK_LENGTH(PSA_KEY_TYPE_AES)];
    size_t output_len;
    size_t total_output;
    uint32_t u32IVCount;
    uint32_t Offset = 0;

    u32IVCount = (((uint32_t) iv[12]) << 24) | (((uint32_t) iv[13]) << 16) | (((uint32_t) iv[14]) << 8) | (iv[15]);
    u32IVCount += (mIVOffset >> 4);

    iv[12] = (uint8_t) ((u32IVCount >> 24) & 0xff);
    iv[13] = (uint8_t) ((u32IVCount >> 16) & 0xff);
    iv[14] = (uint8_t) ((u32IVCount >> 8) & 0xff);
    iv[15] = (uint8_t) (u32IVCount & 0xff);

    while (Offset + 16 <= block.size())
    {
        status = psa_cipher_decrypt_setup(&operation, static_cast<psa_key_id_t>(mId), PSA_ALG_CTR);
        if (status != PSA_SUCCESS)
        {
            printf("Failed to begin cipher operation error:%ld\n", status);
            return CHIP_ERROR_INTERNAL;
        }

        status = psa_cipher_set_iv(&operation, iv, sizeof(iv));
        if (status != PSA_SUCCESS)
        {
            printf("Failed to set IV error:%ld\n", status);
            return CHIP_ERROR_INTERNAL;
        }

        status = psa_cipher_update(&operation, static_cast<uint8_t *>(&block[Offset]), 16, output, sizeof(output), &output_len);
        if (status != PSA_SUCCESS)
        {
            printf("Failed to update cipher operation error:%ld\n", status);
            return CHIP_ERROR_INTERNAL;
        }

        /* increment the IV for the next block  */
        u32IVCount++;

        iv[12] = (uint8_t) ((u32IVCount >> 24) & 0xff);
        iv[13] = (uint8_t) ((u32IVCount >> 16) & 0xff);
        iv[14] = (uint8_t) ((u32IVCount >> 8) & 0xff);
        iv[15] = (uint8_t) (u32IVCount & 0xff);

        memcpy((void *) &block[Offset], &output, output_len);

        Offset += 16; /* increment the buffer offset */
        mIVOffset += 16;
        status = psa_cipher_finish(&operation, output + total_output, sizeof(output) - total_output, &total_output);
        if (status != PSA_SUCCESS)
        {
            printf("Failed to finish cipher operation\n");
            return CHIP_ERROR_INTERNAL;
        }
    }

    printf("Decrypted ciphertext\n");

    psa_cipher_abort(&operation);

    return CHIP_NO_ERROR;
}

} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
