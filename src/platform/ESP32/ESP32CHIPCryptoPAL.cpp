/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPSafeCasts.h>
#include <platform/ESP32/ESP32CHIPCryptoPAL.h>

#include <ecdsa/ecdsa_alt.h>
#include <mbedtls/bignum.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/error.h>

// In mbedTLS 3.0.0 direct access to structure fields was replaced with using MBEDTLS_PRIVATE macro.
#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
#define CHIP_CRYPTO_PAL_PRIVATE(x) MBEDTLS_PRIVATE(x)
#else
#define CHIP_CRYPTO_PAL_PRIVATE(x) x
#endif

#define MAX_ERROR_STR_LEN 128

namespace {

static int CryptoRNG(void * ctxt, uint8_t * out_buffer, size_t out_length)
{
    return (chip::Crypto::DRBG_get_bytes(out_buffer, out_length) == CHIP_NO_ERROR) ? 0 : 1;
}

static inline mbedtls_ecdsa_context * to_ecdsa_ctx(chip::Crypto::P256KeypairContext * context)
{
    return chip::SafePointerCast<mbedtls_ecdsa_context *>(context);
}

static void _log_mbedTLS_error(int error_code)
{
    if (error_code != 0)
    {
#if defined(MBEDTLS_ERROR_C)
        char error_str[MAX_ERROR_STR_LEN];
        mbedtls_strerror(error_code, error_str, sizeof(error_str));
        ChipLogError(Crypto, "mbedTLS error: %s", error_str);
#else
        // Error codes defined in 16-bit negative hex numbers. Ease lookup by printing likewise
        ChipLogError(Crypto, "mbedTLS error: -0x%04X", -static_cast<uint16_t>(error_code));
#endif
    }
}

} // anonymous namespace

namespace chip {
namespace Crypto {

CHIP_ERROR ESP32P256Keypair::Initialize(ECPKeyTarget keyTarget, int efuseBlock)
{
    Clear();

    CHIP_ERROR error = CHIP_NO_ERROR;

    mbedtls_ecdsa_context * ecdsa_ctx = to_ecdsa_ctx(&mKeypair);
    mbedtls_ecdsa_init(ecdsa_ctx);

    int status = mbedtls_ecp_group_load(&ecdsa_ctx->MBEDTLS_PRIVATE(grp), MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(status == 0, error = CHIP_ERROR_INTERNAL);

    status = esp_ecdsa_privkey_load_mpi(&ecdsa_ctx->MBEDTLS_PRIVATE(d), efuseBlock);
    VerifyOrExit(status == 0, error = CHIP_ERROR_INTERNAL);

    mInitialized = true;
    ecdsa_ctx    = nullptr;
    return error;

exit:
    if (ecdsa_ctx)
    {
        mbedtls_ecdsa_free(ecdsa_ctx);
        ecdsa_ctx = nullptr;
    }
    _log_mbedTLS_error(status);
    return error;
}

CHIP_ERROR ESP32P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

#if defined(MBEDTLS_ECDSA_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    mbedtls_ecdsa_context * ecdsa_ctx = to_ecdsa_ctx(&mKeypair);

    result = mbedtls_ecdsa_sign(&ecdsa_ctx->CHIP_CRYPTO_PAL_PRIVATE(grp), &r, &s, &ecdsa_ctx->CHIP_CRYPTO_PAL_PRIVATE(d),
                                Uint8::to_const_uchar(digest), sizeof(digest), CryptoRNG, nullptr);

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit((mbedtls_mpi_size(&r) <= kP256_FE_Length) && (mbedtls_mpi_size(&s) <= kP256_FE_Length),
                 error = CHIP_ERROR_INTERNAL);

    // Concatenate r and s to output. Sizes were checked above.
    result = mbedtls_mpi_write_binary(&r, out_signature.Bytes() + 0u, kP256_FE_Length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_write_binary(&s, out_signature.Bytes() + kP256_FE_Length, kP256_FE_Length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

exit:
    ecdsa_ctx = nullptr;
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

} // namespace Crypto
} // namespace chip
