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
 *      openSSL based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"

#include <type_traits>

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

#include <core/CHIPSafeCasts.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>
#include <support/SafeInt.h>
#include <support/SafePointerCast.h>
#include <support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

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

nlSTATIC_ASSERT_PRINT(kMax_ECDH_Secret_Length >= 32, "ECDH shared secret is too short");
nlSTATIC_ASSERT_PRINT(kMax_ECDSA_Signature_Length >= 72, "ECDSA signature buffer length is too short");

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

static bool _isValidTagLength(size_t tag_length)
{
    return tag_length == 8 || tag_length == 12 || tag_length == 16;
}

static bool _isValidKeyLength(size_t length)
{
    // 16 bytes key for AES-CCM-128, 32 for AES-CCM-256
    return length == 16 || length == 32;
}

static void _logSSLError()
{
    unsigned long ssl_err_code = ERR_get_error();
    while (ssl_err_code != 0)
    {
        const char * err_str_lib     = ERR_lib_error_string(ssl_err_code);
        const char * err_str_routine = ERR_func_error_string(ssl_err_code);
        const char * err_str_reason  = ERR_reason_error_string(ssl_err_code);
        if (err_str_lib)
        {
            ChipLogError(Crypto, " ssl err  %s %s %s\n", err_str_lib, err_str_routine, err_str_reason);
        }
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

CHIP_ERROR AES_CCM_encrypt(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * key, size_t key_length, const uint8_t * iv, size_t iv_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
    EVP_CIPHER_CTX * context = nullptr;
    int bytesWritten         = 0;
    size_t ciphertext_length = 0;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int result               = 1;
    const EVP_CIPHER * type  = nullptr;

    VerifyOrExit(plaintext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<int>(iv_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);

    // 16 bytes key for AES-CCM-128
    type = (key_length == 16) ? EVP_aes_128_ccm() : EVP_aes_256_ccm();

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_EncryptInit_ex(context, type, nullptr, nullptr, nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length.  Cast is safe because we checked with CanCastTo.
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, static_cast<int>(iv_length), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in tag length. Cast is safe because we checked _isValidTagLength.
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, static_cast<int>(tag_length), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_EncryptInit_ex(context, nullptr, nullptr, Uint8::to_const_uchar(key), Uint8::to_const_uchar(iv));
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
    VerifyOrExit(bytesWritten >= 0, error = CHIP_ERROR_INTERNAL);
    ciphertext_length = static_cast<unsigned int>(bytesWritten);

    // Finalize encryption
    result = EVP_EncryptFinal_ex(context, ciphertext + ciphertext_length, &bytesWritten);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(bytesWritten >= 0, error = CHIP_ERROR_INTERNAL);
    ciphertext_length += static_cast<unsigned int>(bytesWritten);

    // Get tag
    VerifyOrExit(CanCastTo<int>(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_GET_TAG, static_cast<int>(tag_length), Uint8::to_uchar(tag));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != nullptr)
    {
        EVP_CIPHER_CTX_free(context);
        context = nullptr;
    }

    return error;
}

CHIP_ERROR AES_CCM_decrypt(const uint8_t * ciphertext, size_t ciphertext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * tag, size_t tag_length, const uint8_t * key, size_t key_length, const uint8_t * iv,
                           size_t iv_length, uint8_t * plaintext)
{
    EVP_CIPHER_CTX * context = nullptr;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int bytesOutput          = 0;
    int result               = 1;
    const EVP_CIPHER * type  = nullptr;

    VerifyOrExit(ciphertext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // 16 bytes key for AES-CCM-128
    type = (key_length == 16) ? EVP_aes_128_ccm() : EVP_aes_256_ccm();

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_DecryptInit_ex(context, type, nullptr, nullptr, nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length
    VerifyOrExit(CanCastTo<int>(iv_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, static_cast<int>(iv_length), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in expected tag
    // Removing "const" from |tag| here should hopefully be safe as
    // we're writing the tag, not reading.
    VerifyOrExit(CanCastTo<int>(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, static_cast<int>(tag_length),
                                 const_cast<void *>(static_cast<const void *>(tag)));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_DecryptInit_ex(context, nullptr, nullptr, Uint8::to_const_uchar(key), Uint8::to_const_uchar(iv));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher text length
    VerifyOrExit(CanCastTo<int>(ciphertext_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_DecryptUpdate(context, nullptr, &bytesOutput, nullptr, static_cast<int>(ciphertext_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in aad
    if (aad_length > 0 && aad != nullptr)
    {
        VerifyOrExit(CanCastTo<int>(aad_length), error = CHIP_ERROR_INVALID_ARGUMENT);
        result = EVP_DecryptUpdate(context, nullptr, &bytesOutput, Uint8::to_const_uchar(aad), static_cast<int>(aad_length));
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Pass in ciphertext. We wont get anything if validation fails.
    VerifyOrExit(CanCastTo<int>(ciphertext_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_DecryptUpdate(context, Uint8::to_uchar(plaintext), &bytesOutput, Uint8::to_const_uchar(ciphertext),
                               static_cast<int>(ciphertext_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != nullptr)
    {
        EVP_CIPHER_CTX_free(context);
        context = nullptr;
    }

    return error;
}

CHIP_ERROR Hash_SHA256(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    // zero data length hash is supported.

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    SHA256(data, data_length, Uint8::to_uchar(out_buffer));

exit:
    return error;
}

CHIP_ERROR Hash_SHA1(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    // zero data length hash is supported.
    VerifyOrReturnError(out_buffer != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    SHA1(data, data_length, Uint8::to_uchar(out_buffer));

    return CHIP_NO_ERROR;
}

Hash_SHA256_stream::Hash_SHA256_stream() {}

Hash_SHA256_stream::~Hash_SHA256_stream() {}

static inline SHA256_CTX * to_inner_hash_sha256_context(HashSHA256OpaqueContext * context)
{
    return SafePointerCast<SHA256_CTX *>(context);
}

CHIP_ERROR Hash_SHA256_stream::Begin()
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    SHA256_CTX * context = to_inner_hash_sha256_context(&mContext);

    result = SHA256_Init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const uint8_t * data, const size_t data_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    SHA256_CTX * context = to_inner_hash_sha256_context(&mContext);

    result = SHA256_Update(context, Uint8::to_const_uchar(data), data_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::Finish(uint8_t * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    SHA256_CTX * context = to_inner_hash_sha256_context(&mContext);

    result = SHA256_Final(Uint8::to_uchar(out_buffer), context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

void Hash_SHA256_stream::Clear()
{
    memset(this, 0, sizeof(*this));
}

CHIP_ERROR HKDF_sha::HKDF_SHA256(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                                 const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
    EVP_PKEY_CTX * context;
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    context = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);
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

    VerifyOrExit(CanCastTo<int>(secret_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_PKEY_CTX_set1_hkdf_key(context, Uint8::to_const_uchar(secret), static_cast<int>(secret_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    if (salt_length > 0 && salt != nullptr)
    {
        VerifyOrExit(CanCastTo<int>(salt_length), error = CHIP_ERROR_INVALID_ARGUMENT);
        result = EVP_PKEY_CTX_set1_hkdf_salt(context, Uint8::to_const_uchar(salt), static_cast<int>(salt_length));
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    VerifyOrExit(CanCastTo<int>(info_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = EVP_PKEY_CTX_add1_hkdf_info(context, Uint8::to_const_uchar(info), static_cast<int>(info_length));
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

CHIP_ERROR PBKDF2_sha256::pbkdf2_sha256(const uint8_t * password, size_t plen, const uint8_t * salt, size_t slen,
                                        unsigned int iteration_count, uint32_t key_length, uint8_t * output)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    int result        = 1;
    const EVP_MD * md = nullptr;

    VerifyOrExit(password != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen >= kMin_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen <= kMax_Salt_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = _digestForType(DigestType::SHA256);
    VerifyOrExit(md != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<int>(plen), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<int>(slen), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<int>(iteration_count), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(CanCastTo<int>(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = PKCS5_PBKDF2_HMAC(Uint8::to_const_char(password), static_cast<int>(plen), Uint8::to_const_uchar(salt),
                               static_cast<int>(slen), static_cast<int>(iteration_count), md, static_cast<int>(key_length),
                               Uint8::to_uchar(output));

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
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(CanCastTo<int>(out_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    result = RAND_priv_bytes(Uint8::to_uchar(out_buffer), static_cast<int>(out_length));
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
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

CHIP_ERROR P256Keypair::ECDSA_sign_msg(const uint8_t * msg, const size_t msg_length, P256ECDSASignature & out_signature)
{
    ERR_clear_error();

    CHIP_ERROR error       = CHIP_NO_ERROR;
    int result             = 0;
    EVP_MD_CTX * context   = nullptr;
    int nid                = NID_undef;
    EC_KEY * ec_key        = nullptr;
    EVP_PKEY * signing_key = nullptr;
    const EVP_MD * md      = nullptr;
    DigestType digest      = DigestType::SHA256;
    size_t out_length      = 0;

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    nid = _nidForCurve(MapECName(mPublicKey.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);
    md = _digestForType(digest);
    VerifyOrExit(md != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = to_EC_KEY(&mKeypair);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    signing_key = EVP_PKEY_new();
    VerifyOrExit(signing_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(signing_key, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    context = EVP_MD_CTX_create();
    VerifyOrExit(context != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestSignInit(context, nullptr, md, nullptr, signing_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestSignUpdate(context, Uint8::to_const_uchar(msg), msg_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Call the EVP_DigestSignFinal with a nullptr param to get length of the signature.

    result = EVP_DigestSignFinal(context, nullptr, &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_signature.Capacity() >= out_length, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = EVP_DigestSignFinal(context, Uint8::to_uchar(out_signature), &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    // This should not happen due to the check above. But check this nonetheless
    SuccessOrExit(out_signature.SetLength(out_length));

exit:
    ec_key = nullptr;

    if (context != nullptr)
    {
        EVP_MD_CTX_destroy(context);
        context = nullptr;
    }
    if (signing_key != nullptr)
    {
        EVP_PKEY_free(signing_key);
        signing_key = nullptr;
    }

    if (error != CHIP_NO_ERROR)
    {
        _logSSLError();
    }

    return error;
}

CHIP_ERROR P256Keypair::ECDSA_sign_hash(const uint8_t * hash, const size_t hash_length, P256ECDSASignature & out_signature)
{
    ERR_clear_error();

    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    int nid          = NID_undef;
    EC_KEY * ec_key  = nullptr;
    uint out_length  = 0;

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    nid = _nidForCurve(MapECName(mPublicKey.Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = to_EC_KEY(&mKeypair);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = ECDSA_sign(0, hash, static_cast<int>(hash_length), Uint8::to_uchar(out_signature), &out_length, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    // This should not happen due to the check above. But check this nonetheless
    SuccessOrExit(out_signature.SetLength(out_length));

exit:
    if (error != CHIP_NO_ERROR)
    {
        _logSSLError();
    }

    return error;
}

CHIP_ERROR P256PublicKey::ECDSA_validate_msg_signature(const uint8_t * msg, const size_t msg_length,
                                                       const P256ECDSASignature & signature) const
{
    ERR_clear_error();
    CHIP_ERROR error            = CHIP_ERROR_INTERNAL;
    int nid                     = NID_undef;
    const EVP_MD * md           = nullptr;
    EC_KEY * ec_key             = nullptr;
    EVP_PKEY * verification_key = nullptr;
    EC_POINT * key_point        = nullptr;
    EC_GROUP * ec_group         = nullptr;
    int result                  = 0;
    EVP_MD_CTX * md_context     = nullptr;
    DigestType digest           = DigestType::SHA256;

    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    nid = _nidForCurve(MapECName(Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = _digestForType(digest);
    VerifyOrExit(md != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(ec_group != nullptr, error = CHIP_ERROR_INTERNAL);

    key_point = EC_POINT_new(ec_group);
    VerifyOrExit(key_point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(ec_group, key_point, Uint8::to_const_uchar(*this), Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, key_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_check_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    verification_key = EVP_PKEY_new();
    VerifyOrExit(verification_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(verification_key, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    md_context = EVP_MD_CTX_create();
    VerifyOrExit(md_context != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestVerifyInit(md_context, nullptr, md, nullptr, verification_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestVerifyUpdate(md_context, Uint8::to_const_uchar(msg), msg_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestVerifyFinal(md_context, Uint8::to_const_uchar(signature), signature.Length());
    VerifyOrExit(result == 1, error = CHIP_ERROR_INVALID_SIGNATURE);
    error = CHIP_NO_ERROR;

exit:
    _logSSLError();
    if (ec_group != nullptr)
    {
        EC_GROUP_free(ec_group);
        ec_group = nullptr;
    }
    if (key_point != nullptr)
    {
        EC_POINT_clear_free(key_point);
        key_point = nullptr;
    }
    if (md_context)
    {
        EVP_MD_CTX_destroy(md_context);
        md_context = nullptr;
    }
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
    }
    if (verification_key != nullptr)
    {
        EVP_PKEY_free(verification_key);
        verification_key = nullptr;
    }
    return error;
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
    int result           = 0;

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(hash_length == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    nid = _nidForCurve(MapECName(Type()));
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(ec_group != nullptr, error = CHIP_ERROR_INTERNAL);

    key_point = EC_POINT_new(ec_group);
    VerifyOrExit(key_point != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(ec_group, key_point, Uint8::to_const_uchar(*this), Length(), nullptr);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != nullptr, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, key_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_check_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // The cast for length arguments is safe because values are small enough to fit.
    result = ECDSA_verify(0, hash, static_cast<int>(hash_length), Uint8::to_const_uchar(signature),
                          static_cast<int>(signature.Length()), ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INVALID_SIGNATURE);
    error = CHIP_NO_ERROR;

exit:
    _logSSLError();
    if (ec_group != nullptr)
    {
        EC_GROUP_free(ec_group);
        ec_group = nullptr;
    }
    if (key_point != nullptr)
    {
        EC_POINT_clear_free(key_point);
        key_point = nullptr;
    }
    if (ec_key != nullptr)
    {
        EC_KEY_free(ec_key);
        ec_key = nullptr;
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

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);

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
    result         = EVP_PKEY_derive(context, Uint8::to_uchar(out_secret), &out_buf_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    SuccessOrExit(out_secret.SetLength(out_buf_length));

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

void ClearSecretData(uint8_t * buf, uint32_t len)
{
    memset(buf, 0, len);
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

CHIP_ERROR P256Keypair::Initialize()
{
    ERR_clear_error();
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
        Encoding::BufferWriter bbuf(output, len);
        bbuf.Put(mPublicKey, mPublicKey.Length());
        bbuf.Put(privkey, sizeof(privkey));
        VerifyOrExit(bbuf.Fit(), error = CHIP_ERROR_NO_MEMORY);
        output.SetLength(bbuf.Needed());
    }

exit:
    memset(privkey, 0, sizeof(privkey));
    _logSSLError();
    return error;
}

CHIP_ERROR P256Keypair::Deserialize(P256SerializedKeypair & input)
{
    Encoding::BufferWriter bbuf(mPublicKey, mPublicKey.Length());

    BIGNUM * pvt_key     = nullptr;
    EC_GROUP * group     = nullptr;
    EC_POINT * key_point = nullptr;

    EC_KEY * ec_key = nullptr;
    ECName curve    = MapECName(mPublicKey.Type());

    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;
    int nid          = NID_undef;

    const uint8_t * privkey = Uint8::to_const_uchar(input) + mPublicKey.Length();

    VerifyOrExit(input.Length() == mPublicKey.Length() + kP256_PrivateKey_Length, error = CHIP_ERROR_INVALID_ARGUMENT);
    bbuf.Put(input, mPublicKey.Length());
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

P256Keypair::~P256Keypair()
{
    if (mInitialized)
    {
        EC_KEY * ec_key = to_EC_KEY(&mKeypair);
        EC_KEY_free(ec_key);
    }
}

CHIP_ERROR P256Keypair::NewCertificateSigningRequest(uint8_t * out_csr, size_t & csr_length)
{
    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    X509_REQ * x509_req = X509_REQ_new();
    EVP_PKEY * evp_pkey = nullptr;

    EC_KEY * ec_key = to_EC_KEY(&mKeypair);

    X509_NAME * subject = X509_NAME_new();
    VerifyOrExit(subject != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);

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
    ERR_clear_error();
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    EVP_PKEY * evp_pkey = nullptr;
    EC_KEY * ec_key     = nullptr;

    const unsigned char * csr_buf = Uint8::to_const_uchar(csr);
    X509_REQ * x509_req           = d2i_X509_REQ(nullptr, &csr_buf, (int) csr_length);
    VerifyOrExit(x509_req != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(X509_REQ_get_version(x509_req) == 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    evp_pkey = X509_REQ_get0_pubkey(x509_req);
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

    _logSSLError();
    return error;
}

#define init_point(_point_)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        _point_ = EC_POINT_new(context->curve);                                                                                    \
        VerifyOrExit(_point_ != nullptr, error = CHIP_ERROR_INTERNAL);                                                             \
    } while (0)

#define init_bn(_bn_)                                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        _bn_ = BN_new();                                                                                                           \
        VerifyOrExit(_bn_ != nullptr, error = CHIP_ERROR_INTERNAL);                                                                \
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
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    context->curve   = nullptr;
    context->bn_ctx  = nullptr;
    context->md_info = nullptr;

    context->curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    VerifyOrExit(context->curve != nullptr, error = CHIP_ERROR_INTERNAL);

    G = EC_GROUP_get0_generator(context->curve);
    VerifyOrExit(G != nullptr, error = CHIP_ERROR_INTERNAL);

    context->bn_ctx = BN_CTX_secure_new();
    VerifyOrExit(context->bn_ctx != nullptr, error = CHIP_ERROR_INTERNAL);

    context->md_info = EVP_sha256();
    VerifyOrExit(context->md_info != nullptr, error = CHIP_ERROR_INTERNAL);

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

    error_openssl = EC_GROUP_get_order(context->curve, static_cast<BIGNUM *>(order), context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::FreeImpl()
{
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

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
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Mac(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, uint8_t * out)
{
    CHIP_ERROR error         = CHIP_ERROR_INTERNAL;
    int error_openssl        = 0;
    unsigned int mac_out_len = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    HMAC_CTX * mac_ctx = HMAC_CTX_new();
    VerifyOrExit(mac_ctx != nullptr, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<int>(key_len), error = CHIP_ERROR_INTERNAL);
    error_openssl = HMAC_Init_ex(mac_ctx, Uint8::to_const_uchar(key), static_cast<int>(key_len), context->md_info, nullptr);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error_openssl = HMAC_Update(mac_ctx, Uint8::to_const_uchar(in), in_len);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CanCastTo<int>(hash_size), error = CHIP_ERROR_INTERNAL);
    mac_out_len   = static_cast<unsigned int>(hash_size);
    error_openssl = HMAC_Final(mac_ctx, Uint8::to_uchar(out), &mac_out_len);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    HMAC_CTX_free(mac_ctx);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::MacVerify(const uint8_t * key, size_t key_len, const uint8_t * mac, size_t mac_len,
                                                    const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    VerifyOrExit(mac_len == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t computed_mac[kSHA256_Hash_Length];
    error = Mac(key, key_len, in, in_len, computed_mac);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CRYPTO_memcmp(mac, computed_mac, mac_len) == 0, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const uint8_t * in, size_t in_len, void * fe)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;
    BIGNUM * bn_fe    = static_cast<BIGNUM *>(fe);

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    VerifyOrExit(CanCastTo<int>(in_len), error = CHIP_ERROR_INTERNAL);
    BN_bin2bn(Uint8::to_const_uchar(in), static_cast<int>(in_len), bn_fe);
    error_openssl = BN_mod(bn_fe, bn_fe, (BIGNUM *) order, context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, uint8_t * out, size_t out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    int bn_out_len;
    VerifyOrExit(CanCastTo<int>(out_len), error = CHIP_ERROR_INTERNAL);
    bn_out_len = BN_bn2binpad(static_cast<const BIGNUM *>(fe), Uint8::to_uchar(out), static_cast<int>(out_len));

    VerifyOrExit(bn_out_len == static_cast<int>(out_len), error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    error_openssl = BN_rand_range(static_cast<BIGNUM *>(fe), static_cast<BIGNUM *>(order));
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    error_openssl = BN_mod_mul(static_cast<BIGNUM *>(fer), static_cast<const BIGNUM *>(fe1), static_cast<const BIGNUM *>(fe2),
                               static_cast<BIGNUM *>(order), context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const uint8_t * in, size_t in_len, void * R)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    error_openssl =
        EC_POINT_oct2point(context->curve, static_cast<EC_POINT *>(R), Uint8::to_const_uchar(in), in_len, context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, uint8_t * out, size_t out_len)
{
    CHIP_ERROR error          = CHIP_ERROR_INTERNAL;
    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    size_t ec_out_len = EC_POINT_point2oct(context->curve, static_cast<const EC_POINT *>(R), POINT_CONVERSION_UNCOMPRESSED,
                                           Uint8::to_uchar(out), out_len, context->bn_ctx);
    VerifyOrExit(ec_out_len == out_len, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    error_openssl = EC_POINT_mul(context->curve, static_cast<EC_POINT *>(R), nullptr, static_cast<const EC_POINT *>(P1),
                                 static_cast<const BIGNUM *>(fe1), context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
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

    error = PointMul(scratch, P1, fe1);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointMul(R, P2, fe2);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

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
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    error_openssl = EC_POINT_invert(context->curve, static_cast<EC_POINT *>(R), context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
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

    VerifyOrExit(CanCastTo<int>(w1in_len), error = CHIP_ERROR_INTERNAL);
    BN_bin2bn(Uint8::to_const_uchar(w1in), static_cast<int>(w1in_len), w1_bn);
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
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    Spake2p_Context * context = to_inner_spake2p_context(&mSpake2pContext);

    error_openssl = EC_POINT_is_on_curve(context->curve, static_cast<EC_POINT *>(R), context->bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

static void security_free_cert_list(X509_LIST * certs)
{
    if (certs)
    {
        sk_X509_pop_free(certs, X509_free);
    }
}

CHIP_ERROR LoadCertsFromPKCS7(const char * pkcs7, X509DerCertificate * x509list, uint32_t * max_certs)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    X509_LIST * certs = NULL;
    BIO * bio_cert    = NULL;
    PKCS7 * p7        = NULL;
    int p7_type       = 0;

    VerifyOrExit(x509list != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(max_certs != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    bio_cert = BIO_new_mem_buf(pkcs7, -1);

    p7 = PEM_read_bio_PKCS7(bio_cert, NULL, NULL, NULL);
    VerifyOrExit(p7 != nullptr, err = CHIP_ERROR_WRONG_CERT_TYPE);

    p7_type = OBJ_obj2nid(p7->type);
    if (p7_type == NID_pkcs7_signed)
    {
        certs = p7->d.sign->cert;
    }
    else if (p7_type == NID_pkcs7_signedAndEnveloped)
    {
        certs = p7->d.signed_and_enveloped->cert;
    }

    VerifyOrExit(certs != NULL, err = CHIP_ERROR_WRONG_CERT_TYPE);
    VerifyOrExit(static_cast<uint32_t>(sk_X509_num(certs)) <= *max_certs, err = CHIP_ERROR_WRONG_CERT_TYPE);

    *max_certs = static_cast<uint32_t>(sk_X509_num(certs));

    certs = X509_chain_up_ref(certs);

    for (uint32_t i = 0; i < *max_certs; ++i)
    {
        size_t bytes_written          = 0;
        unsigned char * pX509ListEnd  = x509list[i];
        unsigned char ** pX509ListAux = &pX509ListEnd;

        bytes_written = static_cast<size_t>(i2d_X509(sk_X509_value(certs, static_cast<int>(i)), pX509ListAux));

        VerifyOrExit(bytes_written <= x509list[i].Capacity(), err = CHIP_ERROR_NO_MEMORY);

        x509list[i].SetLength(bytes_written);
    }

exit:
    BIO_free_all(bio_cert);
    PKCS7_free(p7);
    security_free_cert_list(certs);

    return err;
}

CHIP_ERROR LoadCertFromPKCS7(const char * pkcs7, X509DerCertificate * x509list, uint32_t n_cert)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    X509_LIST * certs = NULL;
    BIO * bio_cert    = NULL;
    PKCS7 * p7        = NULL;
    int p7_type       = 0;

    VerifyOrExit(x509list != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    bio_cert = BIO_new_mem_buf(pkcs7, -1);

    p7 = PEM_read_bio_PKCS7(bio_cert, NULL, NULL, NULL);
    VerifyOrExit(p7 != nullptr, err = CHIP_ERROR_WRONG_CERT_TYPE);

    p7_type = OBJ_obj2nid(p7->type);
    if (p7_type == NID_pkcs7_signed)
    {
        certs = p7->d.sign->cert;
    }
    else if (p7_type == NID_pkcs7_signedAndEnveloped)
    {
        certs = p7->d.signed_and_enveloped->cert;
    }

    VerifyOrExit(certs != NULL, err = CHIP_ERROR_WRONG_CERT_TYPE);
    VerifyOrExit(n_cert < static_cast<uint32_t>(sk_X509_num(certs)), err = CHIP_ERROR_INVALID_ARGUMENT);

    certs = X509_chain_up_ref(certs);

    {
        size_t bytes_written          = 0;
        unsigned char * pX509ListEnd  = reinterpret_cast<unsigned char *>(x509list);
        unsigned char ** pX509ListAux = &pX509ListEnd;

        bytes_written = static_cast<size_t>(i2d_X509(sk_X509_value(certs, static_cast<int>(n_cert)), pX509ListAux));

        VerifyOrExit(bytes_written <= x509list->Capacity(), err = CHIP_ERROR_NO_MEMORY);

        x509list->SetLength(bytes_written);
    }

exit:
    BIO_free_all(bio_cert);
    PKCS7_free(p7);
    security_free_cert_list(certs);

    return err;
}

CHIP_ERROR GetNumberOfCertsFromPKCS7(const char * pkcs7, uint32_t * n_certs)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    X509_LIST * certs = NULL;
    BIO * bio_cert    = NULL;
    PKCS7 * p7        = NULL;
    int p7_type       = 0;

    VerifyOrExit(n_certs != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    bio_cert = BIO_new_mem_buf(pkcs7, -1);

    p7 = PEM_read_bio_PKCS7(bio_cert, NULL, NULL, NULL);
    VerifyOrExit(p7 != nullptr, err = CHIP_ERROR_WRONG_CERT_TYPE);

    p7_type = OBJ_obj2nid(p7->type);
    if (p7_type == NID_pkcs7_signed)
    {
        certs = p7->d.sign->cert;
    }
    else if (p7_type == NID_pkcs7_signedAndEnveloped)
    {
        certs = p7->d.signed_and_enveloped->cert;
    }

    VerifyOrExit(certs != NULL, err = CHIP_ERROR_WRONG_CERT_TYPE);

    *n_certs = static_cast<uint32_t>(sk_X509_num(certs));

exit:
    BIO_free_all(bio_cert);
    PKCS7_free(p7);

    return err;
}

CHIP_ERROR ValidateCertificateChain(const uint8_t * rootCertificate, size_t rootCertificateLen, const uint8_t * caCertificate,
                                    size_t caCertificateLen, const uint8_t * leafCertificate, size_t leafCertificateLen)
{
    CHIP_ERROR err             = CHIP_NO_ERROR;
    X509_STORE_CTX * verifyCtx = nullptr;
    X509_STORE * store         = nullptr;
    X509 * x509RootCertificate = nullptr;
    X509 * x509CACertificate   = nullptr;
    X509 * x509LeafCertificate = nullptr;

    store = X509_STORE_new();
    VerifyOrExit(store != nullptr, err = CHIP_ERROR_NO_MEMORY);

    verifyCtx = X509_STORE_CTX_new();
    VerifyOrExit(verifyCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);

    x509RootCertificate = d2i_X509(NULL, &rootCertificate, static_cast<long>(rootCertificateLen));
    VerifyOrExit(x509RootCertificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = X509_STORE_add_cert(store, x509RootCertificate);
    VerifyOrExit(err == 1, err = CHIP_ERROR_INTERNAL);

    x509CACertificate = d2i_X509(NULL, &caCertificate, static_cast<long>(caCertificateLen));
    VerifyOrExit(x509CACertificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = X509_STORE_add_cert(store, x509CACertificate);
    VerifyOrExit(err == 1, err = CHIP_ERROR_INTERNAL);

    x509LeafCertificate = d2i_X509(NULL, &leafCertificate, static_cast<long>(leafCertificateLen));
    VerifyOrExit(x509LeafCertificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    err = X509_STORE_CTX_init(verifyCtx, store, x509LeafCertificate, NULL);
    VerifyOrExit(err == 1, err = CHIP_ERROR_INTERNAL);

    err = X509_verify_cert(verifyCtx);
    VerifyOrExit(err == 1, err = CHIP_ERROR_CERT_NOT_TRUSTED);

    err = CHIP_NO_ERROR;

exit:
    X509_free(x509LeafCertificate);
    X509_free(x509CACertificate);
    X509_free(x509RootCertificate);
    X509_STORE_CTX_free(verifyCtx);
    X509_STORE_free(store);

    return err;
}

CHIP_ERROR ExtractPubkeyFromX509Cert(const ByteSpan & certificate, Crypto::P256PublicKey & pubkey)
{
    CHIP_ERROR err                       = CHIP_NO_ERROR;
    EVP_PKEY * pkey                      = nullptr;
    X509 * x509certificate               = nullptr;
    const unsigned char * pCertificate   = certificate.data();
    const unsigned char ** ppCertificate = &pCertificate;
    unsigned char * pPubkey              = pubkey;
    unsigned char ** ppPubkey            = &pPubkey;
    int pkeyLen;

    x509certificate = d2i_X509(NULL, ppCertificate, static_cast<long>(certificate.size()));
    VerifyOrExit(x509certificate != nullptr, err = CHIP_ERROR_NO_MEMORY);

    pkey = X509_get_pubkey(x509certificate);
    VerifyOrExit(pkey != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(EVP_PKEY_base_id(pkey) == EVP_PKEY_EC, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(EVP_PKEY_bits(pkey) == 256, err = CHIP_ERROR_INTERNAL);

    pkeyLen = i2d_PublicKey(pkey, NULL);
    VerifyOrExit(pkeyLen == static_cast<int>(pubkey.Length()), err = CHIP_ERROR_INTERNAL);

    VerifyOrExit(i2d_PublicKey(pkey, ppPubkey) == pkeyLen, err = CHIP_ERROR_INTERNAL);

exit:
    EVP_PKEY_free(pkey);
    X509_free(x509certificate);

    return err;
}

} // namespace Crypto
} // namespace chip
