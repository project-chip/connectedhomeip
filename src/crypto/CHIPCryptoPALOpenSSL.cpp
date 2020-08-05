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
 *      openSSL based implementation of CHIP crypto primitives
 */

#include "CHIPCryptoPAL.h"

#include <openssl/bn.h>
#include <openssl/conf.h>
#include <openssl/ecdsa.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include <openssl/ossl_typ.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <string.h>

namespace chip {
namespace Crypto {

#define kKeyLengthInBits 256

enum class DigestType
{
    SHA256
};

enum class ECName
{
    P256v1
};

static_assert(kMax_ECDH_Secret_Length >= 32, "ECDH shared secret is too short");
static_assert(kMax_ECDSA_Signature_Length >= 72, "ECDSA signature buffer length is too short");

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

static void _logSSLError()
{
    int ssl_err_code = ERR_get_error();
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
        return NULL;
        break;
    }
}

CHIP_ERROR AES_CCM_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad, size_t aad_length,
                           const unsigned char * key, size_t key_length, const unsigned char * iv, size_t iv_length,
                           unsigned char * ciphertext, unsigned char * tag, size_t tag_length)
{
    EVP_CIPHER_CTX * context = NULL;
    int bytesWritten         = 0;
    size_t ciphertext_length = 0;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int result               = 1;
    const EVP_CIPHER * type  = NULL;

    VerifyOrExit(plaintext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);

    // 16 bytes key for AES-CCM-128
    type = (key_length == 16) ? EVP_aes_128_ccm() : EVP_aes_256_ccm();

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_EncryptInit_ex(context, type, NULL, NULL, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, iv_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in tag length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, tag_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_EncryptInit_ex(context, NULL, NULL, key, iv);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in plain text length
    result = EVP_EncryptUpdate(context, NULL, &bytesWritten, NULL, plaintext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in AAD
    if (aad_length > 0 && aad != NULL)
    {
        result = EVP_EncryptUpdate(context, NULL, &bytesWritten, aad, aad_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Encrypt
    result = EVP_EncryptUpdate(context, ciphertext, &bytesWritten, plaintext, plaintext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    ciphertext_length = bytesWritten;

    // Finalize encryption
    result = EVP_EncryptFinal_ex(context, ciphertext + ciphertext_length, &bytesWritten);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    ciphertext_length += bytesWritten;

    // Get tag
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_GET_TAG, tag_length, tag);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != NULL)
    {
        EVP_CIPHER_CTX_free(context);
        context = NULL;
    }

    return error;
}

CHIP_ERROR AES_CCM_decrypt(const unsigned char * ciphertext, size_t ciphertext_length, const unsigned char * aad, size_t aad_length,
                           const unsigned char * tag, size_t tag_length, const unsigned char * key, size_t key_length,
                           const unsigned char * iv, size_t iv_length, unsigned char * plaintext)
{
    EVP_CIPHER_CTX * context = NULL;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int bytesOutput          = 0;
    int result               = 1;
    const EVP_CIPHER * type  = NULL;

    VerifyOrExit(ciphertext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidKeyLength(key_length), error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    // 16 bytes key for AES-CCM-128
    type = (key_length == 16) ? EVP_aes_128_ccm() : EVP_aes_256_ccm();

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_DecryptInit_ex(context, type, NULL, NULL, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in IV length
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_IVLEN, iv_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in expected tag
    result = EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_CCM_SET_TAG, tag_length, (void *) tag);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in key + iv
    result = EVP_DecryptInit_ex(context, NULL, NULL, key, iv);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher text length
    result = EVP_DecryptUpdate(context, NULL, &bytesOutput, NULL, ciphertext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Pass in aad
    if (aad_length > 0 && aad != NULL)
    {
        result = EVP_DecryptUpdate(context, NULL, &bytesOutput, aad, aad_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    // Pass in ciphertext. We wont get anything if validation fails.
    result = EVP_DecryptUpdate(context, plaintext, &bytesOutput, ciphertext, ciphertext_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != NULL)
    {
        EVP_CIPHER_CTX_free(context);
        context = NULL;
    }

    return error;
}

CHIP_ERROR Hash_SHA256(const unsigned char * data, const size_t data_length, unsigned char * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    // zero data length hash is supported.

    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    SHA256(data, data_length, out_buffer);

exit:
    return error;
}

Hash_SHA256_stream::Hash_SHA256_stream(void) {}

Hash_SHA256_stream::~Hash_SHA256_stream(void) {}

CHIP_ERROR Hash_SHA256_stream::Begin(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    result = SHA256_Init(&context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::AddData(const unsigned char * data, const size_t data_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    result = SHA256_Update(&context, data, data_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream::Finish(unsigned char * out_buffer)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    result = SHA256_Final(out_buffer, &context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

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
    EVP_PKEY_CTX * context;
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 1;

    context = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

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

    result = EVP_PKEY_derive_init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_set_hkdf_md(context, EVP_sha256());
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_set1_hkdf_key(context, secret, secret_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    if (salt_length > 0 && salt != NULL)
    {
        result = EVP_PKEY_CTX_set1_hkdf_salt(context, salt, salt_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

    result = EVP_PKEY_CTX_add1_hkdf_info(context, info, info_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_CTX_hkdf_mode(context, EVP_PKEY_HKDEF_MODE_EXTRACT_AND_EXPAND);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Get the OKM (Output Key Material)
    result = EVP_PKEY_derive(context, out_buffer, &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (context != NULL)
    {
        EVP_PKEY_CTX_free(context);
    }
    return error;
}

CHIP_ERROR pbkdf2_sha256(const unsigned char * password, size_t plen, const unsigned char * salt, size_t slen,
                         unsigned int iteration_count, uint32_t key_length, unsigned char * output)
{
    CHIP_ERROR error  = CHIP_NO_ERROR;
    int result        = 1;
    const EVP_MD * md = NULL;

    VerifyOrExit(password != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(salt != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(slen > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(output != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = _digestForType(DigestType::SHA256);
    VerifyOrExit(md != NULL, error = CHIP_ERROR_INTERNAL);

    result = PKCS5_PBKDF2_HMAC((const char *) password, plen, salt, slen, iteration_count, md, key_length, output);

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

CHIP_ERROR DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    int result       = 0;

    VerifyOrExit(out_buffer != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = RAND_priv_bytes(out_buffer, out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}

CHIP_ERROR ECDSA_sign_msg(const unsigned char * msg, const size_t msg_length, const unsigned char * private_key,
                          const size_t private_key_length, unsigned char * out_signature, size_t & out_signature_length)
{
    ERR_clear_error();

    CHIP_ERROR error       = CHIP_NO_ERROR;
    int result             = 0;
    EVP_MD_CTX * context   = NULL;
    int nid                = NID_undef;
    EC_KEY * ec_key        = NULL;
    EVP_PKEY * signing_key = NULL;
    char * _hexKey         = NULL;
    BIGNUM * pvt_key       = NULL;
    const EVP_MD * md      = NULL;
    ECName curve_name      = ECName::P256v1;
    DigestType digest      = DigestType::SHA256;
    size_t out_length      = 0;

    VerifyOrExit(msg != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    nid = _nidForCurve(curve_name);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(private_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(private_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    md = _digestForType(digest);
    VerifyOrExit(md != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != NULL, error = CHIP_ERROR_INTERNAL);

    pvt_key = BN_bin2bn(private_key, private_key_length, pvt_key);
    VerifyOrExit(pvt_key != NULL, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_private_key(ec_key, pvt_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    signing_key = EVP_PKEY_new();
    VerifyOrExit(signing_key != NULL, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(signing_key, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    context = EVP_MD_CTX_create();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestSignInit(context, NULL, md, NULL, signing_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestSignUpdate(context, msg, msg_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    // Call the EVP_DigestSignFinal with a NULL param to get length of the signature.

    result = EVP_DigestSignFinal(context, NULL, &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_signature_length >= out_length, error = CHIP_ERROR_INVALID_ARGUMENT);

    result = EVP_DigestSignFinal(context, out_signature, &out_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    // This should not happen due to the check above. But check this nonetheless
    VerifyOrExit(out_signature_length >= out_length, error = CHIP_ERROR_INTERNAL);
    out_signature_length = out_length;

exit:
    if (ec_key != NULL)
    {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }

    if (context != NULL)
    {
        EVP_MD_CTX_destroy(context);
        context = NULL;
    }
    if (signing_key != NULL)
    {
        EVP_PKEY_free(signing_key);
        signing_key = NULL;
    }

    if (error != CHIP_NO_ERROR)
    {
        _logSSLError();
    }

    if (_hexKey != NULL)
    {
        free(_hexKey);
    }

    if (pvt_key != NULL)
    {
        BN_free(pvt_key);
    }

    return error;
}

CHIP_ERROR ECDSA_validate_msg_signature(const unsigned char * msg, const size_t msg_length, const unsigned char * public_key,
                                        const size_t public_key_length, const unsigned char * signature,
                                        const size_t signature_length)
{
    ERR_clear_error();
    CHIP_ERROR error            = CHIP_ERROR_INTERNAL;
    int nid                     = NID_undef;
    const EVP_MD * md           = NULL;
    EC_KEY * ec_key             = NULL;
    EVP_PKEY * verification_key = NULL;
    EC_POINT * key_point        = NULL;
    EC_GROUP * ec_group         = NULL;
    int result                  = 0;
    EVP_MD_CTX * md_context     = NULL;
    ECName curve_name           = ECName::P256v1;
    DigestType digest           = DigestType::SHA256;

    VerifyOrExit(msg != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    nid = _nidForCurve(curve_name);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(public_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(public_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(signature != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(signature_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    md = _digestForType(digest);
    VerifyOrExit(md != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    ec_group = EC_GROUP_new_by_curve_name(nid);
    VerifyOrExit(ec_group != NULL, error = CHIP_ERROR_INTERNAL);

    key_point = EC_POINT_new(ec_group);
    VerifyOrExit(key_point != NULL, error = CHIP_ERROR_INTERNAL);

    result = EC_POINT_oct2point(ec_group, key_point, public_key, public_key_length, NULL);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != NULL, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_set_public_key(ec_key, key_point);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EC_KEY_check_key(ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    verification_key = EVP_PKEY_new();
    VerifyOrExit(verification_key != NULL, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(verification_key, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    md_context = EVP_MD_CTX_create();
    VerifyOrExit(md_context != NULL, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestVerifyInit(md_context, NULL, md, NULL, verification_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestVerifyUpdate(md_context, msg, msg_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_DigestVerifyFinal(md_context, signature, signature_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INVALID_SIGNATURE);
    error = CHIP_NO_ERROR;

exit:
    _logSSLError();
    if (ec_group != NULL)
    {
        EC_GROUP_free(ec_group);
        ec_group = NULL;
    }
    if (key_point != NULL)
    {
        EC_POINT_clear_free(key_point);
        key_point = NULL;
    }
    if (md_context)
    {
        EVP_MD_CTX_destroy(md_context);
        md_context = NULL;
    }
    if (ec_key != NULL)
    {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }
    if (verification_key != NULL)
    {
        EVP_PKEY_free(verification_key);
        verification_key = NULL;
    }
    return error;
}

// helper function to populate octet key into EVP_PKEY out_evp_pkey. Caller must free out_evp_pkey
static CHIP_ERROR _create_evp_key_from_binary_p256_key(const unsigned char * key, const size_t key_length, EVP_PKEY ** out_evp_pkey,
                                                       bool isPrivateKey)
{

    CHIP_ERROR error     = CHIP_NO_ERROR;
    BIGNUM * big_num_key = NULL;
    EC_KEY * ec_key      = NULL;
    int result           = -1;
    EC_POINT * point     = NULL;
    EC_GROUP * group     = NULL;
    int nid              = NID_undef;

    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(*out_evp_pkey == NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    nid = _nidForCurve(ECName::P256v1);
    VerifyOrExit(nid != NID_undef, error = CHIP_ERROR_INTERNAL);

    ec_key = EC_KEY_new_by_curve_name(nid);
    VerifyOrExit(ec_key != NULL, error = CHIP_ERROR_INTERNAL);

    big_num_key = BN_bin2bn(key, key_length, NULL);
    VerifyOrExit(big_num_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (isPrivateKey)
    {
        result = EC_KEY_set_private_key(ec_key, big_num_key);
    }
    else
    {
        group = EC_GROUP_new_by_curve_name(nid);
        VerifyOrExit(group != NULL, error = CHIP_ERROR_INTERNAL);

        point = EC_POINT_new(group);
        VerifyOrExit(point != NULL, error = CHIP_ERROR_INTERNAL);

        result = EC_POINT_oct2point(group, point, key, key_length, NULL);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

        result = EC_KEY_set_public_key(ec_key, point);
    }

    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    *out_evp_pkey = EVP_PKEY_new();
    VerifyOrExit(*out_evp_pkey != NULL, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_set1_EC_KEY(*out_evp_pkey, ec_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

exit:
    if (big_num_key)
    {
        BN_free(big_num_key);
        big_num_key = NULL;
    }

    if (ec_key != NULL)
    {
        EC_KEY_free(ec_key);
        ec_key = NULL;
    }

    if (error != CHIP_NO_ERROR && *out_evp_pkey)
    {
        EVP_PKEY_free(*out_evp_pkey);
        out_evp_pkey = NULL;
    }

    if (point != NULL)
    {
        EC_POINT_free(point);
        point = NULL;
    }

    if (group != NULL)
    {
        EC_GROUP_free(group);
        group = NULL;
    }

    return error;
}

CHIP_ERROR ECDH_derive_secret(const unsigned char * remote_public_key, const size_t remote_public_key_length,
                              const unsigned char * local_private_key, const size_t local_private_key_length,
                              unsigned char * out_secret, size_t & out_secret_length)
{
    ERR_clear_error();
    CHIP_ERROR error      = CHIP_NO_ERROR;
    int result            = -1;
    EVP_PKEY * local_key  = NULL;
    EVP_PKEY * remote_key = NULL;

    EVP_PKEY_CTX * context = NULL;
    size_t out_buf_length  = 0;

    VerifyOrExit(remote_public_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(remote_public_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(local_private_key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(local_private_key_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_secret != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_secret_length >= kMax_ECDH_Secret_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    error = _create_evp_key_from_binary_p256_key(local_private_key, local_private_key_length, &local_key, true);
    SuccessOrExit(error);

    error = _create_evp_key_from_binary_p256_key(remote_public_key, remote_public_key_length, &remote_key, false);
    SuccessOrExit(error);

    context = EVP_PKEY_CTX_new(local_key, NULL);
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_derive_init(context);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    result = EVP_PKEY_derive_set_peer(context, remote_key);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);

    out_buf_length = out_secret_length;
    result         = EVP_PKEY_derive(context, out_secret, &out_buf_length);
    VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    VerifyOrExit(out_secret_length >= out_buf_length, error = CHIP_ERROR_INTERNAL);
    out_secret_length = out_buf_length;

exit:
    if (local_key != NULL)
    {
        EVP_PKEY_free(local_key);
        local_key = NULL;
    }

    if (remote_key != NULL)
    {
        EVP_PKEY_free(remote_key);
        remote_key = NULL;
    }

    if (context != NULL)
    {
        EVP_PKEY_CTX_free(context);
        context = NULL;
    }

    _logSSLError();
    return error;
}

void ClearSecretData(uint8_t * buf, uint32_t len)
{
    memset(buf, 0, len);
}

#define init_point(_point_)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        _point_ = EC_POINT_new(context.curve);                                                                                     \
        VerifyOrExit(_point_ != NULL, error = CHIP_ERROR_INTERNAL);                                                                \
    } while (0)

#define init_bn(_bn_)                                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        _bn_ = BN_new();                                                                                                           \
        VerifyOrExit(_bn_ != NULL, error = CHIP_ERROR_INTERNAL);                                                                   \
    } while (0)

#define free_point(_point_) EC_POINT_clear_free((EC_POINT *) _point_)

#define free_bn(_bn_) BN_clear_free((BIGNUM *) _bn_)

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::InitInternal(void)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    context.curve   = NULL;
    context.bn_ctx  = NULL;
    context.md_info = NULL;

    context.curve = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    VerifyOrExit(context.curve != NULL, error = CHIP_ERROR_INTERNAL);

    G = (void *) EC_GROUP_get0_generator(context.curve);
    VerifyOrExit(G != NULL, error = CHIP_ERROR_INTERNAL);

    context.bn_ctx = BN_CTX_secure_new();
    VerifyOrExit(context.bn_ctx != NULL, error = CHIP_ERROR_INTERNAL);

    context.md_info = EVP_sha256();
    VerifyOrExit(context.md_info != NULL, error = CHIP_ERROR_INTERNAL);

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

    error_openssl = EC_GROUP_get_order(context.curve, (BIGNUM *) order, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

void Spake2p_P256_SHA256_HKDF_HMAC::FreeImpl(void)
{
    EC_GROUP_clear_free(context.curve);
    BN_CTX_free(context.bn_ctx);

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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::Mac(const unsigned char * key, size_t key_len, const unsigned char * in, size_t in_len,
                                              unsigned char * out)
{
    CHIP_ERROR error         = CHIP_ERROR_INTERNAL;
    int error_openssl        = 0;
    unsigned int mac_out_len = 0;

    HMAC_CTX * mac_ctx = HMAC_CTX_new();
    VerifyOrExit(mac_ctx != NULL, error = CHIP_ERROR_INTERNAL);

    error_openssl = HMAC_Init_ex(mac_ctx, key, key_len, context.md_info, NULL);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error_openssl = HMAC_Update(mac_ctx, in, in_len);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    mac_out_len   = hash_size;
    error_openssl = HMAC_Final(mac_ctx, out, &mac_out_len);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    HMAC_CTX_free(mac_ctx);
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::MacVerify(const unsigned char * key, size_t key_len, const unsigned char * mac,
                                                    size_t mac_len, const unsigned char * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    VerifyOrExit(mac_len == kSHA256_Hash_Length, error = CHIP_ERROR_INVALID_ARGUMENT);

    unsigned char computed_mac[kSHA256_Hash_Length];
    error = Mac(key, key_len, in, in_len, computed_mac);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(CRYPTO_memcmp(mac, computed_mac, mac_len) == 0, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FELoad(const unsigned char * in, size_t in_len, void * fe)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;
    BIGNUM * bn_fe    = (BIGNUM *) fe;

    BN_bin2bn(in, in_len, bn_fe);
    error_openssl = BN_mod(bn_fe, bn_fe, (BIGNUM *) order, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEWrite(const void * fe, unsigned char * out, size_t out_len)
{
    CHIP_ERROR error        = CHIP_ERROR_INTERNAL;
    unsigned int bn_out_len = BN_bn2binpad((BIGNUM *) fe, out, out_len);

    VerifyOrExit(bn_out_len == out_len, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEGenerate(void * fe)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    error_openssl = BN_rand_range((BIGNUM *) fe, (BIGNUM *) order);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::FEMul(void * fer, const void * fe1, const void * fe2)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    error_openssl = BN_mod_mul((BIGNUM *) fer, (BIGNUM *) fe1, (BIGNUM *) fe2, (BIGNUM *) order, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointLoad(const unsigned char * in, size_t in_len, void * R)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    error_openssl = EC_POINT_oct2point(context.curve, (EC_POINT *) R, in, in_len, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointWrite(const void * R, unsigned char * out, size_t out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    size_t ec_out_len =
        EC_POINT_point2oct(context.curve, (EC_POINT *) R, POINT_CONVERSION_UNCOMPRESSED, out, out_len, context.bn_ctx);
    VerifyOrExit(ec_out_len == out_len, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::PointMul(void * R, const void * P1, const void * fe1)
{
    CHIP_ERROR error  = CHIP_ERROR_INTERNAL;
    int error_openssl = 0;

    error_openssl = EC_POINT_mul(context.curve, (EC_POINT *) R, NULL, (EC_POINT *) P1, (BIGNUM *) fe1, context.bn_ctx);
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
    EC_POINT * scratch = NULL;

    scratch = EC_POINT_new(context.curve);
    VerifyOrExit(scratch != NULL, error = CHIP_ERROR_INTERNAL);

    error = PointMul(scratch, P1, fe1);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = PointMul(R, P2, fe2);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error_openssl = EC_POINT_add(context.curve, (EC_POINT *) R, (EC_POINT *) R, (const EC_POINT *) scratch, context.bn_ctx);
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

    error_openssl = EC_POINT_invert(context.curve, (EC_POINT *) R, context.bn_ctx);
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

CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC::ComputeL(unsigned char * Lout, size_t * L_len, const unsigned char * w1in,
                                                   size_t w1in_len)
{
    CHIP_ERROR error      = CHIP_ERROR_INTERNAL;
    int error_openssl     = 0;
    BIGNUM * w1_bn        = NULL;
    EC_POINT * Lout_point = NULL;

    w1_bn = BN_new();
    VerifyOrExit(w1_bn != NULL, error = CHIP_ERROR_INTERNAL);

    Lout_point = EC_POINT_new(context.curve);
    VerifyOrExit(Lout_point != NULL, error = CHIP_ERROR_INTERNAL);

    BN_bin2bn(w1in, w1in_len, w1_bn);
    error_openssl = BN_mod(w1_bn, w1_bn, (BIGNUM *) order, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error_openssl = EC_POINT_mul(context.curve, Lout_point, w1_bn, NULL, NULL, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    *L_len = EC_POINT_point2oct(context.curve, Lout_point, POINT_CONVERSION_UNCOMPRESSED, Lout, *L_len, context.bn_ctx);
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

    error_openssl = EC_POINT_is_on_curve(context.curve, (EC_POINT *) R, context.bn_ctx);
    VerifyOrExit(error_openssl == 1, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

} // namespace Crypto
} // namespace chip
