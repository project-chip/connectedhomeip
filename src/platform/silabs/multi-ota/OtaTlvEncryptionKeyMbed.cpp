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
#include "psa/crypto.h"

#include <string.h>

namespace chip {
namespace DeviceLayer {
namespace Silabs {

CHIP_ERROR OtaTlvEncryptionKey::Decrypt(const ByteSpan & key, MutableByteSpan & block, uint32_t & mIVOffset)
{
    uint8_t iv[16]           = { AU8IV_INIT_VALUE };

    // Set IV based on mIVOffset
    uint32_t counter = ((uint32_t) iv[12] << 24) | ((uint32_t) iv[13] << 16) | ((uint32_t) iv[14] << 8) | (uint32_t) iv[15];

    counter += (mIVOffset / 16);
    iv[12] = (counter >> 24) & 0xFF;
    iv[13] = (counter >> 16) & 0xFF;
    iv[14] = (counter >> 8) & 0xFF;
    iv[15] = counter & 0xFF;

    if (psa_crypto_init() != PSA_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Failed to initialize PSA Crypto");
        return CHIP_ERROR_INTERNAL;
    }
    // Key attributes
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attributes, key.size() * 8);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT);
    psa_set_key_algorithm(&attributes, PSA_ALG_CTR);

    // Load key
    psa_key_id_t key_id = PSA_KEY_ID_NULL;
    psa_status_t status = psa_import_key(&attributes, key.data(), key.size(), &key_id);
    if (PSA_SUCCESS != status)
    {
        ChipLogError(DeviceLayer, "Failed to set AES key");
        return CHIP_ERROR_INTERNAL;
    }

    // Operation
    psa_cipher_operation_t operation = PSA_CIPHER_OPERATION_INIT;
    status = psa_cipher_encrypt_setup(&operation, key_id, PSA_ALG_CTR);
    if (PSA_SUCCESS != status)
    {
        ChipLogError(DeviceLayer, "Failed to setup AES-CTR operation");
        psa_destroy_key(key_id);
        return CHIP_ERROR_INTERNAL;
    }

    // IV
    status = psa_cipher_set_iv(&operation, iv, 16);
    if (PSA_SUCCESS != status)
    {
        ChipLogError(DeviceLayer, "Failed to set AES-CTR IV");
        psa_cipher_abort(&operation);
        psa_destroy_key(key_id);
        return CHIP_ERROR_INTERNAL;
    }

    // Encrypt
    size_t output_len = 0;
    status = psa_cipher_update(&operation, 
                            block.data(), block.size(), 
                            block.data(), block.size(), 
                            &output_len);
    if (PSA_SUCCESS != status)
    {
        ChipLogError(DeviceLayer, "AES-CTR decryption failed");
        psa_cipher_abort(&operation);
        psa_destroy_key(key_id);
        return CHIP_ERROR_INTERNAL;
    }

    // Finalize
    size_t final_len = 0;
    psa_cipher_finish(&operation, block.data() + output_len, block.size() - output_len, &final_len);
    psa_destroy_key(key_id);    


    mIVOffset += block.size();

    ChipLogProgress(DeviceLayer, "Decryption complete");
    return CHIP_NO_ERROR;
}
} // namespace Silabs
} // namespace DeviceLayer
} // namespace chip
