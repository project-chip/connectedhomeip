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
 *      PSA Crypto API based implementation of CHIP crypto primitives
 *      with Silicon Labs SDK modifications
 */

#include <crypto/CHIPCryptoPAL.h>

#include <type_traits>

// Include version header to get configuration information
#include <mbedtls/version.h>

#if !defined(MBEDTLS_PSA_CRYPTO_C)
#error "This implementation needs PSA Crypto"
#endif

#if !defined(MBEDTLS_USE_PSA_CRYPTO)
#error "This implementation requires that PSA Crypto keys can be used for CSR generation"
#endif

#include "psa/crypto.h"

// Go straight for the driver wrappers for speed on plaintext keys
extern "C" {
#include "psa_crypto_core.h"
#include "psa_crypto_driver_wrappers.h"
}

// Includes needed for SPAKE2+ ECP operations
#include <mbedtls/bignum.h>
#include <mbedtls/ecp.h>

// Includes needed for certificate parsing
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/md.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

using chip::Platform::MemoryCalloc;
using chip::Platform::MemoryFree;

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

static void _log_PSA_error(psa_status_t status)
{
    if (status != 0)
    {
        // Error codes defined in 16-bit negative hex numbers. Ease lookup by printing likewise
        ChipLogError(Crypto, "PSA error: %ld", status);
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

/**
 * @brief Compare two times
 *
 * @param t1 First time to compare
 * @param t2 Second time to compare
 * @return int  0 If both times are idential to the second, -1 if t1 < t2, 1 if t1 > t2.
 */
static int timeCompare(mbedtls_x509_time * t1, mbedtls_x509_time * t2)
{
    VerifyOrReturnValue(t1->year >= t2->year, -1);
    VerifyOrReturnValue(t1->year <= t2->year, 1);
    // Same year
    VerifyOrReturnValue(t1->mon >= t2->mon, -1);
    VerifyOrReturnValue(t1->mon <= t2->mon, 1);
    // Same month
    VerifyOrReturnValue(t1->day >= t2->day, -1);
    VerifyOrReturnValue(t1->day <= t2->day, 1);
    // Same day
    VerifyOrReturnValue(t1->hour >= t2->hour, -1);
    VerifyOrReturnValue(t1->hour <= t2->hour, 1);
    // Same hour
    VerifyOrReturnValue(t1->min >= t2->min, -1);
    VerifyOrReturnValue(t1->min <= t2->min, 1);
    // Same minute
    VerifyOrReturnValue(t1->sec >= t2->sec, -1);
    VerifyOrReturnValue(t1->sec <= t2->sec, 1);
    // Same second
    return 0;
}

CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * key, size_t key_length, const uint8_t * nonce, size_t nonce_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
    CHIP_ERROR error          = CHIP_NO_ERROR;
    psa_status_t status       = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    size_t output_length      = 0;
    uint8_t * buffer          = nullptr;
    bool allocated_buffer     = false;

    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length == kAES_CCM128_Key_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length == kAES_CCM128_Key_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<int>(nonce_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    // If the ciphertext and tag outputs aren't a contiguous buffer, the PSA API requires buffer copying
    if (Uint8::to_uchar(ciphertext) + plaintext_length != Uint8::to_uchar(tag))
    {
        buffer           = (uint8_t *) MemoryCalloc(1, plaintext_length + tag_length);
        allocated_buffer = true;
        VerifyOrExit(buffer != nullptr, error = CHIP_ERROR_NO_MEMORY);
    }

    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attr, key_length * 8);
    psa_set_key_algorithm(&attr, PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8));
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_ENCRYPT);

    status = psa_driver_wrapper_aead_encrypt(
        &attr, Uint8::to_const_uchar(key), key_length, PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, tag_length),
        Uint8::to_const_uchar(nonce), nonce_length, Uint8::to_const_uchar(aad), aad_length, Uint8::to_const_uchar(plaintext),
        plaintext_length, allocated_buffer ? buffer : ciphertext, plaintext_length + tag_length, &output_length);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(output_length == plaintext_length + tag_length, error = CHIP_ERROR_INTERNAL);

    if (allocated_buffer)
    {
        memcpy(Uint8::to_uchar(ciphertext), buffer, plaintext_length);
        memcpy(Uint8::to_uchar(tag), buffer + plaintext_length, tag_length);
        memset(buffer, 0, plaintext_length + tag_length);
    }

