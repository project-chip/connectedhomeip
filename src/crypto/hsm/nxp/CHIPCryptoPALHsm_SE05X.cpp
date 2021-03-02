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
 *      HSM based implementation of CHIP crypto primitives
 *      Based on configurations in CHIPCryptoPALHsm_config.h file,
 *      chip crypto apis use either HSM or rollback to software implementation.
 */

#include <crypto/CHIPCryptoPAL.h>
#include <type_traits>

#include <core/CHIPSafeCasts.h>
#include <support/BufferWriter.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <crypto/hsm/CHIPCryptoPALHsm_config.h>

#include <string.h>

/* se05x includes */
#include "ex_sss_boot.h"
#include "fsl_sss_api.h"
#include <fsl_sss_se05x_apis.h>
#include <se05x_APDU.h>


#if ENABLE_HSM_HASH_SHA256
#define MAX_SHA_ONE_SHOT_DATA_LEN 900
#endif

#if ENABLE_HSM_RAND_GEN
#define MAX_RANDOM_DATA_LEN 900
#endif

#if ENABLE_HSM_MAC
#define MAX_MAC_ONE_SHOT_DATA_LEN 900
#endif

#if ENABLE_HSM_HASH_SHA256_MULTISTEP
sss_digest_t ctx_digest_ms      = {0};
#endif

#if ((ENABLE_HSM_HASH_SHA256)    || \
    (ENABLE_HSM_HKDF_SHA256)     || \
    (ENABLE_HSM_AES_CCM_ENCRYPT) || \
    (ENABLE_HSM_AES_CCM_DECRYPT) || \
    (ENABLE_HSM_GENERATE_EC_KEY) || \
    (ENABLE_HSM_SPAKE_VERIFIER)  || \
    (ENABLE_HSM_SPAKE_PROVER)    || \
    (ENABLE_HSM_MAC) \
    )

ex_sss_boot_ctx_t gex_sss_chip_ctx;
static void se05x_sessionOpen();

#endif

#if ((ENABLE_HSM_HKDF_SHA256)    || \
    (ENABLE_HSM_AES_CCM_ENCRYPT) || \
    (ENABLE_HSM_AES_CCM_DECRYPT) || \
    (ENABLE_HSM_GENERATE_EC_KEY) || \
    (ENABLE_HSM_MAC) \
    )
static int _getKeyId();
#endif

#if ( (ENABLE_HSM_GENERATE_EC_KEY) || \
      (ENABLE_HSM_SPAKE_VERIFIER)  || \
      (ENABLE_HSM_SPAKE_PROVER)    || \
       (ENABLE_HSM_MAC) \
    )
static void delete_key(int keyid);
#endif


#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )

static CHIP_ERROR _setKey(int keyid, const uint8_t *key, size_t keylen, sss_key_part_t keyPart, sss_cipher_type_t cipherType);
const int m_id       = 0x2345;
const int n_id       = 0x2346;

// NXP-SE
// To store the context
// Context is calculated as SHA256("Const String" || PBKDFParamRequest || PBKDFParamResponse). So context is always 32 bytes.
// But test vectors can pass any context. So keeping the buffer as 128 bytes.
uint8_t spake_context[128] = {0,};
size_t spake_context_len = 0;

#endif

#if ENABLE_HSM_SPAKE_VERIFIER
const int w0in_id_v  = 0x2347;
const int Lin_id_v   = 0x2348;
#endif

#if ENABLE_HSM_SPAKE_PROVER
const int w0in_id_p  = 0x2349;
const int w1in_id_p  = 0x2350;
#endif


