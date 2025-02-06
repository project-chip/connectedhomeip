/*
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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
 *      mbedTLS and Tinycrypt based implementation of CHIP crypto primitives
 */
#include <string.h>

#include <crypto/CHIPCryptoPAL.h>

#include <type_traits>

#include <mbedtls/bignum.h>
#include <mbedtls/ccm.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/oid.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>

#include <tinycrypt/ecc.h>
#include <tinycrypt/ecc_dh.h>
#include <tinycrypt/ecc_dsa.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(SLI_SI91X_MCU_INTERFACE)
#include <sl_si91x_trng.h>
#endif // SLI_SI91X_MCU_INTERFACE

#ifdef __cplusplus
}
#endif

namespace chip {
namespace Crypto {

#define MAX_ERROR_STR_LEN 128
#define NUM_BYTES_IN_SHA256_HASH 32

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

namespace {

typedef struct
{
    bool mInitialized;
    bool mDRBGSeeded;
    mbedtls_ctr_drbg_context mDRBGCtxt;
    mbedtls_entropy_context mEntropy;
} EntropyContext;

typedef struct
{
    uint8_t private_key[NUM_ECC_BYTES];
    uint8_t public_key[2 * NUM_ECC_BYTES];
} mbedtls_uecc_keypair;

static EntropyContext gsEntropyContext;

void _log_mbedTLS_error(int error_code)
{
    if (error_code != 0 && error_code != UECC_SUCCESS)
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

bool _isValidTagLength(size_t tag_length)
{
    if (tag_length == 8 || tag_length == 12 || tag_length == 16)
    {
        return true;
    }
    return false;
}
} // namespace

CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const Aes128KeyHandle & key, const uint8_t * nonce, size_t nonce_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    mbedtls_ccm_context context;
    mbedtls_ccm_init(&context);

