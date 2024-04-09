/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "CHIPCryptoPALHsm_utils_trustm.h"
#include "optiga/optiga_util.h"
#include "optiga_crypt.h"
#include "optiga_lib_types.h"
#include <lib/core/CHIPEncoding.h>

namespace chip {
namespace Crypto {

extern CHIP_ERROR HMAC_SHA256_h(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                uint8_t * out_buffer, size_t out_length);

CHIP_ERROR HMAC_sha::HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                 uint8_t * out_buffer, size_t out_length)

{
#if !ENABLE_TRUSTM_HMAC_SHA256
    return HMAC_SHA256_h(key, key_length, message, message_length, out_buffer, out_length);
#else
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    uint16_t key_length_u16     = static_cast<uint16_t>(key_length);
    uint32_t message_length_u32 = static_cast<uint32_t>(message_length);
    uint32_t out_length_u32     = static_cast<uint32_t>(out_length);

    if (key_length > 64)
    {
        return HMAC_sha::HMAC_SHA256_h(key, key_length, message, message_length, out_buffer, out_length);
    }
    VerifyOrReturnError(key_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(message != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(message_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Trust M init
    trustm_Open();

    // Update the secret key
    write_data(TRUSTM_HMAC_OID_KEY, key, key_length_u16);

    // Start HMAC operation
    return_status = OPTIGA_LIB_BUSY;

    // What is the max length supported ?
    return_status = hmac_sha256(OPTIGA_HMAC_SHA_256, message, message_length_u32, out_buffer, &out_length_u32);

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

CHIP_ERROR HMAC_sha::HMAC_SHA256(const Hmac128KeyHandle & key, const uint8_t * message, size_t message_length, uint8_t * out_buffer,
                                 size_t out_length)
{
    return HMAC_SHA256(key.As<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray), message, message_length,
                       out_buffer, out_length);
}

} // namespace Crypto
} // namespace chip
