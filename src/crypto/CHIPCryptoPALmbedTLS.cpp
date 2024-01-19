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
 *      mbedTLS based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPALmbedTLS.h"
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
#include <mbedtls/version.h>
#include <mbedtls/x509_csr.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

typedef struct
{
    bool mInitialized;
    bool mDRBGSeeded;
    mbedtls_ctr_drbg_context mDRBGCtxt;
    mbedtls_entropy_context mEntropy;
} EntropyContext;

static EntropyContext gsEntropyContext;

static bool _isValidTagLength(size_t tag_length)
{
    if (tag_length == 8 || tag_length == 12 || tag_length == 16)
    {
        return true;
    }
    return false;
}

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

    // Size of key is expressed in bits, hence the multiplication by 8.
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

    // Size of key is expressed in bits, hence the multiplication by 8.
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

Hash_SHA256_stream::Hash_SHA256_stream()
{
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);
    mbedtls_sha256_init(context);
}

Hash_SHA256_stream::~Hash_SHA256_stream()
{
    Clear();
}

CHIP_ERROR Hash_SHA256_stream::Begin()
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

void Hash_SHA256_stream::Clear()
{
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);
    mbedtls_sha256_free(context);

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
#if !defined(MBEDTLS_DEPRECATED_REMOVED) || MBEDTLS_VERSION_NUMBER < 0x03030000
    const mbedtls_md_info_t * md_info;
    mbedtls_md_context_t md_ctxt;
    constexpr int use_hmac = 1;
    bool free_md_ctxt      = false;
#endif // !defined(MBEDTLS_DEPRECATED_REMOVED) || MBEDTLS_VERSION_NUMBER < 0x03030000

    VerifyOrExit(password != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen >= kSpake2p_Min_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen <= kSpake2p_Max_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
#if !defined(MBEDTLS_DEPRECATED_REMOVED) || MBEDTLS_VERSION_NUMBER < 0x03030000
    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(md_info != nullptr, error = CHIP_ERROR_INTERNAL);

    mbedtls_md_init(&md_ctxt);
    free_md_ctxt = true;

    result = mbedtls_md_setup(&md_ctxt, md_info, use_hmac);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_pkcs5_pbkdf2_hmac(&md_ctxt, Uint8::to_const_uchar(password), plen, Uint8::to_const_uchar(salt), slen,
                                       iteration_count, key_length, Uint8::to_uchar(output));
#else
    result = mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256, Uint8::to_const_uchar(password), plen, Uint8::to_const_uchar(salt),
                                           slen, iteration_count, key_length, Uint8::to_uchar(output));
#endif // !defined(MBEDTLS_DEPRECATED_REMOVED) || MBEDTLS_VERSION_NUMBER < 0x03030000
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
#if !defined(MBEDTLS_DEPRECATED_REMOVED) || MBEDTLS_VERSION_NUMBER < 0x03030000
    if (free_md_ctxt)
    {
        mbedtls_md_free(&md_ctxt);
    }
#endif // !defined(MBEDTLS_DEPRECATED_REMOVED) || MBEDTLS_VERSION_NUMBER < 0x03030000

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

static inline mbedtls_ecp_keypair * to_keypair(P256KeypairContext * context)
{
    return SafePointerCast<mbedtls_ecp_keypair *>(context);
}

static inline const mbedtls_ecp_keypair * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const mbedtls_ecp_keypair *>(context);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
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

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_sign(&ecdsa_ctxt.CHIP_CRYPTO_PAL_PRIVATE(grp), &r, &s, &ecdsa_ctxt.CHIP_CRYPTO_PAL_PRIVATE(d),
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

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    result = mbedtls_ecp_group_load(&ecp_grp, MapECPGroupId(remote_public_key.Type()));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result =
        mbedtls_ecp_point_read_binary(&ecp_grp, &ecp_pubkey, Uint8::to_const_uchar(remote_public_key), remote_public_key.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result =
        mbedtls_ecdh_compute_shared(&ecp_grp, &mpi_secret, &ecp_pubkey, &keypair->CHIP_CRYPTO_PAL_PRIVATE(d), CryptoRNG, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_write_binary(&mpi_secret, out_secret.Bytes(), secret_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(secret_length));

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
    Encoding::BufferWriter bbuf(output.Bytes(), len);
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
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    result = mbedtls_ecp_point_read_binary(&keypair->CHIP_CRYPTO_PAL_PRIVATE(grp), &keypair->CHIP_CRYPTO_PAL_PRIVATE(Q),
                                           Uint8::to_const_uchar(mPublicKey), mPublicKey.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    {
        const uint8_t * privkey = input.ConstBytes() + mPublicKey.Length();

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

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
#if defined(MBEDTLS_X509_CSR_WRITE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    size_t out_length;

    mbedtls_x509write_csr csr;
    mbedtls_x509write_csr_init(&csr);

    mbedtls_pk_context pk;
    pk.CHIP_CRYPTO_PAL_PRIVATE(pk_info) = mbedtls_pk_info_from_type(MBEDTLS_PK_ECKEY);
    pk.CHIP_CRYPTO_PAL_PRIVATE(pk_ctx)  = to_keypair(&mKeypair);
    VerifyOrExit(pk.CHIP_CRYPTO_PAL_PRIVATE(pk_info) != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    mbedtls_x509write_csr_set_key(&csr, &pk);

    mbedtls_x509write_csr_set_md_alg(&csr, MBEDTLS_MD_SHA256);

    // TODO: mbedTLS CSR parser fails if the subject name is not set (or if empty).
    //       CHIP Spec doesn't specify the subject name that can be used.
    //       Figure out the correct value and update this code.
    result = mbedtls_x509write_csr_set_subject_name(&csr, "O=CSR");
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_x509write_csr_der(&csr, out_csr, csr_length, CryptoRNG, nullptr);
    VerifyOrExit(result > 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(result), error = CHIP_ERROR_INTERNAL);

    out_length = static_cast<size_t>(result);
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

    _log_mbedTLS_error(result);
    return error;
#else
    ChipLogError(Crypto, "MBEDTLS_X509_CSR_WRITE_C is not enabled. CSR cannot be created");
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

typedef struct Spake2p_Context
{
    mbedtls_ecp_group curve;
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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    memset(context, 0, sizeof(Spake2p_Context));

    mbedtls_ecp_group_init(&context->curve);
    result = mbedtls_ecp_group_load(&context->curve, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256) != nullptr, error = CHIP_ERROR_INTERNAL);

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

} // namespace Crypto
} // namespace chip
