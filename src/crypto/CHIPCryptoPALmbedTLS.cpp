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
#include <mbedtls/x509_csr.h>

#include <core/CHIPSafeCasts.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>
#include <support/SafePointerCast.h>
#include <support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

#define MAX_ERROR_STR_LEN 128
#define NUM_BYTES_IN_SHA256_HASH 32

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

    VerifyOrExit(plaintext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
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

    VerifyOrExit(ciphertext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_len > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
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
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    // zero data length hash is supported.

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_sha256_ret(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer), 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA1(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    int result = 0;

    // zero data length hash is supported.
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_sha1_ret(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer));
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

Hash_SHA256_stream::Hash_SHA256_stream(void) {}

Hash_SHA256_stream::~Hash_SHA256_stream(void) {}

static inline mbedtls_sha256_context * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<mbedtls_sha256_context *>(context);
}

CHIP_ERROR Hash_SHA256_stream::Begin(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);

    result = mbedtls_sha256_starts_ret(context, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const uint8_t * data, const size_t data_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);

    result = mbedtls_sha256_update_ret(context, Uint8::to_const_uchar(data), data_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::Finish(uint8_t * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);

    result = mbedtls_sha256_finish_ret(context, Uint8::to_uchar(out_buffer));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

void Hash_SHA256_stream::Clear(void)
{
    memset(this, 0, sizeof(*this));
}

CHIP_ERROR HKDF_sha::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                 const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;
    const mbedtls_md_info_t * md;

    VerifyOrExit(secret != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Salt is optional
    if (salt_length > 0)
    {
        VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(md != nullptr, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_hkdf(md, Uint8::to_const_uchar(salt), salt_length, Uint8::to_const_uchar(secret), secret_length,
                          Uint8::to_const_uchar(info), info_length, Uint8::to_uchar(out_buffer), out_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
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
    VerifyOrExit(slen >= kMin_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen <= kMax_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
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
    EntropyContext * context = get_entropy_context();

    mbedtls_ctr_drbg_context * drbgCtxt = &context->mDRBGCtxt;

    if (!context->mDRBGSeeded)
    {
        int status = mbedtls_ctr_drbg_seed(drbgCtxt, mbedtls_entropy_func, &context->mEntropy, nullptr, 0);
        VerifyOrExit(status == 0, _log_mbedTLS_error(status));

        context->mDRBGSeeded = true;
    }

    return drbgCtxt;

exit:
    return nullptr;
}

CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    int result                    = 0;
    EntropyContext * entropy_ctxt = nullptr;

    VerifyOrExit(fn_source != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    entropy_ctxt = get_entropy_context();
    VerifyOrExit(entropy_ctxt != nullptr, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_entropy_add_source(&entropy_ctxt->mEntropy, fn_source, p_source, threshold, MBEDTLS_ENTROPY_SOURCE_STRONG);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
exit:
    return error;
}

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_ctr_drbg_context * drbg_ctxt = nullptr;

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    drbg_ctxt = get_drbg_context();
    VerifyOrExit(drbg_ctxt != nullptr, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ctr_drbg_random(drbg_ctxt, Uint8::to_uchar(out_buffer), out_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
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
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    uint8_t hash[NUM_BYTES_IN_SHA256_HASH];
    size_t siglen = out_signature.Capacity();

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_sha256_ret(Uint8::to_const_uchar(msg), msg_length, hash, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_write_signature(&ecdsa_ctxt, MBEDTLS_MD_SHA256, hash, sizeof(hash), Uint8::to_uchar(out_signature),
                                           &siglen, CryptoRNG, nullptr);
    SuccessOrExit(out_signature.SetLength(siglen));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    keypair = nullptr;
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR P256Keypair::ECDSA_sign_hash(const uint8_t * hash, const size_t hash_length, P256ECDSASignature & out_signature)
{
#if defined(MBEDTLS_ECDSA_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    size_t siglen    = out_signature.Capacity();

    const mbedtls_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length == NUM_BYTES_IN_SHA256_HASH, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_write_signature(&ecdsa_ctxt, MBEDTLS_MD_SHA256, hash, hash_length, Uint8::to_uchar(out_signature),
                                           &siglen, CryptoRNG, nullptr);
    SuccessOrExit(out_signature.SetLength(siglen));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    keypair = nullptr;
    mbedtls_ecdsa_free(&ecdsa_ctxt);
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
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    uint8_t hash[NUM_BYTES_IN_SHA256_HASH];

    mbedtls_ecp_keypair keypair;
    mbedtls_ecp_keypair_init(&keypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_group_load(&keypair.grp, MapECPGroupId(Type()));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_point_read_binary(&keypair.grp, &keypair.Q, Uint8::to_const_uchar(*this), Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, &keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_sha256_ret(Uint8::to_const_uchar(msg), msg_length, hash, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_read_signature(&ecdsa_ctxt, hash, sizeof(hash), Uint8::to_const_uchar(signature), signature.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    mbedtls_ecp_keypair_free(&keypair);
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
#if defined(MBEDTLS_ECDSA_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_ecp_keypair keypair;
    mbedtls_ecp_keypair_init(&keypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length == NUM_BYTES_IN_SHA256_HASH, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_group_load(&keypair.grp, MapECPGroupId(Type()));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_point_read_binary(&keypair.grp, &keypair.Q, Uint8::to_const_uchar(*this), Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, &keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_read_signature(&ecdsa_ctxt, hash, hash_length, Uint8::to_const_uchar(signature), signature.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    mbedtls_ecp_keypair_free(&keypair);
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    _log_mbedTLS_error(result);
    return error;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
#if defined(MBEDTLS_ECDH_C) && !defined(MBEDTLS_ECP_ALT)
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

    result = mbedtls_ecdh_compute_shared(&ecp_grp, &mpi_secret, &ecp_pubkey, &keypair->d, CryptoRNG, nullptr);
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

void ClearSecretData(uint8_t * buf, uint32_t len)
{
    memset(buf, 0, len);
}

CHIP_ERROR P256Keypair::Initialize()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    size_t pubkey_size = 0;

    mbedtls_ecp_group_id group = MapECPGroupId(mPublicKey.Type());

    mbedtls_ecp_keypair * keypair = to_keypair(&mKeypair);
    mbedtls_ecp_keypair_init(keypair);

    result = mbedtls_ecp_gen_key(group, keypair, CryptoRNG, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecp_point_write_binary(&keypair->grp, &keypair->Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size,
                                            Uint8::to_uchar(mPublicKey), mPublicKey.Length());
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
    VerifyOrExit(mbedtls_mpi_size(&keypair->d) <= bbuf.Available(), error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_write_binary(&keypair->d, Uint8::to_uchar(privkey), sizeof(privkey));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

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
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    int result       = 0;
    CHIP_ERROR error = CHIP_NO_ERROR;

    mbedtls_ecp_keypair * keypair = to_keypair(&mKeypair);
    mbedtls_ecp_keypair_init(keypair);

    result = mbedtls_ecp_group_load(&keypair->grp, MapECPGroupId(mPublicKey.Type()));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put((const uint8_t *) input, mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    result = mbedtls_ecp_point_read_binary(&keypair->grp, &keypair->Q, Uint8::to_const_uchar(mPublicKey), mPublicKey.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    {
        const uint8_t * privkey = Uint8::to_const_uchar(input) + mPublicKey.Length();
        result                  = mbedtls_mpi_read_binary(&keypair->d, privkey, kP256_PrivateKey_Length);
        VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    mInitialized = true;

exit:
    _log_mbedTLS_error(result);
    return error;
}

P256Keypair::~P256Keypair()
{
    if (mInitialized)
    {
        mbedtls_ecp_keypair * keypair = to_keypair(&mKeypair);
        mbedtls_ecp_keypair_free(keypair);
    }
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
    keypair = nullptr;
    mbedtls_x509write_csr_free(&csr);
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

    mbedtls_x509_csr csr;
    mbedtls_x509_csr_init(&csr);

    int result = mbedtls_x509_csr_parse_der(&csr, csr_buf, csr_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Verify the signature algorithm and public key type
    VerifyOrExit(csr.sig_md == MBEDTLS_MD_SHA256, error = CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE);
    VerifyOrExit(csr.sig_pk == MBEDTLS_PK_ECDSA, error = CHIP_ERROR_WRONG_KEY_TYPE);

    keypair = mbedtls_pk_ec(csr.pk);

    // Copy the public key from the CSR
    result = mbedtls_ecp_point_write_binary(&keypair->grp, &keypair->Q, MBEDTLS_ECP_PF_UNCOMPRESSED, &pubkey_size,
                                            Uint8::to_uchar(pubkey), pubkey.Length());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INTERNAL);

    // Verify the signature using the public key
    VerifyOrExit(kMax_ECDSA_Signature_Length >= csr.sig.len, error = CHIP_ERROR_INTERNAL);
    memmove(Uint8::to_uchar(signature), csr.sig.p, csr.sig.len);
    signature.SetLength(csr.sig.len);

    error = pubkey.ECDSA_validate_msg_signature(csr.cri.p, csr.cri.len, signature);
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

    FreeImpl();
    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::FreeImpl(void)
{
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
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, uint8_t * out)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_md_context_t hmac_ctx;
    mbedtls_md_init(&hmac_ctx);

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    result = mbedtls_md_setup(&hmac_ctx, context->md_info, 1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_md_hmac_starts(&hmac_ctx, Uint8::to_const_uchar(key), key_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_md_hmac_update(&hmac_ctx, Uint8::to_const_uchar(in), in_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_md_hmac_finish(&hmac_ctx, Uint8::to_uchar(out));
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    return error;

exit:
    _log_mbedTLS_error(result);

    mbedtls_md_free(&hmac_ctx);
    return error;
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

    VerifyOrExit(mac_len == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = Mac(key, key_len, in, in_len, computed_mac);
    SuccessOrExit(error);

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

    if (mbedtls_mpi_sub_mpi(&Rp->Y, &context->curve.P, &Rp->Y) != 0)
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
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Crypto
} // namespace chip