    VerifyOrExit(plaintext != nullptr || plaintext_length == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext != nullptr || plaintext_length == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_length > 0)
    {
        VerifyOrExit(aad != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // multiplying by 8 to convert key from bits to byte
    result = mbedtls_ccm_setkey(&context, MBEDTLS_CIPHER_ID_AES, key.As<Symmetric128BitsKeyByteArray>(),
                                sizeof(Symmetric128BitsKeyByteArray) * 8);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Encrypt
    result = mbedtls_ccm_encrypt_and_tag(&context, plaintext_length, Uint8::to_const_uchar(nonce), nonce_length,
                                         Uint8::to_const_uchar(aad), aad_length, Uint8::to_const_uchar(plaintext),
                                         Uint8::to_uchar(ciphertext), Uint8::to_uchar(tag), tag_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ccm_free(&context);
    return error;
}

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_len, const uint8_t * aad, size_t aad_len,
                           const uint8_t * tag, size_t tag_length, const Aes128KeyHandle & key, const uint8_t * nonce,
                           size_t nonce_length, uint8_t * plaintext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    mbedtls_ccm_context context;
    mbedtls_ccm_init(&context);

    VerifyOrExit(plaintext != nullptr || ciphertext_len == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext != nullptr || ciphertext_len == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_len > 0)
    {
        VerifyOrExit(aad != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // multiplying by 8 to convert key from bits to byte
    result = mbedtls_ccm_setkey(&context, MBEDTLS_CIPHER_ID_AES, key.As<Symmetric128BitsKeyByteArray>(),
                                sizeof(Symmetric128BitsKeyByteArray) * 8);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Decrypt
    result = mbedtls_ccm_auth_decrypt(&context, ciphertext_len, Uint8::to_const_uchar(nonce), nonce_length,
                                      Uint8::to_const_uchar(aad), aad_len, Uint8::to_const_uchar(ciphertext),
                                      Uint8::to_uchar(plaintext), Uint8::to_const_uchar(tag), tag_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ccm_free(&context);
    return error;
}

CHIP_ERROR Hash_SHA256(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    // zero data length hash is supported.
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result = mbedtls_sha256(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer), 0);
#else
    const int result = mbedtls_sha256_ret(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer), 0);
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA1(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    // zero data length hash is supported.
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result = mbedtls_sha1(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer));
#else
    const int result = mbedtls_sha1_ret(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer));
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

static_assert(kMAX_Hash_SHA256_Context_Size >= sizeof(mbedtls_sha256_context),
              "kMAX_Hash_SHA256_Context_Size is too small for the size of underlying mbedtls_sha256_context");

static inline mbedtls_sha256_context * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<mbedtls_sha256_context *>(context);
}

Hash_SHA256_stream::Hash_SHA256_stream(void)
{
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);
    mbedtls_sha256_init(context);
}

Hash_SHA256_stream::~Hash_SHA256_stream(void)
{
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);
    mbedtls_sha256_free(context);
    Clear();
}

CHIP_ERROR Hash_SHA256_stream::Begin(void)
{
    mbedtls_sha256_context * const context = to_inner_hash_sha256_context(&mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result = mbedtls_sha256_starts(context, 0);
#else
    const int result = mbedtls_sha256_starts_ret(context, 0);
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const ByteSpan data)
{
    mbedtls_sha256_context * const context = to_inner_hash_sha256_context(&mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result = mbedtls_sha256_update(context, Uint8::to_const_uchar(data.data()), data.size());
#else
    const int result = mbedtls_sha256_update_ret(context, Uint8::to_const_uchar(data.data()), data.size());
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::GetDigest(MutableByteSpan & out_buffer)
{
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);

    // Back-up context as we are about to finalize the hash to extract digest.
    mbedtls_sha256_context previous_ctx;
    mbedtls_sha256_init(&previous_ctx);
    mbedtls_sha256_clone(&previous_ctx, context);

    // Pad + compute digest, then finalize context. It is restored next line to continue.
    CHIP_ERROR result = Finish(out_buffer);

    // Restore context prior to finalization.
    mbedtls_sha256_clone(context, &previous_ctx);
    mbedtls_sha256_free(&previous_ctx);

    return result;
}

CHIP_ERROR Hash_SHA256_stream::Finish(MutableByteSpan & out_buffer)
{
    VerifyOrReturnError(out_buffer.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    mbedtls_sha256_context * const context = to_inner_hash_sha256_context(&mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result = mbedtls_sha256_finish(context, Uint8::to_uchar(out_buffer.data()));
#else
    const int result = mbedtls_sha256_finish_ret(context, Uint8::to_uchar(out_buffer.data()));
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
    out_buffer = out_buffer.SubSpan(0, kSHA256_Hash_Length);

    return CHIP_NO_ERROR;
}

void Hash_SHA256_stream::Clear(void)
{
    mbedtls_platform_zeroize(this, sizeof(*this));
}

CHIP_ERROR HKDF_sha::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                 const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
    VerifyOrReturnError(secret != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(secret_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    // Salt is optional
    if (salt_length > 0)
    {
        VerifyOrReturnError(salt != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrReturnError(info_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(info != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const mbedtls_md_info_t * const md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrReturnError(md != nullptr, CHIP_ERROR_INTERNAL);

    const int result = mbedtls_hkdf(md, Uint8::to_const_uchar(salt), salt_length, Uint8::to_const_uchar(secret), secret_length,
                                    Uint8::to_const_uchar(info), info_length, Uint8::to_uchar(out_buffer), out_length);
    _log_mbedTLS_error(result);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR HMAC_sha::HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                 uint8_t * out_buffer, size_t out_length)
{
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(key_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(message != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(message_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length >= kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    const mbedtls_md_info_t * const md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrReturnError(md != nullptr, CHIP_ERROR_INTERNAL);

    const int result =
        mbedtls_md_hmac(md, Uint8::to_const_uchar(key), key_length, Uint8::to_const_uchar(message), message_length, out_buffer);

    _log_mbedTLS_error(result);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR HMAC_sha::HMAC_SHA256(const Hmac128KeyHandle & key, const uint8_t * message, size_t message_length, uint8_t * out_buffer,
                                 size_t out_length)
{
    return HMAC_SHA256(key.As<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray), message, message_length,
                       out_buffer, out_length);
}

CHIP_ERROR PBKDF2_sha256::pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                        unsigned int iteration_count, uint32_t key_length, uint8_t * output)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    const mbedtls_md_info_t * md_info;
    mbedtls_md_context_t md_ctxt;
    constexpr int use_hmac = 1;

    bool free_md_ctxt = false;

    VerifyOrExit(password != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen >= kSpake2p_Min_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen <= kSpake2p_Max_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(md_info != nullptr, error = CHIP_ERROR_INTERNAL);

    mbedtls_md_init(&md_ctxt);
    free_md_ctxt = true;

    result = mbedtls_md_setup(&md_ctxt, md_info, use_hmac);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_pkcs5_pbkdf2_hmac(&md_ctxt, Uint8::to_const_uchar(password), plen, Uint8::to_const_uchar(salt), slen,
                                       iteration_count, key_length, Uint8::to_uchar(output));

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);

    if (free_md_ctxt)
    {
        mbedtls_md_free(&md_ctxt);
    }

    return error;
}
#if !(SLI_SI91X_MCU_INTERFACE)
static EntropyContext * get_entropy_context()
{
    if (!gsEntropyContext.mInitialized)
    {
        mbedtls_entropy_init(&gsEntropyContext.mEntropy);
        mbedtls_ctr_drbg_init(&gsEntropyContext.mDRBGCtxt);

        gsEntropyContext.mInitialized = true;
    }

    return &gsEntropyContext;
}

static mbedtls_ctr_drbg_context * get_drbg_context()
{
    EntropyContext * const context = get_entropy_context();

    mbedtls_ctr_drbg_context * const drbgCtxt = &context->mDRBGCtxt;

    if (!context->mDRBGSeeded)
    {
        const int status = mbedtls_ctr_drbg_seed(drbgCtxt, mbedtls_entropy_func, &context->mEntropy, nullptr, 0);
        if (status != 0)
        {
            _log_mbedTLS_error(status);
            return nullptr;
        }

        context->mDRBGSeeded = true;
    }

    return drbgCtxt;
}
#endif // !SLI_SI91X_MCU_INTERFACE
CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
#if SLI_SI91X_MCU_INTERFACE
    // SiWx917 has its hardware based generator
    (void) fn_source;
    (void) p_source;
    (void) threshold;
#else
    VerifyOrReturnError(fn_source != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    EntropyContext * const entropy_ctxt = get_entropy_context();
    VerifyOrReturnError(entropy_ctxt != nullptr, CHIP_ERROR_INTERNAL);

    const int result =
        mbedtls_entropy_add_source(&entropy_ctxt->mEntropy, fn_source, p_source, threshold, MBEDTLS_ENTROPY_SOURCE_STRONG);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
#endif // SLI_SI91X_MCU_INTERFACE
    return CHIP_NO_ERROR;
}

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);
#if SLI_SI91X_MCU_INTERFACE
    sl_status_t status;
    status = sl_si91x_trng_get_random_num(reinterpret_cast<uint32_t *>(out_buffer), out_length);
    VerifyOrReturnError(status == SL_STATUS_OK, CHIP_ERROR_RANDOM_DATA_UNAVAILABLE);
#else
    mbedtls_ctr_drbg_context * const drbg_ctxt = get_drbg_context();
    VerifyOrReturnError(drbg_ctxt != nullptr, CHIP_ERROR_INTERNAL);

    const int result = mbedtls_ctr_drbg_random(drbg_ctxt, Uint8::to_uchar(out_buffer), out_length);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
#endif // SLI_SI91X_MCU_INTERFACE

    return CHIP_NO_ERROR;
}

mbedtls_ecp_group_id MapECPGroupId(SupportedECPKeyTypes keyType)
{
    switch (keyType)
    {
    case SupportedECPKeyTypes::ECP256R1:
        return MBEDTLS_ECP_DP_SECP256R1;
    default:
        return MBEDTLS_ECP_DP_NONE;
    }
}

static inline mbedtls_uecc_keypair * to_keypair(P256KeypairContext * context)
{
    return SafePointerCast<mbedtls_uecc_keypair *>(context);
}

static inline const mbedtls_uecc_keypair * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const mbedtls_uecc_keypair *>(context);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = UECC_FAILURE;

    const mbedtls_uecc_keypair * keypair = to_const_keypair(&mKeypair);

    result = uECC_sign(keypair->private_key, digest, sizeof(digest), out_signature.Bytes());

    VerifyOrExit(result == UECC_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    keypair = nullptr;

exit:
    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
#if defined(MBEDTLS_ECDSA_C)
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    return ECDSA_validate_hash_signature(&digest[0], sizeof(digest), signature);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(signature.Length() == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = UECC_FAILURE;

    const uint8_t * public_key = *this;

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    result = uECC_verify(public_key + 1, hash, hash_length, Uint8::to_const_uchar(signature.ConstBytes()));
    VerifyOrExit(result == UECC_SUCCESS, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
#if defined(MBEDTLS_ECDH_C)
    CHIP_ERROR error     = CHIP_NO_ERROR;
    int result           = 0;
    size_t secret_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();

    const mbedtls_uecc_keypair * keypair = to_const_keypair(&mKeypair);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    result = uECC_shared_secret(remote_public_key.ConstBytes() + 1, keypair->private_key, out_secret.Bytes());
    VerifyOrExit(result == UECC_SUCCESS, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = out_secret.SetLength(secret_length));

exit:
    keypair = nullptr;
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

void ClearSecretData(uint8_t * buf, size_t len)
{
    mbedtls_platform_zeroize(buf, len);
}

// THE BELOW IS FROM `third_party/openthread/repo/third_party/mbedtls/repo/library/constant_time.c` since
// mbedtls_ct_memcmp is not available on Linux somehow :(
int mbedtls_ct_memcmp_copy(const void * a, const void * b, size_t n)
{
    size_t i;
    volatile const unsigned char * A = (volatile const unsigned char *) a;
    volatile const unsigned char * B = (volatile const unsigned char *) b;
    volatile unsigned char diff      = 0;

    for (i = 0; i < n; i++)
    {
        /* Read volatile data in order before computing diff.
         * This avoids IAR compiler warning:
         * 'the order of volatile accesses is undefined ..' */
        unsigned char x = A[i], y = B[i];
        diff |= x ^ y;
    }

    return ((int) diff);
}

bool IsBufferContentEqualConstantTime(const void * a, const void * b, size_t n)
{
    return mbedtls_ct_memcmp_copy(a, b, n) == 0;
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = UECC_FAILURE;

    Clear();

    mbedtls_uecc_keypair * keypair = to_keypair(&mKeypair);

    result = uECC_make_key(keypair->public_key, keypair->private_key);
    VerifyOrExit(result == UECC_SUCCESS, error = CHIP_ERROR_INTERNAL);

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    Uint8::to_uchar(mPublicKey)[0] = 0x04;
    memcpy(Uint8::to_uchar(mPublicKey) + 1, keypair->public_key, 2 * NUM_ECC_BYTES);

    keypair      = nullptr;
    mInitialized = true;

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    const mbedtls_uecc_keypair * keypair = to_const_keypair(&mKeypair);
    size_t len                           = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output.Bytes(), len);
    uint8_t privkey[kP256_PrivateKey_Length];
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    bbuf.Put(mPublicKey, mPublicKey.Length());

    VerifyOrExit(bbuf.Available() == sizeof(privkey), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(sizeof(keypair->private_key) <= bbuf.Available(), error = CHIP_ERROR_INTERNAL);

    memcpy(privkey, keypair->private_key, sizeof(privkey));

    bbuf.Put(privkey, sizeof(privkey));
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    output.SetLength(bbuf.Needed());

exit:
    memset(privkey, 0, sizeof(privkey));
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    int result       = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    Clear();

    mbedtls_uecc_keypair * keypair = to_keypair(&mKeypair);

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    memcpy(keypair->public_key, input.ConstBytes() + 1, 2 * NUM_ECC_BYTES);
    memcpy(keypair->private_key, input.ConstBytes() + mPublicKey.Length(), NUM_ECC_BYTES);

    keypair = nullptr;

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    mInitialized = true;

    _log_mbedTLS_error(result);

exit:
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        mbedtls_uecc_keypair * keypair = to_keypair(&mKeypair);
        memset(keypair, 0, sizeof(mbedtls_uecc_keypair));
        mInitialized = false;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    MutableByteSpan csr(out_csr, csr_length);
    CHIP_ERROR err = GenerateCertificateSigningRequest(this, csr);
    csr_length     = (CHIP_NO_ERROR == err) ? csr.size() : 0;
    ChipLogByteSpan(Crypto, csr);
    return err;
}

CHIP_ERROR VerifyCertificateSigningRequest(const uint8_t * csr_buf, size_t csr_length, P256PublicKey & pubkey)
{
#if defined(MBEDTLS_X509_CSR_PARSE_C)
    ReturnErrorOnFailure(VerifyCertificateSigningRequestFormat(csr_buf, csr_length));

    // TODO: For some embedded targets, mbedTLS library doesn't have mbedtls_x509_csr_parse_der, and mbedtls_x509_csr_parse_free.
    //       Taking a step back, embedded targets likely will not process CSR requests. Adding this action item to reevaluate
    //       this if there's a need for this processing for embedded targets.
    CHIP_ERROR error   = CHIP_NO_ERROR;
    size_t pubkey_size = 0;

    mbedtls_ecp_keypair * keypair = nullptr;

    P256ECDSASignature signature;
    MutableByteSpan out_raw_sig_span(signature.Bytes(), signature.Capacity());

    mbedtls_x509_csr csr;
    mbedtls_x509_csr_init(&csr);

    int result = mbedtls_x509_csr_parse_der(&csr, csr_buf, csr_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Verify the signature algorithm and public key type
    VerifyOrExit(csr.CHIP_CRYPTO_PAL_PRIVATE(sig_md) == MBEDTLS_MD_SHA256, error = CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);
    VerifyOrExit(csr.CHIP_CRYPTO_PAL_PRIVATE(sig_pk) == MBEDTLS_PK_ECDSA, error = CHIP_ERROR_WRONG_KEY_TYPE);

    keypair = mbedtls_pk_ec(csr.CHIP_CRYPTO_PAL_PRIVATE_X509(pk));

    // Copy the public key from the CSR
    result = mbedtls_ecp_point_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                            MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, Uint8::to_uchar(pubkey), pubkey.Length());

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INTERNAL);

    // Convert DER signature to raw signature
    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length,
                                    ByteSpan{ csr.CHIP_CRYPTO_PAL_PRIVATE(sig).CHIP_CRYPTO_PAL_PRIVATE_X509(p),
                                              csr.CHIP_CRYPTO_PAL_PRIVATE(sig).CHIP_CRYPTO_PAL_PRIVATE_X509(len) },
                                    out_raw_sig_span);

    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_raw_sig_span.size() == (kP256_FE_Length * 2), error = CHIP_ERROR_INTERNAL);
    signature.SetLength(out_raw_sig_span.size());

    // Verify the signature using the public key
    error = pubkey.ECDSA_validate_msg_signature(csr.CHIP_CRYPTO_PAL_PRIVATE_X509(cri).CHIP_CRYPTO_PAL_PRIVATE_X509(p),
                                                csr.CHIP_CRYPTO_PAL_PRIVATE_X509(cri).CHIP_CRYPTO_PAL_PRIVATE_X509(len), signature);

    SuccessOrExit(error);

exit:
    mbedtls_x509_csr_free(&csr);
    _log_mbedTLS_error(result);
    return error;
#else
    ChipLogError(Crypto, "MBEDTLS_X509_CSR_PARSE_C is not enabled. CSR cannot be parsed");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

typedef struct Spake2p_Context
{
    const mbedtls_md_info_t * md_info;
    uECC_word_t M[2 * NUM_ECC_WORDS];
    uECC_word_t N[2 * NUM_ECC_WORDS];
    uECC_word_t X[2 * NUM_ECC_WORDS];
    uECC_word_t Y[2 * NUM_ECC_WORDS];
    uECC_word_t L[2 * NUM_ECC_WORDS];
    uECC_word_t Z[2 * NUM_ECC_WORDS];
    uECC_word_t V[2 * NUM_ECC_WORDS];

    uECC_word_t w0[NUM_ECC_WORDS];
    uECC_word_t w1[NUM_ECC_WORDS];
    uECC_word_t xy[NUM_ECC_WORDS];
    uECC_word_t tempbn[NUM_ECC_WORDS];
} Spake2p_Context;

static inline Spake2p_Context * to_inner_spake2p_context(Spake2pOpaqueContext * context)
{
    return SafePointerCast<Spake2p_Context *>(context);
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    memset(context, 0, sizeof(Spake2p_Context));

    M = context->M;
    N = context->N;
    X = context->X;
    Y = context->Y;
    L = context->L;
    V = context->V;
    Z = context->Z;

    w0     = context->w0;
    w1     = context->w1;
    xy     = context->xy;
    tempbn = context->tempbn;

    G = curve_G;

    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::Clear()
{
    VerifyOrReturn(state != CHIP_SPAKE2P_STATE::PREINIT);

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);
    memset(&context->M, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->N, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->X, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->Y, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->L, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->Z, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->V, 0, 2 * NUM_ECC_WORDS * sizeof(uECC_word_t));

    memset(&context->w0, 0, NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->w1, 0, NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->xy, 0, NUM_ECC_WORDS * sizeof(uECC_word_t));
    memset(&context->tempbn, 0, NUM_ECC_WORDS * sizeof(uECC_word_t));

    G     = NULL;
    state = CHIP_SPAKE2P_STATE::PREINIT;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len,
                                              MutableByteSpan & out_span)
{
    HMAC_sha hmac;
    VerifyOrReturnError(out_span.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    ReturnErrorOnFailure(hmac.HMAC_SHA256(key, key_len, in, in_len, out_span.data(), kSHA256_Hash_Length));
    out_span = out_span.SubSpan(0, kSHA256_Hash_Length);
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::MacVerify(const uint8_t * key, size_t key_len, const uint8_t * mac, size_t mac_len,
                                                    const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    uint8_t computed_mac[kSHA256_Hash_Length];
    MutableByteSpan computed_mac_span{ computed_mac };
    VerifyOrExit(mac_len == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    SuccessOrExit(error = Mac(key, key_len, in, in_len, computed_mac_span));
    VerifyOrExit(computed_mac_span.size() == mac_len, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(IsBufferContentEqualConstantTime(mac, computed_mac, kSHA256_Hash_Length), error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const uint8_t * in, size_t in_len, void * fe)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    uECC_word_t tmp[2 * NUM_ECC_WORDS] = { 0 };
    uECC_vli_bytesToNative(tmp, in, NUM_ECC_BYTES);

    uECC_vli_mmod((uECC_word_t *) fe, tmp, curve_n);

    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, uint8_t * out, size_t out_len)
{
    (void) out_len;
    uECC_vli_nativeToBytes(out, NUM_ECC_BYTES, (const unsigned int *) fe);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_uecc_keypair keypair;

    result = UECC_FAILURE;

    result = uECC_make_key(keypair.public_key, keypair.private_key);
    VerifyOrExit(result == UECC_SUCCESS, error = CHIP_ERROR_INTERNAL);

    uECC_vli_bytesToNative((uECC_word_t *) fe, keypair.private_key, NUM_ECC_BYTES);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    uECC_vli_modMult((uECC_word_t *) fer, (const uECC_word_t *) fe1, (const uECC_word_t *) fe2, (const uECC_word_t *) curve_n);

    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const uint8_t * in, size_t in_len, void * R)
{
    uint8_t tmp[2 * NUM_ECC_BYTES];

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    memcpy(tmp, in + 1, 2 * NUM_ECC_BYTES);

    uECC_vli_bytesToNative((uECC_word_t *) R, tmp, NUM_ECC_BYTES);
    uECC_vli_bytesToNative((uECC_word_t *) R + NUM_ECC_WORDS, tmp + NUM_ECC_BYTES, NUM_ECC_BYTES);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, uint8_t * out, size_t out_len)
{
    memset(out, 0, out_len);

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    out[0] = 0x04;
    uECC_vli_nativeToBytes(out + 1, NUM_ECC_BYTES, (uECC_word_t *) R);
    uECC_vli_nativeToBytes(out + NUM_ECC_BYTES + 1, NUM_ECC_BYTES, (uECC_word_t *) R + NUM_ECC_WORDS);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{

    if (EccPoint_mult_safer((uECC_word_t *) R, (const uECC_word_t *) P1, (const uECC_word_t *) fe1) != UECC_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointAddMul(void * R, const void * P1, const void * fe1, const void * P2,
                                                      const void * fe2)
{
    uECC_word_t R1[2 * NUM_ECC_WORDS];
    uECC_word_t R2[2 * NUM_ECC_WORDS];
    uECC_word_t z[NUM_ECC_WORDS];

    if (EccPoint_mult_safer(R1, (const uECC_word_t *) P1, (const uECC_word_t *) fe1) != UECC_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    if (EccPoint_mult_safer(R2, (const uECC_word_t *) P2, (const uECC_word_t *) fe2) != UECC_SUCCESS)
    {
        return CHIP_ERROR_INTERNAL;
    }

    uECC_vli_modSub(z, R2, R1, curve_p);
    XYcZ_add(R1, R1 + NUM_ECC_WORDS, R2, R2 + NUM_ECC_WORDS);
    uECC_vli_modInv(z, z, curve_p);
    apply_z(R2, R2 + NUM_ECC_WORDS, z);

    memcpy((uECC_word_t *) R, R2, 2 * NUM_ECC_BYTES);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointInvert(void * R)
{
    uECC_word_t tmp[NUM_ECC_WORDS] = { 0 };

    uECC_vli_sub(tmp, curve_p, (uECC_word_t *) R + NUM_ECC_WORDS);
    memcpy((uECC_word_t *) R + NUM_ECC_WORDS, tmp, NUM_ECC_BYTES);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointCofactorMul(void * R)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1sin, size_t w1sin_len)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = UECC_SUCCESS;
    uECC_word_t tmp[2 * NUM_ECC_WORDS];
    uECC_word_t w1_bn[NUM_ECC_WORDS];
    uECC_word_t L_tmp[2 * NUM_ECC_WORDS];

    uECC_vli_bytesToNative(tmp, w1sin, NUM_ECC_BYTES);

    uECC_vli_mmod(w1_bn, tmp, curve_n);

    result = EccPoint_mult_safer(L_tmp, curve_G, w1_bn);
    VerifyOrExit(result == UECC_SUCCESS, error = CHIP_ERROR_INTERNAL);

    // Fully padded raw uncompressed points expected, first byte is always 0x04 i.e uncompressed
    Lout[0] = 0x04;
    uECC_vli_nativeToBytes(Lout + 1, NUM_ECC_BYTES, L_tmp);
    uECC_vli_nativeToBytes(Lout + NUM_ECC_BYTES + 1, NUM_ECC_BYTES, L_tmp + NUM_ECC_WORDS);

exit:
    _log_mbedTLS_error(result);

    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointIsValid(void * R)
{
    if (uECC_valid_point((const uECC_word_t *) R) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

namespace {

#if defined(MBEDTLS_X509_CRT_PARSE_C)
bool IsTimeGreaterThanEqual(const mbedtls_x509_time * const timeA, const mbedtls_x509_time * const timeB)
{

    // checks if two values are different and if yes, then returns first > second.
#define RETURN_STRICTLY_GREATER_IF_DIFFERENT(component)                                                                            \
    {                                                                                                                              \
        auto valueA = timeA->CHIP_CRYPTO_PAL_PRIVATE_X509(component);                                                              \
        auto valueB = timeB->CHIP_CRYPTO_PAL_PRIVATE_X509(component);                                                              \
                                                                                                                                   \
        if (valueA != valueB)                                                                                                      \
        {                                                                                                                          \
            return valueA > valueB;                                                                                                \
        }                                                                                                                          \
    }

    RETURN_STRICTLY_GREATER_IF_DIFFERENT(year);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(mon);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(day);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(hour);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(min);
    RETURN_STRICTLY_GREATER_IF_DIFFERENT(sec);

    // all above are equal
    return true;
}

CHIP_ERROR IsCertificateValidAtIssuance(const mbedtls_x509_crt * candidateCertificate, const mbedtls_x509_crt * issuerCertificate)
{
    mbedtls_x509_time candidateNotBeforeTime = candidateCertificate->CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from);
    mbedtls_x509_time issuerNotBeforeTime    = issuerCertificate->CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from);
    mbedtls_x509_time issuerNotAfterTime     = issuerCertificate->CHIP_CRYPTO_PAL_PRIVATE_X509(valid_to);

    // check if candidateCertificate is issued at or after issuerCertificate's notBefore timestamp
    VerifyOrReturnError(IsTimeGreaterThanEqual(&candidateNotBeforeTime, &issuerNotBeforeTime), CHIP_ERROR_CERT_EXPIRED);

    // check if candidateCertificate is issued at or before issuerCertificate's notAfter timestamp
    VerifyOrReturnError(IsTimeGreaterThanEqual(&issuerNotAfterTime, &candidateNotBeforeTime), CHIP_ERROR_CERT_EXPIRED);

    return CHIP_NO_ERROR;
}

int CallbackForCustomValidityCheck(void * data, mbedtls_x509_crt * crt, int depth, uint32_t * flags)
{
    mbedtls_x509_crt * leafCert   = reinterpret_cast<mbedtls_x509_crt *>(data);
    mbedtls_x509_crt * issuerCert = crt;

    // Ignore any time validy error performed by the standard mbedTLS code.
    *flags &= ~(static_cast<uint32_t>(MBEDTLS_X509_BADCERT_EXPIRED | MBEDTLS_X509_BADCERT_FUTURE));

    // Verify that the leaf certificate has a notBefore time valid within the validity period of the issuerCertificate.
    // Note that this callback is invoked for each certificate in the chain.
    if (IsCertificateValidAtIssuance(leafCert, issuerCert) != CHIP_NO_ERROR)
    {
        return MBEDTLS_ERR_X509_INVALID_DATE;
    }

    return 0;
}

constexpr uint8_t sOID_AttributeType_CommonName[]         = { 0x55, 0x04, 0x03 };
constexpr uint8_t sOID_AttributeType_MatterVendorId[]     = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x01 };
constexpr uint8_t sOID_AttributeType_MatterProductId[]    = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x02 };
constexpr uint8_t sOID_SigAlgo_ECDSAWithSHA256[]          = { 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02 };
constexpr uint8_t sOID_Extension_BasicConstraints[]       = { 0x55, 0x1D, 0x13 };
constexpr uint8_t sOID_Extension_KeyUsage[]               = { 0x55, 0x1D, 0x0F };
constexpr uint8_t sOID_Extension_SubjectKeyIdentifier[]   = { 0x55, 0x1D, 0x0E };
constexpr uint8_t sOID_Extension_AuthorityKeyIdentifier[] = { 0x55, 0x1D, 0x23 };
constexpr uint8_t sOID_Extension_CRLDistributionPoint[]   = { 0x55, 0x1D, 0x1F };

/**
 * Compares an mbedtls_asn1_buf structure (oidBuf) to a reference OID represented as uint8_t array (oid).
 */
#define OID_CMP(oid, oidBuf)                                                                                                       \
    ((MBEDTLS_ASN1_OID == (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(tag)) &&                                                           \
     (sizeof(oid) == (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(len)) &&                                                                \
     (memcmp((oid), (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(p), (oidBuf).CHIP_CRYPTO_PAL_PRIVATE_X509(len)) == 0))

#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

} // anonymous namespace

CHIP_ERROR VerifyAttestationCertificateFormat(const ByteSpan & cert, AttestationCertType certType)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;
    bool extBasicPresent      = false;
    bool extKeyUsagePresent   = false;

    VerifyOrReturnError(!cert.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbed_cert);

    result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(cert.data()), cert.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // "version" value is 1 higher than the actual encoded value.
    VerifyOrExit(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(version) - 1 == 2, error = CHIP_ERROR_INTERNAL);

    // Verify signature algorithms is ECDSA with SHA256.
    VerifyOrExit(OID_CMP(sOID_SigAlgo_ECDSAWithSHA256, mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(sig_oid)),
                 error = CHIP_ERROR_INTERNAL);

    // Verify public key presence and format.
    {
        Crypto::P256PublicKey pubkey;
        SuccessOrExit(error = ExtractPubkeyFromX509Cert(cert, pubkey));
    }

    p      = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end    = p + mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    while (p < end)
    {
        mbedtls_x509_buf extOID = { 0, 0, nullptr };
        int extCritical         = 0;

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

        /* Get extension ID */
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

        extOID.CHIP_CRYPTO_PAL_PRIVATE_X509(tag) = MBEDTLS_ASN1_OID;
        extOID.CHIP_CRYPTO_PAL_PRIVATE_X509(len) = len;
        extOID.CHIP_CRYPTO_PAL_PRIVATE_X509(p)   = p;
        p += len;

        /* Get optional critical */
        result = mbedtls_asn1_get_bool(&p, end, &extCritical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_INTERNAL);

        /* Data should be octet string type */
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

        if (OID_CMP(sOID_Extension_BasicConstraints, extOID))
        {
            int isCA    = 0;
            int pathLen = -1;

            VerifyOrExit(extCritical, error = CHIP_ERROR_INTERNAL);
            extBasicPresent = true;

            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
            if (len > 0)
            {
                unsigned char * seqStart = p;
                result                   = mbedtls_asn1_get_bool(&p, end, &isCA);
                VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_INTERNAL);

                // Check if pathLen is there by validating if the cursor didn't get to the end of
                // of the internal SEQUENCE for the basic constraints encapsulation.
                // Missing pathLen optional tag will leave pathLen == -1 for following checks.
                bool hasPathLen = (p != (seqStart + len));
                if (hasPathLen)
                {
                    // Extract pathLen value, making sure it's a valid format.
                    result = mbedtls_asn1_get_int(&p, end, &pathLen);
                    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
                }
            }

            if (certType == AttestationCertType::kDAC)
            {
                VerifyOrExit(!isCA && pathLen == -1, error = CHIP_ERROR_INTERNAL);
            }
            else if (certType == AttestationCertType::kPAI)
            {
                VerifyOrExit(isCA && pathLen == 0, error = CHIP_ERROR_INTERNAL);
            }
            else
            {
                // For PAA, pathlen must be absent or equal to 1 (see Matter 1.1 spec 6.2.2.5)
                VerifyOrExit(isCA && (pathLen == -1 || pathLen == 1), error = CHIP_ERROR_INTERNAL);
            }
        }
        else if (OID_CMP(sOID_Extension_KeyUsage, extOID))
        {
            mbedtls_x509_bitstring bs = { 0, 0, nullptr };
            unsigned int keyUsage     = 0;

            VerifyOrExit(extCritical, error = CHIP_ERROR_INTERNAL);
            extKeyUsagePresent = true;

            result = mbedtls_asn1_get_bitstring(&p, p + len, &bs);
            VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

            for (size_t i = 0; i < bs.CHIP_CRYPTO_PAL_PRIVATE_X509(len) && i < sizeof(unsigned int); i++)
            {
                keyUsage |= static_cast<unsigned int>(bs.CHIP_CRYPTO_PAL_PRIVATE_X509(p)[i]) << (8 * i);
            }

            if (certType == AttestationCertType::kDAC)
            {
                // SHALL only have the digitalSignature bit set.
                VerifyOrExit(keyUsage == MBEDTLS_X509_KU_DIGITAL_SIGNATURE, error = CHIP_ERROR_INTERNAL);
            }
            else
            {
                bool keyCertSignFlag = keyUsage & MBEDTLS_X509_KU_KEY_CERT_SIGN;
                bool crlSignFlag     = keyUsage & MBEDTLS_X509_KU_CRL_SIGN;
                bool otherFlags =
                    keyUsage & ~(MBEDTLS_X509_KU_CRL_SIGN | MBEDTLS_X509_KU_KEY_CERT_SIGN | MBEDTLS_X509_KU_DIGITAL_SIGNATURE);
                VerifyOrExit(keyCertSignFlag && crlSignFlag && !otherFlags, error = CHIP_ERROR_INTERNAL);
            }
        }
        else
        {
            p += len;
        }
    }

    // Verify basic and key usage extensions are present.
    VerifyOrExit(extBasicPresent && extKeyUsagePresent, error = CHIP_ERROR_INTERNAL);

    // Verify that SKID and AKID extensions are present.
    {
        uint8_t kidBuf[kSubjectKeyIdentifierLength];
        MutableByteSpan kid(kidBuf);
        SuccessOrExit(error = ExtractSKIDFromX509Cert(cert, kid));
        if (certType == AttestationCertType::kDAC || certType == AttestationCertType::kPAI)
        {
            // Mandatory extension for DAC and PAI certs.
            SuccessOrExit(error = ExtractAKIDFromX509Cert(cert, kid));
        }
    }

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) cert;
    (void) certType;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen,
                                    CertificateChainValidationResult & result)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt certChain;
    mbedtls_x509_crt rootCert;
    int mbedResult;
    uint32_t flags = 0;

    result = CertificateChainValidationResult::kInternalFrameworkError;

    VerifyOrReturnError(rootCertificate != nullptr && rootCertificateLen != 0,
                        (result = CertificateChainValidationResult::kRootArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturnError(leafCertificate != nullptr && leafCertificateLen != 0,
                        (result = CertificateChainValidationResult::kLeafArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));

    mbedtls_x509_crt_init(&certChain);
    mbedtls_x509_crt_init(&rootCert);

    /* Start of chain  */
    mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(leafCertificate), leafCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kLeafFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Add the intermediate to the chain, if present */
    if (caCertificate != nullptr && caCertificateLen > 0)
    {
        mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(caCertificate), caCertificateLen);
        VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kICAFormatInvalid, error = CHIP_ERROR_INTERNAL));
    }

    /* Parse the root cert */
    mbedResult = mbedtls_x509_crt_parse(&rootCert, Uint8::to_const_uchar(rootCertificate), rootCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kRootFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Verify the chain against the root */
    mbedResult =
        mbedtls_x509_crt_verify(&certChain, &rootCert, nullptr, nullptr, &flags, CallbackForCustomValidityCheck, &certChain);

    switch (mbedResult)
    {
    case 0:
        VerifyOrExit(flags == 0, (result = CertificateChainValidationResult::kInternalFrameworkError, error = CHIP_ERROR_INTERNAL));
        result = CertificateChainValidationResult::kSuccess;
        break;
    case MBEDTLS_ERR_X509_INVALID_DATE:
    case MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:
        result = CertificateChainValidationResult::kChainInvalid;
        error  = CHIP_ERROR_CERT_NOT_TRUSTED;
        break;
    default:
        result = CertificateChainValidationResult::kInternalFrameworkError;
        error  = CHIP_ERROR_INTERNAL;
        break;
    }

exit:
    _log_mbedTLS_error(mbedResult);
    mbedtls_x509_crt_free(&certChain);
    mbedtls_x509_crt_free(&rootCert);

#else
    (void) rootCertificate;
    (void) rootCertificateLen;
    (void) caCertificate;
    (void) caCertificateLen;
    (void) leafCertificate;
    (void) leafCertificateLen;
    (void) result;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR IsCertificateValidAtIssuance(const ByteSpan & candidateCertificate, const ByteSpan & issuerCertificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbedCandidateCertificate;
    mbedtls_x509_crt mbedIssuerCertificate;
    int result;

    VerifyOrReturnError(!candidateCertificate.empty() && !issuerCertificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedCandidateCertificate);
    mbedtls_x509_crt_init(&mbedIssuerCertificate);

    result = mbedtls_x509_crt_parse(&mbedCandidateCertificate, Uint8::to_const_uchar(candidateCertificate.data()),
                                    candidateCertificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result =
        mbedtls_x509_crt_parse(&mbedIssuerCertificate, Uint8::to_const_uchar(issuerCertificate.data()), issuerCertificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Verify that the candidateCertificate has a notBefore time valid within the validity period of the issuerCertificate.
    SuccessOrExit(error = IsCertificateValidAtIssuance(&mbedCandidateCertificate, &mbedIssuerCertificate));

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedCandidateCertificate);
    mbedtls_x509_crt_free(&mbedIssuerCertificate);

#else
    (void) candidateCertificate;
    (void) issuerCertificate;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR IsCertificateValidAtCurrentTime(const ByteSpan & certificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbedCertificate;
    int result;

    VerifyOrReturnError(!certificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedCertificate);

    result = mbedtls_x509_crt_parse(&mbedCertificate, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // check if certificate's notBefore timestamp is earlier than or equal to current time.
    result = mbedtls_x509_time_is_past(&mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from));
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);

    // check if certificate's notAfter timestamp is later than current time.
    result = mbedtls_x509_time_is_future(&mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_to));
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedCertificate);

#else
    (void) certificate;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbed_cert;
    mbedtls_uecc_keypair * keypair = nullptr;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mbedtls_pk_get_type(&(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(pk))) == MBEDTLS_PK_ECKEY,
                 error = CHIP_ERROR_INVALID_ARGUMENT);

    keypair                    = (mbedtls_uecc_keypair *) (mbedtls_pk_ec(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(pk)));
    Uint8::to_uchar(pubkey)[0] = 0x04; // uncompressed type
    memcpy(Uint8::to_uchar(pubkey) + 1, keypair->public_key, 2 * NUM_ECC_BYTES);

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) pubkey;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

namespace {

CHIP_ERROR ExtractKIDFromX509Cert(bool extractSKID, const ByteSpan & certificate, MutableByteSpan & kid)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_ERROR_NOT_FOUND;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // TODO: The mbedTLS team is working on supporting SKID and AKID extensions processing.
    // Once it is supported, this code should be updated.

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        mbedtls_x509_buf extOID    = { MBEDTLS_ASN1_OID, len, p };
        bool extractCurrentExtSKID = extractSKID && OID_CMP(sOID_Extension_SubjectKeyIdentifier, extOID);
        bool extractCurrentExtAKID = !extractSKID && OID_CMP(sOID_Extension_AuthorityKeyIdentifier, extOID);
        p += len;

        int is_critical = 0;
        result          = mbedtls_asn1_get_bool(&p, end, &is_critical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_WRONG_CERT_TYPE);

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        if (extractCurrentExtSKID || extractCurrentExtAKID)
        {
            if (extractCurrentExtSKID)
            {
                result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
                VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
            }
            else
            {
                result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
                VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
                result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC);
                VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
                // Other optional fields, authorityCertIssuer and authorityCertSerialNumber,
                // will be skipped if present.
            }
            VerifyOrExit(len == kSubjectKeyIdentifierLength, error = CHIP_ERROR_WRONG_CERT_TYPE);
            VerifyOrExit(len <= kid.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);
            memcpy(kid.data(), p, len);
            if (kid.size() > len)
            {
                kid.reduce_size(len);
            }
            ExitNow(error = CHIP_NO_ERROR);
            break;
        }
        p += len;
    }

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) kid;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

} // namespace

CHIP_ERROR ExtractSKIDFromX509Cert(const ByteSpan & certificate, MutableByteSpan & skid)
{
    return ExtractKIDFromX509Cert(true, certificate, skid);
}

CHIP_ERROR ExtractAKIDFromX509Cert(const ByteSpan & certificate, MutableByteSpan & akid)
{
    return ExtractKIDFromX509Cert(false, certificate, akid);
}

CHIP_ERROR ExtractCRLDistributionPointURIFromX509Cert(const ByteSpan & certificate, MutableCharSpan & cdpurl)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_ERROR_NOT_FOUND;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;
    size_t cdpExtCount        = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        mbedtls_x509_buf extOID = { MBEDTLS_ASN1_OID, len, p };
        bool isCurrentExtCDP    = OID_CMP(sOID_Extension_CRLDistributionPoint, extOID);
        p += len;

        int is_critical = 0;
        result          = mbedtls_asn1_get_bool(&p, end, &is_critical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_WRONG_CERT_TYPE);

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        unsigned char * end_of_ext = p + len;

        if (isCurrentExtCDP)
        {
            // Only one CRL Distribution Point Extension is allowed.
            cdpExtCount++;
            VerifyOrExit(cdpExtCount <= 1, error = CHIP_ERROR_NOT_FOUND);

            // CRL Distribution Point Extension is encoded as a sequence of DistributionPoint:
            //     CRLDistributionPoints ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
            //
            // This implementation only supports a single DistributionPoint (sequence of size 1),
            // which is verified by comparing (p + len == end_of_ext)
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // The DistributionPoint is a sequence of three optional elements:
            //     DistributionPoint ::= SEQUENCE {
            //         distributionPoint       [0]     DistributionPointName OPTIONAL,
            //         reasons                 [1]     ReasonFlags OPTIONAL,
            //         cRLIssuer               [2]     GeneralNames OPTIONAL }
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // The DistributionPointName is:
            //     DistributionPointName ::= CHOICE {
            //         fullName                [0]     GeneralNames,
            //         nameRelativeToCRLIssuer [1]     RelativeDistinguishedName }
            //
            // The URI should be encoded in the fullName element.
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 0);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            //     GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            unsigned char * end_of_general_names = p + len;

            // The CDP URI is encoded as a uniformResourceIdentifier field of the GeneralName:
            //     GeneralName ::= CHOICE {
            //         otherName                       [0]     OtherName,
            //         rfc822Name                      [1]     IA5String,
            //         dNSName                         [2]     IA5String,
            //         x400Address                     [3]     ORAddress,
            //         directoryName                   [4]     Name,
            //         ediPartyName                    [5]     EDIPartyName,
            //         uniformResourceIdentifier       [6]     IA5String,
            //         iPAddress                       [7]     OCTET STRING,
            //         registeredID                    [8]     OBJECT IDENTIFIER }
            result =
                mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_X509_SAN_UNIFORM_RESOURCE_IDENTIFIER);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            // Only single URI instance in the GeneralNames is supported
            VerifyOrExit(p + len == end_of_general_names, error = CHIP_ERROR_NOT_FOUND);

            const char * urlptr = reinterpret_cast<const char *>(p);
            VerifyOrExit((len > strlen(kValidCDPURIHttpPrefix) &&
                          strncmp(urlptr, kValidCDPURIHttpPrefix, strlen(kValidCDPURIHttpPrefix)) == 0) ||
                             (len > strlen(kValidCDPURIHttpsPrefix) &&
                              strncmp(urlptr, kValidCDPURIHttpsPrefix, strlen(kValidCDPURIHttpsPrefix)) == 0),
                         error = CHIP_ERROR_NOT_FOUND);
            error = CopyCharSpanToMutableCharSpan(CharSpan(urlptr, len), cdpurl);
            SuccessOrExit(error);
        }
        p = end_of_ext;
    }

    VerifyOrExit(cdpExtCount == 1, error = CHIP_ERROR_NOT_FOUND);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) cdpurl;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractCDPExtensionCRLIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & crlIssuer)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_ERROR_NOT_FOUND;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p         = nullptr;
    const unsigned char * end = nullptr;
    size_t len                = 0;
    size_t cdpExtCount        = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(v3_ext).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        mbedtls_x509_buf extOID = { MBEDTLS_ASN1_OID, len, p };
        bool isCurrentExtCDP    = OID_CMP(sOID_Extension_CRLDistributionPoint, extOID);
        p += len;

        int is_critical = 0;
        result          = mbedtls_asn1_get_bool(&p, end, &is_critical);
        VerifyOrExit(result == 0 || result == MBEDTLS_ERR_ASN1_UNEXPECTED_TAG, error = CHIP_ERROR_WRONG_CERT_TYPE);

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        unsigned char * end_of_ext = p + len;

        if (isCurrentExtCDP)
        {
            // Only one CRL Distribution Point Extension is allowed.
            cdpExtCount++;
            VerifyOrExit(cdpExtCount <= 1, error = CHIP_ERROR_NOT_FOUND);

            // CRL Distribution Point Extension is encoded as a sequence of DistributionPoint:
            //     CRLDistributionPoints ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
            //
            // This implementation only supports a single DistributionPoint (sequence of size 1),
            // which is verified by comparing (p + len == end_of_ext)
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // The DistributionPoint is a sequence of three optional elements:
            //     DistributionPoint ::= SEQUENCE {
            //         distributionPoint       [0]     DistributionPointName OPTIONAL,
            //         reasons                 [1]     ReasonFlags OPTIONAL,
            //         cRLIssuer               [2]     GeneralNames OPTIONAL }
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            // If distributionPoint element presents, ignore it
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 0);
            if (result == 0)
            {
                p += len;
                VerifyOrExit(p < end_of_ext, error = CHIP_ERROR_NOT_FOUND);
            }

            // Check if cRLIssuer element present
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | 2);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);

            // The CRL Issuer is encoded as a directoryName field of the GeneralName:
            //     GeneralName ::= CHOICE {
            //         otherName                       [0]     OtherName,
            //         rfc822Name                      [1]     IA5String,
            //         dNSName                         [2]     IA5String,
            //         x400Address                     [3]     ORAddress,
            //         directoryName                   [4]     Name,
            //         ediPartyName                    [5]     EDIPartyName,
            //         uniformResourceIdentifier       [6]     IA5String,
            //         iPAddress                       [7]     OCTET STRING,
            //         registeredID                    [8]     OBJECT IDENTIFIER }
            result = mbedtls_asn1_get_tag(
                &p, end, &len, MBEDTLS_ASN1_CONTEXT_SPECIFIC | MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_X509_SAN_DIRECTORY_NAME);
            VerifyOrExit(result == 0, error = CHIP_ERROR_NOT_FOUND);
            VerifyOrExit(p + len == end_of_ext, error = CHIP_ERROR_NOT_FOUND);

            error = CopySpanToMutableSpan(ByteSpan(p, len), crlIssuer);
            SuccessOrExit(error);
        }
        p = end_of_ext;
    }

    VerifyOrExit(cdpExtCount == 1, error = CHIP_ERROR_NOT_FOUND);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) crlIssuer;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractSerialNumberFromX509Cert(const ByteSpan & certificate, MutableByteSpan & serialNumber)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    uint8_t * p      = nullptr;
    size_t len       = 0;
    mbedtls_x509_crt mbed_cert;

    mbedtls_x509_crt_init(&mbed_cert);

    result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(serial).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    len = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(serial).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
    VerifyOrExit(len <= serialNumber.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(serialNumber.data(), p, len);
    serialNumber.reduce_size(len);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) serialNumber;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

CHIP_ERROR ExtractVIDPIDFromX509Cert(const ByteSpan & certificate, AttestationCertVidPid & vidpid)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbed_cert;
    mbedtls_asn1_named_data * dnIterator = nullptr;
    AttestationCertVidPid vidpidFromCN;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    for (dnIterator = &mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(subject); dnIterator != nullptr;
         dnIterator = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(next))
    {
        DNAttrType attrType = DNAttrType::kUnspecified;
        if (OID_CMP(sOID_AttributeType_CommonName, dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid)))
        {
            attrType = DNAttrType::kCommonName;
        }
        else if (OID_CMP(sOID_AttributeType_MatterVendorId, dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid)))
        {
            attrType = DNAttrType::kMatterVID;
        }
        else if (OID_CMP(sOID_AttributeType_MatterProductId, dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid)))
        {
            attrType = DNAttrType::kMatterPID;
        }