exit:
    if (allocated_buffer)
    {
        MemoryFree(buffer);
    }
    psa_reset_key_attributes(&attr);
    return error;
}

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_len, const uint8_t * aad, size_t aad_len,
                           const uint8_t * tag, size_t tag_length, const uint8_t * key, size_t key_length, const uint8_t * nonce,
                           size_t nonce_length, uint8_t * plaintext)
{
    CHIP_ERROR error          = CHIP_NO_ERROR;
    psa_status_t status       = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    size_t output_length      = 0;
    uint8_t * buffer          = nullptr;
    bool allocated_buffer     = false;

    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length == kAES_CCM128_Key_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // If the ciphertext and tag outputs aren't a contiguous buffer, the PSA API requires buffer copying
    if (Uint8::to_const_uchar(ciphertext) + ciphertext_len != Uint8::to_const_uchar(tag))
    {
        buffer           = (uint8_t *) MemoryCalloc(1, ciphertext_len + tag_length);
        allocated_buffer = true;
        VerifyOrExit(buffer != nullptr, error = CHIP_ERROR_NO_MEMORY);
    }

    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_AES);
    psa_set_key_bits(&attr, key_length * 8);
    psa_set_key_algorithm(&attr, PSA_ALG_AEAD_WITH_AT_LEAST_THIS_LENGTH_TAG(PSA_ALG_CCM, 8));
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DECRYPT);

    if (allocated_buffer)
    {
        memcpy(buffer, ciphertext, ciphertext_len);
        memcpy(buffer + ciphertext_len, tag, tag_length);
    }

    status = psa_driver_wrapper_aead_decrypt(
        &attr, Uint8::to_const_uchar(key), key_length, PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, tag_length),
        Uint8::to_const_uchar(nonce), nonce_length, Uint8::to_const_uchar(aad), aad_len, allocated_buffer ? buffer : ciphertext,
        ciphertext_len + tag_length, plaintext, ciphertext_len, &output_length);

    if (allocated_buffer)
    {
        memset(buffer, 0, ciphertext_len + tag_length);
    }

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(output_length == ciphertext_len, error = CHIP_ERROR_INTERNAL);
exit:
    if (allocated_buffer)
    {
        MemoryFree(buffer);
    }

    psa_reset_key_attributes(&attr);
    return error;
}

