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
 *      openSSL based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"

#include <type_traits>

#if CHIP_CRYPTO_BORINGSSL
#include <openssl/aead.h>
#endif // CHIP_CRYPTO_BORINGSSL

#include <openssl/bn.h>
#include <openssl/conf.h>
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include <openssl/ossl_typ.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <lib/asn1/ASN1.h>
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

// BoringSSL is designed to implement the same interface as OpenSSL in most
// cases. However, it removes some APIs that can allow very weak configuration.
// (Example: CCM ciphers with low tag lengths.) In order to support Matter,
// a more specific inteface is required.
#if CHIP_CRYPTO_BORINGSSL
#define RAND_priv_bytes RAND_bytes
#define BN_CTX_secure_new BN_CTX_new
#define EC_GROUP_clear_free EC_GROUP_free
using boringssl_size_t_openssl_int = size_t;
using boringssl_uint_openssl_int   = unsigned int;
using libssl_err_type              = uint32_t;
#else
using boringssl_size_t_openssl_int = int;
using boringssl_uint_openssl_int   = int;
using libssl_err_type              = unsigned long;
#endif // CHIP_CRYPTO_BORINGSSL

#define kKeyLengthInBits 256

typedef struct stack_st_X509 X509_LIST;

enum class DigestType
{
    SHA256
};

enum class ECName
{
    None   = 0,
    P256v1 = 1,
};

static int _nidForCurve(ECName name)
{
    switch (name)
    {
    case ECName::P256v1:
        return EC_curve_nist2nid("P-256");
        break;

    default:
        return NID_undef;
        break;
    }
}

static void _logSSLError()
{
    unsigned long ssl_err_code = ERR_get_error();
    while (ssl_err_code != 0)
    {
#if CHIP_ERROR_LOGGING
        const char * err_str_lib     = ERR_lib_error_string(static_cast<libssl_err_type>(ssl_err_code));
        const char * err_str_routine = ERR_func_error_string(static_cast<libssl_err_type>(ssl_err_code));
        const char * err_str_reason  = ERR_reason_error_string(static_cast<libssl_err_type>(ssl_err_code));
        if (err_str_lib)
        {
            ChipLogError(Crypto, " ssl err  %s %s %s\n", StringOrNullMarker(err_str_lib), StringOrNullMarker(err_str_routine),
                         StringOrNullMarker(err_str_reason));
        }
#endif // CHIP_ERROR_LOGGING
        ssl_err_code = ERR_get_error();
    }
}

static const EVP_MD * _digestForType(DigestType digestType)
{
    switch (digestType)
    {
    case DigestType::SHA256:
        return EVP_sha256();
        break;

    default:
        return nullptr;
        break;
    }
}

static int _compareDaysAndSeconds(const int days, const int seconds)
{
    if (days > 0 || seconds > 0)
        return 1;
    if (days < 0 || seconds < 0)
        return -1;
    return 0;
}

CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const Aes128KeyHandle & key, const uint8_t * nonce, size_t nonce_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
#if CHIP_CRYPTO_BORINGSSL
    EVP_AEAD_CTX * context = nullptr;
    size_t written_tag_len = 0;
    const EVP_AEAD * aead  = nullptr;
#else
    EVP_CIPHER_CTX * context = nullptr;
    int bytesWritten         = 0;
    size_t ciphertext_length = 0;
    const EVP_CIPHER * type  = nullptr;
