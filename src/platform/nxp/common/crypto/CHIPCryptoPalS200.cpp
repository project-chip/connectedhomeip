/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *      mbedTLS and S200 based implementation of CHIP crypto primitives
 */

#include <crypto/CHIPCryptoPAL.h>

#include <type_traits>

#include <mbedtls/bignum.h>
#include <mbedtls/ccm.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/ecp.h>
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
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string.h>

#include "sss_crypto.h"
extern "C" {
#include "SecLib.h"
}

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

typedef struct
{
    bool mInitialized;
    bool mDRBGSeeded;
    mbedtls_ctr_drbg_context mDRBGCtxt;
    entropy_source fn_source;
} DRBGContext;

static DRBGContext gsDrbgContext;

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
#if defined(USE_HW_AES)
    if (!aad_length)
    {
#endif
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
#if defined(USE_HW_AES)
    }
    else
    {
        // Encrypt
        result = AES_128_CCM(Uint8::to_const_uchar(plaintext), plaintext_length, Uint8::to_const_uchar(aad), aad_length,
                             Uint8::to_const_uchar(nonce), nonce_length, key.As<Symmetric128BitsKeyByteArray>(), ciphertext, tag,
                             tag_length, gSecLib_CCM_Encrypt_c);
        VerifyOrExit(result == kStatus_Success, error = CHIP_ERROR_INTERNAL);
    }
#endif
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
#if defined(USE_HW_AES)
    if (!aad_len)
    {
#endif
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
#if defined(USE_HW_AES)
    }
    else
    {
        // Decrypt
        result = AES_128_CCM(Uint8::to_const_uchar(ciphertext), ciphertext_len, Uint8::to_const_uchar(aad), aad_len,
                             Uint8::to_const_uchar(nonce), nonce_length, key.As<Symmetric128BitsKeyByteArray>(), plaintext,
                             (uint8_t *) tag, tag_length, gSecLib_CCM_Decrypt_c);
        VerifyOrExit(result == kStatus_Success, error = CHIP_ERROR_INTERNAL);
    }
#endif
exit:
    mbedtls_ccm_free(&context);
    return error;
}

CHIP_ERROR Hash_SHA256(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    // zero data length hash is supported.
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

#if defined(USE_HW_SHA256)
    SHA256_Hash(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer));
#else
#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result = mbedtls_sha256(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer), 0);
#else
    const int result = mbedtls_sha256_ret(Uint8::to_const_uchar(data), data_length, Uint8::to_uchar(out_buffer), 0);
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
#endif

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

#if defined(USE_HW_SHA256)
/*
 * These structures are used to save the intermediate
 * non-hashed data on heap (in a linked list)
 * and compute the hash on demand.
 * This solution bypases the sha256 context save/restore
 * S200 limitation.
 * */
typedef struct sha256_node
{
    uint8_t * data;
    uint16_t size;
    sha256_node * next;
} sha256_node;

typedef struct S200_context
{
    void * sss_context;
    sha256_node * head;
    sha256_node * tail;
} S200_context;

static_assert(kMAX_Hash_SHA256_Context_Size >= sizeof(S200_context),
              "kMAX_Hash_SHA256_Context_Size is too small for the size of underlying mbedtls_sha256_context");

static inline S200_context * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<S200_context *>(context);
}
#else
static_assert(kMAX_Hash_SHA256_Context_Size >= sizeof(mbedtls_sha256_context),
              "kMAX_Hash_SHA256_Context_Size is too small for the size of underlying mbedtls_sha256_context");

static inline mbedtls_sha256_context * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<mbedtls_sha256_context *>(context);
}
#endif

Hash_SHA256_stream::Hash_SHA256_stream(void)
{
#if defined(USE_HW_SHA256)
    S200_context * context = to_inner_hash_sha256_context(&mContext);

    context->sss_context = nullptr;
    context->head        = nullptr;
    context->tail        = nullptr;
#else
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);
    mbedtls_sha256_init(context);
#endif
}

Hash_SHA256_stream::~Hash_SHA256_stream(void)
{
#if defined(USE_HW_SHA256)
    S200_context * context = to_inner_hash_sha256_context(&mContext);

    context->sss_context = nullptr;
    context->head        = nullptr;
    context->tail        = nullptr;
#else
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);
    mbedtls_sha256_free(context);
#endif
    Clear();
}