CHIP_ERROR Hash_SHA256(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    size_t output_length = 0;

    psa_crypto_init();

    const psa_status_t result =
        psa_hash_compute(PSA_ALG_SHA_256, data, data_length, out_buffer, PSA_HASH_LENGTH(PSA_ALG_SHA_256), &output_length);

    VerifyOrReturnError(result == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA1(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    size_t output_length = 0;

    psa_crypto_init();

    const psa_status_t result =
        psa_hash_compute(PSA_ALG_SHA_1, data, data_length, out_buffer, PSA_HASH_LENGTH(PSA_ALG_SHA_1), &output_length);

    VerifyOrReturnError(result == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_1), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

static_assert(kMAX_Hash_SHA256_Context_Size >= sizeof(psa_hash_operation_t),
              "kMAX_Hash_SHA256_Context_Size is too small for the size of underlying psa_hash_operation_t");

static inline psa_hash_operation_t * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<psa_hash_operation_t *>(context);
}

Hash_SHA256_stream::Hash_SHA256_stream(void)
{
    psa_hash_operation_t * context             = to_inner_hash_sha256_context(&mContext);
    const psa_hash_operation_t initial_context = PSA_HASH_OPERATION_INIT;
    memcpy(context, &initial_context, sizeof(psa_hash_operation_t));
}

Hash_SHA256_stream::~Hash_SHA256_stream(void)
{
    psa_hash_operation_t * context = to_inner_hash_sha256_context(&mContext);
    psa_hash_abort(context);
    Clear();
}

CHIP_ERROR Hash_SHA256_stream::Begin(void)
{
    psa_crypto_init();

    psa_hash_operation_t * context = to_inner_hash_sha256_context(&mContext);
    *context                       = PSA_HASH_OPERATION_INIT;
    const psa_status_t result      = psa_hash_setup(context, PSA_ALG_SHA_256);

    VerifyOrReturnError(result == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const ByteSpan data)
{
    psa_hash_operation_t * context = to_inner_hash_sha256_context(&mContext);
    const psa_status_t result      = psa_hash_update(context, Uint8::to_const_uchar(data.data()), data.size());

    VerifyOrReturnError(result == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::GetDigest(MutableByteSpan & out_buffer)
{
    CHIP_ERROR result              = CHIP_NO_ERROR;
    psa_status_t status            = PSA_ERROR_BAD_STATE;
    size_t output_length           = 0;
    psa_hash_operation_t * context = to_inner_hash_sha256_context(&mContext);

    VerifyOrReturnError(out_buffer.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    // Clone the context first since calculating the digest finishes the operation
    psa_hash_operation_t digest_context = PSA_HASH_OPERATION_INIT;
    status                              = psa_hash_clone(context, &digest_context);

    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    // Calculate digest on the cloned context
    status = psa_hash_finish(&digest_context, Uint8::to_uchar(out_buffer.data()), out_buffer.size(), &output_length);

    VerifyOrExit(status == PSA_SUCCESS, result = CHIP_ERROR_INTERNAL);
    VerifyOrExit(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), result = CHIP_ERROR_INTERNAL);
exit:
    psa_hash_abort(&digest_context);
    return result;
}

CHIP_ERROR Hash_SHA256_stream::Finish(MutableByteSpan & out_buffer)
{
    psa_hash_operation_t * context = to_inner_hash_sha256_context(&mContext);
    size_t output_length           = 0;

    VerifyOrReturnError(out_buffer.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);
    const psa_status_t status = psa_hash_finish(context, Uint8::to_uchar(out_buffer.data()), out_buffer.size(), &output_length);

    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

void Hash_SHA256_stream::Clear(void)
{
    psa_hash_operation_t * context = to_inner_hash_sha256_context(&mContext);
    psa_hash_abort(context);
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

    CHIP_ERROR error                         = CHIP_NO_ERROR;
    psa_status_t status                      = PSA_ERROR_BAD_STATE;
    psa_key_derivation_operation_t operation = PSA_KEY_DERIVATION_OPERATION_INIT;

    psa_crypto_init();

    status = psa_key_derivation_setup(&operation, PSA_ALG_HKDF(PSA_ALG_SHA_256));
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    if (salt_length > 0)
    {
        status =
            psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_SALT, Uint8::to_const_uchar(salt), salt_length);
        VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    }

    status =
        psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_SECRET, Uint8::to_const_uchar(secret), secret_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_INFO, Uint8::to_const_uchar(info), info_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_output_bytes(&operation, out_buffer, out_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
exit:
    psa_key_derivation_abort(&operation);

    return error;
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

    CHIP_ERROR error              = CHIP_NO_ERROR;
    psa_status_t status           = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr     = PSA_KEY_ATTRIBUTES_INIT;
    psa_mac_operation_t operation = PSA_MAC_OPERATION_INIT;
    size_t output_length          = 0;

    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_HMAC);
    psa_set_key_bits(&attr, key_length * 8);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attr, PSA_ALG_HMAC(PSA_ALG_SHA_256));

    status = psa_driver_wrapper_mac_compute(&attr, Uint8::to_const_uchar(key), key_length, PSA_ALG_HMAC(PSA_ALG_SHA_256),
                                            Uint8::to_const_uchar(message), message_length, out_buffer, out_length, &output_length);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), error = CHIP_ERROR_INTERNAL);
exit:
    psa_mac_abort(&operation);
    psa_reset_key_attributes(&attr);
    return error;
}

CHIP_ERROR PBKDF2_sha256::pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                        unsigned int iteration_count, uint32_t key_length, uint8_t * output)
{
    // TODO: replace inlined algorithm with usage of the PSA key derivation API once implemented
    CHIP_ERROR error          = CHIP_NO_ERROR;
    psa_status_t status       = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    size_t output_length      = 0;

    // Align these buffers on the native data size to speed up the XOR
    static const size_t hash_size_in_native =
        ((PSA_HASH_LENGTH(PSA_ALG_SHA_256) + sizeof(unsigned int) - 1) / sizeof(unsigned int));
    static_assert(hash_size_in_native * sizeof(unsigned int) >= PSA_HASH_LENGTH(PSA_ALG_SHA_256));

    unsigned int md1_buffer[hash_size_in_native];
    unsigned int work_buffer[hash_size_in_native];
    uint8_t * md1  = (uint8_t *) md1_buffer;
    uint8_t * work = (uint8_t *) work_buffer;

    size_t use_len;
    unsigned char * out_p = output;
    uint8_t * U1          = (uint8_t *) MemoryCalloc(1, slen + 4);

    VerifyOrExit(U1 != nullptr, error = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(password != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen >= kSpake2p_Min_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen <= kSpake2p_Max_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_HMAC);
    psa_set_key_bits(&attr, plen * 8);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_HASH);
    psa_set_key_algorithm(&attr, PSA_ALG_HMAC(PSA_ALG_SHA_256));

    // Start with initializing the salt + counter
    memcpy(U1, salt, slen);
    U1[slen]     = 0;
    U1[slen + 1] = 0;
    U1[slen + 2] = 0;
    U1[slen + 3] = 1;

    // Loop until we have generated the requested key length
    while (key_length)
    {
        // U1 ends up in work
        status = psa_driver_wrapper_mac_compute(&attr, password, plen, PSA_ALG_HMAC(PSA_ALG_SHA_256), U1, slen + 4, work,
                                                PSA_HASH_LENGTH(PSA_ALG_SHA_256), &output_length);

        VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
        VerifyOrExit(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), error = CHIP_ERROR_INTERNAL);

        memcpy(md1, work, PSA_HASH_LENGTH(PSA_ALG_SHA_256));

        for (size_t i = 1; i < iteration_count; i++)
        {
            // U2 ends up in md1
            //

            status = psa_driver_wrapper_mac_compute(&attr, password, plen, PSA_ALG_HMAC(PSA_ALG_SHA_256), md1, sizeof(md1_buffer),
                                                    md1, sizeof(md1_buffer), &output_length);

            VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
            VerifyOrExit(output_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), error = CHIP_ERROR_INTERNAL);

            // U1 xor U2
            //
            for (size_t j = 0; j < hash_size_in_native; j++)
            {
                work_buffer[j] ^= md1_buffer[j];
            }
        }

        use_len = (key_length < PSA_HASH_LENGTH(PSA_ALG_SHA_256)) ? key_length : PSA_HASH_LENGTH(PSA_ALG_SHA_256);
        memcpy(out_p, work, use_len);

        key_length -= (uint32_t) use_len;
        out_p += use_len;

        for (size_t i = 4; i > 0; i--)
        {
            if (++U1[slen + i - 1] != 0)
            {
                break;
            }
        }
    }

