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
#include <lib/core/CHIPEncoding.h>

#if ENABLE_HSM_PBKDF2_SHA256

namespace chip {
namespace Crypto {

PBKDF2_sha256HSM::PBKDF2_sha256HSM()
{
    keyid = kKeyId_pbkdf2_sha256_hmac_keyid;
}
PBKDF2_sha256HSM::~PBKDF2_sha256HSM() {}

CHIP_ERROR PBKDF2_sha256HSM::pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                           unsigned int iteration_count, uint32_t key_length, uint8_t * output)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    VerifyOrReturnError(password != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(plen > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(key_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(output != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(slen >= chip::Crypto::kMin_Salt_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(slen <= chip::Crypto::kMax_Salt_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(salt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(keyid != kKeyId_NotInitialized, CHIP_ERROR_HSM);

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    sss_object_t hmacKeyObj = {
        0,
    };
    sss_status_t status = sss_key_object_init(&hmacKeyObj, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&hmacKeyObj, keyid, kSSS_KeyPart_Default, kSSS_CipherType_HMAC, sizeof(password),
                                            kKeyObject_Mode_Transient);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &hmacKeyObj, password, plen, plen * 8, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    const smStatus_t smStatus =
        Se05x_API_PBKDF2(&((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyid, kSE05x_Pbkdf2_HMAC_SHA256, salt, slen,
                         (uint16_t) iteration_count, (uint16_t) key_length, output, (size_t *) &key_length);
    VerifyOrExit(smStatus == SM_OK, error = CHIP_ERROR_INTERNAL);
    error = CHIP_NO_ERROR;
exit:
    sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &hmacKeyObj);
    return error;
}

} // namespace Crypto
} // namespace chip

#endif //#if ENABLE_HSM_PBKDF2_SHA256
