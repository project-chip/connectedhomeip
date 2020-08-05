/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <mbedtls/bignum.h>
#include <mbedtls/ccm.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/entropy.h>
#include <mbedtls/error.h>
#include <mbedtls/hkdf.h>
#include <mbedtls/md.h>
#include <mbedtls/pkcs5.h>
#include <mbedtls/sha256.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

#define MAX_ERROR_STR_LEN 128
#define NUM_BYTES_IN_SHA256_HASH 32

typedef struct
{
    bool initialized;
    mbedtls_ctr_drbg_context drbg_ctxt;
    mbedtls_entropy_context entropy;
} EntropyContext;

static EntropyContext gsEntropyContext;

static void _log_mbedTLS_error(int error_code)
{
    if (error_code != 0)
    {
        char error_str[MAX_ERROR_STR_LEN];
        mbedtls_strerror(error_code, error_str, sizeof(error_str));
        ChipLogError(Crypto, "mbedTLS error: %s\n", error_str);
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

CHIP_ERROR AES_CCM_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad, size_t aad_length,
                           const unsigned char * key, size_t key_length, const unsigned char * iv, size_t iv_length,
                           unsigned char * ciphertext, unsigned char * tag, size_t tag_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    mbedtls_ccm_context context;
    mbedtls_ccm_init(&context);

    VerifyOrExit(plaintext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_length > 0)
    {
        VerifyOrExit(aad != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Size of key = key_length * number of bits in a byte (8)
    result = mbedtls_ccm_setkey(&context, MBEDTLS_CIPHER_ID_AES, key, key_length * 8);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Encrypt
    result = mbedtls_ccm_encrypt_and_tag(&context, plaintext_length, iv, iv_length, aad, aad_length, plaintext, ciphertext, tag,
                                         tag_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ccm_free(&context);
    return error;
}

CHIP_ERROR AES_CCM_decrypt(const unsigned char * ciphertext, size_t ciphertext_len, const unsigned char * aad, size_t aad_len,
                           const unsigned char * tag, size_t tag_length, const unsigned char * key, size_t key_length,
                           const unsigned char * iv, size_t iv_length, unsigned char * plaintext)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    mbedtls_ccm_context context;
    mbedtls_ccm_init(&context);

    VerifyOrExit(ciphertext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_len > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_len > 0)
    {
        VerifyOrExit(aad != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    // Size of key = key_length * number of bits in a byte (8)
    result = mbedtls_ccm_setkey(&context, MBEDTLS_CIPHER_ID_AES, key, key_length * 8);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    // Decrypt
    result =
        mbedtls_ccm_auth_decrypt(&context, ciphertext_len, iv, iv_length, aad, aad_len, ciphertext, plaintext, tag, tag_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ccm_free(&context);
    return error;
}

CHIP_ERROR Hash_SHA256(const unsigned char * data, const size_t data_length, unsigned char * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    // zero data length hash is supported.

    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_sha256_ret(data, data_length, out_buffer, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

Hash_SHA256_stream::Hash_SHA256_stream(void) {}

Hash_SHA256_stream::~Hash_SHA256_stream(void) {}

CHIP_ERROR Hash_SHA256_stream::Begin(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_sha256_starts_ret(&context, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const unsigned char * data, const size_t data_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_sha256_update_ret(&context, data, data_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::Finish(unsigned char * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_sha256_finish_ret(&context, out_buffer);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

void Hash_SHA256_stream::Clear(void)
{
    memset(this, 0, sizeof(*this));
}

CHIP_ERROR HKDF_SHA256(const unsigned char * secret, const size_t secret_length, const unsigned char * salt,
                       const size_t salt_length, const unsigned char * info, const size_t info_length, unsigned char * out_buffer,
                       size_t out_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;
    const mbedtls_md_info_t * md;

    VerifyOrExit(secret != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // Salt is optional
    if (salt_length > 0)
    {
        VerifyOrExit(salt != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(md != NULL, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_hkdf(md, salt, salt_length, secret, secret_length, info, info_length, out_buffer, out_length);
    _log_mbedTLS_error(result);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR pbkdf2_sha256(const unsigned char * password, size_t plen, const unsigned char * salt, size_t slen,
                         unsigned int iteration_count, uint32_t key_length, unsigned char * output)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    const mbedtls_md_info_t * md_info;
    mbedtls_md_context_t md_ctxt;
    constexpr int use_hmac = 1;

    bool free_md_ctxt = false;

    VerifyOrExit(password != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(md_info != NULL, error = CHIP_ERROR_INTERNAL);

    mbedtls_md_init(&md_ctxt);
    free_md_ctxt = true;

    result = mbedtls_md_setup(&md_ctxt, md_info, use_hmac);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_pkcs5_pbkdf2_hmac(&md_ctxt, password, plen, salt, slen, iteration_count, key_length, output);

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
    EntropyContext * context = NULL;
    int status               = 0;

    VerifyOrExit(!gsEntropyContext.initialized, context = &gsEntropyContext);

    mbedtls_entropy_init(&gsEntropyContext.entropy);
    mbedtls_ctr_drbg_init(&gsEntropyContext.drbg_ctxt);

    status = mbedtls_ctr_drbg_seed(&gsEntropyContext.drbg_ctxt, mbedtls_entropy_func, &gsEntropyContext.entropy, NULL, 0);
    VerifyOrExit(status == 0, _log_mbedTLS_error(status));

    gsEntropyContext.initialized = true;

    context = &gsEntropyContext;

exit:
    return context;
}

CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    int result                    = 0;
    EntropyContext * entropy_ctxt = NULL;

    VerifyOrExit(fn_source != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    entropy_ctxt = get_entropy_context();
    VerifyOrExit(entropy_ctxt != NULL, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_entropy_add_source(&entropy_ctxt->entropy, fn_source, p_source, threshold, MBEDTLS_ENTROPY_SOURCE_STRONG);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);
exit:
    return error;
}

CHIP_ERROR DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length)
{
    CHIP_ERROR error              = CHIP_NO_ERROR;
    int result                    = 0;
    EntropyContext * entropy_ctxt = NULL;

    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    entropy_ctxt = get_entropy_context();
    VerifyOrExit(entropy_ctxt != NULL, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ctr_drbg_random(&entropy_ctxt->drbg_ctxt, out_buffer, out_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

static int ECDSA_sign_rng(void * ctxt, unsigned char * out_buffer, size_t out_length)
{
    return (chip::Crypto::DRBG_get_bytes(out_buffer, out_length) == CHIP_NO_ERROR) ? 0 : 1;
}

CHIP_ERROR ECDSA_sign_msg(const unsigned char * msg, const size_t msg_length, const unsigned char * private_key,
                          const size_t private_key_length, unsigned char * out_signature, size_t & out_signature_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    unsigned char hash[NUM_BYTES_IN_SHA256_HASH];

    mbedtls_ecp_keypair keypair;
    mbedtls_ecp_keypair_init(&keypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    VerifyOrExit(msg != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(private_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(private_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature_length >= kMax_ECDSA_Signature_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_group_load(&keypair.grp, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_mpi_read_binary(&keypair.d, private_key, private_key_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, &keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_sha256_ret(msg, msg_length, hash, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_write_signature(&ecdsa_ctxt, MBEDTLS_MD_SHA256, hash, sizeof(hash), out_signature, &out_signature_length,
                                           ECDSA_sign_rng, NULL);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ecp_keypair_free(&keypair);
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR ECDSA_validate_msg_signature(const unsigned char * msg, const size_t msg_length, const unsigned char * public_key,
                                        const size_t public_key_length, const unsigned char * signature,
                                        const size_t signature_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    unsigned char hash[NUM_BYTES_IN_SHA256_HASH];

    mbedtls_ecp_keypair keypair;
    mbedtls_ecp_keypair_init(&keypair);

    mbedtls_ecdsa_context ecdsa_ctxt;
    mbedtls_ecdsa_init(&ecdsa_ctxt);

    VerifyOrExit(msg != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(public_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(public_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(signature != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(signature_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_group_load(&keypair.grp, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_point_read_binary(&keypair.grp, &keypair.Q, public_key, public_key_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdsa_from_keypair(&ecdsa_ctxt, &keypair);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_sha256_ret(msg, msg_length, hash, 0);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecdsa_read_signature(&ecdsa_ctxt, hash, sizeof(hash), signature, signature_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    mbedtls_ecp_keypair_free(&keypair);
    mbedtls_ecdsa_free(&ecdsa_ctxt);
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR ECDH_derive_secret(const unsigned char * remote_public_key, const size_t remote_public_key_length,
                              const unsigned char * local_private_key, const size_t local_private_key_length,
                              unsigned char * out_secret, size_t & out_secret_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_ecp_group ecp_grp;
    mbedtls_ecp_group_init(&ecp_grp);

    mbedtls_mpi mpi_secret;
    mbedtls_mpi_init(&mpi_secret);

    mbedtls_ecp_point ecp_pubkey;
    mbedtls_ecp_point_init(&ecp_pubkey);

    mbedtls_mpi mpi_privkey;
    mbedtls_mpi_init(&mpi_privkey);

    mbedtls_ecdh_context ecdh_ctxt;
    mbedtls_ecdh_init(&ecdh_ctxt);

    VerifyOrExit(remote_public_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(remote_public_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(local_private_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(local_private_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_secret != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_secret_length >= kMax_ECDH_Secret_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_group_load(&ecp_grp, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_read_binary(&mpi_privkey, local_private_key, local_private_key_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecp_point_read_binary(&ecp_grp, &ecp_pubkey, remote_public_key, remote_public_key_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = mbedtls_ecdh_compute_shared(&ecp_grp, &mpi_secret, &ecp_pubkey, &mpi_privkey, ECDSA_sign_rng, NULL);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_write_binary(&mpi_secret, out_secret, out_secret_length);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    mbedtls_ecdh_free(&ecdh_ctxt);
    mbedtls_ecp_group_free(&ecp_grp);
    mbedtls_mpi_free(&mpi_secret);
    mbedtls_ecp_point_free(&ecp_pubkey);
    mbedtls_mpi_free(&mpi_privkey);
    _log_mbedTLS_error(result);
    return error;
}

void ClearSecretData(uint8_t * buf, uint32_t len)
{
    memset(buf, 0, len);
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    memset(&context, 0, sizeof(context));
    mbedtls_ecp_group_init(&context.curve);
    result = mbedtls_ecp_group_load(&context.curve, MBEDTLS_ECP_DP_SECP256R1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    context.md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrExit(context.md_info != NULL, error = CHIP_ERROR_INTERNAL);

    mbedtls_ecp_point_init(&context.M);
    mbedtls_ecp_point_init(&context.N);
    mbedtls_ecp_point_init(&context.X);
    mbedtls_ecp_point_init(&context.Y);
    mbedtls_ecp_point_init(&context.L);
    mbedtls_ecp_point_init(&context.V);
    mbedtls_ecp_point_init(&context.Z);
    M = &context.M;
    N = &context.N;
    X = &context.X;
    Y = &context.Y;
    L = &context.L;
    V = &context.V;
    Z = &context.Z;

    mbedtls_mpi_init(&context.w0);
    mbedtls_mpi_init(&context.w1);
    mbedtls_mpi_init(&context.xy);
    mbedtls_mpi_init(&context.tempbn);
    w0     = &context.w0;
    w1     = &context.w1;
    xy     = &context.xy;
    tempbn = &context.tempbn;

    G     = &context.curve.G;
    order = &context.curve.N;

    return error;

exit:
    _log_mbedTLS_error(result);

    FreeImpl();
    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::FreeImpl(void)
{
    mbedtls_ecp_point_free(&context.M);
    mbedtls_ecp_point_free(&context.N);
    mbedtls_ecp_point_free(&context.X);
    mbedtls_ecp_point_free(&context.Y);
    mbedtls_ecp_point_free(&context.L);
    mbedtls_ecp_point_free(&context.Z);
    mbedtls_ecp_point_free(&context.V);

    mbedtls_mpi_free(&context.w0);
    mbedtls_mpi_free(&context.w1);
    mbedtls_mpi_free(&context.xy);
    mbedtls_mpi_free(&context.tempbn);

    mbedtls_ecp_group_free(&context.curve);
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Mac(const unsigned char * key, size_t key_len, const unsigned char * in, size_t in_len,
                                              unsigned char * out)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    mbedtls_md_context_t hmac_ctx;
    mbedtls_md_init(&hmac_ctx);
    result = mbedtls_md_setup(&hmac_ctx, context.md_info, 1);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_md_hmac_starts(&hmac_ctx, key, key_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_md_hmac_update(&hmac_ctx, in, in_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_md_hmac_finish(&hmac_ctx, out);
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
    const unsigned char * A = (const unsigned char *) a;
    const unsigned char * B = (const unsigned char *) b;
    unsigned char diff      = 0;

    for (size_t i = 0; i < n; i++)
    {
        diff |= (A[i] ^ B[i]);
    }

    return diff;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::MacVerify(const unsigned char * key, size_t key_len, const unsigned char * mac,
                                                    size_t mac_len, const unsigned char * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    unsigned char computed_mac[kSHA256_Hash_Length];

    VerifyOrExit(mac_len == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = Mac(key, key_len, in, in_len, computed_mac);
    SuccessOrExit(error);

    VerifyOrExit(constant_time_memcmp(mac, computed_mac, kSHA256_Hash_Length) == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const unsigned char * in, size_t in_len, void * fe)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_mpi_read_binary((mbedtls_mpi *) fe, in, in_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_mod_mpi((mbedtls_mpi *) fe, (mbedtls_mpi *) fe, (const mbedtls_mpi *) order);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

exit:
    _log_mbedTLS_error(result);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, unsigned char * out, size_t out_len)
{
    if (mbedtls_mpi_write_binary((const mbedtls_mpi *) fe, out, out_len) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    result = mbedtls_ecp_gen_privkey(&context.curve, (mbedtls_mpi *) fe, ECDSA_sign_rng, nullptr);
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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const unsigned char * in, size_t in_len, void * R)
{
    if (mbedtls_ecp_point_read_binary(&context.curve, (mbedtls_ecp_point *) R, in, in_len) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, unsigned char * out, size_t out_len)
{
    memset(out, 0, out_len);
    size_t mbedtls_out_len = out_len;

    if (mbedtls_ecp_point_write_binary(&context.curve, (const mbedtls_ecp_point *) R, MBEDTLS_ECP_PF_UNCOMPRESSED, &mbedtls_out_len,
                                       out, out_len) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    if (mbedtls_ecp_mul(&context.curve, (mbedtls_ecp_point *) R, (const mbedtls_mpi *) fe1, (const mbedtls_ecp_point *) P1,
                        ECDSA_sign_rng, nullptr) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointAddMul(void * R, const void * P1, const void * fe1, const void * P2,
                                                      const void * fe2)
{

    if (mbedtls_ecp_muladd(&context.curve, (mbedtls_ecp_point *) R, (const mbedtls_mpi *) fe1, (const mbedtls_ecp_point *) P1,
                           (const mbedtls_mpi *) fe2, (const mbedtls_ecp_point *) P2) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointInvert(void * R)
{
    mbedtls_ecp_point * Rp = (mbedtls_ecp_point *) R;
    if (mbedtls_mpi_sub_mpi(&Rp->Y, &context.curve.P, &Rp->Y) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointCofactorMul(void * R)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(unsigned char * Lout, size_t * L_len, const unsigned char * w1in,
                                                   size_t w1in_len)
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

    result = mbedtls_mpi_read_binary(&w1_bn, w1in, w1in_len);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_mpi_mod_mpi(&w1_bn, &w1_bn, &curve.N);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    result = mbedtls_ecp_mul(&curve, &Ltemp, &w1_bn, &curve.G, ECDSA_sign_rng, nullptr);
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    memset(Lout, 0, *L_len);

    result = mbedtls_ecp_point_write_binary(&curve, &Ltemp, MBEDTLS_ECP_PF_UNCOMPRESSED, L_len, Lout, *L_len);
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
    if (mbedtls_ecp_check_pubkey(&context.curve, (mbedtls_ecp_point *) R) != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