exit:
    MemoryFree(U1);
    psa_reset_key_attributes(&attr);
    return error;
}

CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
    // PSA Crypto has its own entropy and doesn't support an override mechanism
    (void) fn_source;
    (void) p_source;
    (void) threshold;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    psa_crypto_init();
    const psa_status_t result = psa_generate_random(Uint8::to_uchar(out_buffer), out_length);
    VerifyOrReturnError(result == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

// CryptoRNG's definition is needed to use the mbedTLS-backed SPAKE2+ and certificate operations
static int CryptoRNG(void * ctxt, uint8_t * out_buffer, size_t out_length)
{
    return (chip::Crypto::DRBG_get_bytes(out_buffer, out_length) == CHIP_NO_ERROR) ? 0 : 1;
}

// Mapping function is used as part of the certificate operations
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

/*******************************************************************************
 *
 * WARNING: The default (base) implementation of P256Keypair is UNSAFE!
 *
 * Because of the way CHIPCryptoPAL has evolved, it is dictating how a base
 * P256Keypair should behave. This includes:
 *      * Allowing using a P256 key for both ECDSA and ECDH operations
 *      * Needing to support copying a key through Serialize()/Deserialize()
 *        operations. This can't easily be done opaquely on an opaque backend
 *        without convoluted forms of reference tracking.
 *      * Not including a way to figure out whether the created key is supposed
 *        to be ephemeral or long-lived.
 *      * Needing to support ingestion of specific-format keys through
 *        Deserialize() (to support e.g. the example DAC provider).
 *
 * These conditions have lead to the base implementation of this class in this
 * crypto backend being backed by a plaintext key buffer instead of opaque key
 * references. Usage of the base class is strongly discouraged, and is only
 * implemented to support the in-tree examples which instantiate keys of this
 * base class.
 *
 ******************************************************************************/

typedef struct
{
    uint8_t privkey[32];
    size_t bitlen;
} psa_plaintext_ecp_keypair;

static inline psa_plaintext_ecp_keypair * to_keypair(P256KeypairContext * context)
{
    return SafePointerCast<psa_plaintext_ecp_keypair *>(context);
}

static inline const psa_plaintext_ecp_keypair * to_const_keypair(const P256KeypairContext * context)
{
    return SafePointerCast<const psa_plaintext_ecp_keypair *>(context);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error                          = CHIP_NO_ERROR;
    psa_status_t status                       = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr                 = PSA_KEY_ATTRIBUTES_INIT;
    size_t output_length                      = 0;
    const psa_plaintext_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_WELL_UNINITIALIZED);
    VerifyOrExit((msg != nullptr) && (msg_length > 0), error = CHIP_ERROR_INVALID_ARGUMENT);

    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, keypair->bitlen);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_SIGN_MESSAGE);

    // use imported key to sign a message
    status =
        psa_driver_wrapper_sign_message(&attr, keypair->privkey, PSA_BITS_TO_BYTES(keypair->bitlen), PSA_ALG_ECDSA(PSA_ALG_SHA_256),
                                        msg, msg_length, out_signature.Bytes(), out_signature.Capacity(), &output_length);

    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(output_length == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INTERNAL);
    VerifyOrReturnError(out_signature.SetLength(output_length) == CHIP_NO_ERROR, CHIP_ERROR_INTERNAL);