CHIP_ERROR Hash_SHA256_stream::Begin(void)
{
#if defined(USE_HW_SHA256)
    S200_context * context = to_inner_hash_sha256_context(&mContext);
    context->sss_context   = SHA256_AllocCtx();

    SHA256_Init(context->sss_context);
#else
    mbedtls_sha256_context * const context = to_inner_hash_sha256_context(&mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result                       = mbedtls_sha256_starts(context, 0);
#else
    const int result = mbedtls_sha256_starts_ret(context, 0);
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const ByteSpan data)
{
#if defined(USE_HW_SHA256)
    S200_context * context = to_inner_hash_sha256_context(&mContext);
    sha256_node * node     = nullptr;

    VerifyOrReturnError(context->sss_context != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    SHA256_HashUpdate(context->sss_context, Uint8::to_const_uchar(data.data()), data.size());

    node       = (sha256_node *) malloc(sizeof(sha256_node));
    node->size = data.size();
    node->data = (uint8_t *) malloc(data.size());
    node->next = nullptr;
    memcpy(node->data, Uint8::to_const_uchar(data.data()), node->size);

    if (context->head == nullptr)
    {
        context->head = node;
        context->tail = node;
    }
    else
    {
        context->tail->next = node;
    }
#else
    mbedtls_sha256_context * const context = to_inner_hash_sha256_context(&mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result                       = mbedtls_sha256_update(context, Uint8::to_const_uchar(data.data()), data.size());
#else
    const int result = mbedtls_sha256_update_ret(context, Uint8::to_const_uchar(data.data()), data.size());
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::GetDigest(MutableByteSpan & out_buffer)
{
    CHIP_ERROR result = CHIP_NO_ERROR;

#if defined(USE_HW_SHA256)
    S200_context * context = to_inner_hash_sha256_context(&mContext);
    sha256_node * node     = context->head;
    void * ctx             = SHA256_AllocCtx();

    SHA256_Init(ctx);

    while (node)
    {
        SHA256_HashUpdate(ctx, node->data, node->size);
        node = node->next;
    }

    SHA256_HashFinish(ctx, Uint8::to_uchar(out_buffer.data()));

    SHA256_FreeCtx(ctx);
#else
    mbedtls_sha256_context * context = to_inner_hash_sha256_context(&mContext);

    // Back-up context as we are about to finalize the hash to extract digest.
    mbedtls_sha256_context previous_ctx;
    mbedtls_sha256_init(&previous_ctx);
    mbedtls_sha256_clone(&previous_ctx, context);

    // Pad + compute digest, then finalize context. It is restored next line to continue.
    result = Finish(out_buffer);

    // Restore context prior to finalization.
    mbedtls_sha256_clone(context, &previous_ctx);
    mbedtls_sha256_free(&previous_ctx);
#endif

    return result;
}

CHIP_ERROR Hash_SHA256_stream::Finish(MutableByteSpan & out_buffer)
{
#if defined(USE_HW_SHA256)
    S200_context * context = to_inner_hash_sha256_context(&mContext);

    sha256_node * node = nullptr;

    SHA256_HashFinish(context->sss_context, Uint8::to_uchar(out_buffer.data()));
    SHA256_FreeCtx(context->sss_context);

    while (context->head)
    {
        node          = context->head;
        context->head = context->head->next;
        free(node->data);
        free(node);
    }
#else
    VerifyOrReturnError(out_buffer.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    mbedtls_sha256_context * const context = to_inner_hash_sha256_context(&mContext);

#if (MBEDTLS_VERSION_NUMBER >= 0x03000000)
    const int result                       = mbedtls_sha256_finish(context, Uint8::to_uchar(out_buffer.data()));
#else
    const int result = mbedtls_sha256_finish_ret(context, Uint8::to_uchar(out_buffer.data()));
#endif

    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
    out_buffer                         = out_buffer.SubSpan(0, kSHA256_Hash_Length);
#endif

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

#if defined(USE_HW_SHA256)
    ::HMAC_SHA256(Uint8::to_const_uchar(key), (uint32_t) key_length, Uint8::to_const_uchar(message), (uint32_t) message_length,
                  out_buffer);
#else
    const mbedtls_md_info_t * const md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    VerifyOrReturnError(md != nullptr, CHIP_ERROR_INTERNAL);

    const int result =
        mbedtls_md_hmac(md, Uint8::to_const_uchar(key), key_length, Uint8::to_const_uchar(message), message_length, out_buffer);

    _log_mbedTLS_error(result);
    VerifyOrReturnError(result == 0, CHIP_ERROR_INTERNAL);
#endif

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

static int strong_entropy_func(void * data, uint8_t * output, size_t len)
{
    int result = -1;
    size_t olen;

    if (gsDrbgContext.fn_source)
    {
        result = gsDrbgContext.fn_source(data, output, len, &olen);
    }
    return result;
}

static mbedtls_ctr_drbg_context * get_drbg_context()
{
    if (!gsDrbgContext.mInitialized)
    {
        mbedtls_ctr_drbg_init(&gsDrbgContext.mDRBGCtxt);

        gsDrbgContext.mInitialized = true;
    }

    if (!gsDrbgContext.mDRBGSeeded)
    {
        const int status = mbedtls_ctr_drbg_seed(&gsDrbgContext.mDRBGCtxt, strong_entropy_func, nullptr, nullptr, 0);
        if (status != 0)
        {
            _log_mbedTLS_error(status);
            return nullptr;
        }

        gsDrbgContext.mDRBGSeeded = true;
    }

    return &gsDrbgContext.mDRBGCtxt;
}

CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
    VerifyOrReturnError(fn_source != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    gsDrbgContext.fn_source = fn_source;

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

static inline sss_sscp_object_t * to_keypair(P256KeypairContext * context)
{
    return SafePointerCast<sss_sscp_object_t *>(context);
}

static inline const sss_sscp_object_t * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const sss_sscp_object_t *>(context);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    sss_sscp_asymmetric_t asyc;
    size_t signatureSize = kP256_ECDSA_Signature_Length_Raw;

    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));
    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    VerifyOrExit((sss_sscp_asymmetric_context_init(&asyc, &g_sssSession, keypair, kAlgorithm_SSS_ECDSA_SHA256, kMode_SSS_Sign) ==
                  kStatus_SSS_Success),
                 CHIP_ERROR_INTERNAL);
    VerifyOrExit((sss_sscp_asymmetric_sign_digest(&asyc, digest, kP256_FE_Length, out_signature.Bytes(), &signatureSize) ==
                  kStatus_SSS_Success),
                 CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

exit:
    (void) sss_sscp_asymmetric_context_free(&asyc);
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

    sss_sscp_object_t ecdsaPublic;
    sss_sscp_asymmetric_t asyc;
    bool bFreeAsyncCtx = false;

    size_t coordinateLen     = kP256_FE_Length;   /* always 32 for P256 */
    size_t coordinateBitsLen = coordinateLen * 8; /* always 256 for P256 */
    size_t keySize           = SSS_ECP_KEY_SZ(coordinateLen);

    VerifyOrReturnError(sss_sscp_key_object_init(&ecdsaPublic, &g_keyStore) == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(sss_sscp_key_object_allocate_handle(&ecdsaPublic, 0u, kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P,
                                                            keySize, SSS_KEYPROP_OPERATION_ASYM) == kStatus_SSS_Success,
                        CHIP_ERROR_INTERNAL);

    // The first byte of the public key is the uncompressed marker
    VerifyOrExit(SSS_KEY_STORE_SET_KEY(&ecdsaPublic, Uint8::to_const_uchar(*this) + 1, Length() - 1, coordinateBitsLen,
                                       (uint32_t) kSSS_KeyPart_Public) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(sss_sscp_asymmetric_context_init(&asyc, &g_sssSession, &ecdsaPublic, kAlgorithm_SSS_ECDSA_SHA256,
                                                  kMode_SSS_Verify) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);

    bFreeAsyncCtx = true;
    VerifyOrExit(sss_sscp_asymmetric_verify_digest(&asyc, (uint8_t *) hash, hash_length, (uint8_t *) signature.ConstBytes(),
                                                   signature.Length()) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);
exit:

    if (bFreeAsyncCtx)
    {
        /* Need to be very careful, if we try to free something that is not initialized with success we will get an hw fault */
        (void) sss_sscp_asymmetric_context_free(&asyc);
    }
    (void) SSS_KEY_OBJ_FREE(&ecdsaPublic);

    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{

    CHIP_ERROR error     = CHIP_NO_ERROR;
    size_t secret_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();

    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    size_t coordinateLen     = kP256_FE_Length;   /* always 32 for P256 */
    size_t coordinateBitsLen = coordinateLen * 8; /* always 256 for P256 */
    size_t keySize           = SSS_ECP_KEY_SZ(coordinateLen);

    sss_sscp_derive_key_t dCtx;
    sss_sscp_object_t pEcdhPubKey;
    sss_sscp_object_t sharedSecret;

    bool bFreeSharedSecret = false;
    bool bFreeDeriveContex = false;

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    /* Remote public key */
    VerifyOrReturnError(sss_sscp_key_object_init(&pEcdhPubKey, &g_keyStore) == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(sss_sscp_key_object_allocate_handle(&pEcdhPubKey, 0u, kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P,
                                                            keySize, SSS_KEYPROP_OPERATION_KDF) == kStatus_SSS_Success,
                        CHIP_ERROR_INTERNAL);

    // The first byte of the public key is the uncompressed marker
    VerifyOrExit(SSS_KEY_STORE_SET_KEY(&pEcdhPubKey, Uint8::to_const_uchar(remote_public_key) + 1, keySize, coordinateBitsLen,
                                       kSSS_KeyPart_Public) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);

    /* Shared secret */
    VerifyOrExit(sss_sscp_key_object_init(&sharedSecret, &g_keyStore) == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(sss_sscp_key_object_allocate_handle(&sharedSecret, 0u, kSSS_KeyPart_Default, kSSS_CipherType_AES, coordinateLen,
                                                     SSS_KEYPROP_OPERATION_NONE) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);
    bFreeSharedSecret = true;

    /* Secret Key generated inside SSS */
    VerifyOrExit(sss_sscp_derive_key_context_init(&dCtx, &g_sssSession, keypair, kAlgorithm_SSS_ECDH,
                                                  kMode_SSS_ComputeSharedSecret) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);
    bFreeDeriveContex = true;

    VerifyOrExit(sss_sscp_asymmetric_dh_derive_key(&dCtx, &pEcdhPubKey, &sharedSecret) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(SSS_KEY_STORE_GET_PUBKEY(&sharedSecret, out_secret.Bytes(), &coordinateLen, &coordinateBitsLen) ==
                     kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(secret_length));

exit:
    (void) SSS_KEY_OBJ_FREE(&pEcdhPubKey);

    /* Need to be very careful, if we try to free something that is not initialized with success we will get a hw fault */
    if (bFreeSharedSecret)
        (void) SSS_KEY_OBJ_FREE(&sharedSecret);
    if (bFreeDeriveContex)
        (void) sss_sscp_derive_key_context_free(&dCtx);

    return error;
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

    size_t keyBitsLen = kP256_PrivateKey_Length * 8;
    size_t keySize    = SSS_ECP_KEY_SZ(kP256_PrivateKey_Length);

    Clear();

    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    VerifyOrReturnError(sss_sscp_key_object_init(keypair, &g_keyStore) == kStatus_SSS_Success, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(sss_sscp_key_object_allocate_handle(
                            keypair, 0x0u, kSSS_KeyPart_Pair, kSSS_CipherType_EC_NIST_P, 3 * kP256_PrivateKey_Length,
                            SSS_KEYPROP_OPERATION_KDF | SSS_KEYPROP_OPERATION_ASYM) == kStatus_SSS_Success,
                        error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(SSS_ECP_GENERATE_KEY(keypair, keyBitsLen) == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    // The first byte of the public key is the uncompressed marker
    Uint8::to_uchar(mPublicKey)[0] = 0x04;

    // Extract public key, write from the second byte
    VerifyOrExit(SSS_KEY_STORE_GET_PUBKEY(keypair, Uint8::to_uchar(mPublicKey) + 1, &keySize, &keyBitsLen) == kStatus_SSS_Success,
                 CHIP_ERROR_INTERNAL);

    mInitialized = true;

exit:
    if (mInitialized != true)
        (void) SSS_KEY_OBJ_FREE(keypair);

    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());
    CHIP_ERROR error = CHIP_NO_ERROR;

    Clear();

    const uint8_t * privkey     = input.ConstBytes() + mPublicKey.Length();
    sss_sscp_object_t * keypair = to_keypair(&mKeypair);

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    /* must load plain text private key inside SSS */
    VerifyOrExit((sss_sscp_key_object_init(keypair, &g_keyStore) == kStatus_SSS_Success), error = CHIP_ERROR_INTERNAL);

    /* Allocate key handle */
    VerifyOrExit(sss_sscp_key_object_allocate_handle(keypair, 0x0u, kSSS_KeyPart_Private, kSSS_CipherType_EC_NIST_P,
                                                     kP256_PrivateKey_Length, SSS_KEYPROP_OPERATION_ASYM) == kStatus_SSS_Success,
                 error = CHIP_ERROR_INTERNAL);

    if (SSS_KEY_STORE_SET_KEY(keypair, privkey, kP256_PrivateKey_Length, kP256_PrivateKey_Length * 8, kSSS_KeyPart_Private) !=
        kStatus_SSS_Success)
    {
        (void) SSS_KEY_OBJ_FREE(keypair);
        error = CHIP_ERROR_INTERNAL;
    }
    else
    {
        mInitialized = true;
    }

exit:
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        sss_sscp_object_t * keypair = to_keypair(&mKeypair);
        (void) SSS_KEY_OBJ_FREE(keypair);
        mInitialized = false;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    MutableByteSpan csr(out_csr, csr_length);
    CHIP_ERROR err = GenerateCertificateSigningRequest(this, csr);
    csr_length     = (CHIP_NO_ERROR == err) ? csr.size() : 0;
    return err;
}

typedef struct Spake2p_Context
{
    ecp256Point_t M;
    ecp256Point_t N;
    ecp256Point_t X;
    ecp256Point_t Y;
    ecp256Point_t L;
    ecp256Point_t Z;
    ecp256Point_t V;

    big_int256_t w0;
    big_int256_t w1;
    big_int256_t xy;
    big_int256_t tempbn;

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

    M = &context->M;
    N = &context->N;
    X = &context->X;
    Y = &context->Y;
    L = &context->L;
    V = &context->V;
    Z = &context->Z;

    w0     = &context->w0;
    w1     = &context->w1;
    xy     = &context->xy;
    tempbn = &context->tempbn;

    return error;

exit:
    _log_mbedTLS_error(result);
    Clear();
    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::Clear()
{
    VerifyOrReturn(state != CHIP_SPAKE2P_STATE::PREINIT);

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
    secEcp256Status_t result;
    uint32_t FE[SEC_ECP256_COORDINATE_WLEN];

    result = ECP256_ModularReductionN(FE, in, in_len);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    result = ECP256_FieldLoad((uint32_t *) fe, (const uint8_t *) FE, in_len);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, uint8_t * out, size_t out_len)
{
    secEcp256Status_t result;

    result = ECP256_FieldWrite(out, (uint8_t *) fe);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    secEcp256Status_t result;
    big_int256_t PrivateKey;

    result = ECP256_GeneratePrivateKey(&PrivateKey);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    result = ECP256_FieldWrite((uint8_t *) fe, (uint8_t *) &PrivateKey);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    secEcp256Status_t result;

    result = ECP256_ScalarMultiplicationModN((uint32_t *) fer, (const uint32_t *) fe1, (const uint32_t *) fe2);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const uint8_t * in, size_t in_len, void * R)
{
    ECP256_PointLoad((ecp256Point_t *) R, in, false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, uint8_t * out, size_t out_len)
{
    ECP256_PointWrite(out, (ecp256Point_t *) R, false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    secEcp256Status_t result;

    result = ECP256_PointMult((ecp256Point_t *) R, (const uint8_t *) P1, (const uint8_t *) fe1);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointAddMul(void * R, const void * P1, const void * fe1, const void * P2,
                                                      const void * fe2)
{
    secEcp256Status_t result;

    result = ECP256_DoublePointMulAdd(R, P1, fe1, P2, fe2);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointInvert(void * R)
{
    secEcp256Status_t result;

    result = ECP256_PointInvert((uint32_t *) R, (const uint32_t *) R);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointCofactorMul(void * R)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1in, size_t w1in_len)
{
    secEcp256Status_t result;
    ecp256Point_t gen_point;
    uint32_t W1[SEC_ECP256_COORDINATE_WLEN];

    result = ECP256_ModularReductionN(W1, w1in, w1in_len);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    result = ECP256_GeneratePublicKey((uint8_t *) &gen_point, (uint8_t *) &W1, NULL);
    VerifyOrReturnError(result == gSecEcp256Success_c, CHIP_ERROR_INTERNAL);

    ECP256_PointWrite(Lout, (ecp256Point_t *) &gen_point, false);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointIsValid(void * R)
{
    VerifyOrReturnError(ECP256_PointValid((ecp256Point_t *) R), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

} // namespace Crypto
} // namespace chip
