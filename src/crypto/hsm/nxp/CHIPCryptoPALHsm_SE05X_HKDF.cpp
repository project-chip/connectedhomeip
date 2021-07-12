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

#if ENABLE_HSM_HKDF_SHA256

namespace chip {
namespace Crypto {

HKDF_shaHSM::HKDF_shaHSM()
{
    keyid = kKeyId_hkdf_sha256_hmac_keyid;
}
HKDF_shaHSM::~HKDF_shaHSM() {}

CHIP_ERROR HKDF_shaHSM::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt,
                                    const size_t salt_length, const uint8_t * info, const size_t info_length, uint8_t * out_buffer,
                                    size_t out_length)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    if (salt_length > 64 || info_length > 80 || secret_length > 256 || out_length > 768)
    {
        /* Length not supported by se05x. Rollback to SW */
        return HKDF_sha::HKDF_SHA256(secret, secret_length, salt, salt_length, info, info_length, out_buffer, out_length);
    }

    // Salt is optional
    if (salt_length > 0)
    {
        VerifyOrReturnError(salt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrReturnError(info_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(info != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(secret != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(keyid != kKeyId_NotInitialized, CHIP_ERROR_HSM);

    se05x_sessionOpen();
    VerifyOrReturnError(gex_sss_chip_ctx.ks.session != NULL, CHIP_ERROR_INTERNAL);

    sss_object_t keyObject = { 0 };
    sss_status_t status    = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject, keyid, kSSS_KeyPart_Default, kSSS_CipherType_HMAC, secret_length,
                                            kKeyObject_Mode_Transient);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, secret, secret_length, secret_length * 8, NULL, 0);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    const smStatus_t smstatus = Se05x_API_HKDF_Extended(
        &((sss_se05x_session_t *) &gex_sss_chip_ctx.session)->s_ctx, keyObject.keyId, kSE05x_DigestMode_SHA256,
        kSE05x_HkdfMode_ExtractExpand, salt, salt_length, 0, info, info_length, 0, (uint16_t) out_length, out_buffer, &out_length);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);

    return error;
}

} // namespace Crypto
} // namespace chip

#endif //#if ENABLE_HSM_HKDF_SHA256