exit:
    _log_PSA_error(status);
    psa_reset_key_attributes(&attr);

    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
    CHIP_ERROR error          = CHIP_NO_ERROR;
    psa_status_t status       = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

    VerifyOrExit((msg != nullptr) && (msg_length > 0), error = CHIP_ERROR_INVALID_ARGUMENT);

    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_VERIFY_MESSAGE);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    // use imported key to verify a message
    status = psa_driver_wrapper_verify_message(&attr, Uint8::to_const_uchar(*this), Length(), PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg,
                                               msg_length, signature.ConstBytes(), signature.Length());

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INVALID_SIGNATURE);
exit:
    _log_PSA_error(status);
    psa_reset_key_attributes(&attr);

    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
    VerifyOrReturnError(hash != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(hash_length == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(signature.Length() == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error          = CHIP_NO_ERROR;
    psa_status_t status       = PSA_ERROR_BAD_STATE;
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;

    // Step 1: import public key as volatile
    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, 256);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    // use imported key to verify a hash
    status = psa_driver_wrapper_verify_hash(&attr, Uint8::to_const_uchar(*this), Length(), PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash,
                                            hash_length, signature.ConstBytes(), signature.Length());

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INVALID_SIGNATURE);
exit:
    _log_PSA_error(status);
    psa_reset_key_attributes(&attr);
    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    // Todo: replace with driver call once key derivation through the driver wrapper has been figured out
    CHIP_ERROR error                          = CHIP_NO_ERROR;
    psa_status_t status                       = PSA_ERROR_BAD_STATE;
    mbedtls_svc_key_id_t key_id               = 0;
    psa_key_attributes_t attr                 = PSA_KEY_ATTRIBUTES_INIT;
    size_t output_length                      = 0;
    const psa_plaintext_ecp_keypair * keypair = to_const_keypair(&mKeypair);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_WELL_UNINITIALIZED);

    // Step 1: import plaintext key as volatile for ECDH
    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, keypair->bitlen);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDH);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DERIVE);

    status = psa_import_key(&attr, keypair->privkey, PSA_BITS_TO_BYTES(keypair->bitlen), &key_id);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    // Step 2: do key derivation
    status = psa_raw_key_agreement(PSA_ALG_ECDH, key_id, Uint8::to_const_uchar(remote_public_key), remote_public_key.Length(),
                                   Uint8::to_uchar(out_secret),
                                   (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length(), &output_length);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(out_secret.SetLength(output_length));

exit:
    _log_PSA_error(status);
    // Step 3: destroy imported key
    psa_reset_key_attributes(&attr);
    if (key_id != 0)
    {
        psa_destroy_key(key_id);
    }
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

CHIP_ERROR P256Keypair::Initialize()
{
    CHIP_ERROR error                    = CHIP_NO_ERROR;
    psa_status_t status                 = PSA_ERROR_BAD_STATE;
    psa_plaintext_ecp_keypair * keypair = to_keypair(&mKeypair);
    size_t output_length;

    if (mInitialized)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Step 1: Generate a volatile new key
    psa_key_attributes_t attr = PSA_KEY_ATTRIBUTES_INIT;
    psa_crypto_init();

    psa_set_key_type(&attr, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attr, 256);
    psa_set_key_algorithm(&attr, PSA_ALG_ECDH);
    psa_set_key_usage_flags(&attr, PSA_KEY_USAGE_DERIVE | PSA_KEY_USAGE_EXPORT);

    status = psa_driver_wrapper_generate_key(&attr, keypair->privkey, sizeof(keypair->privkey), &output_length);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(output_length == kP256_PrivateKey_Length, error = CHIP_ERROR_INTERNAL);

    keypair->bitlen = 256;

    // Step 2: Export the public key into the pubkey member
    status = psa_driver_wrapper_export_public_key(&attr, keypair->privkey, sizeof(keypair->privkey), Uint8::to_uchar(mPublicKey),
                                                  mPublicKey.Length(), &output_length);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(output_length == kP256_PublicKey_Length, error = CHIP_ERROR_INTERNAL);

exit:
    _log_PSA_error(status);
    if (error == CHIP_NO_ERROR)
    {
        mInitialized = true;
    }
    psa_reset_key_attributes(&attr);

    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    CHIP_ERROR error                          = CHIP_NO_ERROR;
    const psa_plaintext_ecp_keypair * keypair = to_const_keypair(&mKeypair);
    size_t len                                = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output, len);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_WELL_UNINITIALIZED);

    bbuf.Put(mPublicKey, mPublicKey.Length());

    VerifyOrExit(bbuf.Available() == sizeof(keypair->privkey), error = CHIP_ERROR_INTERNAL);

    bbuf.Put(keypair->privkey, PSA_BITS_TO_BYTES(keypair->bitlen));
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_BUFFER_TOO_SMALL);

    output.SetLength(bbuf.Needed());