#if ENABLE_HSM_AES_CCM_ENCRYPT
CHIP_ERROR AES_CCM_encrypt_HSM(const uint8_t * plaintext, size_t plaintext_length, const uint8_t * aad, size_t aad_length,
                           const uint8_t * key, size_t key_length, const uint8_t * iv, size_t iv_length, uint8_t * ciphertext,
                           uint8_t * tag, size_t tag_length)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    sss_object_t keyObject   = {0};
    int keyid =  _getKeyId();
    sss_aead_t aead_ctx = {0};
    size_t tempOutbufLen = plaintext_length;
    size_t outIndex = 0;

    if (tag_length != 16 ) {
        /* Length not supported by se05x. Rollback to SW */
        return CHIP_ERROR_HSM;
    }

    /* Check if valid key length */
    if (!(key_length == 16 || key_length == 32)) {
        return CHIP_ERROR_INTERNAL;
    }

    VerifyOrExit(plaintext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(plaintext_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (iv_length > 0) {
        VerifyOrExit(iv != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_length > 0) {
        VerifyOrExit(aad != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    se05x_sessionOpen();

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(
        &keyObject, keyid, kSSS_KeyPart_Default, kSSS_CipherType_AES, key_length, kKeyObject_Mode_Transient);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, key, key_length, key_length * 8, NULL, 0);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_context_init(&aead_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_AES_CCM, kMode_SSS_Encrypt);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_init(&aead_ctx, (uint8_t*)iv, iv_length, tag_length, aad_length, plaintext_length);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_update_aad(&aead_ctx, aad, aad_length);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_update(&aead_ctx, plaintext, plaintext_length, ciphertext, &tempOutbufLen);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    outIndex = outIndex + tempOutbufLen ;
    tempOutbufLen = plaintext_length - tempOutbufLen;

    VerifyOrExit(outIndex <= plaintext_length, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_finish(&aead_ctx, NULL, 0, (ciphertext + outIndex), &tempOutbufLen, tag, &tag_length);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    if (aead_ctx.session != NULL) {
        sss_aead_context_free(&aead_ctx);
    }
    return error;
}
#endif

#if ENABLE_HSM_AES_CCM_DECRYPT
CHIP_ERROR AES_CCM_decrypt_HSM(const uint8_t * ciphertext, size_t ciphertext_len, const uint8_t * aad, size_t aad_len,
                           const uint8_t * tag, size_t tag_length, const uint8_t * key, size_t key_length, const uint8_t * iv,
                           size_t iv_length, uint8_t * plaintext)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    sss_object_t keyObject   = {0};
    int keyid =  _getKeyId();
    sss_aead_t aead_ctx = {0};
    size_t tempOutbufLen = ciphertext_len;
    size_t outIndex = 0;

    if (tag_length != 16 ) {
        /* Length not supported by se05x. Rollback to SW */
        return CHIP_ERROR_HSM;
    }

    /* Check if valid key length */
    if (!(key_length == 16 || key_length == 32)) {
        return CHIP_ERROR_INTERNAL;
    }

    VerifyOrExit(ciphertext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(ciphertext_len > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(tag != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (iv_length > 0) {
        VerifyOrExit(iv != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrExit(plaintext != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (aad_len > 0) {
        VerifyOrExit(aad != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    se05x_sessionOpen();

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(
        &keyObject, keyid, kSSS_KeyPart_Default, kSSS_CipherType_AES, key_length, kKeyObject_Mode_Transient);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, key, key_length, key_length * 8, NULL, 0);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_context_init(&aead_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_AES_CCM, kMode_SSS_Decrypt);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_init(&aead_ctx, (uint8_t*)iv, iv_length, tag_length, aad_len, ciphertext_len);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_update_aad(&aead_ctx, aad, aad_len);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_update(&aead_ctx, ciphertext, ciphertext_len, plaintext, &tempOutbufLen);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    outIndex = outIndex + tempOutbufLen ;
    tempOutbufLen = ciphertext_len - tempOutbufLen;

    VerifyOrExit(outIndex <= ciphertext_len, error = CHIP_ERROR_INTERNAL);

    status = sss_aead_finish(&aead_ctx, NULL, 0, (plaintext + outIndex), &tempOutbufLen, (uint8_t*)tag, &tag_length);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    if (aead_ctx.session != NULL) {
        sss_aead_context_free(&aead_ctx);
    }
    return error;
}
#endif


#if ( (ENABLE_HSM_HASH_SHA256) || (ENABLE_HSM_GENERATE_EC_KEY) )
CHIP_ERROR Hash_SHA256_HSM(const uint8_t * data, const size_t data_length, uint8_t * out_buffer)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_digest_t ctx_digest      = {0};
    sss_status_t status          = kStatus_SSS_Success;
    size_t digestLen             = 32;

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (data_length > 0)
    {
        VerifyOrExit(data != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    se05x_sessionOpen();

    status = sss_digest_context_init(&ctx_digest, &gex_sss_chip_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    if (data_length <= MAX_SHA_ONE_SHOT_DATA_LEN) {
        status = sss_digest_one_go(&ctx_digest, data, data_length, out_buffer, &digestLen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }
    else {
        /* Calculate SHA using multistep calls */
        size_t datalenTemp = 0;
        size_t rem_len = data_length;

        status = sss_digest_init(&ctx_digest);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        while(rem_len > 0)
        {
            datalenTemp = (rem_len > MAX_SHA_ONE_SHOT_DATA_LEN) ? MAX_SHA_ONE_SHOT_DATA_LEN : rem_len;
            status = sss_digest_update(&ctx_digest, (data + (data_length - rem_len)), datalenTemp);
            VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
            rem_len = rem_len - datalenTemp;
        }

        status = sss_digest_finish(&ctx_digest, out_buffer, &digestLen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    if (ctx_digest.session != NULL) {
        sss_digest_context_free(&ctx_digest);
    }
    return error;
}
#endif


/*
 * Warning: Multiple steams of data are not supported.
 * Note: - Spake implementation uses multistep.
 * Disable this with spake tests.
 */

#if ENABLE_HSM_HASH_SHA256_MULTISTEP

CHIP_ERROR Hash_SHA256_stream_Begin_HSM(void)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;

    se05x_sessionOpen();

    status = sss_digest_context_init(&ctx_digest_ms, &gex_sss_chip_ctx.session, kAlgorithm_SSS_SHA256, kMode_SSS_Digest);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_digest_init(&ctx_digest_ms);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}


CHIP_ERROR Hash_SHA256_stream_AddData_HSM(const uint8_t * data, const size_t data_length)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;

    if (data_length > 0)
    {
        VerifyOrExit(data != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    se05x_sessionOpen();

    status = sss_digest_update(&ctx_digest_ms, data, data_length);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Hash_SHA256_stream_Finish_HSM(uint8_t * out_buffer)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    size_t digestLen = 32;

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    se05x_sessionOpen();

    status = sss_digest_finish(&ctx_digest_ms, out_buffer, &digestLen);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

void Hash_SHA256_stream_Clear_HSM(void)
{
    if (ctx_digest_ms.session != NULL) {
        sss_digest_context_free(&ctx_digest_ms);
    }
}

#endif

#if ENABLE_HSM_HKDF_SHA256
CHIP_ERROR HKDF_SHA256_HSM(const uint8_t * secret, const size_t secret_length, const uint8_t * salt, const size_t salt_length,
                       const uint8_t * info, const size_t info_length, uint8_t * out_buffer, size_t out_length)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    smStatus_t smstatus     = SM_NOT_OK;
    sss_object_t keyObject   = {0};
    int keyid = _getKeyId();

    if (salt_length > 64 || info_length > 80 || secret_length > 256 || out_length > 768) {
        /* Length not supported by se05x. Rollback to SW */
        return CHIP_ERROR_HSM;
    }

    // Salt is optional
    if (salt_length > 0)
    {
        VerifyOrExit(salt != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrExit(info_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(info != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    se05x_sessionOpen();

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(
        &keyObject, keyid, kSSS_KeyPart_Default, kSSS_CipherType_HMAC, secret_length, kKeyObject_Mode_Transient);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, secret, secret_length, secret_length * 8, NULL, 0);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_HKDF_Extended(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        keyObject.keyId,
        kSE05x_DigestMode_SHA256,
        kSE05x_HkdfMode_ExtractExpand,
        salt,
        salt_length,
        0,
        info,
        info_length,
        0,
        (uint16_t)out_length,
        out_buffer,
        &out_length);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_erase_key(&gex_sss_chip_ctx.ks, &keyObject);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}
#endif


#if ENABLE_HSM_RAND_GEN
CHIP_ERROR DRBG_get_bytes_HSM(uint8_t * out_buffer, const size_t out_length)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    sss_rng_context_t rng_ctx = {0,};
    size_t rem_len = out_length;
    size_t out_buf_len = 0;

    VerifyOrExit(out_buffer != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    se05x_sessionOpen();

    status = sss_rng_context_init(&rng_ctx, &gex_sss_chip_ctx.session);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    while(rem_len > 0) {
        out_buf_len = (rem_len > MAX_RANDOM_DATA_LEN) ? MAX_RANDOM_DATA_LEN : rem_len;
        rem_len = rem_len - out_buf_len;

        status = sss_rng_get_random(&rng_ctx, out_buffer, out_buf_len);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    if (rng_ctx.session != NULL) {
        sss_rng_context_free(&rng_ctx);
    }

    return error;
}
#endif


#if ENABLE_HSM_GENERATE_EC_KEY
CHIP_ERROR P256Keypair_ECDSA_sign_msg_HSM(const uint8_t * msg, const size_t msg_length, uint8_t *out_signature, size_t *siglen, int keyid)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_asymmetric_t asymm_ctx = {0};
    uint8_t hash[32] = {0,};

    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(siglen != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (keyid == 0)
    {
        /* Key not stored in SE. Use software implementation */
        return CHIP_ERROR_HSM;
    }
    else
    {
        sss_status_t status = kStatus_SSS_Success;
        sss_object_t keyObject   = {0};

        VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

        se05x_sessionOpen();

        if (CHIP_NO_ERROR != Hash_SHA256_HSM(msg, msg_length, hash)) {
            goto exit;
        }

        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_get_handle(&keyObject, keyid);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Sign);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_sign_digest(&asymm_ctx, hash, 32, out_signature, siglen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL) {
        sss_asymmetric_context_free(&asymm_ctx);
    }
    return error;
}


CHIP_ERROR P256Keypair_ECDSA_sign_hash_HSM(const uint8_t * hash, const size_t hash_length, uint8_t *out_signature, size_t *siglen, int keyid)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_asymmetric_t asymm_ctx = {0};

    se05x_sessionOpen();

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(siglen != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (keyid == 0)
    {
        /* Key not stored in SE. Use software implementation */
        return CHIP_ERROR_HSM;
    }
    else
    {
        sss_status_t status = kStatus_SSS_Success;
        sss_object_t keyObject   = {0};

        se05x_sessionOpen();

        VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(hash_length == 32, error = CHIP_ERROR_INVALID_ARGUMENT);

        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_get_handle(&keyObject, keyid);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Sign);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_sign_digest(&asymm_ctx, (uint8_t*)hash, hash_length, out_signature, siglen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL) {
        sss_asymmetric_context_free(&asymm_ctx);
    }
    return error;
}


CHIP_ERROR P256PublicKey_ECDSA_validate_msg_signature_HSM(const uint8_t * msg, const size_t msg_length, const uint8_t *out_signature, size_t siglen, int keyid)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_asymmetric_t asymm_ctx = {0};
    uint8_t hash[32] = {0,};

    VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (keyid == 0)
    {
        /* Key not stored in SE. Use software implementation */
        return CHIP_ERROR_HSM;
    }
    else
    {
        sss_status_t status = kStatus_SSS_Success;
        sss_object_t keyObject   = {0};

        VerifyOrExit(msg != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(msg_length > 0, error = CHIP_ERROR_INVALID_ARGUMENT);

        se05x_sessionOpen();

        if (CHIP_NO_ERROR != Hash_SHA256_HSM(msg, msg_length, hash)) {
            goto exit;
        }

        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_get_handle(&keyObject, keyid);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Verify);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_verify_digest(&asymm_ctx, hash, sizeof(hash), (uint8_t*)out_signature, siglen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INVALID_SIGNATURE);
    }

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL) {
        sss_asymmetric_context_free(&asymm_ctx);
    }
    return error;
}


CHIP_ERROR P256PublicKey_ECDSA_validate_hash_signature_HSM(const uint8_t * hash, const size_t hash_length, const uint8_t *out_signature, size_t siglen, int keyid)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_asymmetric_t asymm_ctx = {0};

    VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_signature != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (keyid == 0)
    {
        /* Key not stored in SE. Use software implementation */
        return CHIP_ERROR_HSM;
    }
    else
    {
        sss_status_t status = kStatus_SSS_Success;
        sss_object_t keyObject   = {0};

        VerifyOrExit(hash != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(hash_length == 32, error = CHIP_ERROR_INVALID_ARGUMENT);

        se05x_sessionOpen();

        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_get_handle(&keyObject, keyid);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_context_init(&asymm_ctx, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_SHA256, kMode_SSS_Verify);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_asymmetric_verify_digest(&asymm_ctx, (uint8_t*)hash, hash_length, (uint8_t*)out_signature, siglen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INVALID_SIGNATURE);
    }

    error = CHIP_NO_ERROR;
exit:
    if (asymm_ctx.session != NULL) {
        sss_asymmetric_context_free(&asymm_ctx);
    }
    return error;
}


CHIP_ERROR P256Keypair_ECDH_derive_secret_HSM(const uint8_t *pubKey, size_t pubKeyLen, int priv_keyid, uint8_t* out_secret, size_t *secret_length)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    VerifyOrExit(pubKey != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_secret != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(secret_length != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (priv_keyid == 0)
    {
        /* Key not stored in SE. Use software implementation */
        return CHIP_ERROR_HSM;
    }
    else
    {
        smStatus_t smstatus = SM_NOT_OK;

        VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

        smstatus = Se05x_API_ECGenSharedSecret(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
            priv_keyid,
            pubKey,
            pubKeyLen,
            out_secret,
            secret_length);
        VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    return error;
}


CHIP_ERROR P256Keypair_Initialize_HSM(uint32_t keyid, uint8_t* pubkey, size_t* pbKeyLen)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    sss_object_t keyObject   = {0};
    size_t pbKeyBitLen = 0;

    VerifyOrExit(pubkey != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pbKeyLen != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (keyid == 0)
    {
        return CHIP_ERROR_HSM;
    }
    else
    {
        se05x_sessionOpen();

        pbKeyBitLen = (*pbKeyLen) * 8;

        status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_object_allocate_handle(&keyObject,
            keyid,
            kSSS_KeyPart_Pair,
            kSSS_CipherType_EC_NIST_P,
            256,
            kKeyObject_Mode_Transient);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_store_generate_key(&gex_sss_chip_ctx.ks, &keyObject, 256, 0);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        status = sss_key_store_get_key(&gex_sss_chip_ctx.ks,
            &keyObject,
            pubkey,
            pbKeyLen,
            &pbKeyBitLen);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    return error;
}

void P256Keypair_deleteKey_HSM(int keyid)
{
    if (keyid != 0){
        delete_key(keyid);
    }
}

#endif


#if ENABLE_HSM_MAC
CHIP_ERROR Spake2p_P256_SHA256_HKDF_HMAC_Mac_HSM(const uint8_t * key, size_t key_len, const uint8_t * in, size_t in_len, uint8_t * out)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    sss_status_t status = kStatus_SSS_Success;
    sss_mac_t ctx_mac = {0};
    sss_object_t keyObject = {0};
    int keyid =  _getKeyId();
    size_t out_len = 32;

    VerifyOrExit(key != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    if (key_len > 256) {
        return CHIP_ERROR_HSM;
    }

    se05x_sessionOpen();

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(
        &keyObject, keyid, kSSS_KeyPart_Default, kSSS_CipherType_HMAC, key_len, kKeyObject_Mode_Transient);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, key, key_len, key_len * 8, NULL, 0);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_mac_context_init(&ctx_mac, &gex_sss_chip_ctx.session, &keyObject, kAlgorithm_SSS_HMAC_SHA256, kMode_SSS_Mac);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    if (in_len <= MAX_MAC_ONE_SHOT_DATA_LEN) {
        status = sss_mac_one_go(&ctx_mac, in, in_len, out, &out_len);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }
    else {
        /* Calculate MAC using multistep calls */
        size_t datalenTemp = 0;
        size_t rem_len = in_len;

        status = sss_mac_init(&ctx_mac);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

        while(rem_len > 0)
        {
            datalenTemp = (rem_len > MAX_MAC_ONE_SHOT_DATA_LEN) ? MAX_MAC_ONE_SHOT_DATA_LEN : rem_len;
            status = sss_mac_update(&ctx_mac, (in + (in_len - rem_len)), datalenTemp);
            VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
            rem_len = rem_len - datalenTemp;
        }

        status = sss_mac_finish(&ctx_mac, out, &out_len);
        VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);
    }

    error = CHIP_NO_ERROR;
exit:
    if (keyid != 0) {
        delete_key(keyid);
    }
    return error;
}
#endif


#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )

void Spake2p_Finish_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role)
{
#if SSS_HAVE_SE05X_VER_GTE_16_03
    return;
#else

    if (gex_sss_chip_ctx.ks.session != NULL) {
        #if ENABLE_HSM_SPAKE_VERIFIER
            if(role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) {
                Se05x_API_DeleteCryptoObject(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx, kSE05x_CryptoObject_SPAKE_VERIFIER);
            }
        #endif

        #if ENABLE_HSM_SPAKE_PROVER
            if(role == chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER) {
                Se05x_API_DeleteCryptoObject(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx, kSE05x_CryptoObject_SPAKE_PROVER);
            }
        #endif
    }
    return;
#endif
}

CHIP_ERROR initPake(chip::Crypto::CHIP_SPAKE2P_ROLE role)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;
    uint8_t list[1024] = {0,};
    size_t listlen = sizeof(list);
    size_t i;
    uint8_t create_crypto_obj = 1;
    SE05x_CryptoModeSubType_t subtype;

    SE05x_CryptoObjectID_t spakeObjectId = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? kSE05x_CryptoObject_SPAKE_VERIFIER : kSE05x_CryptoObject_SPAKE_PROVER;

    se05x_sessionOpen();

    // To clear memory
    Spake2p_Finish_HSM(role);

    delete_key(m_id);
    error = _setKey(m_id, chip::Crypto::spake2p_M_p256, sizeof(chip::Crypto::spake2p_M_p256), kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    delete_key(n_id);
    error = _setKey(n_id, chip::Crypto::spake2p_N_p256, sizeof(chip::Crypto::spake2p_N_p256), kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_ReadCryptoObjectList(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        list,
        &listlen);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    for (i = 0; i < listlen; i += 4) {
        uint32_t cryptoObjectId = list[i + 1] | (list[i + 0] << 8);
        if (cryptoObjectId == spakeObjectId) {
            create_crypto_obj = 0;
        }
    }

    subtype.spakeAlgo = kSE05x_SpakeAlgo_P256_SHA256_HKDF_HMAC;

    if (create_crypto_obj) {
        smstatus = Se05x_API_CreateCryptoObject(
            &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
            spakeObjectId,
            kSE05x_CryptoContext_SPAKE,
            subtype);
        VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
    }

    smstatus = Se05x_API_PAKEInitProtocol(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        spakeObjectId,
        m_id,
        n_id);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}
#endif

#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )
CHIP_ERROR Spake2p_Init_HSM(const uint8_t * context, size_t context_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;

    if (context_len > 0) {
        VerifyOrExit(context != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    VerifyOrExit(context_len <= sizeof(spake_context), error = CHIP_ERROR_INTERNAL);

    memcpy(spake_context, context, context_len);
    spake_context_len = context_len;

    error = CHIP_NO_ERROR;
exit:
    return error;
}

#endif //#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )

#if ENABLE_HSM_SPAKE_VERIFIER
CHIP_ERROR Spake2p_BeginVerifier_HSM(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                  size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * Lin,
                                  size_t Lin_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;

    VerifyOrExit(w0in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(Lin != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (my_identity_len > 0)
    {
        VerifyOrExit(my_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (peer_identity_len > 0)
    {
        VerifyOrExit(peer_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    error = initPake(chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        SE05x_SPAKEDevice_B,
        kSE05x_CryptoObject_SPAKE_VERIFIER);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        kSE05x_CryptoObject_SPAKE_VERIFIER,
        (uint8_t*)spake_context,
        spake_context_len,
        (uint8_t*)peer_identity,
        peer_identity_len,
        (uint8_t*)my_identity,
        my_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#else
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        SE05x_SPAKEDevice_B,
        kSE05x_CryptoObject_SPAKE_VERIFIER,
        (uint8_t*)spake_context,
        spake_context_len,
        (uint8_t*)peer_identity,
        peer_identity_len,
        (uint8_t*)my_identity,
        my_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#endif

    error = _setKey(w0in_id_v, w0in, w0in_len, kSSS_KeyPart_Default, kSSS_CipherType_AES);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = _setKey(Lin_id_v, Lin, Lin_len, kSSS_KeyPart_Public, kSSS_CipherType_EC_NIST_P);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEInitCredentials(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        kSE05x_CryptoObject_SPAKE_VERIFIER,
        w0in_id_v,
        0,
        Lin_id_v);
#else
    smstatus = Se05x_API_PAKEInitDevice(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        kSE05x_CryptoObject_SPAKE_VERIFIER,
        w0in_id_v,
        0,
        Lin_id_v);
#endif
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}
#endif

#if ENABLE_HSM_SPAKE_PROVER
CHIP_ERROR Spake2p_BeginProver_HSM(const uint8_t * my_identity, size_t my_identity_len, const uint8_t * peer_identity,
                                size_t peer_identity_len, const uint8_t * w0in, size_t w0in_len, const uint8_t * w1in,
                                size_t w1in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;

    VerifyOrExit(w0in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(w1in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (my_identity_len > 0)
    {
        VerifyOrExit(my_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }
    if (peer_identity_len > 0)
    {
        VerifyOrExit(peer_identity != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    error = initPake(chip::Crypto::CHIP_SPAKE2P_ROLE::PROVER);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        SE05x_SPAKEDevice_A,
        kSE05x_CryptoObject_SPAKE_PROVER);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEInitDevice(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        kSE05x_CryptoObject_SPAKE_PROVER,
        (uint8_t*)spake_context,
        spake_context_len,
        (uint8_t*)my_identity,
        my_identity_len,
        (uint8_t*)peer_identity,
        peer_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#else
    smstatus = Se05x_API_PAKEConfigDevice(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        SE05x_SPAKEDevice_A,
        kSE05x_CryptoObject_SPAKE_PROVER,
        (uint8_t*)spake_context,
        spake_context_len,
        (uint8_t*)peer_identity,
        peer_identity_len,
        (uint8_t*)my_identity,
        my_identity_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);
#endif

    error = _setKey(w0in_id_p, w0in, w0in_len, kSSS_KeyPart_Default, kSSS_CipherType_AES);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

    error = _setKey(w1in_id_p, w1in, w1in_len, kSSS_KeyPart_Default, kSSS_CipherType_AES);
    VerifyOrExit(error == CHIP_NO_ERROR, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_02
    smstatus = Se05x_API_PAKEInitCredentials(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        kSE05x_CryptoObject_SPAKE_PROVER,
        w0in_id_p,
        w1in_id_p,
        0);
#else
    smstatus = Se05x_API_PAKEInitDevice(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        kSE05x_CryptoObject_SPAKE_PROVER,
        w0in_id_p,
        w1in_id_p,
        0);
#endif
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}
#endif


#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )
CHIP_ERROR Spake2p_ComputeRoundOne_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * pab, size_t pab_len,  uint8_t * out, size_t * out_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;

#if SSS_HAVE_SE05X_VER_GTE_16_03
#else
    uint8_t *prand = NULL;
    size_t prand_len = 0;
    uint8_t tempBuf[32] = {0,};
#endif
    SE05x_CryptoObjectID_t spakeObjectId = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? kSE05x_CryptoObject_SPAKE_VERIFIER : kSE05x_CryptoObject_SPAKE_PROVER;

    VerifyOrExit(out != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_len != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    if (pab_len > 0)
    {
        VerifyOrExit(pab != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    }

    if(role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) {
        if (pab == NULL) {
            /* Need X/Y value to verify abort condition */
            goto exit;
        }
    }

#if SSS_HAVE_SE05X_VER_GTE_16_03
#else
    /* Generate random numbers from SE */
    sss_status_t status = kStatus_SSS_Success;
    sss_rng_context_t rng_ctx;

    status = sss_rng_context_init(&rng_ctx, &gex_sss_chip_ctx.session);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_rng_get_random(&rng_ctx, tempBuf, sizeof(tempBuf));
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    sss_rng_context_free(&rng_ctx);

    prand = tempBuf;
    prand_len = sizeof(tempBuf);

#endif

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

#if SSS_HAVE_SE05X_VER_GTE_16_03
    smstatus = Se05x_API_PAKEComputeKeyShare(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        spakeObjectId,
        (uint8_t*)pab,
        pab_len,
        out,
        out_len);
#else
    smstatus = Se05x_API_PAKEComputeKeyShare(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        spakeObjectId,
        (uint8_t*)pab,
        pab_len,
        out,
        out_len,
        prand,
        prand_len);
#endif
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}

CHIP_ERROR Spake2p_ComputeRoundTwo_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * in, size_t in_len,
    uint8_t *out, size_t *out_len,
    uint8_t *pKeyKe, size_t *pkeyKeLen)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    smStatus_t smstatus = SM_NOT_OK;
    SE05x_CryptoObjectID_t spakeObjectId = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? kSE05x_CryptoObject_SPAKE_VERIFIER : kSE05x_CryptoObject_SPAKE_PROVER;
    const uint8_t * pab = NULL;
    size_t pab_len = 0;

    VerifyOrExit(in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(out_len != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pKeyKe != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(pkeyKeLen != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    pab = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? NULL : in;
    pab_len = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? 0 : in_len;

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEComputeSessionKeys(
        &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
        spakeObjectId,
        (uint8_t*)pab,
        pab_len,
        pKeyKe,
        pkeyKeLen,
        out,
        out_len);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = CHIP_NO_ERROR;
exit:
    return error;
}


CHIP_ERROR Spake2p_KeyConfirm_HSM(chip::Crypto::CHIP_SPAKE2P_ROLE role, const uint8_t * in, size_t in_len)
{
    CHIP_ERROR error = CHIP_ERROR_INTERNAL;
    SE05x_CryptoObjectID_t spakeObjectId = (role == chip::Crypto::CHIP_SPAKE2P_ROLE::VERIFIER) ? kSE05x_CryptoObject_SPAKE_VERIFIER : kSE05x_CryptoObject_SPAKE_PROVER;
    smStatus_t smstatus = SM_NOT_OK;
    uint8_t presult = 0;

    VerifyOrExit(in != nullptr, error = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(gex_sss_chip_ctx.ks.session != NULL, error = CHIP_ERROR_INTERNAL);

    smstatus = Se05x_API_PAKEVerifySessionKeys(
            &((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx,
            spakeObjectId,
            (uint8_t*)in,
            in_len,
            &presult);
    VerifyOrExit(smstatus == SM_OK, error = CHIP_ERROR_INTERNAL);

    error = (presult == 1) ?  CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
exit:
    return error;
}

#endif

/******** Static functions ***********************/

#if ((ENABLE_HSM_HASH_SHA256)    || \
    (ENABLE_HSM_HKDF_SHA256)     || \
    (ENABLE_HSM_AES_CCM_ENCRYPT) || \
    (ENABLE_HSM_AES_CCM_DECRYPT) || \
    (ENABLE_HSM_GENERATE_EC_KEY) || \
    (ENABLE_HSM_SPAKE_VERIFIER)  || \
    (ENABLE_HSM_SPAKE_PROVER)    || \
    (ENABLE_HSM_MAC) \
    )
static void se05x_sessionOpen()
{
    static int is_session_open = 0;
    sss_status_t status = kStatus_SSS_Fail;
    const char *portName;

    if (is_session_open) {
        return;
    }

    memset(&gex_sss_chip_ctx, 0, sizeof(gex_sss_chip_ctx));

    status = ex_sss_boot_connectstring(0, NULL, &portName);
    if (kStatus_SSS_Success != status) {
        ChipLogError(Crypto, "se05x error: %s\n", "ex_sss_boot_connectstring failed");
        goto exit;
    }

    status = ex_sss_boot_open(&gex_sss_chip_ctx, portName);
    if (kStatus_SSS_Success != status) {
        ChipLogError(Crypto, "se05x error: %s\n", "ex_sss_boot_open failed");
        goto exit;
    }

    status = ex_sss_key_store_and_object_init(&gex_sss_chip_ctx);
    if (kStatus_SSS_Success != status) {
        ChipLogError(Crypto, "se05x error: %s\n", "ex_sss_key_store_and_object_init failed");
        goto exit;
    }

    is_session_open = 1;

exit:
    return;
}
#endif


#if ((ENABLE_HSM_HKDF_SHA256)    || \
    (ENABLE_HSM_AES_CCM_ENCRYPT) || \
    (ENABLE_HSM_AES_CCM_DECRYPT) || \
    (ENABLE_HSM_GENERATE_EC_KEY) || \
    (ENABLE_HSM_MAC) \
    )
static int _getKeyId()
{
    int keyid =  0;
    smStatus_t smstatus = SM_NOT_OK;
    SE05x_Result_t exists = kSE05x_Result_NA;
    static int _key_offset = 1;

    se05x_sessionOpen();

    if(gex_sss_chip_ctx.ks.session == NULL) {
        return 0;
    }

generate_again:
    keyid = 0x1234 + _key_offset;
    _key_offset++;

    smstatus = Se05x_API_CheckObjectExists(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx, keyid, &exists);
    if (smstatus == SM_OK) {
        if (exists == kSE05x_Result_SUCCESS) {
            goto generate_again;
        }
        return keyid;
    }

    return 0;
}
#endif


#if ( (ENABLE_HSM_GENERATE_EC_KEY) || \
      (ENABLE_HSM_SPAKE_VERIFIER)  || \
      (ENABLE_HSM_SPAKE_PROVER)    || \
       (ENABLE_HSM_MAC) \
    )
static void delete_key(int keyid)
{
    smStatus_t smstatus = SM_NOT_OK;
    SE05x_Result_t exists = kSE05x_Result_NA;

    se05x_sessionOpen();

    if(gex_sss_chip_ctx.ks.session != NULL) {

        smstatus = Se05x_API_CheckObjectExists(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx, keyid, &exists);
        if (smstatus == SM_OK) {
            if (exists == kSE05x_Result_SUCCESS) {
                smstatus = Se05x_API_DeleteSecureObject(&((sss_se05x_session_t*)&gex_sss_chip_ctx.session)->s_ctx, keyid);
                if (smstatus != SM_OK) {
                    ChipLogError(Crypto, "se05x error: %s\n", "Error in deleting key");
                }
            }
            else {
                ChipLogError(Crypto, "se05x error: %s\n", "Key doesnot exists");
            }
        }
        else {
            ChipLogError(Crypto, "se05x error: %s\n", "Error in Se05x_API_CheckObjectExists");
        }
    }
}
#endif

#if ( (ENABLE_HSM_SPAKE_VERIFIER) || (ENABLE_HSM_SPAKE_PROVER) )
static CHIP_ERROR _setKey(int keyid, const uint8_t *key, size_t keylen, sss_key_part_t keyPart, sss_cipher_type_t cipherType)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    sss_status_t status = kStatus_SSS_Success;
    sss_object_t keyObject   = {0};
    const uint8_t keyBuf[128] = {0,};
    size_t keyBufLen = 0;
    uint8_t header1[] = {
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, \
        0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, \
        0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, \
        0x42, 0x00
    };
    size_t bitlen = 0;

    if (cipherType == kSSS_CipherType_EC_NIST_P)
    {
        VerifyOrExit(keylen < (sizeof(keyBuf) - sizeof(header1)), error = CHIP_ERROR_INTERNAL);

        memcpy((void*)keyBuf, (const uint8_t *) header1, sizeof(header1));
        keyBufLen = keyBufLen + sizeof(header1);

        memcpy((void*)(keyBuf + keyBufLen), key, keylen);
        keyBufLen = keyBufLen + keylen;

        bitlen = 256;
    }
    else {
        VerifyOrExit(keylen < sizeof(keyBuf), error = CHIP_ERROR_INTERNAL);

        memcpy((void*)keyBuf, (const uint8_t *)key, keylen);
        keyBufLen = keylen;
        bitlen = (size_t)keylen * 8;
    }

    status = sss_key_object_init(&keyObject, &gex_sss_chip_ctx.ks);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_object_allocate_handle(&keyObject,
        keyid,
        keyPart,
        cipherType,
        keyBufLen,
        kKeyObject_Mode_Persistent);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

    status = sss_key_store_set_key(&gex_sss_chip_ctx.ks, &keyObject, keyBuf, keyBufLen, bitlen, NULL, 0);
    VerifyOrExit(status == kStatus_SSS_Success, error = CHIP_ERROR_INTERNAL);

exit:
    return error;
}
#endif
