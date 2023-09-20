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
#include <mbedtls/oid.h>
#include <mbedtls/x509.h>
#include <mbedtls/x509_csr.h>
#if defined(MBEDTLS_X509_CRT_PARSE_C)
#include <mbedtls/x509_crt.h>
#endif // defined(MBEDTLS_X509_CRT_PARSE_C)

#include <string.h>
#include <type_traits>

constexpr size_t kMaxErrorStrLen = 128;

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

namespace chip {
namespace Crypto {

namespace {

void logMbedTLSError(int errorCode)
{
    if (errorCode != 0)
    {
#if defined(MBEDTLS_ERROR_C)
        char errorStr[kMaxErrorStrLen];
        mbedtls_strerror(errorCode, errorStr, sizeof(errorStr));
        ChipLogError(Crypto, "mbedTLS error: %s", errorStr);
#else
        // Error codes defined in 16-bit negative hex numbers. Ease lookup by printing likewise
        ChipLogError(Crypto, "mbedTLS error: -0x%04X", static_cast<uint16_t>(-errorCode));
#endif
    }
}

void logPsaError(psa_status_t status)
{
    if (status != 0)
    {
        ChipLogError(Crypto, "PSA error: %d", static_cast<int>(status));
    }
}

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
    size_t out_length;
    size_t tag_out_length;

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

    if (plaintext_length != 0)
    {
        status = psa_aead_update(&operation, plaintext, plaintext_length, ciphertext,
                                 PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, plaintext_length), &out_length);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        ciphertext += out_length;

        status = psa_aead_finish(&operation, ciphertext, PSA_AEAD_FINISH_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm), &out_length, tag,
                                 tag_length, &tag_out_length);
    }
    else
    {
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
    size_t outLength;

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

    if (ciphertext_length != 0)
    {
        status = psa_aead_update(&operation, ciphertext, ciphertext_length, plaintext,
                                 PSA_AEAD_UPDATE_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm, ciphertext_length), &outLength);
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

        plaintext += outLength;

        status = psa_aead_verify(&operation, plaintext, PSA_AEAD_VERIFY_OUTPUT_SIZE(PSA_KEY_TYPE_AES, algorithm), &outLength, tag,
                                 tag_length);
    }
    else
    {
        status = psa_aead_verify(&operation, nullptr, 0, &outLength, tag, tag_length);
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

CHIP_ERROR PsaKdf::Init(psa_algorithm_t algorithm, const ByteSpan & secret, const ByteSpan & salt, const ByteSpan & info)
{
    psa_status_t status        = PSA_SUCCESS;
    psa_key_attributes_t attrs = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_type(&attrs, PSA_KEY_TYPE_DERIVE);
    psa_set_key_algorithm(&attrs, PSA_ALG_HKDF(PSA_ALG_SHA_256));
    psa_set_key_usage_flags(&attrs, PSA_KEY_USAGE_DERIVE);

    status = psa_import_key(&attrs, secret.data(), secret.size(), &mSecretKeyId);
    psa_reset_key_attributes(&attrs);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_setup(&mOperation, algorithm);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    if (salt.size() > 0)
    {
        status = psa_key_derivation_input_bytes(&mOperation, PSA_KEY_DERIVATION_INPUT_SALT, salt.data(), salt.size());
        VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
    }

    status = psa_key_derivation_input_key(&mOperation, PSA_KEY_DERIVATION_INPUT_SECRET, mSecretKeyId);
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    status = psa_key_derivation_input_bytes(&mOperation, PSA_KEY_DERIVATION_INPUT_INFO, info.data(), info.size());
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PsaKdf::DeriveBytes(const MutableByteSpan & output)
{
    psa_status_t status = psa_key_derivation_output_bytes(&mOperation, output.data(), output.size());
    VerifyOrReturnError(status == PSA_SUCCESS, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PsaKdf::DeriveKey(const psa_key_attributes_t & attributes, psa_key_id_t & keyId)
{
    psa_status_t status = psa_key_derivation_output_key(&attributes, &mOperation, &keyId);
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

    ReturnErrorOnFailure(kdf.Init(PSA_ALG_HKDF(PSA_ALG_SHA_256), ByteSpan(secret, secret_length), ByteSpan(salt, salt_length),
                                  ByteSpan(info, info_length)));

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
    psa_destroy_key(keyId);
    psa_reset_key_attributes(&attrs);

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
        uint8_t in[chip::max(kMacLength, kSpake2p_Max_PBKDF_Salt_Length + 4)];
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

        const size_t usedKeyLength = chip::min<size_t>(key_length, kMacLength);
        memcpy(key, result, usedKeyLength);
        key += usedKeyLength;
        key_length -= usedKeyLength;
    }

exit:
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

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_WELL_UNINITIALIZED);
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
    logPsaError(status);
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
    logPsaError(status);
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
    logPsaError(status);
    psa_destroy_key(keyId);
    psa_reset_key_attributes(&attributes);

    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    VerifyOrReturnError(mInitialized, CHIP_ERROR_WELL_UNINITIALIZED);

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
    logPsaError(status);

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
    logPsaError(status);
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
    logPsaError(status);

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
    logPsaError(status);

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
    VerifyOrReturnError(mInitialized, CHIP_ERROR_WELL_UNINITIALIZED);

    MutableByteSpan csr(out_csr, csr_length);
    ReturnErrorOnFailure(GenerateCertificateSigningRequest(this, csr));
    csr_length = csr.size();

    return CHIP_NO_ERROR;
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(mbedResult);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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
    logMbedTLSError(result);
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

    ReturnErrorCodeIf(certificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

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
    logMbedTLSError(result);
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

    ReturnErrorCodeIf(candidateCertificates == nullptr || candidateCertificatesCount == 0, CHIP_NO_ERROR);

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
