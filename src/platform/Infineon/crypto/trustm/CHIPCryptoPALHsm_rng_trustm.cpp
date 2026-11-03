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
#include <stdint.h>
#include <string.h>

#if ENABLE_TRUSTM_RANDOM

namespace chip {
namespace Crypto {

namespace {
// Trust M optiga_crypt_rng requires a minimum of 8 bytes per request.
constexpr uint16_t kTrustMRngMin = 8;
} // namespace

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    CHIP_ERROR error                  = CHIP_ERROR_INTERNAL;
    optiga_lib_status_t return_status = OPTIGA_LIB_BUSY;

    ChipLogDetail(Crypto, "Random Number: Using TrustM for Random Number Generation !");

    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length <= UINT16_MAX, CHIP_ERROR_INVALID_ARGUMENT);

    // Trust M init
    return_status = trustm_Open();
    VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);

    if (out_length < kTrustMRngMin)
    {
        uint8_t scratch[kTrustMRngMin];
        return_status = optiga_crypt_rng(scratch, kTrustMRngMin);
        VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
        memcpy(out_buffer, scratch, out_length);
        ClearSecretData(scratch, sizeof(scratch));
    }
    else
    {
        return_status = optiga_crypt_rng(out_buffer, static_cast<uint16_t>(out_length));
        VerifyOrExit(return_status == OPTIGA_LIB_SUCCESS, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;

exit:
    if (error != CHIP_NO_ERROR)
    {
        ClearSecretData(out_buffer, out_length);
        trustm_close();
    }
    return error;
}

} // namespace Crypto
} // namespace chip

#endif // ENABLE_TRUSTM_RANDOM
