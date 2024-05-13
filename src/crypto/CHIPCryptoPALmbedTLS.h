/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "CHIPCryptoPAL.h"

#include <lib/support/logging/CHIPLogging.h>

#include <mbedtls/ecp.h>
#include <mbedtls/error.h>
#include <mbedtls/version.h>

namespace chip {
namespace Crypto {

// In mbedTLS 3.0.0 direct access to structure fields was replaced with using MBEDTLS_PRIVATE macro.
#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
#define CHIP_CRYPTO_PAL_PRIVATE(x) MBEDTLS_PRIVATE(x)
#else
#define CHIP_CRYPTO_PAL_PRIVATE(x) x
#endif

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000 && MBEDTLS_VERSION_NUMBER < 0x03010000)
#define CHIP_CRYPTO_PAL_PRIVATE_X509(x) MBEDTLS_PRIVATE(x)
#else
#define CHIP_CRYPTO_PAL_PRIVATE_X509(x) x
#endif

static inline void _log_mbedTLS_error(int errorCode)
{

    if (errorCode != 0)
    {
#if defined(MBEDTLS_ERROR_C)
        constexpr size_t kMaxErrorStrLen = 128;
        char errorStr[kMaxErrorStrLen];
        mbedtls_strerror(errorCode, errorStr, sizeof(errorStr));
        ChipLogError(Crypto, "mbedTLS error: %s", errorStr);
#else
        // Error codes defined in 16-bit negative hex numbers. Ease lookup by printing likewise
        ChipLogError(Crypto, "mbedTLS error: -0x%04X", -static_cast<uint16_t>(errorCode));
#endif
    }
}

static inline mbedtls_ecp_group_id MapECPGroupId(SupportedECPKeyTypes keyType)
{
    switch (keyType)
    {
    case SupportedECPKeyTypes::ECP256R1:
        return MBEDTLS_ECP_DP_SECP256R1;
    default:
        return MBEDTLS_ECP_DP_NONE;
    }
}

} // namespace Crypto
} // namespace chip
