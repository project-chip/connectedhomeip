/*
 *
 * Copyright (c) 2024 Project CHIP Authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * HSM based implementation of CHIP crypto primitives
 * Based on configurations in CHIPCryptoPALHsm_config.h file,
 * chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_utils_trustm.h"
#include "optiga/optiga_util.h"
#include "optiga_crypt.h"
#include "optiga_lib_types.h"
#include <lib/core/CHIPEncoding.h>

namespace chip {
namespace Crypto {

extern CHIP_ERROR HKDF_SHA256_H(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length);

CHIP_ERROR HKDF_sha::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                 const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
#if !ENABLE_TRUSTM_HKDF_SHA256
    return HKDF_SHA256_H(secret, secret_length, salt, salt_length, info, info_length, out_buffer, out_length);
#else
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    uint16_t salt_length_u16   = static_cast<uint16_t>(salt_length);
    uint16_t info_length_u16   = static_cast<uint16_t>(info_length);
    uint16_t out_length_u16    = static_cast<uint16_t>(out_length);
    uint16_t secret_length_u16 = static_cast<uint16_t>(secret_length);

    if (salt_length > 64 || info_length > 80 || secret_length > 256 || out_length > 768)
    {
        /* Length not supported by trustm. Rollback to SW */
        return HKDF_SHA256_H(secret, secret_length, salt, salt_length, info, info_length, out_buffer, out_length);
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

    // Trust M init
    trustm_Open();

    // Write the secret key
    write_data(TRUSTM_HKDF_OID_KEY, secret, secret_length_u16);

    return_status = OPTIGA_LIB_BUSY;

    return_status = deriveKey_HKDF(salt, salt_length_u16, info, info_length_u16, out_length_u16, TRUE, out_buffer);

    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    error = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        trustm_close();
    }
    return error;
#endif
}
} // namespace Crypto
} // namespace chip