exit:
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    CHIP_ERROR error                    = CHIP_NO_ERROR;
    psa_plaintext_ecp_keypair * keypair = to_keypair(&mKeypair);
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    Clear();

    memcpy(keypair->privkey, Uint8::to_uchar(input) + mPublicKey.Length(), kP256_PrivateKey_Length);
    keypair->bitlen = 256;

    bbuf.Put((const uint8_t *) input, mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    mInitialized = true;

exit:
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        psa_plaintext_ecp_keypair * keypair = to_keypair(&mKeypair);
        memset(keypair, 0, sizeof(psa_plaintext_ecp_keypair));
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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal(void)
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

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen,
                                    CertificateChainValidationResult & result)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    CHIP_ERROR error = CHIP_NO_ERROR;
    mbedtls_x509_crt certChain;
    mbedtls_x509_crt rootCert;
    mbedtls_x509_time leaf_valid_from;
    mbedtls_x509_crt * cert = NULL;
    int mbedResult;
    uint32_t flags;
    int compare_from  = 0;
    int compare_until = 0;

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
    leaf_valid_from = certChain.valid_from;

    /* Add the intermediate to the chain, if present */
    if (caCertificate != nullptr && caCertificateLen > 0)
    {
        mbedResult = mbedtls_x509_crt_parse(&certChain, Uint8::to_const_uchar(caCertificate), caCertificateLen);
        VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kICAFormatInvalid, error = CHIP_ERROR_INTERNAL));
    }

    /* Parse the root cert */
    mbedResult = mbedtls_x509_crt_parse(&rootCert, Uint8::to_const_uchar(rootCertificate), rootCertificateLen);
    VerifyOrExit(mbedResult == 0, (result = CertificateChainValidationResult::kRootFormatInvalid, error = CHIP_ERROR_INTERNAL));

    /* Validates that intermediate and root certificates are valid at the time of the leaf certificate's start time.  */
    compare_from  = timeCompare(&leaf_valid_from, &rootCert.valid_from);
    compare_until = timeCompare(&leaf_valid_from, &rootCert.valid_to);
    VerifyOrExit((compare_from >= 0) && (compare_until <= 0),
                 (result = CertificateChainValidationResult::kChainInvalid, error = CHIP_ERROR_CERT_NOT_TRUSTED));
    cert = certChain.next;
    while (cert)
    {
        compare_from  = timeCompare(&leaf_valid_from, &cert->valid_from);
        compare_until = timeCompare(&leaf_valid_from, &cert->valid_to);
        VerifyOrExit((compare_from >= 0) && (compare_until <= 0),
                     (result = CertificateChainValidationResult::kChainInvalid, error = CHIP_ERROR_CERT_NOT_TRUSTED));
        cert = cert->next;
    }

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

    refNotBeforeTime = mbedReferenceCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from);
    tbeNotBeforeTime = mbedToBeEvaluatedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_from);
    tbeNotAfterTime  = mbedToBeEvaluatedCertificate.CHIP_CRYPTO_PAL_PRIVATE_X509(valid_to);

    // check if referenceCertificate is issued at or after tbeCertificate's notBefore timestamp
    VerifyOrExit(IsTimeGreaterThanEqual(&refNotBeforeTime, &tbeNotBeforeTime), error = CHIP_ERROR_CERT_EXPIRED);

    // check if referenceCertificate is issued at or before tbeCertificate's notAfter timestamp
    VerifyOrExit(IsTimeGreaterThanEqual(&tbeNotAfterTime, &refNotBeforeTime), error = CHIP_ERROR_CERT_EXPIRED);

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
    mbedtls_ecp_keypair * keypair = nullptr;
    size_t pubkey_size            = 0;

    mbedtls_x509_crt_init(&mbed_cert);

    int result = mbedtls_x509_crt_parse(&mbed_cert, Uint8::to_const_uchar(certificate.data()), certificate.size());
    VerifyOrExit(result == 0, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mbedtls_pk_get_type(&(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(pk))) == MBEDTLS_PK_ECKEY,
                 error = CHIP_ERROR_INVALID_ARGUMENT);

    keypair = mbedtls_pk_ec(mbed_cert.CHIP_CRYPTO_PAL_PRIVATE_X509(pk));
    VerifyOrExit(keypair->CHIP_CRYPTO_PAL_PRIVATE(grp).id == MapECPGroupId(pubkey.Type()), error = CHIP_ERROR_INVALID_ARGUMENT);
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

