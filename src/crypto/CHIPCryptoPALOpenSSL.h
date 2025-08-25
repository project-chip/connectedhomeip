/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *      Header that exposes some ECC-related API which used by P256KeyPair class
 */

#pragma once

#include "CHIPCryptoPAL.h"

#include <openssl/err.h>
#include <openssl/x509.h>

#include <lib/core/CHIPSafeCasts.h>

namespace chip {
namespace Crypto {

#if CHIP_CRYPTO_BORINGSSL
using boringssl_size_t_openssl_int = size_t;
using boringssl_uint_openssl_int   = unsigned int;
using libssl_err_type              = uint32_t;
#else
using boringssl_size_t_openssl_int = int;
using boringssl_uint_openssl_int   = int;
using libssl_err_type              = unsigned long;
#endif // CHIP_CRYPTO_BORINGSSL

enum class ECName
{
    None   = 0,
    P256v1 = 1,
};

/**
 * @brief Collect and print SSL-related error information
 **/
static void CryptoPAL_logSSLError()
{
    unsigned long ssl_err_code = ERR_get_error();
    while (ssl_err_code != 0)
    {
#if CHIP_ERROR_LOGGING
        const char * err_str_lib     = ERR_lib_error_string(static_cast<libssl_err_type>(ssl_err_code));
        const char * err_str_routine = ERR_func_error_string(static_cast<libssl_err_type>(ssl_err_code));
        const char * err_str_reason  = ERR_reason_error_string(static_cast<libssl_err_type>(ssl_err_code));
        if (err_str_lib)
        {
            ChipLogError(Crypto, " ssl err  %s %s %s\n", StringOrNullMarker(err_str_lib), StringOrNullMarker(err_str_routine),
                         StringOrNullMarker(err_str_reason));
        }
#endif // CHIP_ERROR_LOGGING
        ssl_err_code = ERR_get_error();
    }
}

/**
 * @brief Get the NID of an elliptic curve by name
 **/
static int CryptoPAL_nidForCurve(ECName name)
{
    switch (name)
    {
    case ECName::P256v1:
        return EC_curve_nist2nid("P-256");
        break;

    default:
        return NID_undef;
        break;
    }
}

/**
 * @brief Get the elliptic curve name by key type
 **/
static ECName CryptoPAL_MapECName(SupportedECPKeyTypes keyType)
{
    switch (keyType)
    {
    case SupportedECPKeyTypes::ECP256R1:
        return ECName::P256v1;
    default:
        return ECName::None;
    }
}

/**
 * @brief Get public key via EC_KEY
 **/
static CHIP_ERROR P256PublicKeyFromECKey(EC_KEY * ec_key, P256PublicKey & pubkey)
{
    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;

    int nid            = NID_undef;
    ECName curve       = CryptoPAL_MapECName(pubkey.Type());
    EC_GROUP * group   = nullptr;
    size_t pubkey_size = 0;

    const EC_POINT * pubkey_ecp = EC_KEY_get0_public_key(ec_key);
    VerifyOrExit(pubkey_ecp != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = CryptoPAL_nidForCurve(curve);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, error = CHIP_ERROR_INTERNAL);

    pubkey_size =
        EC_POINT_point2oct(group, pubkey_ecp, POINT_CONVERSION_UNCOMPRESSED, Uint8::to_uchar(pubkey), pubkey.Length(), nullptr);
    pubkey_ecp = nullptr;

    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (group != nullptr)
    {
        EC_GROUP_free(group);
        group = nullptr;
    }

    CryptoPAL_logSSLError();
    return error;
}

} // namespace Crypto
} // namespace chip
