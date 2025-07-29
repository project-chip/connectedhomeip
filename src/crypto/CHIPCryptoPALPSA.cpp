/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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
 */

#include "CHIPCryptoPALPSA.h"
#include "CHIPCryptoPALmbedTLS.h"

#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CHIPArgParser.hpp>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/SafePointerCast.h>
#include <lib/support/logging/CHIPLogging.h>

#include <psa/crypto.h>

#include <mbedtls/bignum.h>
#include <mbedtls/ecp.h>
#include <mbedtls/error.h>
#include <mbedtls/x509_csr.h>

#include <string.h>
#include <type_traits>

namespace chip {
namespace Crypto {

namespace {

bool isBufferNonEmpty(const uint8_t * data, size_t data_length)
{
    return data != nullptr && data_length > 0;
}

bool isValidTag(const uint8_t * tag, size_t tag_length)
{
    return tag != nullptr && (tag_length == 8 || tag_length == 12 || tag_length == 16);
}

} // namespace

CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const Aes128KeyHandle & key, const uint8_t * nonce, size_t nonce_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
    VerifyOrReturnError(isBufferNonEmpty(nonce, nonce_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isValidTag(tag, tag_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((ciphertext != nullptr && plaintext != nullptr) || plaintext_length == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aad != nullptr || aad_length == 0, CHIP_ERROR_INVALID_ARGUMENT);

    const psa_algorithm_t algorithm = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, tag_length);
    psa_status_t status             = PSA_SUCCESS;
    psa_aead_operation_t operation  = PSA_AEAD_OPERATION_INIT;
    size_t out_length               = 0;
    size_t tag_out_length           = 0;

    status = psa_aead_encrypt_setup(&operation, key.As<psa_key_id_t>(), algorithm);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_aead_set_lengths(&operation, aad_length, plaintext_length);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_aead_set_nonce(&operation, nonce, nonce_length);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    if (aad_length != 0)
    {
        status = psa_aead_update_ad(&operation, aad, aad_length);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    }
    else
    {
        ChipLogDetail(Crypto, "AES_CCM_encrypt: Using aad == null path");
    }

    // psa_aead_update() requires use of the macro PSA_AEAD_UPDATE_OUTPUT_SIZE to determine the output buffer size.
    // For AES-CCM, PSA_AEAD_UPDATE_OUTPUT_SIZE will round up the size to the next multiple of the block size (16).
    // If the ciphertext length is not a multiple of the block size, we will encrypt in two steps, first with the
    // block_aligned_length, and then with a rounded up partial_block_length, where a temporary buffer will be used for the output.
    constexpr uint8_t kBlockSize = PSA_BLOCK_CIPHER_BLOCK_LENGTH(PSA_KEY_TYPE_AES);
    size_t block_aligned_length  = (plaintext_length / kBlockSize) * kBlockSize;
    size_t partial_block_length  = plaintext_length % kBlockSize;

    // Make sure the calculated block_aligned_length is compliant with PSA's output size requirements.
    VerifyOrReturnError(block_aligned_length == PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, block_aligned_length),
                        CHIP_ERROR_INTERNAL);

    if (block_aligned_length > 0)
    {
        status = psa_aead_update(&operation, plaintext, block_aligned_length, ciphertext, block_aligned_length, &out_length);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        ciphertext += out_length;
    }

    if (partial_block_length > 0)
    {
        uint8_t temp_buffer[kBlockSize];
        size_t rounded_up_length = PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, partial_block_length);

        VerifyOrReturnError(rounded_up_length <= sizeof(temp_buffer), CHIP_ERROR_BUFFER_TOO_SMALL);

        out_length = 0;
        status     = psa_aead_update(&operation, plaintext + block_aligned_length, partial_block_length, &temp_buffer[0],
                                     rounded_up_length, &out_length);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        VerifyOrReturnError(partial_block_length == out_length, CHIP_ERROR_INTERNAL);
        memcpy(ciphertext, temp_buffer, partial_block_length);

        ciphertext += out_length;
    }

    if (plaintext_length != 0)
    {
        out_length     = 0;
        tag_out_length = 0;

        status = psa_aead_finish(&operation, ciphertext, PSA_AEAD_FINISH_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm), &out_length, tag,
                                 tag_length, &tag_out_length);
    }

