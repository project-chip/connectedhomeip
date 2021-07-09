/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_SE05X_utils.h"
#include <core/CHIPEncoding.h>

#if ENABLE_HSM_HMAC_SHA256

#define MAX_MAC_ONE_SHOT_DATA_LEN 900

namespace chip {
namespace Crypto {

HMAC_shaHSM::HMAC_shaHSM()
{
    keyid = kKeyId_hmac_sha256_keyid;
}
HMAC_shaHSM::~HMAC_shaHSM() {}

CHIP_ERROR HMAC_shaHSM::HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                    uint8_t * out_buffer, size_t out_length)

{
    sss_mac_t ctx_mac      = { 0 };
    sss_object_t keyObject = { 0 };

    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(key_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(message != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(message_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length >= CHIP_CRYPTO_HASH_LEN_BYTES, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    if (key_length > 256)
    {
        return HMAC_sha::HMAC_SHA256(key, key_length, message, message_length, out_buffer, out_length);
    }

    VerifyOrReturnError(keyid != kKeyId_NotInitialized, CHIP_ERROR_HSM);

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    sss_status_t status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject, keyid, kSSS_KeyPart_Default, kSSS_CipherType_HMAC, key_length,
                                            kKeyObject_Mode_Transient);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, key, key_length, key_length * 8, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_mac_context_init(&ctx_mac, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_HMAC_SHA256, kMode_SSS_Mac);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    if (message_length <= MAX_MAC_ONE_SHOT_DATA_LEN)
    {
        status = sss_mac_one_go(&ctx_mac, message, message_length, out_buffer, &out_length);
        VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    }
    else
    {
        /* Calculate MAC using multistep calls */
        size_t datalenTemp = 0;
        size_t rem_len     = message_length;

        status = sss_mac_init(&ctx_mac);
        VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

        while (rem_len > 0)
        {
            datalenTemp = (rem_len > MAX_MAC_ONE_SHOT_DATA_LEN) ? MAX_MAC_ONE_SHOT_DATA_LEN : rem_len;
            status      = sss_mac_update(&ctx_mac, (message + (message_length - rem_len)), datalenTemp);
            VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
            rem_len = rem_len - datalenTemp;
        }

        status = sss_mac_finish(&ctx_mac, out_buffer, &out_length);
        VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    }

    status = sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    if (ctx_mac.session != NULL)
    {
        sss_mac_context_free(&ctx_mac);
    }

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip

#endif //#if ENABLE_HSM_HMAC_SHA256
