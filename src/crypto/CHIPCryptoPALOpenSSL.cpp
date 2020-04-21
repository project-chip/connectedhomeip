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

#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/kdf.h>
#include <openssl/ossl_typ.h>
#include <support/CodeUtils.h>
#include <string.h>

#define kKeyLengthInBits 256

enum class DigestType
{
    SHA256
};

enum class ECName
{
    P256v1
};

using namespace chip::Crypto;

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
            printf("\nssl err  %s %s %s\n", err_str_lib, err_str_routine, err_str_reason);
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

CHIP_ERROR chip::Crypto::AES_CCM_256_encrypt(const unsigned char * plaintext, size_t plaintext_length, const unsigned char * aad,
                                             size_t aad_length, const unsigned char * key, const unsigned char * iv,
                                             size_t iv_length, unsigned char * ciphertext, unsigned char * tag, size_t tag_length)
{
    EVP_CIPHER_CTX * context = NULL;
    int bytesWritten         = 0;
    size_t ciphertext_length = 0;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int result               = 1;

    VerifyOrExit(plaintext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(_isValidTagLength(tag_length), error = CHIP_ERROR_INVALID_ARGUMENT);

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_EncryptInit_ex(context, EVP_aes_256_ccm(), NULL, NULL, NULL);
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

CHIP_ERROR chip::Crypto::AES_CCM_256_decrypt(const unsigned char * ciphertext, size_t ciphertext_length, const unsigned char * aad,
                                             size_t aad_length, const unsigned char * tag, size_t tag_length,
                                             const unsigned char * key, const unsigned char * iv, size_t iv_length,
                                             unsigned char * plaintext)
{
    EVP_CIPHER_CTX * context = NULL;
    CHIP_ERROR error         = CHIP_NO_ERROR;
    int bytesOutput          = 0;
    int result               = 1;

    VerifyOrExit(ciphertext != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(iv_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

    context = EVP_CIPHER_CTX_new();
    VerifyOrExit(context != NULL, error = CHIP_ERROR_INTERNAL);

    // Pass in cipher
    result = EVP_DecryptInit_ex(context, EVP_aes_256_ccm(), NULL, NULL, NULL);
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

CHIP_ERROR chip::Crypto::HKDF_SHA256(const unsigned char * secret, const size_t secret_length, const unsigned char * salt,
                                     const size_t salt_length, const unsigned char * info, const size_t info_length,
                                     unsigned char * out_buffer, size_t out_length)
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

    // Info is optional
    if (info_length > 0)
    {
        VerifyOrExit(info != NULL, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
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

    if (info_length > 0 && info != NULL)
    {
        result = EVP_PKEY_CTX_add1_hkdf_info(context, info, info_length);
        VerifyOrExit(result == 1, error = CHIP_ERROR_INTERNAL);
    }

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

CHIP_ERROR chip::Crypto::DRBG_get_bytes(unsigned char * out_buffer, const size_t out_length)
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

CHIP_ERROR chip::Crypto::ECDSA_sign_msg(const unsigned char * msg, const size_t msg_length, const unsigned char * private_key,
                                        const size_t private_key_length, unsigned char * out_signature,
                                        size_t & out_signature_length)
{
    ERR_clear_error();
    static_assert(kMax_ECDSA_Signature_Length >= 72, "ECDSA signature buffer length is too short");

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

    return error;
}

CHIP_ERROR chip::Crypto::ECDSA_validate_msg_signature(const unsigned char * msg, const size_t msg_length,
                                                      const unsigned char * public_key, const size_t public_key_length,
                                                      const unsigned char * signature, const size_t signature_length)
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