    else
    {
        out_length     = 0;
        tag_out_length = 0;

        status = psa_aead_finish(&operation, nullptr, 0, &out_length, tag, tag_length, &tag_out_length);
    }
    VerifyOrReturnError(status == PSA_SUCCESS && tag_length == tag_out_length, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * tag, size_t tag_length, const Aes128KeyHandle & key, const uint8_t * nonce,
                           size_t nonce_length, uint8_t * plaintext)
{
    VerifyOrReturnError(isBufferNonEmpty(nonce, nonce_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isValidTag(tag, tag_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError((ciphertext != nullptr && plaintext != nullptr) || ciphertext_length == 0, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(aad != nullptr || aad_length == 0, CHIP_ERROR_INVALID_ARGUMENT);

    const psa_algorithm_t algorithm = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, tag_length);
    psa_status_t status             = PSA_SUCCESS;
    psa_aead_operation_t operation  = PSA_AEAD_OPERATION_INIT;
    size_t outLength                = 0;

    status = psa_aead_decrypt_setup(&operation, key.As<psa_key_id_t>(), algorithm);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_aead_set_lengths(&operation, aad_length, ciphertext_length);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_aead_set_nonce(&operation, nonce, nonce_length);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    if (aad_length != 0)
    {
        status = psa_aead_update_ad(&operation, aad, aad_length);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    }
    else
    {
        ChipLogDetail(Crypto, "AES_CCM_decrypt: Using aad == null path");
    }

    // psa_aead_update() requires use of the macro PSA_AEAD_UPDATE_OUTPUT_SIZE to determine the output buffer size.
    // For AES-CCM, PSA_AEAD_UPDATE_OUTPUT_SIZE will round up the size to the next multiple of the block size (16).
    // If the ciphertext length is not a multiple of the block size, we will decrypt in two steps, first with the
    // block_aligned_length, and then with a rounded up partial_block_length, where a temporary buffer will be used for the output.
    constexpr uint8_t kBlockSize = PSA_BLOCK_CIPHER_BLOCK_LENGTH(PSA_KEY_TYPE_AES);
    size_t block_aligned_length  = (ciphertext_length / kBlockSize) * kBlockSize;
    size_t partial_block_length  = ciphertext_length % kBlockSize;

    // Make sure the calculated block_aligned_length is compliant with PSA's output size requirements.
    VerifyOrReturnError(block_aligned_length == PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, block_aligned_length),
                        CHIP_ERROR_INTERNAL);

    if (block_aligned_length > 0)
    {
        status = psa_aead_update(&operation, ciphertext, block_aligned_length, plaintext, block_aligned_length, &outLength);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        plaintext += outLength;
    }

    if (partial_block_length > 0)
    {
        uint8_t temp_buffer[kBlockSize];
        size_t rounded_up_length = PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, partial_block_length);

        VerifyOrReturnError(rounded_up_length <= sizeof(temp_buffer), CHIP_ERROR_BUFFER_TOO_SMALL);

        outLength = 0;
        status    = psa_aead_update(&operation, ciphertext + block_aligned_length, partial_block_length, &temp_buffer[0],
                                    rounded_up_length, &outLength);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        VerifyOrReturnError(partial_block_length == outLength, CHIP_ERROR_INTERNAL);
        memcpy(plaintext, &temp_buffer[0], partial_block_length);

        plaintext += outLength;
    }

    if (ciphertext_length != 0)
    {
        outLength = 0;
        status = psa_aead_verify(&operation, plaintext, PSA_AEAD_VERIFY_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm), &outLength, tag,
                                 tag_length);
    }
    else
    {
        outLength = 0;
        status    = psa_aead_verify(&operation, nullptr, 0, &outLength, tag, tag_length);
    }

    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    size_t outLength = 0;

    const psa_status_t status =
        psa_hash_compute(PSA_ALG_SHA_256, data, data_length, out_buffer, PSA_HASH_LENGTH(PSA_ALG_SHA_256), &outLength);

    return status == PSA_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR Hash_SHA1(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    size_t outLength = 0;

    const psa_status_t status =
        psa_hash_compute(PSA_ALG_SHA_1, data, data_length, out_buffer, PSA_HASH_LENGTH(PSA_ALG_SHA_1), &outLength);

    return status == PSA_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

static inline psa_hash_operation_t * toHashOperation(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<psa_hash_operation_t *>(context);
}

static inline psa_hash_operation_t & toHashOperation(HashSHA256OpaqueContext & context)
{
    return *SafePointerCast<psa_hash_operation_t *>(&context);
}

Hash_SHA256_stream::Hash_SHA256_stream()
{
    toHashOperation(mContext) = PSA_HASH_OPERATION_INIT;
}

Hash_SHA256_stream::~Hash_SHA256_stream()
{
    Clear();
}

CHIP_ERROR Hash_SHA256_stream::Begin()
{
    toHashOperation(mContext) = PSA_HASH_OPERATION_INIT;
    const psa_status_t status = psa_hash_setup(toHashOperation(&mContext), PSA_ALG_SHA_256);

    return status == PSA_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const ByteSpan data)
{
    const psa_status_t status = psa_hash_update(toHashOperation(&mContext), data.data(), data.size());

    return status == PSA_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

CHIP_ERROR Hash_SHA256_stream::GetDigest(MutableByteSpan & out_buffer)
{
    VerifyOrReturnError(out_buffer.size() >= PSA_HASH_LENGTH(PSA_ALG_SHA_256), CHIP_ERROR_BUFFER_TOO_SMALL);

    CHIP_ERROR error               = CHIP_NO_ERROR;
    psa_status_t status            = PSA_SUCCESS;
    psa_hash_operation_t operation = PSA_HASH_OPERATION_INIT;
    size_t outLength;

    status = psa_hash_clone(toHashOperation(&mContext), &operation);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_hash_finish(&operation, out_buffer.data(), out_buffer.size(), &outLength);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    out_buffer.reduce_size(outLength);

exit:
    psa_hash_abort(&operation);

    return error;
}

CHIP_ERROR Hash_SHA256_stream::Finish(MutableByteSpan & out_buffer)
{
    VerifyOrReturnError(out_buffer.size() >= PSA_HASH_LENGTH(PSA_ALG_SHA_256), CHIP_ERROR_BUFFER_TOO_SMALL);

    size_t outLength;

    const psa_status_t status = psa_hash_finish(toHashOperation(&mContext), out_buffer.data(), out_buffer.size(), &outLength);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    out_buffer.reduce_size(outLength);

    return CHIP_NO_ERROR;
}

void Hash_SHA256_stream::Clear()
{
    psa_hash_abort(toHashOperation(&mContext));
}

CHIP_ERROR FindFreeKeySlotInRange(psa_key_id_t & keyId, psa_key_id_t start, uint32_t range)
{
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t end                = start + range;

    VerifyOrReturnError(start >= PSA_KEY_ID_USER_MIN && end - 1 <= PSA_KEY_ID_USER_MAX, CHIP_ERROR_INVALID_ARGUMENT);

    for (keyId = start; keyId < end; keyId++)
    {
        psa_status_t status = psa_get_key_attributes(keyId, &attributes);
        if (status == PSA_ERROR_INVALID_HANDLE)
        {
            return CHIP_NO_ERROR;
        }
        else if (status != PSA_SUCCESS)
        {
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR PsaKdf::Init(const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info)
{
    psa_status_t status        = PSA_SUCCESS;
    psa_key_attributes_t attrs = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_type(&attrs, PSA_KEY_TYPE_DERIVE);
    psa_set_key_algorithm(&attrs, PSA_ALG_HKDF(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attrs, PSA_KEY_USAGE_DERIVE);

    status = psa_import_key(&attrs, secret.data(), secret.size(), &mSecretKeyId);
    LogPsaError(status);
    psa_reset_key_attributes(&attrs);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return InitOperation(mSecretKeyId, salt, info);
}

CHIP_ERROR PsaKdf::Init(const HkdfKeyHandle & hkdfKey, const ByteSpan & salt, const ByteSpan & info)
{
    return InitOperation(hkdfKey.As<psa_key_id_t>(), salt, info);
}

CHIP_ERROR PsaKdf::InitOperation(psa_key_id_t hkdfKey, const ByteSpan & salt, const ByteSpan & info)
{
    psa_status_t status = psa_key_derivation_setup(&mOperation, PSA_ALG_HKDF(PSA_ALG_SHA_256));
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    if (salt.size() > 0)
    {
        status = psa_key_derivation_input_bytes(&mOperation, PSA_KEY_DERIVATION_INPUT_SALT, salt.data(), salt.size());
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    }

    status = psa_key_derivation_input_key(&mOperation, PSA_KEY_DERIVATION_INPUT_SECRET, hkdfKey);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_input_bytes(&mOperation, PSA_KEY_DERIVATION_INPUT_INFO, info.data(), info.size());
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

void LogPsaError(psa_status_t status)
{
    if (status != PSA_SUCCESS)
    {
        ChipLogError(Crypto, "PSA error: %d", static_cast<int>(status));
    }
}

CHIP_ERROR PsaKdf::DeriveBytes(const MutableByteSpan & output)
{
    psa_status_t status = psa_key_derivation_output_bytes(&mOperation, output.data(), output.size());
    LogPsaError(status);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PsaKdf::DeriveKey(const psa_key_attributes_t & attributes, psa_key_id_t & keyId)
{
    psa_status_t status = psa_key_derivation_output_key(&attributes, &mOperation, &keyId);
    LogPsaError(status);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR HKDF_sha::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                 const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
    VerifyOrReturnError(isBufferNonEmpty(secret, secret_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isBufferNonEmpty(info, info_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isBufferNonEmpty(out_buffer, out_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(salt != nullptr || salt_length == 0, CHIP_ERROR_INVALID_ARGUMENT);

    PsaKdf kdf;

    ReturnErrorOnFailure(kdf.Init(ByteSpan(secret, secret_length), ByteSpan(salt, salt_length), ByteSpan(info, info_length)));

    return kdf.DeriveBytes(MutableByteSpan(out_buffer, out_length));
}

CHIP_ERROR HMAC_sha::HMAC_SHA256(const uint8_t * key, size_t key_length, const uint8_t * message, size_t message_length,
                                 uint8_t * out_buffer, size_t out_length)
{
    VerifyOrReturnError(isBufferNonEmpty(key, key_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isBufferNonEmpty(message, message_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr && out_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), CHIP_ERROR_INVALID_ARGUMENT);

    const psa_algorithm_t algorithm = PSA_ALG_HMAC(PSA_ALG_SHA_256);
    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attrs      = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t keyId              = 0;

    psa_set_key_type(&attrs, PSA_KEY_TYPE_HMAC);
    psa_set_key_algorithm(&attrs, algorithm);
    psa_set_key_usage_flags(&attrs, PSA_KEY_USAGE_SIGN_HASH);

    status = psa_import_key(&attrs, key, key_length, &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_mac_compute(keyId, algorithm, message, message_length, out_buffer, out_length, &out_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

exit:
    LogPsaError(status);
    psa_destroy_key(keyId);
    psa_reset_key_attributes(&attrs);

    return CHIP_NO_ERROR;
}

CHIP_ERROR HMAC_sha::HMAC_SHA256(const Hmac128KeyHandle & key, const uint8_t * message, size_t message_length, uint8_t * out_buffer,
                                 size_t out_length)
{
    VerifyOrReturnError(isBufferNonEmpty(message, message_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr && out_length == PSA_HASH_LENGTH(PSA_ALG_SHA_256), CHIP_ERROR_INVALID_ARGUMENT);

    const psa_algorithm_t algorithm = PSA_ALG_HMAC(PSA_ALG_SHA_256);
    psa_status_t status             = PSA_SUCCESS;

    status = psa_mac_compute(key.As<psa_key_id_t>(), algorithm, message, message_length, out_buffer, out_length, &out_length);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PBKDF2_sha256::pbkdf2_sha256(const uint8_t * pass, size_t pass_length, const uint8_t * salt, size_t salt_length,
                                        unsigned int iteration_count, uint32_t key_length, uint8_t * key)
{
    /*
    TODO: Switch to the following implementation once mbedTLS gets support for PBKDF2

    VerifyOrReturnError(isBufferNonEmpty(pass, pass_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(key != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(salt_length >= kSpake2p_Min_PBKDF_Salt_Length && salt_length <= kSpake2p_Max_PBKDF_Salt_Length,
                        CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error                         = CHIP_NO_ERROR;
    psa_status_t status                      = PSA_SUCCESS;
    psa_key_derivation_operation_t operation = PSA_KEY_DERIVATION_OPERATION_INIT;

    status = psa_key_derivation_setup(&operation, PSA_ALG_PBKDF2_HMAC(PSA_ALG_SHA_256));
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_SALT, salt, salt_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_input_integer(&operation, PSA_KEY_DERIVATION_INPUT_COST, iteration_count);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_input_bytes(&operation, PSA_KEY_DERIVATION_INPUT_PASSWORD, pass, pass_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_output_bytes(&operation, key, key_length);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

exit:
    psa_key_derivation_abort(&operation);

    return error;
    */

    VerifyOrReturnError(isBufferNonEmpty(pass, pass_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(salt != nullptr && salt_length >= kSpake2p_Min_PBKDF_Salt_Length &&
                            salt_length <= kSpake2p_Max_PBKDF_Salt_Length,
                        CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(isBufferNonEmpty(key, key_length), CHIP_ERROR_INVALID_ARGUMENT);

    constexpr size_t kMacLength     = PSA_MAC_LENGTH(PSA_KEY_TYPE_HMAC, pass_length * 8, PSA_ALG_HMAC(PSA_ALG_SHA_256));
    const psa_algorithm_t algorithm = PSA_ALG_HMAC(PSA_ALG_SHA_256);
    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attrs      = PSA_KEY_ATTRIBUTES_INIT;
    psa_key_id_t keyId              = 0;

    psa_set_key_type(&attrs, PSA_KEY_TYPE_HMAC);
    psa_set_key_algorithm(&attrs, algorithm);
    psa_set_key_usage_flags(&attrs, PSA_KEY_USAGE_SIGN_HASH);

    status = psa_import_key(&attrs, pass, pass_length, &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    for (uint32_t blockNo = 1; key_length != 0; ++blockNo)
    {
        uint8_t in[std::max(kMacLength, kSpake2p_Max_PBKDF_Salt_Length + 4)];
        size_t inLength = salt_length + 4;
        uint8_t out[kMacLength];
        size_t outLength;
        uint8_t result[kMacLength] = {};

        memcpy(in, salt, salt_length);
        Encoding::BigEndian::Put32(&in[salt_length], blockNo);

        for (size_t iteration = 0; iteration < iteration_count; ++iteration)
        {
            status = psa_mac_compute(keyId, algorithm, in, inLength, out, sizeof(out), &outLength);
            VerifyOrExit(status == PSA_SUCCESS && outLength == kMacLength, error = CHIP_ERROR_INTERNAL);

            for (size_t byteNo = 0; byteNo < kMacLength; ++byteNo)
            {
                result[byteNo] ^= out[byteNo];
                in[byteNo] = out[byteNo];
            }

            inLength = outLength;
        }

        const size_t usedKeyLength = std::min<size_t>(key_length, kMacLength);
        memcpy(key, result, usedKeyLength);
        key += usedKeyLength;
        key_length -= usedKeyLength;
    }

exit:
    LogPsaError(status);
    psa_destroy_key(keyId);
    psa_reset_key_attributes(&attrs);

    return CHIP_NO_ERROR;
}

CHIP_ERROR add_entropy_source(entropy_source /* fn_source */, void * /* p_source */, size_t /* threshold */)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    VerifyOrReturnError(isBufferNonEmpty(out_buffer, out_length), CHIP_ERROR_INVALID_ARGUMENT);

    const psa_status_t status = psa_generate_random(out_buffer, out_length);

    return status == PSA_SUCCESS ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

static int CryptoRNG(void * ctxt, uint8_t * out_buffer, size_t out_length)
{
    return (chip::Crypto::DRBG_get_bytes(out_buffer, out_length) == CHIP_NO_ERROR) ? 0 : 1;
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);
    VerifyOrReturnError(isBufferNonEmpty(msg, msg_length), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error                      = CHIP_NO_ERROR;
    psa_status_t status                   = PSA_SUCCESS;
    size_t outputLen                      = 0;
    const PsaP256KeypairContext & context = ToConstPsaContext(mKeypair);

    status = psa_sign_message(context.key_id, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg, msg_length, out_signature.Bytes(),
                              out_signature.Capacity(), &outputLen);

    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(outputLen == kP256_ECDSA_Signature_Length_Raw, error = CHIP_ERROR_INTERNAL);
    error = out_signature.SetLength(outputLen);

exit:
    LogPsaError(status);
    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
    VerifyOrReturnError(isBufferNonEmpty(msg, msg_length), CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_id_t keyId              = 0;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_VERIFY_MESSAGE);

    status = psa_import_key(&attributes, ConstBytes(), Length(), &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_verify_message(keyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), msg, msg_length, signature.ConstBytes(), signature.Length());
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    LogPsaError(status);
    psa_destroy_key(keyId);
    psa_reset_key_attributes(&attributes);

    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
    VerifyOrReturnError(hash != nullptr && hash_length == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(signature.Length() == kP256_ECDSA_Signature_Length_Raw, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_id_t keyId              = 0;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_PUBLIC_KEY(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_VERIFY_HASH);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));

    status = psa_import_key(&attributes, ConstBytes(), Length(), &keyId);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_verify_hash(keyId, PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash, hash_length, signature.ConstBytes(), signature.Length());
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INVALID_SIGNATURE);

exit:
    LogPsaError(status);
    psa_destroy_key(keyId);
    psa_reset_key_attributes(&attributes);

    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    CHIP_ERROR error                      = CHIP_NO_ERROR;
    psa_status_t status                   = PSA_SUCCESS;
    const PsaP256KeypairContext & context = ToConstPsaContext(mKeypair);
    const size_t outputSize               = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();
    size_t outputLength;

    status = psa_raw_key_agreement(PSA_ALG_ECDH, context.key_id, remote_public_key.ConstBytes(), remote_public_key.Length(),
                                   out_secret.Bytes(), outputSize, &outputLength);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(outputLength));

exit:
    LogPsaError(status);

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
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    PsaP256KeypairContext & context = ToPsaContext(mKeypair);
    size_t publicKeyLength          = 0;

    // Type based on ECC with the elliptic curve SECP256r1 -> PSA_ECC_FAMILY_SECP_R1
    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);

    if (key_target == ECPKeyTarget::ECDH)
    {
        psa_set_key_algorithm(&attributes, PSA_ALG_ECDH);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);
    }
    else if (key_target == ECPKeyTarget::ECDSA)
    {
        psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_SIGN_MESSAGE);
    }
    else
    {
        ExitNow(error = CHIP_ERROR_UNKNOWN_KEY_TYPE);
    }

    status = psa_generate_key(&attributes, &context.key_id);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    status = psa_export_public_key(context.key_id, mPublicKey.Bytes(), mPublicKey.Length(), &publicKeyLength);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(publicKeyLength == kP256_PublicKey_Length, error = CHIP_ERROR_INTERNAL);

    mInitialized = true;

exit:
    LogPsaError(status);
    psa_reset_key_attributes(&attributes);

    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    CHIP_ERROR error                      = CHIP_NO_ERROR;
    psa_status_t status                   = PSA_SUCCESS;
    const PsaP256KeypairContext & context = ToConstPsaContext(mKeypair);
    const size_t outputSize               = output.Length() == 0 ? output.Capacity() : output.Length();
    Encoding::BufferWriter bbuf(output.Bytes(), outputSize);
    uint8_t privateKey[kP256_PrivateKey_Length];
    size_t privateKeyLength = 0;

    status = psa_export_key(context.key_id, privateKey, sizeof(privateKey), &privateKeyLength);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(privateKeyLength == kP256_PrivateKey_Length, error = CHIP_ERROR_INTERNAL);

    bbuf.Put(mPublicKey, mPublicKey.Length());
    bbuf.Put(privateKey, privateKeyLength);
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    error = output.SetLength(bbuf.Needed());

exit:
    LogPsaError(status);

    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    VerifyOrReturnError(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, CHIP_ERROR_INVALID_ARGUMENT);

    CHIP_ERROR error                = CHIP_NO_ERROR;
    psa_status_t status             = PSA_SUCCESS;
    psa_key_attributes_t attributes = PSA_KEY_ATTRIBUTES_INIT;
    PsaP256KeypairContext & context = ToPsaContext(mKeypair);
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    Clear();

    psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));
    psa_set_key_bits(&attributes, kP256_PrivateKey_Length * 8);
    psa_set_key_algorithm(&attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_SIGN_MESSAGE);

    status = psa_import_key(&attributes, input.ConstBytes() + mPublicKey.Length(), kP256_PrivateKey_Length, &context.key_id);
    VerifyOrExit(status == PSA_SUCCESS, error = CHIP_ERROR_INTERNAL);

    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    mInitialized = true;

exit:
    LogPsaError(status);

    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        PsaP256KeypairContext & context = ToPsaContext(mKeypair);
        psa_destroy_key(context.key_id);
        memset(&context, 0, sizeof(context));
        mInitialized = false;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    VerifyOrReturnError(isBufferNonEmpty(out_csr, csr_length), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mInitialized, CHIP_ERROR_UNINITIALIZED);

    MutableByteSpan csr(out_csr, csr_length);
    ReturnErrorOnFailure(GenerateCertificateSigningRequest(this, csr));
    csr_length = csr.size();

    return CHIP_NO_ERROR;
}

// We should compile this SPAKE2P implementation only if the PSA implementation is not in use.
#if !CHIP_CRYPTO_PSA_SPAKE2P

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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1sin, size_t w1sin_len)
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

    result = mbedtls_mpi_read_binary(&w1_bn, Uint8::to_const_uchar(w1sin), w1sin_len);
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

#endif // !CHIP_CRYPTO_PSA_SPAKE2P

} // namespace Crypto
} // namespace chip