CHIP_ERROR ExtractKIDFromX509Cert(bool extractSKID, const ByteSpan & certificate, MutableByteSpan & kid)
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

        bool extractCurrentExtSKID = extractSKID && (sizeof(sOID_Extension_SubjectKeyIdentifier) == len) &&
            (memcmp(p, sOID_Extension_SubjectKeyIdentifier, len) == 0);
        bool extractCurrentExtAKID = !extractSKID && (sizeof(sOID_Extension_AuthorityKeyIdentifier) == len) &&
            (memcmp(p, sOID_Extension_AuthorityKeyIdentifier, len) == 0);
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

CHIP_ERROR ExtractVIDPIDFromX509Cert(const ByteSpan & certificate, AttestationCertVidPid & vidpid)
{
#if defined(MBEDTLS_X509_CRT_PARSE_C)
    constexpr uint8_t sOID_AttributeType_CommonName[]      = { 0x55, 0x04, 0x03 };
    constexpr uint8_t sOID_AttributeType_MatterVendorId[]  = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x01 };
    constexpr uint8_t sOID_AttributeType_MatterProductId[] = { 0x2B, 0x06, 0x01, 0x04, 0x01, 0x82, 0xA2, 0x7C, 0x02, 0x02 };

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
        size_t oid_len  = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        uint8_t * oid_p = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(oid).CHIP_CRYPTO_PAL_PRIVATE_X509(p);
        size_t val_len  = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(val).CHIP_CRYPTO_PAL_PRIVATE_X509(len);
        uint8_t * val_p = dnIterator->CHIP_CRYPTO_PAL_PRIVATE_X509(val).CHIP_CRYPTO_PAL_PRIVATE_X509(p);

        if (oid_p != nullptr && val_p != nullptr)
        {
            DNAttrType attrType = DNAttrType::kUnspecified;
            if ((oid_len == sizeof(sOID_AttributeType_CommonName)) && (memcmp(sOID_AttributeType_CommonName, oid_p, oid_len) == 0))
            {
                attrType = DNAttrType::kCommonName;
            }
            else if ((oid_len == sizeof(sOID_AttributeType_MatterVendorId)) &&
                     (memcmp(sOID_AttributeType_MatterVendorId, oid_p, oid_len) == 0))
            {
                attrType = DNAttrType::kMatterVID;
            }
            else if ((oid_len == sizeof(sOID_AttributeType_MatterProductId)) &&
                     (memcmp(sOID_AttributeType_MatterProductId, oid_p, oid_len) == 0))
            {
                attrType = DNAttrType::kMatterPID;
            }

            error = ExtractVIDPIDFromAttributeString(attrType, ByteSpan(val_p, val_len), vidpid, vidpidFromCN);
            SuccessOrExit(error);
        }
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

} // namespace Crypto
} // namespace chip