#endif
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    // Placeholder location for avoiding null params for plaintexts when
    // size is zero.
    uint8_t placeholder_empty_plaintext = 0;

    // Ciphertext block to hold a finalized ciphertext block if output
    // `ciphertext` buffer is nullptr or plaintext_length is zero (i.e.
    // we are only doing auth and don't care about output).
    uint8_t placeholder_ciphertext[kAES_CCM128_Block_Length];
    bool ciphertext_was_null = (ciphertext == nullptr);

    if (plaintext_length == 0)
    {
        if (plaintext == nullptr)
        {
            plaintext = &placeholder_empty_plaintext;
        }
        // Make sure we have at least 1 full block size buffer for the
        // extraction of final block (required by OpenSSL EVP_EncryptFinal_ex)
        if (ciphertext_was_null)
        {
            ciphertext = &placeholder_ciphertext[0];
        }
    }

    VerifyOrExit((plaintext_length != 0) || ciphertext_was_null, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<int>(nonce_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
#if CHIP_CRYPTO_BORINGSSL
    VerifyOrExit(tag_length == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, error = CHIP_ERROR_INVALID_ARGUMENT);
#else
    VerifyOrExit(tag_length == 8 || tag_length == 12 || tag_length == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES,
                              error = CHIP_ERROR_INVALID_ARGUMENT);
#endif // CHIP_CRYPTO_BORINGSSL

#if CHIP_CRYPTO_BORINGSSL
    aead = EVP_aead_aes_128_ccm_matter();

    context = EVP_AEAD_CTX_new(aead, key.As<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray), tag_length);
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_NO_MEMORY);

    result = EVP_AEAD_CTX_seal_scatter(context, ciphertext, tag, &written_tag_len, tag_length, nonce, nonce_length, plaintext,
                                       plaintext_length, nullptr, 0, aad, aad_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(written_tag_len == tag_length, error = CHIP_ERROR_INTERNAL);
#else

    type = EVP_aes_128_ccm();

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_NO_MEMORY);

    // Pass in cipher
    result = EVP_EncryptInit_ex(context, type, nullptr, nullptr, nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in nonce length.  Cast is safe because we checked with CanCastTo.
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, static_cast<int>(nonce_length), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in tag length. Cast is safe because we checked against CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES.
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, static_cast<int>(tag_length), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + nonce
    static_assert(kAES_CCM128_Key_Length == sizeof(Symmetric128BitsKeyByteArray), "Unexpected key length");
    result = EVP_EncryptInit_ex(context, nullptr, nullptr, key.As<Symmetric128BitsKeyByteArray>(), Uint8::to_const_uchar(nonce));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in plain text length
    VerifyOrExit(CanCastTo<int>(plaintext_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_EncryptUpdate(context, nullptr, &bytesWritten, nullptr, static_cast<int>(plaintext_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in AAD
    if (aad_length > 0 && aad != nullptr)
    {
        VerifyOrExit(CanCastTo<int>(aad_length), error = CHIP_ERROR_INVALID_ARGUMENT);
        result = EVP_EncryptUpdate(context, nullptr, &bytesWritten, Uint8::to_const_uchar(aad), static_cast<int>(aad_length));
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Encrypt
    VerifyOrExit(CanCastTo<int>(plaintext_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_EncryptUpdate(context, Uint8::to_uchar(ciphertext), &bytesWritten, Uint8::to_const_uchar(plaintext),
                                            static_cast<int>(plaintext_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((ciphertext_was_null && bytesWritten == 0) || (bytesWritten >= 0), error = CHIP_ERROR_INTERNAL);
    ciphertext_length = static_cast<unsigned int>(bytesWritten);

    // Finalize encryption
    result = EVP_EncryptFinal_ex(context, ciphertext + ciphertext_length, &bytesWritten);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(bytesWritten >= 0 && bytesWritten <= static_cast<int>(plaintext_length), error = CHIP_ERROR_INTERNAL);

    // Get tag
    VerifyOrExit(CanCastTo<int>(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_GET_TAG, static_cast<int>(tag_length), Uint8::to_uchar(tag));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
#endif // CHIP_CRYPTO_BORINGSSL

exit:
    if (context != nullptr)
    {
#if CHIP_CRYPTO_BORINGSSL
        EVP_AEAD_CTX_free(context);
#else
        EVP_CIPHER_CTX_free(context);
#endif // CHIP_CRYPTO_BORINGSSL
        context = nullptr;
    }

    return error;
}

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * tag, size_t tag_length, const Aes128KeyHandle & key, const uint8_t * nonce,
                           size_t nonce_length, uint8_t * plaintext)
{
#if CHIP_CRYPTO_BORINGSSL
    EVP_AEAD_CTX * context = nullptr;
    const EVP_AEAD * aead  = nullptr;
#else

    EVP_CIPHER_CTX * context = nullptr;
    int bytesOutput          = 0;
    const EVP_CIPHER * type  = nullptr;
#endif // CHIP_CRYPTO_BORINGSSL
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    // Placeholder location for avoiding null params for ciphertext when
    // size is zero.
    uint8_t placeholder_empty_ciphertext = 0;

    // Plaintext block to hold a finalized plaintext block if output
    // `plaintext` buffer is nullptr or ciphertext_length is zero (i.e.
    // we are only doing auth and don't care about output).
    uint8_t placeholder_plaintext[kAES_CCM128_Block_Length];
    bool plaintext_was_null = (plaintext == nullptr);

    if (ciphertext_length == 0)
    {
        if (ciphertext == nullptr)
        {
            ciphertext = &placeholder_empty_ciphertext;
        }
        // Make sure we have at least 1 full block size buffer for the
        // extraction of final block (required by OpenSSL EVP_DecryptFinal_ex)
        if (plaintext_was_null)
        {
            plaintext = &placeholder_plaintext[0];
        }
    }

    VerifyOrExit(ciphertext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
#if CHIP_CRYPTO_BORINGSSL
    VerifyOrExit(tag_length == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES, error = CHIP_ERROR_INVALID_ARGUMENT);
#else
    VerifyOrExit(tag_length == 8 || tag_length == 12 || tag_length == CHIP_CRYPTO_AEAD_MIC_LENGTH_BYTES,
                              error = CHIP_ERROR_INVALID_ARGUMENT);
#endif // CHIP_CRYPTO_BORINGSSL
    VerifyOrExit(nonce != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(nonce_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

#if CHIP_CRYPTO_BORINGSSL
    aead = EVP_aead_aes_128_ccm_matter();

    context = EVP_AEAD_CTX_new(aead, key.As<Symmetric128BitsKeyByteArray>(), sizeof(Symmetric128BitsKeyByteArray), tag_length);
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_NO_MEMORY);

    result = EVP_AEAD_CTX_open_gather(context, plaintext, nonce, nonce_length, ciphertext, ciphertext_length, tag, tag_length, aad,
                                      aad_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
#else
    type = EVP_aes_128_ccm();

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_NO_MEMORY);

    // Pass in cipher
    result = EVP_DecryptInit_ex(context, type, nullptr, nullptr, nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in nonce length
    VerifyOrExit(CanCastTo<int>(nonce_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, static_cast<int>(nonce_length), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in expected tag
    // Removing "const" from |tag| here should hopefully be safe as
    // we're writing the tag, not reading.
    VerifyOrExit(CanCastTo<int>(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, static_cast<int>(tag_length),
                                              const_cast<void *>(static_cast<const void *>(tag)));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + nonce
    static_assert(kAES_CCM128_Key_Length == sizeof(Symmetric128BitsKeyByteArray), "Unexpected key length");
    result = EVP_DecryptInit_ex(context, nullptr, nullptr, key.As<Symmetric128BitsKeyByteArray>(), Uint8::to_const_uchar(nonce));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher text length
    VerifyOrExit(CanCastTo<int>(ciphertext_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_DecryptUpdate(context, nullptr, &bytesOutput, nullptr, static_cast<int>(ciphertext_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(bytesOutput <= static_cast<int>(ciphertext_length), error = CHIP_ERROR_INTERNAL);

    // Pass in aad
    if (aad_length > 0 && aad != nullptr)
    {
        VerifyOrExit(CanCastTo<int>(aad_length), error = CHIP_ERROR_INVALID_ARGUMENT);
        result = EVP_DecryptUpdate(context, nullptr, &bytesOutput, Uint8::to_const_uchar(aad), static_cast<int>(aad_length));
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
        VerifyOrExit(bytesOutput <= static_cast<int>(aad_length), error = CHIP_ERROR_INTERNAL);
    }

    // Pass in ciphertext. We wont get anything if validation fails.
    VerifyOrExit(CanCastTo<int>(ciphertext_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_DecryptUpdate(context, Uint8::to_uchar(plaintext), &bytesOutput, Uint8::to_const_uchar(ciphertext),
                                            static_cast<int>(ciphertext_length));
    if (plaintext_was_null)
    {
        VerifyOrExit(bytesOutput <= static_cast<int>(sizeof(placeholder_plaintext)), error = CHIP_ERROR_INTERNAL);
    }
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
#endif // CHIP_CRYPTO_BORINGSSL

exit:
    if (context != nullptr)
    {
#if CHIP_CRYPTO_BORINGSSL
        EVP_AEAD_CTX_free(context);
#else
        EVP_CIPHER_CTX_free(context);
#endif // CHIP_CRYPTO_BORINGSSL

        context = nullptr;
    }

    return error;
}

CHIP_ERROR Hash_SHA256(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    // zero data length hash is supported.
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    SHA256(data, data_length, Uint8::to_uchar(out_buffer));

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA1(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    // zero data length hash is supported.
    VerifyOrReturnError(data != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    SHA1(data, data_length, Uint8::to_uchar(out_buffer));

    return CHIP_NO_ERROR;
}

Hash_SHA256_stream::Hash_SHA256_stream() {}

Hash_SHA256_stream::~Hash_SHA256_stream()
{
    Clear();
}

static_assert(kMAX_Hash_SHA256_Context_Size >= sizeof(SHA256_CTX),
              "kMAX_Hash_SHA256_Context_Size is too small for the size of underlying SHA256_CTX");

static inline SHA256_CTX * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<SHA256_CTX *>(context);
}

CHIP_ERROR Hash_SHA256_stream::Begin()
{
    SHA256_CTX * const context = to_inner_hash_sha256_context(&mContext);

    const int result = SHA256_Init(context);
    VerifyOrReturnError(result == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const ByteSpan data)
{
    SHA256_CTX * const context = to_inner_hash_sha256_context(&mContext);

    const int result = SHA256_Update(context, Uint8::to_const_uchar(data.data()), data.size());
    VerifyOrReturnError(result == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Hash_SHA256_stream::GetDigest(MutableByteSpan & out_buffer)
{
    SHA256_CTX * context = to_inner_hash_sha256_context(&mContext);

    // Back-up context as we are about to finalize the hash to extract digest.
    SHA256_CTX previous_ctx = *context;

    // Pad + compute digest, then finalize context. It is restored next line to continue.
    CHIP_ERROR result = Finish(out_buffer);

    // Restore context prior to finalization.
    *context = previous_ctx;

    return result;
}

CHIP_ERROR Hash_SHA256_stream::Finish(MutableByteSpan & out_buffer)
{
    VerifyOrReturnError(out_buffer.size() >= kSHA256_Hash_Length, CHIP_ERROR_BUFFER_TOO_SMALL);

    SHA256_CTX * const context = to_inner_hash_sha256_context(&mContext);
    const int result           = SHA256_Final(Uint8::to_uchar(out_buffer.data()), context);
    VerifyOrReturnError(result == 1, CHIP_ERROR_INTERNAL);
    out_buffer = out_buffer.SubSpan(0, kSHA256_Hash_Length);

    return CHIP_NO_ERROR;
}

void Hash_SHA256_stream::Clear()
{
    OPENSSL_cleanse(this, sizeof(*this));
}

CHIP_ERROR HKDF_sha::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                 const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    EVP_PKEY_CTX * const context = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_INTERNAL);

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

    result = EVP_PKEY_derive_init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_set_hkdf_md(context, EVP_sha256());
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(secret_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_PKEY_CTX_set1_hkdf_key(context, Uint8::to_const_uchar(secret),
                                        static_cast<boringssl_size_t_openssl_int>(secret_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    if (salt_length > 0 && salt != nullptr)
    {
        VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(salt_length), error = CHIP_ERROR_INVALID_ARGUMENT);
        result = EVP_PKEY_CTX_set1_hkdf_salt(context, Uint8::to_const_uchar(salt),
                                             static_cast<boringssl_size_t_openssl_int>(salt_length));
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(info_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result =
        EVP_PKEY_CTX_add1_hkdf_info(context, Uint8::to_const_uchar(info), static_cast<boringssl_size_t_openssl_int>(info_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_hkdf_mode(context, EVP_PKEY_HKDEF_MODE_EXTRACT_AND_EXPAND);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Get the OKM (Output Key Material)
    result = EVP_PKEY_derive(context, Uint8::to_uchar(out_buffer), &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != nullptr)
    {
        EVP_PKEY_CTX_free(context);
    }
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

    CHIP_ERROR error         = CHIP_ERROR_INTERNAL;
    int error_openssl        = 0;
    unsigned int mac_out_len = 0;

    HMAC_CTX * mac_ctx = HMAC_CTX_new();
    VerifyOrExit(mac_ctx != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    error_openssl = HMAC_Init_ex(mac_ctx, Uint8::to_const_uchar(key), static_cast<boringssl_size_t_openssl_int>(key_length),
                                 EVP_sha256(), nullptr);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error_openssl = HMAC_Update(mac_ctx, Uint8::to_const_uchar(message), message_length);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    mac_out_len   = static_cast<unsigned int>(CHIP_CRYPTO_HASH_LEN_BYTES);
    error_openssl = HMAC_Final(mac_ctx, Uint8::to_uchar(out_buffer), &mac_out_len);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    HMAC_CTX_free(mac_ctx);
    return error;
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
    CHIP_ERROR error  = CHIP_NO_ERROR;
    int result        = 1;
    const EVP_MD * md = nullptr;

    VerifyOrExit(password != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen >= kSpake2p_Min_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen <= kSpake2p_Max_PBKDF_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = _digestForType(DigestType::SHA256);
    VerifyOrExit(md != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(plen), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(slen), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<boringssl_uint_openssl_int>(iteration_count), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = PKCS5_PBKDF2_HMAC(Uint8::to_const_char(password), static_cast<boringssl_size_t_openssl_int>(plen),
                               Uint8::to_const_uchar(salt), static_cast<boringssl_size_t_openssl_int>(slen),
                               static_cast<boringssl_uint_openssl_int>(iteration_count), md,
                               static_cast<boringssl_size_t_openssl_int>(key_length), Uint8::to_uchar(output));

    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (error != CHIP_NO_ERROR)
    {
        _logSSLError();
    }

    return error;
}

CHIP_ERROR add_entropy_source(entropy_source fn_source, void * p_source, size_t threshold)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DRBG_get_bytes(uint8_t * out_buffer, const size_t out_length)
{
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(out_length > 0, CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrReturnError(CanCastTo<boringssl_size_t_openssl_int>(out_length), CHIP_ERROR_INVALID_ARGUMENT);
    const int result = RAND_priv_bytes(Uint8::to_uchar(out_buffer), static_cast<boringssl_size_t_openssl_int>(out_length));
    VerifyOrReturnError(result == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

ECName MapECName(SupportedECPKeyTypes keyType)
{
    switch (keyType)
    {
    case SupportedECPKeyTypes::ECP256R1:
        return ECName::P256v1;
    default:
        return ECName::None;
    }
}

static inline void from_EC_KEY(EC_KEY * key, P256KeypairContext * context)
{
    *SafePointerCast<EC_KEY **>(context) = key;
}

static inline EC_KEY * to_EC_KEY(P256KeypairContext * context)
{
    return *SafePointerCast<EC_KEY **>(context);
}

static inline const EC_KEY * to_const_EC_KEY(const P256KeypairContext * context)
{
    return *SafePointerCast<const EC_KEY * const *>(context);
}

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int nid          = NID_undef;
    EC_KEY * ec_key  = nullptr;
    ECDSA_SIG * sig  = nullptr;
    const BIGNUM * r = nullptr;
    const BIGNUM * s = nullptr;

    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));

    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));

    ERR_clear_error();

    static_assert(P256ECDSASignature::Capacity() >= kP256_ECDSA_Signature_Length_Raw, "P256ECDSASignature must be large enough");
    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);
    nid = _nidForCurve(MapECName(mPublicKey.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = to_EC_KEY(&mKeypair);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    sig = ECDSA_do_sign(Uint8::to_const_uchar(&digest[0]), static_cast<boringssl_size_t_openssl_int>(sizeof(digest)), ec_key);
    VerifyOrExit(sig != nullptr, error = CHIP_ERROR_INTERNAL);
    ECDSA_SIG_get0(sig, &r, &s);
    VerifyOrExit((r != nullptr) && (s != nullptr), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(BN_num_bytes(r)) && CanCastTo<size_t>(BN_num_bytes(s)), error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((static_cast<size_t>(BN_num_bytes(r)) <= kP256_FE_Length) &&
                     (static_cast<size_t>(BN_num_bytes(s)) <= kP256_FE_Length),
                 error = CHIP_ERROR_INTERNAL);

    // Concatenate r and s to output. Sizes were checked above.
    VerifyOrExit(out_signature.SetLength(kP256_ECDSA_Signature_Length_Raw) == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(BN_bn2binpad(r, out_signature.Bytes() + 0u, kP256_FE_Length) == kP256_FE_Length, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(BN_bn2binpad(s, out_signature.Bytes() + kP256_FE_Length, kP256_FE_Length) == kP256_FE_Length,
                 error = CHIP_ERROR_INTERNAL);

exit:
    if (sig != nullptr)
    {
        // SIG owns the memory of r, s
        ECDSA_SIG_free(sig);
    }

    if (error != CHIP_NO_ERROR)
    {
        _logSSLError();
    }

    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
    VerifyOrReturnError((msg != nullptr) && (msg_length > 0), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t digest[kSHA256_Hash_Length];
    memset(&digest[0], 0, sizeof(digest));

    ReturnErrorOnFailure(Hash_SHA256(msg, msg_length, &digest[0]));
    return ECDSA_validate_hash_signature(&digest[0], sizeof(digest), signature);
}

CHIP_ERROR P256PublicKey::ECDSA_validate_hash_signature(const uint8_t * hash, const size_t hash_length,
                                                        const P256ECDSASignature & signature) const
{
    ERR_clear_error();
    CHIP_ERROR error     = CHIP_ERROR_INTERNAL;
    int nid              = NID_undef;
    EC_KEY * ec_key      = nullptr;
    EC_POINT * key_point = nullptr;
    EC_GROUP * ec_group  = nullptr;
    ECDSA_SIG * ec_sig   = nullptr;
    BIGNUM * r           = nullptr;
    BIGNUM * s           = nullptr;
    int result           = 0;

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(signature.Length() == kP256_ECDSA_Signature_Length_Raw, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = _nidForCurve(MapECName(Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(ec_group != nullptr, error = CHIP_ERROR_NO_MEMORY);

    key_point = EC_POINT_new(ec_group);
    VerifyOrExit(key_point != nullptr, error = CHIP_ERROR_NO_MEMORY);

    result = EC_POINT_oct2point(ec_group, key_point, Uint8::to_const_uchar(*this), Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_NO_MEMORY);

    result = EC_KEY_set_public_key(ec_key, key_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_check_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Build-up the signature object from raw <r,s> tuple
    r = BN_bin2bn(Uint8::to_const_uchar(signature.ConstBytes()) + 0u, kP256_FE_Length, nullptr);
    VerifyOrExit(r != nullptr, error = CHIP_ERROR_NO_MEMORY);

    s = BN_bin2bn(Uint8::to_const_uchar(signature.ConstBytes()) + kP256_FE_Length, kP256_FE_Length, nullptr);
    VerifyOrExit(s != nullptr, error = CHIP_ERROR_NO_MEMORY);

    ec_sig = ECDSA_SIG_new();
    VerifyOrExit(ec_sig != nullptr, error = CHIP_ERROR_NO_MEMORY);

    result = ECDSA_SIG_set0(ec_sig, r, s);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = ECDSA_do_verify(Uint8::to_const_uchar(hash), static_cast<boringssl_size_t_openssl_int>(hash_length), ec_sig, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INVALID_SIGNATURE);
    error = CHIP_NO_ERROR;

exit:
    _logSSLError();
    if (ec_sig != nullptr)
    {
        ECDSA_SIG_free(ec_sig);

        // After ECDSA_SIG_set0 succeeds, r and s memory is managed by ECDSA_SIG object.
        // We set to nullptr so that we don't try to double-free
        r = nullptr;
        s = nullptr;
    }
    if (s != nullptr)
    {
        BN_clear_free(s);
    }
    if (r != nullptr)
    {
        BN_clear_free(r);
    }
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
    }
    if (key_point != nullptr)
    {
        EC_POINT_clear_free(key_point);
    }
    if (ec_group != nullptr)
    {
        EC_GROUP_free(ec_group);
    }
    return error;
}

// helper function to populate octet key into EVP_PKEY out_evp_pkey. Caller must free out_evp_pkey
static CHIP_ERROR _create_evp_key_from_binary_p256_key(const P256PublicKey & key, EVP_PKEY ** out_evp_pkey)
{

    CHIP_ERROR error = CHIP_NO_ERROR;
    EC_KEY * ec_key  = nullptr;
    int result       = -1;
    EC_POINT * point = nullptr;
    EC_GROUP * group = nullptr;
    int nid          = NID_undef;

    VerifyOrExit(*out_evp_pkey == nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = _nidForCurve(MapECName(key.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, error = CHIP_ERROR_INTERNAL);

    point = EC_POINT_new(group);
    VerifyOrExit(point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(group, point, Uint8::to_const_uchar(key), key.Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, point);

    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    *out_evp_pkey = EVP_PKEY_new();
    VerifyOrExit(*out_evp_pkey != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(*out_evp_pkey, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (error != CHIP_NO_ERROR && *out_evp_pkey)
    {
        EVP_PKEY_free(*out_evp_pkey);
        out_evp_pkey = nullptr;
    }

    if (point != nullptr)
    {
        EC_POINT_free(point);
        point = nullptr;
    }

    if (group != nullptr)
    {
        EC_GROUP_free(group);
        group = nullptr;
    }

    return error;
}

CHIP_ERROR P256Keypair::ECDH_derive_secret(const P256PublicKey & remote_public_key, P256ECDHDerivedSecret & out_secret) const
{
    ERR_clear_error();
    CHIP_ERROR error      = CHIP_NO_ERROR;
    int result            = -1;
    EVP_PKEY * local_key  = nullptr;
    EVP_PKEY * remote_key = nullptr;

    EVP_PKEY_CTX * context = nullptr;
    size_t out_buf_length  = 0;

    EC_KEY * ec_key = EC_KEY_dup(to_const_EC_KEY(&mKeypair));
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    local_key = EVP_PKEY_new();
    VerifyOrExit(local_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(local_key, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    error = _create_evp_key_from_binary_p256_key(remote_public_key, &remote_key);
    SuccessOrExit(error);

    context = EVP_PKEY_CTX_new(local_key, nullptr);
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_derive_init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_derive_set_peer(context, remote_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    out_buf_length = (out_secret.Length() == 0) ? out_secret.Capacity() : out_secret.Length();
    result         = EVP_PKEY_derive(context, out_secret.Bytes(), &out_buf_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(error = out_secret.SetLength(out_buf_length));

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (local_key != nullptr)
    {
        EVP_PKEY_free(local_key);
        local_key = nullptr;
    }

    if (remote_key != nullptr)
    {
        EVP_PKEY_free(remote_key);
        remote_key = nullptr;
    }

    if (context != nullptr)
    {
        EVP_PKEY_CTX_free(context);
        context = nullptr;
    }

    _logSSLError();
    return error;
}

void ClearSecretData(uint8_t * buf, size_t len)
{
    OPENSSL_cleanse(buf, len);
}

bool IsBufferContentEqualConstantTime(const void * a, const void * b, size_t n)
{
    return CRYPTO_memcmp(a, b, n) == 0;
}

static CHIP_ERROR P256PublicKeyFromECKey(EC_KEY * ec_key, P256PublicKey & pubkey)
{
    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;

    int nid            = NID_undef;
    ECName curve       = MapECName(pubkey.Type());
    EC_GROUP * group   = nullptr;
    size_t pubkey_size = 0;

    const EC_POINT * pubkey_ecp = EC_KEY_get0_public_key(ec_key);
    VerifyOrExit(pubkey_ecp != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = _nidForCurve(curve);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, error = CHIP_ERROR_INTERNAL);

    pubkey_size =
        EC_POINT_point2oct(group, pubkey_ecp, POINT_CONVERSION_UNCOMPRESSED, Uint8::to_uchar(pubkey), pubkey.Length(), nullptr);
    pubkey_ecp = nullptr;

    VerifyOrExit(pubkey_size == pubkey.Length(), error = CHIP_ERROR_INVALID_ARGUMENT);

exit:
    if (group != nullptr)
    {
        EC_GROUP_free(group);
        group = nullptr;
    }

    _logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Initialize(ECPKeyTarget key_target)
{
    ERR_clear_error();

    Clear();

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    EC_KEY * ec_key  = nullptr;
    ECName curve     = MapECName(mPublicKey.Type());

    int nid = _nidForCurve(curve);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_generate_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    error = P256PublicKeyFromECKey(ec_key, mPublicKey);
    SuccessOrExit(error);

    from_EC_KEY(ec_key, &mKeypair);
    mInitialized = true;
    ec_key       = nullptr;

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    _logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Serialize(P256SerializedKeypair & output) const
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    const EC_KEY * ec_key = to_const_EC_KEY(&mKeypair);
    uint8_t privkey[kP256_PrivateKey_Length];

    int privkey_size          = 0;
    const BIGNUM * privkey_bn = EC_KEY_get0_private_key(ec_key);
    VerifyOrExit(privkey_bn != nullptr, error = CHIP_ERROR_INTERNAL);

    privkey_size = BN_bn2binpad(privkey_bn, privkey, sizeof(privkey));
    privkey_bn   = nullptr;

    VerifyOrExit(privkey_size > 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit((size_t) privkey_size == sizeof(privkey), error = CHIP_ERROR_INTERNAL);

    {
        size_t len = output.Length() == 0 ? output.Capacity() : output.Length();
        Encoding::BufferWriter bbuf(output.Bytes(), len);
        bbuf.Put(mPublicKey, mPublicKey.Length());
        bbuf.Put(privkey, sizeof(privkey));
        VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);
        output.SetLength(bbuf.Needed());
    }

exit:
    ClearSecretData(privkey, sizeof(privkey));
    _logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    Clear();

    BIGNUM * pvt_key     = nullptr;
    EC_GROUP * group     = nullptr;
    EC_POINT * key_point = nullptr;

    EC_KEY * ec_key = nullptr;
    ECName curve    = MapECName(mPublicKey.Type());

    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    int nid          = NID_undef;

    const uint8_t * privkey = input.ConstBytes() + mPublicKey.Length();

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input.ConstBytes(), mPublicKey.Length());
    VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);

    nid = _nidForCurve(curve);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(group != nullptr, error = CHIP_ERROR_INTERNAL);

    key_point = EC_POINT_new(group);
    VerifyOrExit(key_point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(group, key_point, Uint8::to_const_uchar(mPublicKey), mPublicKey.Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, key_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    pvt_key = BN_bin2bn(privkey, kP256_PrivateKey_Length, nullptr);
    VerifyOrExit(pvt_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_private_key(ec_key, pvt_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    from_EC_KEY(ec_key, &mKeypair);
    mInitialized = true;
    ec_key       = nullptr;

exit:
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }

    if (group != nullptr)
    {
        EC_GROUP_free(group);
        group = nullptr;
    }

    if (pvt_key != nullptr)
    {
        BN_free(pvt_key);
        pvt_key = nullptr;
    }

    if (key_point != nullptr)
    {
        EC_POINT_free(key_point);
        key_point = nullptr;
    }
    _logSSLError();
    return error;
}

void P256Keypair::Clear()
{
    if (mInitialized)
    {
        EC_KEY * ec_key = to_EC_KEY(&mKeypair);
        EC_KEY_free(ec_key);
        mInitialized = false;
    }
}

P256Keypair::~P256Keypair()
{
    Clear();
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length) const
{
    ERR_clear_error();
    CHIP_ERROR error     = CHIP_NO_ERROR;
    int result           = 0;
    int csr_length_local = 0;

    X509_REQ * x509_req = X509_REQ_new();
    EVP_PKEY * evp_pkey = nullptr;

    EC_KEY * ec_key = to_EC_KEY(&mKeypair);

    X509_NAME * subject = X509_NAME_new();
    VerifyOrExit(subject != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_UNINITIALIZED);

    result = X509_REQ_set_version(x509_req, 0);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_check_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    evp_pkey = EVP_PKEY_new();
    VerifyOrExit(evp_pkey != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(evp_pkey, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = X509_REQ_set_pubkey(x509_req, evp_pkey);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // TODO: mbedTLS CSR parser fails if the subject name is not set (or if empty).
    //       CHIP Spec doesn't specify the subject name that can be used.
    //       Figure out the correct value and update this code.
    result = X509_NAME_add_entry_by_txt(subject, "O", MBSTRING_ASC, Uint8::from_const_char("CSR"), -1, -1, 0);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = X509_REQ_set_subject_name(x509_req, subject);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = X509_REQ_sign(x509_req, evp_pkey, EVP_sha256());
    VerifyOrExit(result > 0, error = CHIP_ERROR_INTERNAL);

    csr_length_local = i2d_X509_REQ(x509_req, nullptr);
    VerifyOrExit(csr_length_local >= 0, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(csr_length_local), error = CHIP_ERROR_BUFFER_TOO_SMALL);
    VerifyOrExit(static_cast<size_t>(csr_length_local) <= csr_length, error = CHIP_ERROR_BUFFER_TOO_SMALL);
    csr_length = static_cast<size_t>(i2d_X509_REQ(x509_req, &out_csr));

exit:
    ec_key = nullptr;

    if (evp_pkey != nullptr)
    {
        EVP_PKEY_free(evp_pkey);
        evp_pkey = nullptr;
    }

    X509_NAME_free(subject);
    subject = nullptr;

    X509_REQ_free(x509_req);

    _logSSLError();
    return error;
}

CHIP_ERROR VerifyCertificateSigningRequest(const uint8_t * csr, size_t csr_length, P256PublicKey & pubkey)
{
    ReturnErrorOnFailure(VerifyCertificateSigningRequestFormat(csr, csr_length));

    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    EVP_PKEY * evp_pkey = nullptr;
    EC_KEY * ec_key     = nullptr;

    const unsigned char * csr_buf = Uint8::to_const_uchar(csr);
    X509_REQ * x509_req           = d2i_X509_REQ(nullptr, &csr_buf, (int) csr_length);
    VerifyOrExit(x509_req != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(X509_REQ_get_version(x509_req) == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    evp_pkey = X509_REQ_get_pubkey(x509_req);
    VerifyOrExit(evp_pkey != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = X509_REQ_verify(x509_req, evp_pkey);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = EVP_PKEY_get1_EC_KEY(evp_pkey);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = P256PublicKeyFromECKey(ec_key, pubkey);
    SuccessOrExit(error);

exit:

    if (x509_req != nullptr)
    {
        X509_REQ_free(x509_req);
    }

    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
    }

    if (evp_pkey != nullptr)
    {
        EVP_PKEY_free(evp_pkey);
    }
    _logSSLError();
    return error;
}

#define init_point(_point_)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        _point_ = EC_POINT_new(context->curve);                                                                                    \
        VerifyOrReturnError(_point_ != nullptr, CHIP_ERROR_INTERNAL);                                                              \
    } while (0)

#define init_bn(_bn_)                                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        _bn_ = BN_new();                                                                                                           \
        VerifyOrReturnError(_bn_ != nullptr, CHIP_ERROR_INTERNAL);                                                                 \
    } while (0)

#define free_point(_point_)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        if (_point_ != nullptr)                                                                                                    \
        {                                                                                                                          \
            EC_POINT_clear_free(static_cast<EC_POINT *>(_point_));                                                                 \
        }                                                                                                                          \
    } while (0)

#define free_bn(_bn_)                                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        if (_bn_ != nullptr)                                                                                                       \
        {                                                                                                                          \
            BN_clear_free(static_cast<BIGNUM *>(_bn_));                                                                            \
        }                                                                                                                          \
    } while (0)

typedef struct Spake2p_Context
{
    EC_GROUP * curve;
    BN_CTX * bn_ctx;
    const EVP_MD * md_info;
} Spake2p_Context;

static inline Spake2p_Context * to_inner_spake2p_context(Spake2pOpaqueContext * context)
{
    return SafePointerCast<Spake2p_Context *>(context);
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal()
{
    Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    context->curve   = nullptr;
    context->bn_ctx  = nullptr;
    context->md_info = nullptr;

    context->curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    VerifyOrReturnError(context->curve != nullptr, CHIP_ERROR_INTERNAL);

    G = EC_GROUP_get0_generator(context->curve);
    VerifyOrReturnError(G != nullptr, CHIP_ERROR_INTERNAL);

    context->bn_ctx = BN_CTX_secure_new();
    VerifyOrReturnError(context->bn_ctx != nullptr, CHIP_ERROR_INTERNAL);

    context->md_info = EVP_sha256();
    VerifyOrReturnError(context->md_info != nullptr, CHIP_ERROR_INTERNAL);

    init_point(M);
    init_point(N);
    init_point(X);
    init_point(Y);
    init_point(L);
    init_point(V);
    init_point(Z);
    init_bn(w0);
    init_bn(w1);
    init_bn(xy);
    init_bn(tempbn);
    init_bn(order);

    const int error_openssl = EC_GROUP_get_order(context->curve, static_cast<BIGNUM *>(order), context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

void Spake2p_P256_SHA256_HKDF_HMAC::Clear()
{
    VerifyOrReturn(state != CHIP_SPAKE2P_STATE::PREINIT);

    Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    if (context->curve != nullptr)
    {
        EC_GROUP_clear_free(context->curve);
    }

    if (context->bn_ctx != nullptr)
    {
        BN_CTX_free(context->bn_ctx);
    }

    free_point(M);
    free_point(N);
    free_point(X);
    free_point(Y);
    free_point(L);
    free_point(V);
    free_point(Z);
    free_bn(w0);
    free_bn(w1);
    free_bn(xy);
    free_bn(tempbn);
    free_bn(order);

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
    VerifyOrReturnError(mac_len == kSHA256_Hash_Length, CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t computed_mac[kSHA256_Hash_Length];
    MutableByteSpan computed_mac_span{ computed_mac };
    ReturnErrorOnFailure(Mac(key, key_len, in, in_len, computed_mac_span));
    VerifyOrReturnError(computed_mac_span.size() == mac_len, CHIP_ERROR_INTERNAL);

    VerifyOrReturnError(CRYPTO_memcmp(mac, computed_mac_span.data(), computed_mac_span.size()) == 0, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const uint8_t * in, size_t in_len, void * fe)
{
    BIGNUM * const bn_fe = static_cast<BIGNUM *>(fe);

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    VerifyOrReturnError(CanCastTo<boringssl_size_t_openssl_int>(in_len), CHIP_ERROR_INTERNAL);
    BN_bin2bn(Uint8::to_const_uchar(in), static_cast<boringssl_size_t_openssl_int>(in_len), bn_fe);
    const int error_openssl = BN_mod(bn_fe, bn_fe, (BIGNUM *) order, context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, uint8_t * out, size_t out_len)
{
    VerifyOrReturnError(CanCastTo<int>(out_len), CHIP_ERROR_INTERNAL);
    const int bn_out_len = BN_bn2binpad(static_cast<const BIGNUM *>(fe), Uint8::to_uchar(out), static_cast<int>(out_len));
    VerifyOrReturnError(bn_out_len == static_cast<int>(out_len), CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    const int error_openssl = BN_rand_range(static_cast<BIGNUM *>(fe), static_cast<BIGNUM *>(order));
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    const Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    const int error_openssl = BN_mod_mul(static_cast<BIGNUM *>(fer), static_cast<const BIGNUM *>(fe1),
                                         static_cast<const BIGNUM *>(fe2), static_cast<BIGNUM *>(order), context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const uint8_t * in, size_t in_len, void * R)
{
    const Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    const int error_openssl =
        EC_POINT_oct2point(context->curve, static_cast<EC_POINT *>(R), Uint8::to_const_uchar(in), in_len, context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, uint8_t * out, size_t out_len)
{
    const Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    const size_t ec_out_len = EC_POINT_point2oct(context->curve, static_cast<const EC_POINT *>(R), POINT_CONVERSION_UNCOMPRESSED,
                                                 Uint8::to_uchar(out), out_len, context->bn_ctx);
    VerifyOrReturnError(ec_out_len == out_len, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    const Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    const int error_openssl = EC_POINT_mul(context->curve, static_cast<EC_POINT *>(R), nullptr, static_cast<const EC_POINT *>(P1),
                                           static_cast<const BIGNUM *>(fe1), context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointAddMul(void * R, const void * P1, const void * fe1, const void * P2,
                                                      const void * fe2)
{
    CHIP_ERROR error   = CHIP_ERROR_INTERNAL;
    int error_openssl  = 0;
    EC_POINT * scratch = nullptr;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    scratch = EC_POINT_new(context->curve);
    VerifyOrExit(scratch != nullptr, error = CHIP_ERROR_INTERNAL);

    SuccessOrExit(error = PointMul(scratch, P1, fe1));
    SuccessOrExit(error = PointMul(R, P2, fe2));

    error_openssl = EC_POINT_add(context->curve, static_cast<EC_POINT *>(R), static_cast<EC_POINT *>(R),
                                 static_cast<const EC_POINT *>(scratch), context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    EC_POINT_clear_free(scratch);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointInvert(void * R)
{
    const Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    const int error_openssl = EC_POINT_invert(context->curve, static_cast<EC_POINT *>(R), context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointCofactorMul(void * R)
{
    // Cofactor on P256 is 1 so this is a NOP
    return CHIP_NO_ERROR;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(uint8_t * Lout, size_t * L_len, const uint8_t * w1in, size_t w1in_len)
{
    CHIP_ERROR error      = CHIP_ERROR_INTERNAL;
    int error_openssl     = 0;
    BIGNUM * w1_bn        = nullptr;
    EC_POINT * Lout_point = nullptr;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    w1_bn = BN_new();
    VerifyOrExit(w1_bn != nullptr, error = CHIP_ERROR_INTERNAL);

    Lout_point = EC_POINT_new(context->curve);
    VerifyOrExit(Lout_point != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<boringssl_size_t_openssl_int>(w1in_len), error = CHIP_ERROR_INTERNAL);
    BN_bin2bn(Uint8::to_const_uchar(w1in), static_cast<boringssl_size_t_openssl_int>(w1in_len), w1_bn);
    error_openssl = BN_mod(w1_bn, w1_bn, (BIGNUM *) order, context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error_openssl = EC_POINT_mul(context->curve, Lout_point, w1_bn, nullptr, nullptr, context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    *L_len = EC_POINT_point2oct(context->curve, Lout_point, POINT_CONVERSION_UNCOMPRESSED, Uint8::to_uchar(Lout), *L_len,
                                context->bn_ctx);
    VerifyOrExit(*L_len != 0, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    BN_clear_free(w1_bn);
    EC_POINT_clear_free(Lout_point);

    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointIsValid(void * R)
{
    const Spake2p_Context * const context = to_inner_spake2p_context(&mSpake2pContext);

    const int error_openssl = EC_POINT_is_on_curve(context->curve, static_cast<EC_POINT *>(R), context->bn_ctx);
    VerifyOrReturnError(error_openssl == 1, CHIP_ERROR_INTERNAL);

    return CHIP_NO_ERROR;
}

CHIP_ERROR VerifyAttestationCertificateFormat(const ByteSpan & cert, AttestationCertType certType)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    const uint8_t * certPtr = cert.data();
    X509 * x509Cert         = nullptr;
    bool extBasicPresent    = false;
    bool extKeyUsagePresent = false;
    bool extSKIDPresent     = false;
    bool extAKIDPresent     = false;

    VerifyOrReturnError(!cert.empty() && CanCastTo<long>(cert.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509Cert = d2i_X509(nullptr, &certPtr, static_cast<long>(cert.size()));
    VerifyOrExit(x509Cert != nullptr, err = CHIP_ERROR_INTERNAL);

    VerifyOrExit(X509_get_version(x509Cert) == 2, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(X509_get_serialNumber(x509Cert) != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(X509_get_signature_nid(x509Cert) == NID_ecdsa_with_SHA256, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(X509_get_issuer_name(x509Cert) != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(X509_get_notBefore(x509Cert) != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(X509_get_notAfter(x509Cert) != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(X509_get_subject_name(x509Cert) != nullptr, err = CHIP_ERROR_INTERNAL);

    // Verify public key presence and format.
    {
        Crypto::P256PublicKey pubkey;
        SuccessOrExit(err = ExtractPubkeyFromX509Cert(cert, pubkey));
    }

    for (int i = 0; i < X509_get_ext_count(x509Cert); i++)
    {
        X509_EXTENSION * ex = X509_get_ext(x509Cert, i);
        ASN1_OBJECT * obj   = X509_EXTENSION_get_object(ex);
        bool isCritical     = X509_EXTENSION_get_critical(ex) == 1;

        switch (OBJ_obj2nid(obj))
        {
        case NID_basic_constraints:
            VerifyOrExit(isCritical && !extBasicPresent, err = CHIP_ERROR_INTERNAL);
            extBasicPresent = true;
            {
                bool isCA    = X509_get_extension_flags(x509Cert) & EXFLAG_CA;
                long pathLen = X509_get_pathlen(x509Cert);
                if (certType == AttestationCertType::kDAC)
                {
                    VerifyOrExit(!isCA && pathLen == -1, err = CHIP_ERROR_INTERNAL);
                }
                else if (certType == AttestationCertType::kPAI)
                {
                    VerifyOrExit(isCA && pathLen == 0, err = CHIP_ERROR_INTERNAL);
                }
                else
                {
                    // For PAA, pathlen must be absent or equal to 1 (see Matter 1.1 spec 6.2.2.5)
                    VerifyOrExit(isCA && (pathLen == -1 || pathLen == 1), err = CHIP_ERROR_INTERNAL);
                }
            }
            break;
        case NID_key_usage:
            VerifyOrExit(isCritical && !extKeyUsagePresent, err = CHIP_ERROR_INTERNAL);
            extKeyUsagePresent = true;
            {
                uint32_t keyUsage = X509_get_key_usage(x509Cert);
                if (certType == AttestationCertType::kDAC)
                {
                    // SHALL only have the digitalSignature bit set.
                    VerifyOrExit(keyUsage == X509v3_KU_DIGITAL_SIGNATURE, err = CHIP_ERROR_INTERNAL);
                }
                else
                {
                    bool keyCertSignFlag = keyUsage & X509v3_KU_KEY_CERT_SIGN;
                    bool crlSignFlag     = keyUsage & X509v3_KU_CRL_SIGN;
                    bool otherFlags      = keyUsage &
                        ~static_cast<uint32_t>(X509v3_KU_CRL_SIGN | X509v3_KU_KEY_CERT_SIGN | X509v3_KU_DIGITAL_SIGNATURE);
                    VerifyOrExit(keyCertSignFlag && crlSignFlag && !otherFlags, err = CHIP_ERROR_INTERNAL);
                }
            }
            break;
        case NID_subject_key_identifier:
            VerifyOrExit(!isCritical && !extSKIDPresent, err = CHIP_ERROR_INTERNAL);
            VerifyOrExit(X509_get0_subject_key_id(x509Cert)->length == kSubjectKeyIdentifierLength, err = CHIP_ERROR_INTERNAL);
            extSKIDPresent = true;
            break;
        case NID_authority_key_identifier:
            VerifyOrExit(!isCritical && !extAKIDPresent, err = CHIP_ERROR_INTERNAL);
            VerifyOrExit(X509_get0_authority_key_id(x509Cert)->length == kAuthorityKeyIdentifierLength, err = CHIP_ERROR_INTERNAL);
            extAKIDPresent = true;
            break;
        default:
            break;
        }
    }
    // Mandatory extensions for all certs.
    VerifyOrExit(extBasicPresent && extKeyUsagePresent && extSKIDPresent, err = CHIP_ERROR_INTERNAL);

    if (certType == AttestationCertType::kDAC || certType == AttestationCertType::kPAI)
    {
        // Mandatory extension for DAC and PAI certs.
        VerifyOrExit(extAKIDPresent, err = CHIP_ERROR_INTERNAL);
    }

exit:
    X509_free(x509Cert);

    return err;
}

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen,
                                    CertificateChainValidationResult & result)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    int status                 = 0;
    X509_STORE_CTX * verifyCtx = nullptr;
    X509_STORE * store         = nullptr;
    STACK_OF(X509) * chain     = nullptr;
    X509 * x509RootCertificate = nullptr;
    X509 * x509CACertificate   = nullptr;
    X509 * x509LeafCertificate = nullptr;

    result = CertificateChainValidationResult::kInternalFrameworkError;

    VerifyOrReturnError(rootCertificate != nullptr && rootCertificateLen != 0 && CanCastTo<long>(rootCertificateLen),
                        (result = CertificateChainValidationResult::kRootArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturnError(leafCertificate != nullptr && leafCertificateLen != 0 && CanCastTo<long>(leafCertificateLen),
                        (result = CertificateChainValidationResult::kLeafArgumentInvalid, CHIP_ERROR_INVALID_ARGUMENT));

    store = X509_STORE_new();
    VerifyOrExit(store != nullptr, (result = CertificateChainValidationResult::kNoMemory, err = CHIP_ERROR_NO_MEMORY));

    verifyCtx = X509_STORE_CTX_new();
    VerifyOrExit(verifyCtx != nullptr, (result = CertificateChainValidationResult::kNoMemory, err = CHIP_ERROR_NO_MEMORY));

    chain = sk_X509_new_null();
    VerifyOrExit(chain != nullptr, (result = CertificateChainValidationResult::kNoMemory, err = CHIP_ERROR_NO_MEMORY));

    VerifyOrExit(CanCastTo<long>(rootCertificateLen),
                 (result = CertificateChainValidationResult::kRootArgumentInvalid, err = CHIP_ERROR_INVALID_ARGUMENT));
    x509RootCertificate = d2i_X509(nullptr, &rootCertificate, static_cast<long>(rootCertificateLen));
    VerifyOrExit(x509RootCertificate != nullptr,
                 (result = CertificateChainValidationResult::kRootFormatInvalid, err = CHIP_ERROR_INTERNAL));

    status = X509_STORE_add_cert(store, x509RootCertificate);
    VerifyOrExit(status == 1, (result = CertificateChainValidationResult::kInternalFrameworkError, err = CHIP_ERROR_INTERNAL));

    if (caCertificate != nullptr && caCertificateLen > 0)
    {
        VerifyOrExit(CanCastTo<long>(caCertificateLen),
                     (result = CertificateChainValidationResult::kICAArgumentInvalid, err = CHIP_ERROR_INVALID_ARGUMENT));
        x509CACertificate = d2i_X509(nullptr, &caCertificate, static_cast<long>(caCertificateLen));
        VerifyOrExit(x509CACertificate != nullptr,
                     (result = CertificateChainValidationResult::kICAFormatInvalid, err = CHIP_ERROR_INTERNAL));

        status = static_cast<int>(sk_X509_push(chain, x509CACertificate));
        VerifyOrExit(status == 1, (result = CertificateChainValidationResult::kInternalFrameworkError, err = CHIP_ERROR_INTERNAL));
    }

    VerifyOrExit(CanCastTo<long>(leafCertificateLen),
                 (result = CertificateChainValidationResult::kLeafArgumentInvalid, err = CHIP_ERROR_INVALID_ARGUMENT));
    x509LeafCertificate = d2i_X509(nullptr, &leafCertificate, static_cast<long>(leafCertificateLen));
    VerifyOrExit(x509LeafCertificate != nullptr,
                 (result = CertificateChainValidationResult::kLeafFormatInvalid, err = CHIP_ERROR_INTERNAL));

    status = X509_STORE_CTX_init(verifyCtx, store, x509LeafCertificate, chain);
    VerifyOrExit(status == 1, (result = CertificateChainValidationResult::kInternalFrameworkError, err = CHIP_ERROR_INTERNAL));

    // Set time used in the X509 certificate chain validation to the notBefore time of the leaf certificate.
    // That way the X509_verify_cert() validates that intermediate and root certificates were
    // valid at the time of the leaf certificate generation.
    {
        X509_VERIFY_PARAM * param = X509_STORE_CTX_get0_param(verifyCtx);
        chip::ASN1::ASN1UniversalTime asn1Time;
        char * asn1TimeStr = reinterpret_cast<char *>(X509_get_notBefore(x509LeafCertificate)->data);
        uint32_t unixEpoch;

        VerifyOrExit(param != nullptr, (result = CertificateChainValidationResult::kNoMemory, err = CHIP_ERROR_NO_MEMORY));

        VerifyOrExit(CHIP_NO_ERROR == asn1Time.ImportFrom_ASN1_TIME_string(CharSpan(asn1TimeStr, strlen(asn1TimeStr))),
                     (result = CertificateChainValidationResult::kLeafFormatInvalid, err = CHIP_ERROR_INTERNAL));

        VerifyOrExit(asn1Time.ExportTo_UnixTime(unixEpoch),
                     (result = CertificateChainValidationResult::kLeafFormatInvalid, err = CHIP_ERROR_INTERNAL));

        VerifyOrExit(CanCastTo<time_t>(unixEpoch),
                     (result = CertificateChainValidationResult::kLeafFormatInvalid, err = CHIP_ERROR_INTERNAL));
        X509_VERIFY_PARAM_set_time(param, static_cast<time_t>(unixEpoch));
        X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_X509_STRICT);
    }

    status = X509_verify_cert(verifyCtx);
    VerifyOrExit(status == 1, (result = CertificateChainValidationResult::kChainInvalid, err = CHIP_ERROR_CERT_NOT_TRUSTED));

    err    = CHIP_NO_ERROR;
    result = CertificateChainValidationResult::kSuccess;

exit:
    X509_free(x509LeafCertificate);
    X509_free(x509CACertificate);
    X509_free(x509RootCertificate);
    sk_X509_free(chain);
    X509_STORE_CTX_free(verifyCtx);
    X509_STORE_free(store);

    return err;
}

CHIP_ERROR IsCertificateValidAtIssuance(const ByteSpan & candidateCertificate, const ByteSpan & issuerCertificate)
{
    CHIP_ERROR error                            = CHIP_NO_ERROR;
    X509 * x509CandidateCertificate             = nullptr;
    X509 * x509issuerCertificate                = nullptr;
    const unsigned char * pCandidateCertificate = candidateCertificate.data();
    const unsigned char * pIssuerCertificate    = issuerCertificate.data();
    ASN1_TIME * candidateNotBeforeTime          = nullptr;
    ASN1_TIME * issuerNotBeforeTime             = nullptr;
    ASN1_TIME * issuerNotAfterTime              = nullptr;
    int result                                  = 0;
    int days                                    = 0;
    int seconds                                 = 0;

    VerifyOrReturnError(!candidateCertificate.empty() && CanCastTo<long>(candidateCertificate.size()) &&
                            !issuerCertificate.empty() && CanCastTo<long>(issuerCertificate.size()),
                        CHIP_ERROR_INVALID_ARGUMENT);

    x509CandidateCertificate = d2i_X509(nullptr, &pCandidateCertificate, static_cast<long>(candidateCertificate.size()));
    VerifyOrExit(x509CandidateCertificate != nullptr, error = CHIP_ERROR_NO_MEMORY);

    x509issuerCertificate = d2i_X509(nullptr, &pIssuerCertificate, static_cast<long>(issuerCertificate.size()));
    VerifyOrExit(x509issuerCertificate != nullptr, error = CHIP_ERROR_NO_MEMORY);

    candidateNotBeforeTime = X509_get_notBefore(x509CandidateCertificate);
    issuerNotBeforeTime    = X509_get_notBefore(x509issuerCertificate);
    issuerNotAfterTime     = X509_get_notAfter(x509issuerCertificate);
    VerifyOrExit(candidateNotBeforeTime && issuerNotBeforeTime && issuerNotAfterTime, error = CHIP_ERROR_INTERNAL);

    result = ASN1_TIME_diff(&days, &seconds, issuerNotBeforeTime, candidateNotBeforeTime);
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);
    result = _compareDaysAndSeconds(days, seconds);

    // check if candidateCertificate is issued at or after tbeCertificate's notBefore timestamp
    VerifyOrExit(result >= 0, error = CHIP_ERROR_CERT_EXPIRED);

    result = ASN1_TIME_diff(&days, &seconds, issuerNotAfterTime, candidateNotBeforeTime);
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);
    result = _compareDaysAndSeconds(days, seconds);

    // check if candidateCertificate is issued at or before tbeCertificate's notAfter timestamp
    VerifyOrExit(result <= 0, error = CHIP_ERROR_CERT_EXPIRED);

exit:
    X509_free(x509CandidateCertificate);
    X509_free(x509issuerCertificate);

    return error;
}

CHIP_ERROR IsCertificateValidAtCurrentTime(const ByteSpan & certificate)
{
    CHIP_ERROR error                   = CHIP_NO_ERROR;
    X509 * x509Certificate             = nullptr;
    const unsigned char * pCertificate = certificate.data();
    ASN1_TIME * time                   = nullptr;
    int result                         = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509Certificate = d2i_X509(nullptr, &pCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509Certificate != nullptr, error = CHIP_ERROR_NO_MEMORY);

    time = X509_get_notBefore(x509Certificate);
    VerifyOrExit(time, error = CHIP_ERROR_INTERNAL);

    result = X509_cmp_current_time(time);
    // check if certificate's notBefore timestamp is earlier than or equal to current time.
    VerifyOrExit(result == -1, error = CHIP_ERROR_CERT_EXPIRED);

    time = X509_get_notAfter(x509Certificate);
    VerifyOrExit(time, error = CHIP_ERROR_INTERNAL);

    result = X509_cmp_current_time(time);
    // check if certificate's notAfter timestamp is later than current time.
    VerifyOrExit(result == 1, error = CHIP_ERROR_CERT_EXPIRED);

exit:
    X509_free(x509Certificate);

    return error;
}

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey)
{
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    EC_KEY * ec_key                      = nullptr;
    EVP_PKEY * pkey                      = nullptr;
    X509 * x509certificate               = nullptr;
    const unsigned char * pCertificate   = certificate.data();
    const unsigned char ** ppCertificate = &pCertificate;
    unsigned char * pPubkey              = pubkey;
    unsigned char ** ppPubkey            = &pPubkey;
    int pkeyLen;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    pkey = X509_get_pubkey(x509certificate);
    VerifyOrExit(pkey != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(EVP_PKEY_base_id(pkey) == EVP_PKEY_EC, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(EVP_PKEY_bits(pkey) == 256, err = CHIP_ERROR_INTERNAL);

    ec_key = EVP_PKEY_get1_EC_KEY(pkey);
    VerifyOrExit(ec_key != nullptr, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(EC_GROUP_get_curve_name(EC_KEY_get0_group(ec_key)) == NID_X9_62_prime256v1, err = CHIP_ERROR_INTERNAL);

    pkeyLen = i2d_PublicKey(pkey, nullptr);
    VerifyOrExit(pkeyLen == static_cast<int>(pubkey.Length()), err = CHIP_ERROR_INTERNAL);

    VerifyOrExit(i2d_PublicKey(pkey, ppPubkey) == pkeyLen, err = CHIP_ERROR_INTERNAL);

exit:
    EC_KEY_free(ec_key);
    EVP_PKEY_free(pkey);
    X509_free(x509certificate);

    return err;
}

namespace {

CHIP_ERROR ExtractKIDFromX509Cert(bool isSKID, const ByteSpan & certificate, MutableByteSpan & kid)
{
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    X509 * x509certificate               = nullptr;
    const unsigned char * pCertificate   = certificate.data();
    const unsigned char ** ppCertificate = &pCertificate;
    const ASN1_OCTET_STRING * kidString  = nullptr;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    kidString = isSKID ? X509_get0_subject_key_id(x509certificate) : X509_get0_authority_key_id(x509certificate);
    VerifyOrExit(kidString != nullptr, err = CHIP_ERROR_NOT_FOUND);
    VerifyOrExit(CanCastTo<size_t>(kidString->length), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(kidString->length == kSubjectKeyIdentifierLength, err = CHIP_ERROR_WRONG_CERT_TYPE);
    VerifyOrExit(static_cast<size_t>(kidString->length) <= kid.size(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(kid.data(), kidString->data, static_cast<size_t>(kidString->length));

    kid.reduce_size(static_cast<size_t>(kidString->length));

exit:
    X509_free(x509certificate);

    return err;
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
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    X509 * x509certificate               = nullptr;
    const unsigned char * pCertificate   = certificate.data();
    const unsigned char ** ppCertificate = &pCertificate;
    STACK_OF(DIST_POINT) * crldp         = nullptr;
    DIST_POINT * dp                      = nullptr;
    GENERAL_NAMES * gens                 = nullptr;
    GENERAL_NAME * gen                   = nullptr;
    ASN1_STRING * uri                    = nullptr;
    const char * urlptr                  = nullptr;
    size_t len                           = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // CRL Distribution Point Extension is encoded as a sequence of DistributionPoint:
    //     CRLDistributionPoints ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
    //
    // This implementation only supports a single DistributionPoint (sequence of size 1)
    crldp =
        reinterpret_cast<STACK_OF(DIST_POINT) *>(X509_get_ext_d2i(x509certificate, NID_crl_distribution_points, nullptr, nullptr));
    VerifyOrExit(crldp != nullptr, err = CHIP_ERROR_NOT_FOUND);
    VerifyOrExit(sk_DIST_POINT_num(crldp) == 1, err = CHIP_ERROR_NOT_FOUND);

    dp = sk_DIST_POINT_value(crldp, 0);
    VerifyOrExit(dp != nullptr, err = CHIP_ERROR_NOT_FOUND);
    VerifyOrExit(dp->distpoint != nullptr && dp->distpoint->type == 0, err = CHIP_ERROR_NOT_FOUND);

    // The DistributionPoint is a sequence of three optional elements:
    //     DistributionPoint ::= SEQUENCE {
    //         distributionPoint       [0]     DistributionPointName OPTIONAL,
    //         reasons                 [1]     ReasonFlags OPTIONAL,
    //         cRLIssuer               [2]     GeneralNames OPTIONAL }
    //
    // where the DistributionPointName is a CHOICE of:
    //     DistributionPointName ::= CHOICE {
    //         fullName                [0]     GeneralNames,
    //         nameRelativeToCRLIssuer [1]     RelativeDistinguishedName }
    //
    // The URI should be encoded in the fullName element.
    // This implementation only supports a single GeneralName in the fullName sequence:
    //     GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
    gens = dp->distpoint->name.fullname;
    VerifyOrExit(sk_GENERAL_NAME_num(gens) == 1, err = CHIP_ERROR_NOT_FOUND);

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
    gen = sk_GENERAL_NAME_value(gens, 0);
    VerifyOrExit(gen->type == GEN_URI, err = CHIP_ERROR_NOT_FOUND);

    uri    = reinterpret_cast<ASN1_STRING *>(GENERAL_NAME_get0_value(gen, nullptr));
    urlptr = reinterpret_cast<const char *>(ASN1_STRING_get0_data(uri));
    VerifyOrExit(CanCastTo<size_t>(ASN1_STRING_length(uri)), err = CHIP_ERROR_NOT_FOUND);
    len = static_cast<size_t>(ASN1_STRING_length(uri));
    VerifyOrExit(
        (len > strlen(kValidCDPURIHttpPrefix) && strncmp(urlptr, kValidCDPURIHttpPrefix, strlen(kValidCDPURIHttpPrefix)) == 0) ||
            (len > strlen(kValidCDPURIHttpsPrefix) &&
             strncmp(urlptr, kValidCDPURIHttpsPrefix, strlen(kValidCDPURIHttpsPrefix)) == 0),
        err = CHIP_ERROR_NOT_FOUND);
    err = CopyCharSpanToMutableCharSpan(CharSpan(urlptr, len), cdpurl);

exit:
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
    X509_free(x509certificate);

    return err;
}

CHIP_ERROR ExtractCDPExtensionCRLIssuerFromX509Cert(const ByteSpan & certificate, MutableByteSpan & crlIssuer)
{
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    int result                           = 1;
    X509 * x509certificate               = nullptr;
    const unsigned char * pCertificate   = certificate.data();
    const unsigned char ** ppCertificate = &pCertificate;
    STACK_OF(DIST_POINT) * crldp         = nullptr;
    DIST_POINT * dp                      = nullptr;
    GENERAL_NAMES * gens                 = nullptr;
    GENERAL_NAME * gen                   = nullptr;
    X509_NAME * dirName                  = nullptr;
    const uint8_t * pDirName             = nullptr;
    size_t dirNameLen                    = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    // CRL Distribution Point Extension is encoded as a sequence of DistributionPoint:
    //     CRLDistributionPoints ::= SEQUENCE SIZE (1..MAX) OF DistributionPoint
    //
    // This implementation only supports a single DistributionPoint (sequence of size 1)
    crldp =
        reinterpret_cast<STACK_OF(DIST_POINT) *>(X509_get_ext_d2i(x509certificate, NID_crl_distribution_points, nullptr, nullptr));
    VerifyOrExit(crldp != nullptr, err = CHIP_ERROR_NOT_FOUND);
    VerifyOrExit(sk_DIST_POINT_num(crldp) == 1, err = CHIP_ERROR_NOT_FOUND);

    dp = sk_DIST_POINT_value(crldp, 0);
    VerifyOrExit(dp != nullptr, err = CHIP_ERROR_NOT_FOUND);

    // The DistributionPoint is a sequence of three optional elements:
    //     DistributionPoint ::= SEQUENCE {
    //         distributionPoint       [0]     DistributionPointName OPTIONAL,
    //         reasons                 [1]     ReasonFlags OPTIONAL,
    //         cRLIssuer               [2]     GeneralNames OPTIONAL }
    //
    // the cRLIssuer is encoded as a GeneralNames, where:
    //     GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
    // This implementation only supports a single GeneralName element in the cRLIssuer sequence:
    gens = dp->CRLissuer;
    VerifyOrExit(sk_GENERAL_NAME_num(gens) == 1, err = CHIP_ERROR_NOT_FOUND);

    // In this implementation the cRLIssuer is expected to be encoded as a directoryName field of the GeneralName:
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
    gen = sk_GENERAL_NAME_value(gens, 0);
    VerifyOrExit(gen->type == GEN_DIRNAME, err = CHIP_ERROR_NOT_FOUND);

    dirName = reinterpret_cast<X509_NAME *>(GENERAL_NAME_get0_value(gen, nullptr));
    VerifyOrExit(dirName != nullptr, err = CHIP_ERROR_NOT_FOUND);

    // Extract directoryName as a raw DER Encoded data
    result = X509_NAME_get0_der(dirName, &pDirName, &dirNameLen);
    VerifyOrExit(result == 1, err = CHIP_ERROR_INTERNAL);
    err = CopySpanToMutableSpan(ByteSpan(pDirName, dirNameLen), crlIssuer);

exit:
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
    X509_free(x509certificate);

    return err;
}

CHIP_ERROR ExtractSerialNumberFromX509Cert(const ByteSpan & certificate, MutableByteSpan & serialNumber)
{
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    X509 * x509certificate                = nullptr;
    auto * pCertificate                   = Uint8::to_const_uchar(certificate.data());
    const unsigned char ** ppCertificate  = &pCertificate;
    const ASN1_INTEGER * serialNumberASN1 = nullptr;
    size_t serialNumberLen                = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    serialNumberASN1 = X509_get_serialNumber(x509certificate);
    VerifyOrExit(serialNumberASN1 != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(serialNumberASN1->data != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(CanCastTo<size_t>(serialNumberASN1->length), err = CHIP_ERROR_INTERNAL);

    serialNumberLen = static_cast<size_t>(serialNumberASN1->length);
    VerifyOrExit(serialNumberLen <= serialNumber.size(), err = CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(serialNumber.data(), serialNumberASN1->data, serialNumberLen);
    serialNumber.reduce_size(serialNumberLen);

exit:
    X509_free(x509certificate);

    return err;
}

namespace {
CHIP_ERROR ExtractRawDNFromX509Cert(bool extractSubject, const ByteSpan & certificate, MutableByteSpan & dn)
{
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    int result                           = 1;
    X509 * x509certificate               = nullptr;
    auto * pCertificate                  = Uint8::to_const_uchar(certificate.data());
    const unsigned char ** ppCertificate = &pCertificate;
    X509_NAME * distinguishedName        = nullptr;
    const uint8_t * pDistinguishedName   = nullptr;
    size_t distinguishedNameLen          = 0;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    if (extractSubject)
    {
        distinguishedName = X509_get_subject_name(x509certificate);
    }
    else
    {
        distinguishedName = X509_get_issuer_name(x509certificate);
    }
    VerifyOrExit(distinguishedName != nullptr, err = CHIP_ERROR_INTERNAL);

    result = X509_NAME_get0_der(distinguishedName, &pDistinguishedName, &distinguishedNameLen);
    VerifyOrExit(result == 1, err = CHIP_ERROR_INTERNAL);
    err = CopySpanToMutableSpan(ByteSpan(pDistinguishedName, distinguishedNameLen), dn);

exit:
    X509_free(x509certificate);

    return err;
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

CHIP_ERROR ExtractVIDPIDFromX509Cert(const ByteSpan & certificate, AttestationCertVidPid & vidpid)
{
    ASN1_OBJECT * commonNameObj = OBJ_txt2obj("2.5.4.3", 1);
    ASN1_OBJECT * matterVidObj  = OBJ_txt2obj("1.3.6.1.4.1.37244.2.1", 1); // Matter VID OID - taken from Spec
    ASN1_OBJECT * matterPidObj  = OBJ_txt2obj("1.3.6.1.4.1.37244.2.2", 1); // Matter PID OID - taken from Spec

    CHIP_ERROR err                     = CHIP_NO_ERROR;
    X509 * x509certificate             = nullptr;
    const unsigned char * pCertificate = certificate.data();
    X509_NAME * subject                = nullptr;
    int x509EntryCountIdx              = 0;
    AttestationCertVidPid vidpidFromCN;

    VerifyOrReturnError(!certificate.empty() && CanCastTo<long>(certificate.size()), CHIP_ERROR_INVALID_ARGUMENT);

    x509certificate = d2i_X509(nullptr, &pCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    subject = X509_get_subject_name(x509certificate);
    VerifyOrExit(subject != nullptr, err = CHIP_ERROR_INTERNAL);

    for (x509EntryCountIdx = 0; x509EntryCountIdx < X509_NAME_entry_count(subject); ++x509EntryCountIdx)
    {
        X509_NAME_ENTRY * name_entry = X509_NAME_get_entry(subject, x509EntryCountIdx);
        VerifyOrExit(name_entry != nullptr, err = CHIP_ERROR_INTERNAL);
        ASN1_OBJECT * object = X509_NAME_ENTRY_get_object(name_entry);
        VerifyOrExit(object != nullptr, err = CHIP_ERROR_INTERNAL);

        DNAttrType attrType = DNAttrType::kUnspecified;
        if (OBJ_cmp(object, commonNameObj) == 0)
        {
            attrType = DNAttrType::kCommonName;
        }
        else if (OBJ_cmp(object, matterVidObj) == 0)
        {
            attrType = DNAttrType::kMatterVID;
        }
        else if (OBJ_cmp(object, matterPidObj) == 0)
        {
            attrType = DNAttrType::kMatterPID;
        }

        if (attrType != DNAttrType::kUnspecified)
        {
            ASN1_STRING * data_entry = X509_NAME_ENTRY_get_data(name_entry);
            VerifyOrExit(data_entry != nullptr, err = CHIP_ERROR_INTERNAL);
            unsigned char * str = ASN1_STRING_data(data_entry);
            VerifyOrExit(str != nullptr, err = CHIP_ERROR_INTERNAL);
            int len = ASN1_STRING_length(data_entry);
            VerifyOrExit(CanCastTo<size_t>(len), err = CHIP_ERROR_INTERNAL);

            err = ExtractVIDPIDFromAttributeString(attrType, ByteSpan(str, static_cast<size_t>(len)), vidpid, vidpidFromCN);
            SuccessOrExit(err);
        }
    }

    // If Matter Attributes were not found use values extracted from the CN Attribute,
    // which might be uninitialized as well.
    if (!vidpid.Initialized())
    {
        vidpid = vidpidFromCN;
    }

exit:
    ASN1_OBJECT_free(commonNameObj);
    ASN1_OBJECT_free(matterVidObj);
    ASN1_OBJECT_free(matterPidObj);
    X509_free(x509certificate);

    return err;
}

CHIP_ERROR ReplaceCertIfResignedCertFound(const ByteSpan & referenceCertificate, const ByteSpan * candidateCertificates,
                                          size_t candidateCertificatesCount, ByteSpan & outCertificate)
{
    CHIP_ERROR err                        = CHIP_NO_ERROR;
    X509 * x509ReferenceCertificate       = nullptr;
    X509 * x509CandidateCertificate       = nullptr;
    const uint8_t * pReferenceCertificate = referenceCertificate.data();
    X509_NAME * referenceSubject          = nullptr;
    X509_NAME * candidateSubject          = nullptr;
    uint8_t referenceSKIDBuf[kSubjectKeyIdentifierLength];
    uint8_t candidateSKIDBuf[kSubjectKeyIdentifierLength];
    MutableByteSpan referenceSKID(referenceSKIDBuf);
    MutableByteSpan candidateSKID(candidateSKIDBuf);

    ReturnErrorCodeIf(referenceCertificate.empty(), CHIP_ERROR_INVALID_ARGUMENT);

    outCertificate = referenceCertificate;

    ReturnErrorCodeIf(candidateCertificates == nullptr || candidateCertificatesCount == 0, CHIP_NO_ERROR);

    ReturnErrorOnFailure(ExtractSKIDFromX509Cert(referenceCertificate, referenceSKID));

    x509ReferenceCertificate = d2i_X509(nullptr, &pReferenceCertificate, static_cast<long>(referenceCertificate.size()));
    VerifyOrExit(x509ReferenceCertificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    referenceSubject = X509_get_subject_name(x509ReferenceCertificate);
    VerifyOrExit(referenceSubject != nullptr, err = CHIP_ERROR_INTERNAL);

    for (size_t i = 0; i < candidateCertificatesCount; i++)
    {
        const ByteSpan candidateCertificate   = candidateCertificates[i];
        const uint8_t * pCandidateCertificate = candidateCertificate.data();

        VerifyOrExit(!candidateCertificate.empty(), err = CHIP_ERROR_INVALID_ARGUMENT);

        SuccessOrExit(err = ExtractSKIDFromX509Cert(candidateCertificate, candidateSKID));

        x509CandidateCertificate = d2i_X509(nullptr, &pCandidateCertificate, static_cast<long>(candidateCertificate.size()));
        VerifyOrExit(x509CandidateCertificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

        candidateSubject = X509_get_subject_name(x509CandidateCertificate);
        VerifyOrExit(candidateSubject != nullptr, err = CHIP_ERROR_INTERNAL);

        if (referenceSKID.data_equal(candidateSKID) && X509_NAME_cmp(referenceSubject, candidateSubject) == 0)
        {
            outCertificate = candidateCertificate;
            ExitNow();
        }

        X509_free(x509CandidateCertificate);
        x509CandidateCertificate = nullptr;
    }

exit:
    X509_free(x509ReferenceCertificate);
    X509_free(x509CandidateCertificate);

    return err;
}

} // namespace Crypto
} // namespace chip
