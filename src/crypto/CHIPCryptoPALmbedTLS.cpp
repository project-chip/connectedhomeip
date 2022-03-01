/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      mbedTLS based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"

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
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/oid.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string.h>

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

typedef struct
{
    bool mInitialized;
    bool mDRBGSeeded;
    mbedtls_ctr_drbg_context mDRBGCtxt;
    mbedtls_entropy_context mEntropy;
} EntropyContext;

static EntropyContext gsEntropyContext;

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

static bool _isValidTagLength(size_t tag_length)
{
    if (tag_length == 8 || tag_length == 12 || tag_length == 16)
    {
        return true;
    }
    return false;
}

static bool _isValidKeyLength(size_t length)
{
    // 16 bytes key for AES-CCM-128, 32 for AES-CCM-256
    if (length == 16 || length == 32)
    {
        return true;
    }
    return false;
}

CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * key, size_t key_length, const uint8_t * iv, size_t iv_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    mbedtls_ccm_context context;
    mbedtls_ccm_init(&context);

    VerifyOrExit(plaintext != nullptr || plaintext_length == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext != nullptr || plaintext_length == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);
    VerifyOrExit(iv != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_length > 0)
    {
        VerifyOrExit(aad != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Size of key = key_length * number of bits in a byte (8)
    // Cast is safe because we called _isValidKeyLength above.
    result =
        mbedtls_ccm_setkey(&context, MBEDTLS_CIPHER_ID_AES, Uint8::to_const_uchar(key), static_cast<unsigned int>(key_length * 8));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Encrypt
    result = mbedtls_ccm_encrypt_and_tag(&context, plaintext_length, Uint8::to_const_uchar(iv), iv_length,
                                         Uint8::to_const_uchar(aad), aad_length, Uint8::to_const_uchar(plaintext),
                                         Uint8::to_uchar(ciphertext), Uint8::to_uchar(tag), tag_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ccm_free(&context);
    return error;
}

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_len, const uint8_t * aad, size_t aad_len,
                           const uint8_t * tag, size_t tag_length, const uint8_t * key, size_t key_length, const uint8_t * iv,
                           size_t iv_length, uint8_t * plaintext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    mbedtls_ccm_context context;
    mbedtls_ccm_init(&context);

    VerifyOrExit(plaintext != nullptr || ciphertext_len == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext != nullptr || ciphertext_len == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);
    VerifyOrExit(iv != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_len > 0)
    {
        VerifyOrExit(aad != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Size of key = key_length * number of bits in a byte (8)
    // Cast is safe because we called _isValidKeyLength above.
    result =
        mbedtls_ccm_setkey(&context, MBEDTLS_CIPHER_ID_AES, Uint8::to_const_uchar(key), static_cast<unsigned int>(key_length * 8));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Decrypt
    result = mbedtls_ccm_auth_decrypt(&context, ciphertext_len, Uint8::to_const_uchar(iv), iv_length, Uint8::to_const_uchar(aad),
                                      aad_len, Uint8::to_const_uchar(ciphertext), Uint8::to_uchar(plaintext),
                                      Uint8::to_const_uchar(tag), tag_length);
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
    mbedtls_sha256_clone(&previous_ctx, context);

    // Pad + compute digest, then finalize context. It is restored next line to continue.
    CHIP_ERROR result = Finish(out_buffer);

    // Restore context prior to finalization.
    mbedtls_sha256_clone(context, &previous_ctx);

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

CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
    VerifyOrReturnError(fn_source != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    EntropyContext * const entropy_ctxt = get_entropy_context();
    VerifyOrReturnError(entropy_ctxt != nullptr, CHIP_ERROR_INTERNAL);

    const int result =
        mbedtls_entropy_add_source(&entropy_ctxt->mEntropy, fn_source, p_source, threshold, MBEDTLS_ENTROPY_SOURCE_STRONG);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_ctr_drbg_context * const drbg_ctxt = get_drbg_context();
    VerifyOrReturnError(drbg_ctxt != nullptr, CHIP_ERROR_INTERNAL);

    const int result = mbedtls_ctr_drbg_random(drbg_ctxt, Uint8::to_uchar(out_buffer), out_length);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

static int CryptoRNG(void * ctxt, uint8_t * out_buffer, size_t out_length)
{
    return (chip::Crypto::DRBG_get_bytes(out_buffer, out_length) == CHIP_NO_ERROR) ? 0 : 1;
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

static inline mbedtls_ecp_keypair * to_keypair(P256KeypairContext * context)
{
    return SafePointerCast<mbedtls_ecp_keypair *>(context);
}

static inline const mbedtls_ecp_keypair * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const mbedtls_ecp_keypair *>(context);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature)
{
#if defined(MBEDTLS_ECDSA_C)
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    return ECDSA_sign_hash(&digest[0], sizeof(digest), out_signature);
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR P256Keypair::ECDSA_sign_hash(const uint8_t * hash, const size_t hash_length, P256ECDSASignature & out_signature)
{
#if defined(MBEDTLS_ECDSA_C)
    VerifyOrReturnError(mInitialized, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    mbedtls_mpi r, s;
    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_sign(&ecdsa_ctxt.CHIP_CRYPTO_PAL_PRIVATE(grp), &r, &s, &ecdsa_ctxt.CHIP_CRYPTO_PAL_PRIVATE(d),
                                Uint8::to_const_uchar(hash), hash_length, CryptoRNG, nullptr);

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit((mbedtls_mpi_size(&r) <= kP256_FE_Length) || (mbedtls_mpi_size(&s) <= kP256_FE_Length),
                 error = CHIP_ERROR_INTERNAL);

    // Concatenate r and s to output. Sizes were checked above.
    result = mbedtls_mpi_write_binary(&r, out_signature.Bytes() + 0u, kP256_FE_Length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_write_binary(&s, out_signature.Bytes() + kP256_FE_Length, kP256_FE_Length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

exit:
    keypair = nullptr;
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
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
#if defined(MBEDTLS_ECDSA_C)
    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(signature.Length() == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    mbedtls_mpi r, s;

    mbedtls_mpi_init(&r);
    mbedtls_mpi_init(&s);

    mbedtls_ecp_keypair keypair;
    mbedtls_ecp_keypair_init(&keypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    result = mbedtls_ecp_group_load(&keypair.CHIP_CRYPTO_PAL_PRIVATE(grp), MapECPGroupId(Type()));

    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_point_read_binary(&keypair.CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair.CHIP_CRYPTO_PAL_PRIVATE(Q),
                                           Uint8::to_const_uchar(*this), Length());

    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, &keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Read the <r, s> big nums from the signature
    result = mbedtls_mpi_read_binary(&r, Uint8::to_const_uchar(signature.ConstBytes()) + 0u, kP256_FE_Length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_read_binary(&s, Uint8::to_const_uchar(signature.ConstBytes()) + kP256_FE_Length, kP256_FE_Length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_verify(&ecdsa_ctxt.CHIP_CRYPTO_PAL_PRIVATE(grp), Uint8::to_const_uchar(hash), hash_length,
                                  &ecdsa_ctxt.CHIP_CRYPTO_PAL_PRIVATE(Q), &r, &s);

    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    mbedtls_ecp_keypair_free(&keypair);
    mbedtls_mpi_free(&s);
    mbedtls_mpi_free(&r);
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
#if defined(MBEDTLS_ECDH_C)
    CHIP_ERROR error     = CHIP_NO_ERROR;
    int result           = 0;
    size_t secret_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();

    mbedtls_ecp_group ecp_grp;
    mbedtls_ecp_group_init(&ecp_grp);

    mbedtls_mpi mpi_secret;
    mbedtls_mpi_init(&mpi_secret);

    mbedtls_ecp_point ecp_pubkey;
    mbedtls_ecp_point_init(&ecp_pubkey);

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);

    result = mbedtls_ecp_group_load(&ecp_grp, MapECPGroupId(remote_public_key.Type()));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result =
        mbedtls_ecp_point_read_binary(&ecp_grp, &ecp_pubkey, Uint8::to_const_uchar(remote_public_key), remote_public_key.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result =
        mbedtls_ecdh_compute_shared(&ecp_grp, &mpi_secret, &ecp_pubkey, &keypair->CHIP_CRYPTO_PAL_PRIVATE(d), CryptoRNG, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_write_binary(&mpi_secret, Uint8::to_uchar(out_secret), secret_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(out_secret.SetLength(secret_length));

exit:
    keypair = nullptr;
    mbedtls_ecp_group_free(&ecp_grp);
    mbedtls_mpi_free(&mpi_secret);
    mbedtls_ecp_point_free(&ecp_pubkey);
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

CHIP_ERROR P256Keypair::Initialize()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    size_t pubkey_size = 0;

    Clear();

    mbedtls_ecp_group_id group = MapECPGroupId(mPublicKey.Type());

    mbedtls_ecp_keypair * keypair = to_keypair(&mKeypair);
    mbedtls_ecp_keypair_init(keypair);

    result = mbedtls_ecp_gen_key(group, keypair, CryptoRNG, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result =
        mbedtls_ecp_point_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                       MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, Uint8::to_uchar(mPublicKey), mPublicKey.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pubkey_size == mPublicKey.Length(), error = CHIP_ERROR_INVALID_ARGUMENT);

    keypair      = nullptr;
    mInitialized = true;

exit:
    if (keypair != nullptr)
    {
        mbedtls_ecp_keypair_free(keypair);
        keypair = nullptr;
    }

    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);
    size_t len                          = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output, len);
    uint8_t privkey[kP256_PrivateKey_Length];
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    bbuf.Put(mPublicKey, mPublicKey.Length());

    VerifyOrExit(bbuf.Available() == sizeof(privkey), error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mbedtls_mpi_size(&keypair->CHIP_CRYPTO_PAL_PRIVATE(d)) <= bbuf.Available(), error = CHIP_ERROR_INTERNAL);
    result = mbedtls_mpi_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(d), Uint8::to_uchar(privkey), sizeof(privkey));

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    bbuf.Put(privkey, sizeof(privkey));
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    output.SetLength(bbuf.Needed());

exit:
    ClearSecretData(privkey, sizeof(privkey));
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    int result       = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;

    Clear();

    mbedtls_ecp_keypair * keypair = to_keypair(&mKeypair);
    mbedtls_ecp_keypair_init(keypair);

    result = mbedtls_ecp_group_load(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), MapECPGroupId(mPublicKey.Type()));

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put((const uint8_t *) input, mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    result = mbedtls_ecp_point_read_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                           Uint8::to_const_uchar(mPublicKey), mPublicKey.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    {
        const uint8_t * privkey = Uint8::to_const_uchar(input) + mPublicKey.Length();

        result = mbedtls_mpi_read_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(d), privkey, kP256_PrivateKey_Length);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    mInitialized = true;

exit:
    _log_mbedTLS_error(result);
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        mbedtls_ecp_keypair * keypair = to_keypair(&mKeypair);
        mbedtls_ecp_keypair_free(keypair);
        mInitialized = false;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    int result = 0;

    size_t out_length;

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    mbedtls_x509write_csr csr;
    mbedtls_x509write_csr_init(&csr);

    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);

    const mbedtls_pk_info_t * pk_info = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
    VerifyOrExit(pk_info != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);

    result = mbedtls_pk_setup(&pk, pk_info);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    memcpy(mbedtls_pk_ec(pk), keypair, sizeof(mbedtls_ecp_keypair));

    mbedtls_x509write_csr_set_key(&csr, &pk);

    mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);

    // TODO: mbedTLS CSR parser fails if the subject name is not set (or if empty).
    //       CHIP Spec doesn't specify the subject name that can be used.
    //       Figure out the correct value and update this code.
    result = mbedtls_x509write_csr_set_subject_name(&csr, "O=CSR");
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_x509write_csr_der(&csr, out_csr, csr_length, CryptoRNG, nullptr);
    VerifyOrExit(result > 0, error = CHIP_ERROR_INTERNAL);

    out_length = (size_t) result;
    result     = 0;
    VerifyOrExit(out_length <= csr_length, error = CHIP_ERROR_INTERNAL);

    if (csr_length != out_length)
    {
        // mbedTLS API writes the CSR at the end of the provided buffer.
        // Let's move it to the start of the buffer.
        size_t offset = csr_length - out_length;
        memmove(out_csr, &out_csr[offset], out_length);
    }

    csr_length = out_length;

exit:
    mbedtls_x509write_csr_free(&csr);

    // TODO: Figure-out why the next 2 lines are OK. Valgrind complains or crash occurs if either is deleted.
    // Oddly, the following `mbedtls_ecp_keypair_init` is needed to avoid a double-free
    // with the following pk_free, which is needed.
    mbedtls_ecp_keypair_init(mbedtls_pk_ec(pk));
    mbedtls_pk_free(&pk);

    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR VerifyCertificateSigningRequest(const uint8_t * csr_buf, size_t csr_length, P256PublicKey & pubkey)
{
#if defined(MBEDTLS_X509_CSR_PARSE_C)
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

    keypair = mbedtls_pk_ec(csr.CHIP_CRYPTO_PAL_PRIVATE(pk));

    // Copy the public key from the CSR
    result = mbedtls_ecp_point_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                            MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, Uint8::to_uchar(pubkey), pubkey.Length());

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INTERNAL);

    // Convert DER signature to raw signature
    error = EcdsaAsn1SignatureToRaw(kP256_FE_Length,
                                    ByteSpan{ csr.CHIP_CRYPTO_PAL_PRIVATE(sig).CHIP_CRYPTO_PAL_PRIVATE(p),
                                              csr.CHIP_CRYPTO_PAL_PRIVATE(sig).CHIP_CRYPTO_PAL_PRIVATE(len) },
                                    out_raw_sig_span);

    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_raw_sig_span.size() == (kP256_FE_Length * 2), error = CHIP_ERROR_INTERNAL);
    signature.SetLength(out_raw_sig_span.size());

    // Verify the signature using the public key
    error = pubkey.ECDSA_validate_msg_signature(csr.CHIP_CRYPTO_PAL_PRIVATE(cri).CHIP_CRYPTO_PAL_PRIVATE(p),
                                                csr.CHIP_CRYPTO_PAL_PRIVATE(cri).CHIP_CRYPTO_PAL_PRIVATE(len), signature);

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
    mbedtls_ecp_group curve;
    const mbedtls_md_info_t * md_info;
    mbedtls_ecp_point M;
    mbedtls_ecp_point N;
    mbedtls_ecp_point X;
    mbedtls_ecp_point Y;
    mbedtls_ecp_point L;
    mbedtls_ecp_point Z;
    mbedtls_ecp_point V;

    mbedtls_mpi w0;
    mbedtls_mpi w1;
    mbedtls_mpi xy;
    mbedtls_mpi tempbn;
} Spake2p_Context;

static inline Spake2p_Context * to_inner_spake2p_context(Spake2pOpaqueContext * context)
{
    return SafePointerCast<Spake2p_Context *>(context);
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    memset(context, 0, sizeof(Spake2p_Context));
    mbedtls_ecp_group_init(&context->curve);
    result = mbedtls_ecp_group_load(&context->curve, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    context->md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(context->md_info != nullptr, error = CHIP_ERROR_INTERNAL);

    mbedtls_ecp_point_init(&context->M);
    mbedtls_ecp_point_init(&context->N);
    mbedtls_ecp_point_init(&context->X);
    mbedtls_ecp_point_init(&context->Y);
    mbedtls_ecp_point_init(&context->L);
    mbedtls_ecp_point_init(&context->V);
    mbedtls_ecp_point_init(&context->Z);
    M = &context->M;
    N = &context->N;
    X = &context->X;
    Y = &context->Y;
    L = &context->L;
    V = &context->V;
    Z = &context->Z;

    mbedtls_mpi_init(&context->w0);
    mbedtls_mpi_init(&context->w1);
    mbedtls_mpi_init(&context->xy);
    mbedtls_mpi_init(&context->tempbn);
    w0     = &context->w0;
    w1     = &context->w1;
    xy     = &context->xy;
    tempbn = &context->tempbn;

    G     = &context->curve.G;
    order = &context->curve.N;

    return error;

exit:
    _log_mbedTLS_error(result);
    Clear();
    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::Clear()
{
    VerifyOrReturn(state != CHIP_SPAKE2P_STATE::PREINIT);

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    mbedtls_ecp_point_free(&context->M);
    mbedtls_ecp_point_free(&context->N);
    mbedtls_ecp_point_free(&context->X);
    mbedtls_ecp_point_free(&context->Y);
    mbedtls_ecp_point_free(&context->L);
    mbedtls_ecp_point_free(&context->Z);
    mbedtls_ecp_point_free(&context->V);

    mbedtls_mpi_free(&context->w0);
    mbedtls_mpi_free(&context->w1);
    mbedtls_mpi_free(&context->xy);
    mbedtls_mpi_free(&context->tempbn);

    mbedtls_ecp_group_free(&context->curve);

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

/**
 * This function implements constant time memcmp. It's good practice
 * to use constant time functions for cryptographic functions.
 */
static inline int constant_time_memcmp(const void * a, const void * b, size_t n)
{
    const uint8_t * A = (const uint8_t *) a;
    const uint8_t * B = (const uint8_t *) b;
    uint8_t diff      = 0;

    for (size_t i = 0; i < n; i++)
    {
        diff |= (A[i] ^ B[i]);
    }

    return diff;
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

    VerifyOrExit(constant_time_memcmp(mac, computed_mac, kSHA256_Hash_Length) == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const uint8_t * in, size_t in_len, void * fe)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_mpi_read_binary((mbedtls_mpi *) fe, Uint8::to_const_uchar(in), in_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_mod_mpi((mbedtls_mpi *) fe, (mbedtls_mpi *) fe, (const mbedtls_mpi *) order);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, uint8_t * out, size_t out_len)
{
    if (mbedtls_mpi_write_binary((const mbedtls_mpi *) fe, Uint8::to_uchar(out), out_len) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    result = mbedtls_ecp_gen_privkey(&context->curve, (mbedtls_mpi *) fe, CryptoRNG, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_mpi_mul_mpi((mbedtls_mpi *) fer, (const mbedtls_mpi *) fe1, (const mbedtls_mpi *) fe2);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_mod_mpi((mbedtls_mpi *) fer, (mbedtls_mpi *) fer, (const mbedtls_mpi *) order);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const uint8_t * in, size_t in_len, void * R)
{
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    if (mbedtls_ecp_point_read_binary(&context->curve, (mbedtls_ecp_point *) R, Uint8::to_const_uchar(in), in_len) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, uint8_t * out, size_t out_len)
{
    memset(out, 0, out_len);
    size_t mbedtls_out_len = out_len;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    if (mbedtls_ecp_point_write_binary(&context->curve, (const mbedtls_ecp_point *) R, MBEDTLS_ECP_PF_UNCOMPRESSED,
                                       &mbedtls_out_len, Uint8::to_uchar(out), out_len) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    if (mbedtls_ecp_mul(&context->curve, (mbedtls_ecp_point *) R, (const mbedtls_mpi *) fe1, (const mbedtls_ecp_point *) P1,
                        CryptoRNG, nullptr) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointAddMul(void * R, const void * P1, const void * fe1, const void * P2,
                                                      const void * fe2)
{
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    if (mbedtls_ecp_muladd(&context->curve, (mbedtls_ecp_point *) R, (const mbedtls_mpi *) fe1, (const mbedtls_ecp_point *) P1,
                           (const mbedtls_mpi *) fe2, (const mbedtls_ecp_point *) P2) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointInvert(void * R)
{
    mbedtls_ecp_point * Rp    = (mbedtls_ecp_point *) R;
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    if (mbedtls_mpi_sub_mpi(&Rp->CHIP_CRYPTO_PAL_PRIVATE(Y), &context->curve.P, &Rp->CHIP_CRYPTO_PAL_PRIVATE(Y)) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointCofactorMul(void * R)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1in, size_t w1in_len)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_ecp_group curve;
    mbedtls_mpi w1_bn;
    mbedtls_ecp_point Ltemp;

    mbedtls_ecp_group_init(&curve);
    mbedtls_mpi_init(&w1_bn);
    mbedtls_ecp_point_init(&Ltemp);

    result = mbedtls_ecp_group_load(&curve, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_read_binary(&w1_bn, Uint8::to_const_uchar(w1in), w1in_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_mod_mpi(&w1_bn, &w1_bn, &curve.N);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecp_mul(&curve, &Ltemp, &w1_bn, &curve.G, CryptoRNG, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    memset(Lout, 0, *L_len);

    result = mbedtls_ecp_point_write_binary(&curve, &Ltemp, MBEDTLS_ECP_PF_UNCOMPRESSED, L_len, Uint8::to_uchar(Lout), *L_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    mbedtls_ecp_point_free(&Ltemp);
    mbedtls_mpi_free(&w1_bn);
    mbedtls_ecp_group_free(&curve);

    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointIsValid(void * R)
{
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    if (mbedtls_ecp_check_pubkey(&context->curve, (mbedtls_ecp_point *) R) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR LoadCertsFromPKCS7(const char * pkcs7, X509DerCertificate * x509list, uint32_t * max_certs)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR LoadCertFromPKCS7(const char * pkcs7, X509DerCertificate * x509list, uint32_t n_cert)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GetNumberOfCertsFromPKCS7(const char * pkcs7, uint32_t * n_certs)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
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
    uint32_t flags;

    result = CertificateChainValidationResult::kInternalFrameworkError;

    VerifyOrReturnError(rootCertificate != nullptr && rootCertificateLen != 0,
                        (result = CertificateChainValidationResult::kRootArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturnError(caCertificate != nullptr && caCertificateLen != 0,
                        (result = CertificateChainValidationResult::kICAArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturnError(leafCertificate != nullptr && leafCertificateLen != 0,
                        (result = CertificateChainValidationResult::kLeafArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));

    mbedtls_x509_crt_init(&certChain);
    mbedtls_x509_crt_init(&rootCert);

    /* Start of chain  */
    mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(leafCertificate), leafCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kLeafFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Add the intermediate to the chain  */
    mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(caCertificate), caCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kICAFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Add the root to the chain */
    mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(rootCertificate), rootCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kRootFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Parse the root cert */
    mbedResult = mbedtls_x509_crt_parse(&rootCert, Uint8::to_const_uchar(rootCertificate), rootCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kRootFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Verify the chain against the root */
    mbedResult = mbedtls_x509_crt_verify(&certChain, &rootCert, NULL, NULL, &flags, NULL, NULL);

    switch (mbedResult)
    {
    case 0:
        VerifyOrExit(flags == 0, (result = CertificateChainValidationResult::kInternalFrameworkError, error = CHIP_ERROR_INTERNAL));
        result = CertificateChainValidationResult::kSuccess;
        break;
    case MBEDTLS_ERR_X509_CERT_VERIFY_FAILED:
        result = CertificateChainValidationResult::kChainInvalid;
        error  = CHIP_ERROR_CERT_NOT_TRUSTED;
        break;
    default:
        SuccessOrExit((result = CertificateChainValidationResult::kInternalFrameworkError, error = CHIP_ERROR_INTERNAL));
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

inline bool IsTimeGreaterThanEqual(const mbedtls_x509_time * const timeA, const mbedtls_x509_time * const timeB)
{
    return timeA->CHIP_CRYPTO_PAL_PRIVATE(year) > timeB->CHIP_CRYPTO_PAL_PRIVATE(year) ||
        (timeA->CHIP_CRYPTO_PAL_PRIVATE(year) == timeB->CHIP_CRYPTO_PAL_PRIVATE(year) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(mon) > timeB->CHIP_CRYPTO_PAL_PRIVATE(mon)) ||
        (timeA->CHIP_CRYPTO_PAL_PRIVATE(year) == timeB->CHIP_CRYPTO_PAL_PRIVATE(year) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(mon) == timeB->CHIP_CRYPTO_PAL_PRIVATE(mon) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(day) > timeB->CHIP_CRYPTO_PAL_PRIVATE(day)) ||
        (timeA->CHIP_CRYPTO_PAL_PRIVATE(year) == timeB->CHIP_CRYPTO_PAL_PRIVATE(year) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(mon) == timeB->CHIP_CRYPTO_PAL_PRIVATE(mon) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(day) == timeB->CHIP_CRYPTO_PAL_PRIVATE(day) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(hour) > timeB->CHIP_CRYPTO_PAL_PRIVATE(hour)) ||
        (timeA->CHIP_CRYPTO_PAL_PRIVATE(year) == timeB->CHIP_CRYPTO_PAL_PRIVATE(year) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(mon) == timeB->CHIP_CRYPTO_PAL_PRIVATE(mon) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(day) == timeB->CHIP_CRYPTO_PAL_PRIVATE(day) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(hour) == timeB->CHIP_CRYPTO_PAL_PRIVATE(hour) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(min) > timeB->CHIP_CRYPTO_PAL_PRIVATE(min)) ||
        (timeA->CHIP_CRYPTO_PAL_PRIVATE(year) == timeB->CHIP_CRYPTO_PAL_PRIVATE(year) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(mon) == timeB->CHIP_CRYPTO_PAL_PRIVATE(mon) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(day) == timeB->CHIP_CRYPTO_PAL_PRIVATE(day) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(hour) == timeB->CHIP_CRYPTO_PAL_PRIVATE(hour) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(min) == timeB->CHIP_CRYPTO_PAL_PRIVATE(min) &&
         timeA->CHIP_CRYPTO_PAL_PRIVATE(sec) >= timeB->CHIP_CRYPTO_PAL_PRIVATE(sec));
}

CHIP_ERROR IsCertificateValidAtIssuance(const ByteSpan & referenceCertificate, const ByteSpan & toBeEvaluatedCertificate)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbedReferenceCertificate;
    mbedtls_x509_crt mbedToBeEvaluatedCertificate;
    mbedtls_x509_time refNotBeforeTime;
    mbedtls_x509_time tbeNotBeforeTime;
    mbedtls_x509_time tbeNotAfterTime;
    int result;

    VerifyOrReturnError(!referenceCertificate.empty() && !toBeEvaluatedCertificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    mbedtls_x509_crt_init(&mbedReferenceCertificate);
    mbedtls_x509_crt_init(&mbedToBeEvaluatedCertificate);

    result = mbedtls_x509_crt_parse(&mbedReferenceCertificate, Uint8::to_const_uchar(referenceCertificate.data()),
                                    referenceCertificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_x509_crt_parse(&mbedToBeEvaluatedCertificate, Uint8::to_const_uchar(toBeEvaluatedCertificate.data()),
                                    toBeEvaluatedCertificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    refNotBeforeTime = mbedReferenceCertificate.CHIP_CRYPTO_PAL_PRIVATE(valid_from);
    tbeNotBeforeTime = mbedToBeEvaluatedCertificate.CHIP_CRYPTO_PAL_PRIVATE(valid_from);
    tbeNotAfterTime  = mbedToBeEvaluatedCertificate.CHIP_CRYPTO_PAL_PRIVATE(valid_to);

    // TODO: Handle PAA/PAI re-issue and enable below time validation
    // check if referenceCertificate is issued at or after tbeCertificate's notBefore timestamp
    // VerifyOrExit(IsTimeGreaterThanEqual(&refNotBeforeTime, &tbeNotBeforeTime), error = CHIP_ERROR_CERT_EXPIRED);

    // check if referenceCertificate is issued at or before tbeCertificate's notAfter timestamp
    // VerifyOrExit(IsTimeGreaterThanEqual(&tbeNotAfterTime, &refNotBeforeTime), error = CHIP_ERROR_CERT_EXPIRED);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbedReferenceCertificate);
    mbedtls_x509_crt_free(&mbedToBeEvaluatedCertificate);

#else
    (void) referenceCertificate;
    (void) toBeEvaluatedCertificate;
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
    result = mbedtls_x509_time_is_past(&mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE(valid_from));
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);

    // check if certificate's notAfter timestamp is later than current time.
    result = mbedtls_x509_time_is_future(&mbedCertificate.CHIP_CRYPTO_PAL_PRIVATE(valid_to));
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
    mbedtls_ecp_keypair * keypair = nullptr;
    size_t pubkey_size            = 0;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    keypair = mbedtls_pk_ec(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE(pk));
    // Copy the public key from the cert in raw point format
    result =
        mbedtls_ecp_point_write_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                       MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size, Uint8::to_uchar(pubkey.Bytes()), pubkey.Length());

    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INTERNAL);

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

CHIP_ERROR ExtractKIDFromX509Cert(bool isSKID, const ByteSpan & certificate, MutableByteSpan & kid)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbed_cert;
    unsigned char * p;
    const unsigned char * end;
    size_t len;

    constexpr uint8_t sOID_Extension_SubjectKeyIdentifier[]   = { 0x55, 0x1D, 0x0E };
    constexpr uint8_t sOID_Extension_AuthorityKeyIdentifier[] = { 0x55, 0x1D, 0x23 };

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    p   = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE(v3_ext).CHIP_CRYPTO_PAL_PRIVATE(p);
    end = mbed_cert.CHIP_CRYPTO_PAL_PRIVATE(v3_ext).CHIP_CRYPTO_PAL_PRIVATE(p) +
        mbed_cert.CHIP_CRYPTO_PAL_PRIVATE(v3_ext).CHIP_CRYPTO_PAL_PRIVATE(len);
    result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
    VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

    while (p < end)
    {
        int is_critical = 0;

        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_CONSTRUCTED | MBEDTLS_ASN1_SEQUENCE);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OID);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

        bool isRequiredKID = false;
        if (isSKID)
        {
            isRequiredKID =
                sizeof(sOID_Extension_SubjectKeyIdentifier) == len && memcmp(p, sOID_Extension_SubjectKeyIdentifier, len) == 0;
        }
        else
        {
            isRequiredKID =
                sizeof(sOID_Extension_AuthorityKeyIdentifier) == len && memcmp(p, sOID_Extension_AuthorityKeyIdentifier, len) == 0;
        }

        if (isRequiredKID)
        {
            size_t keyid_skip         = isSKID ? 2 : 4;
            constexpr size_t kid_size = 20;

            p += len;
            result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
            VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

            VerifyOrExit((len - keyid_skip) == kid_size, error = CHIP_ERROR_WRONG_CERT_TYPE);
            VerifyOrExit((len - keyid_skip) <= kid.size(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

            memcpy(kid.data(), p + keyid_skip, kid_size);
            if (kid.size() > kid_size)
            {
                kid.reduce_size(kid_size);
            }

            break;
        }

        p += len;

        mbedtls_asn1_get_bool(&p, end, &is_critical);
        result = mbedtls_asn1_get_tag(&p, end, &len, MBEDTLS_ASN1_OCTET_STRING);
        VerifyOrExit(result == 0, error = CHIP_ERROR_WRONG_CERT_TYPE);

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

namespace {

CHIP_ERROR ExtractDNAttributeFromX509Cert(const uint8_t * oidAttribute, size_t oidAttributeLen, const ByteSpan & certificate,
                                          uint16_t & id)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt mbed_cert;
    mbedtls_asn1_named_data * dnIterator    = nullptr;
    constexpr size_t dnAttributeSize        = 4;
    constexpr size_t dnAttributeStringSize  = dnAttributeSize + 1;
    char dnAttribute[dnAttributeStringSize] = { 0 };

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    for (dnIterator = &mbed_cert.CHIP_CRYPTO_PAL_PRIVATE(subject); dnIterator != nullptr;
         dnIterator = dnIterator->CHIP_CRYPTO_PAL_PRIVATE(next))
    {
        if (dnIterator != nullptr && dnIterator->CHIP_CRYPTO_PAL_PRIVATE(oid).CHIP_CRYPTO_PAL_PRIVATE(p) != nullptr &&
            dnIterator->CHIP_CRYPTO_PAL_PRIVATE(oid).CHIP_CRYPTO_PAL_PRIVATE(len) == oidAttributeLen &&
            memcmp(oidAttribute, dnIterator->CHIP_CRYPTO_PAL_PRIVATE(oid).CHIP_CRYPTO_PAL_PRIVATE(p),
                   dnIterator->CHIP_CRYPTO_PAL_PRIVATE(oid).CHIP_CRYPTO_PAL_PRIVATE(len)) == 0 &&
            dnIterator->CHIP_CRYPTO_PAL_PRIVATE(val).CHIP_CRYPTO_PAL_PRIVATE(p) != nullptr &&
            dnIterator->CHIP_CRYPTO_PAL_PRIVATE(val).CHIP_CRYPTO_PAL_PRIVATE(len) == dnAttributeSize)
        {
            // vendor id is of size 4, we should ensure the string is null terminated before passing in to strtoul to avoid
            // undefined behavior
            memcpy(dnAttribute, dnIterator->CHIP_CRYPTO_PAL_PRIVATE(val).CHIP_CRYPTO_PAL_PRIVATE(p), dnAttributeSize);
            dnAttribute[dnAttributeSize] = 0;
            VerifyOrExit(ArgParser::ParseInt(dnAttribute, id, 16), error = CHIP_ERROR_INTERNAL);
            break;
        }
    }

    // returning CHIP_ERROR_KEY_NOT_FOUND to indicate that the DN Attribute is not present in the certificate.
    VerifyOrExit(dnIterator != nullptr, error = CHIP_ERROR_KEY_NOT_FOUND);

exit:
    _log_mbedTLS_error(result);
    mbedtls_x509_crt_free(&mbed_cert);

#else
    (void) oidAttribute;
    (void) oidAttributeLen;
    (void) certificate;
    (void) id;
    CHIP_ERROR error = CHIP_ERROR_NOT_IMPLEMENTED;
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

    return error;
}

} // namespace

CHIP_ERROR ExtractDNAttributeFromX509Cert(MatterOid matterOid, const ByteSpan & certificate, uint16_t & id)
{
    constexpr uint8_t sOID_AttributeType_ChipVendorId[]  = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x01 };
    constexpr uint8_t sOID_AttributeType_ChipProductId[] = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x02 };

    switch (matterOid)
    {
    case MatterOid::kVendorId:
        id = VendorId::NotSpecified;
        return ExtractDNAttributeFromX509Cert(sOID_AttributeType_ChipVendorId, sizeof(sOID_AttributeType_ChipVendorId), certificate,
                                              id);
    case MatterOid::kProductId:
        id = 0; // PID not specified value
        return ExtractDNAttributeFromX509Cert(sOID_AttributeType_ChipProductId, sizeof(sOID_AttributeType_ChipProductId),
                                              certificate, id);
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

} // namespace Crypto
} // namespace chip