        size_t val_len  = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(val).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        uint8_t * val_p = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(val).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
        error           = ExtractVIDPIDFromAttributeString(attrType, ByteSpan(val_p, val_len), vidpid, vidpidFromCN);
        SuccessOrExit(error);
    }

    // If Matter Attributes were not found use values extracted from the CN Attribute,
    // which might be uninitialized as well.
    if (!vidpid.Initialized())
    {
        vidpid = vidpidFromCN;
    }

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) certificate;
    (void) vidpid;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

namespace {
CHIP_ERROR ExtractRawDNFromX509Cert(bool extractSubject, const ByteSpan & certificate, MutableByteSpan & dn)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    uint8_t * p      = nullptr;
    size_t len       = 0;
    mbedtls_x509_crt mbedCertificate;

    VerifyOrReturnError(!certificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedCertificate);
    result = mbedtls_x509_crt_parse(&mbedCertificate, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    if (extractSubject)
    {
        len = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(subject_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        p   = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(subject_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    }
    else
    {
        len = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(issuer_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        p   = mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(issuer_raw).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
    }

    VerifyOrExit(len <= dn.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(dn.data(), p, len);
    dn.reduce_size(len);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedCertificate);

#else
    (void) certificate;
    (void) dn;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}
} // namespace

CHIP_ERROR ExtractSubjectFromX509Cert(const ByteSpan & certificate, MutableByteSpan & subject)
{
    return ExtractRawDNFromX509Cert(true, certificate, subject);
}

CHIP_ERROR ExtractIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & issuer)
{
    return ExtractRawDNFromX509Cert(false, certificate, issuer);
}

CHIP_ERROR ReplaceCertIfResignedCertFound(const ByteSpan & referenceCertificate, const ByteSpan * candidateCertificates,
                                          size_t candidateCertificatesCount, ByteSpan & outCertificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    uint8_t referenceSubjectBuf[kMaxCertificateDistinguishedNameLength];
    uint8_t referenceSKIDBuf[kSubjectKeyIdentifierLength];
    MutableByteSpan referenceSubject(referenceSubjectBuf);
    MutableByteSpan referenceSKID(referenceSKIDBuf);

    outCertificate = referenceCertificate;

    VerifyOrReturnError(candidateCertificates != nullptr && candidateCertificatesCount != 0, CHIP_NO_ERROR);

    ReturnErrorOnFailure(ExtractSubjectFromX509Cert(referenceCertificate, referenceSubject));
    ReturnErrorOnFailure(ExtractSKIDFromX509Cert(referenceCertificate, referenceSKID));

    for (size_t i = 0; i < candidateCertificatesCount; i++)
    {
        const ByteSpan candidateCertificate = candidateCertificates[i];
        uint8_t candidateSubjectBuf[kMaxCertificateDistinguishedNameLength];
        uint8_t candidateSKIDBuf[kSubjectKeyIdentifierLength];
        MutableByteSpan candidateSubject(candidateSubjectBuf);
        MutableByteSpan candidateSKID(candidateSKIDBuf);

        ReturnErrorOnFailure(ExtractSubjectFromX509Cert(candidateCertificate, candidateSubject));
        ReturnErrorOnFailure(ExtractSKIDFromX509Cert(candidateCertificate, candidateSKID));

        if (referenceSKID.data_equal(candidateSKID) && referenceSubject.data_equal(candidateSubject))
        {
            outCertificate = candidateCertificate;
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_NO_ERROR;
#else
    (void) referenceCertificate;
    (void) candidateCertificates;
    (void) candidateCertificatesCount;
    (void) outCertificate;
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)
}

} // namespace Crypto
} // namespace chip
