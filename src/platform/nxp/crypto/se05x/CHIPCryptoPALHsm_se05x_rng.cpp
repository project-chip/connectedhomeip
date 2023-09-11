/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

#include "CHIPCryptoPALHsm_se05x_utils.h"
#include <lib/core/CHIPEncoding.h>

namespace chip {
namespace Crypto {

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    // TODO - Add rollback
    sss_status_t status;
    sss_rng_context_t ctx_rng = { 0 };

    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(se05x_sessionOpen() == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

    status = sss_rng_context_init(&ctx_rng, &gex_sss_chip_ctx.session /* Session */);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    status = sss_rng_get_random(&ctx_rng, out_buffer, out_length);
    VerifyOrReturnError(status == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    sss_rng_context_free(&ctx_rng);

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
