/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "mbedtls/ecjpake.h"

#if defined(MBEDTLS_ECJPAKE_ALT)
#include "ecjpake_alt.h"

#include <string.h>

#include "ti_drivers_config.h"

#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/cryptoutils/ecc/ECCParams.h>

/*
 * NOTE: On calling convention in this file.
 *
 * Many of the helper functions in this file take a pointer to a `uint8_t*`,
 * usually called `p`. This is the current working pointer for the input or
 * output buffer. They will also take a pointer to a `uint8_t`, usually called
 * `end`. This is the pointer to the end of the current working buffer. The
 * difference between these two pointers is calculated and used as the current
 * available length of the working buffer. This is checked before anything is
 * written to or read from the buffer. While values are read or written to the
 * data at the pointer pointed at by `p`, the pointer pointed at by `p` is
 * updated to point to the next available value. The callee updates the value
 * of `p` for the caller, almost in the same fashion as strtok.
 *
 * Here is an example:
 * ```
 * static int read_be_uint16(const uint8_t **p,
 *                           const uint8_t *end,
 *                           uint16_t      *value)
 * {
 *     if ((end < *p) || (end - *p < sizeof(uint16_t)))
 *     {
 *         return (MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);
 *     }
 *     *value = ((uint16_t)(*p)[0] << 8) | (*p)[1]);
 *     return (0);
 * }
 *
 * void sample_func(void)
 * {
 *     uint8_t  buffer[4] = {0x01, 0x02, 0x03, 0x04};
 *     uint8_t* p = buffer + sizeof(buffer);
 *     uint8_t* end = buffer + sizeof(buffer);
 *     uint16_t value;
 *
 *     read_be_uint16(&p, end, &value);
 *     // value is now 0x0102
 *     // p points to buffer[2]
 *     read_be_uint16(&p, end, &value);
 *     // value is now 0x0304
 *     // p points to buffer[4] (invalid)
 * }
 */

/*
 * Convert a mbedtls_ecjpake_role to identifier string.
 *
 * depends on the value of the enumeration of mbedtls_ecjpake_role.
 */
static const char * const ecjpake_id[] = { "client", "server" };

#define ID_MINE (ecjpake_id[ctx->role])
#define ID_PEER (ecjpake_id[1 - ctx->role])

/*
 * Size of the temporary buffer for ecjpake_hash:
 * 3 EC points plus their length, plus ID and its length (4 + 6 bytes)
 */
#define ECJPAKE_HASH_BUF_LEN (3 * (4 + MBEDTLS_ECP_MAX_PT_LEN) + 4 + 6)

/**
 * \breif           execute the contents of this macro, save the return in
 *                  `ret` and goto `cleanup` if it is not `0`
 *
 * The internal functions of this file driver are designed to return `0` on
 * success. The caller is expected to have an `int ret` in the translation unit
 * of the call-site, and a target `cleanup`. It is common practice to return
 * `ret` in the caller.
 *
 * ```
 * int example_func(void)
 * {
 *     int ret;
 *     ECJPAKE_ALT_CHK(some_func());
 *
 *     some_other_func();
 *
 * cleanup:
 *     return ret;
 * }
 * ```
 *
 * \param f         code to execute
 */
#define ECJPAKE_ALT_CHK(f)                                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        if ((ret = f) != 0)                                                                                                        \
            goto cleanup;                                                                                                          \
    } while (0)

/**
 * \brief           writes a buffer prepended with a length byte
 *
 * \param p         pointer to output buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the output buffer
 * \param key       the CryptoKey to write
 * \param bin       the source to copy
 * \param len       the length to copy and write
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 *
 * \retval MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL
 *                  the input buffer is too small
 */
static int tls_write_binary(uint8_t ** p, const uint8_t * end, const uint8_t * bin, size_t len)
{
    if ((end < *p) || ((size_t)(end - *p) < 1 + len))
    {
        return (MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);
    }

    /* write the length to the buffer */
    (*p)[0] = len;
    *p += 1;

    /* write the binary data as is */
    memcpy(*p, bin, len);
    *p += len;

    return (0);
}

/**
 * \brief           read a binary value prepended by a length bit
 *
 * \param p         pointer to input buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the input buffer
 * \param bin       the array to fill
 * \param len       the size of \p bin
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 *
 * \retval MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL
 *                  the input buffer is too small
 */
static int tls_read_binary(const uint8_t ** p, const uint8_t * end, uint8_t * bin, size_t len)
{
    uint8_t data_len;

    /* length byte plus the length of the crypto key */
    if ((end < *p) || ((size_t)(end - *p) < 1 + len))
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* read the length byte */
    data_len = (*p)[0];
    *p += 1;
    /* check the length matches */
    if (data_len != len)
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    memcpy(bin, *p, len);

    *p += len;

    return (0);
}

/**
 * \brief           writes a tls point into a CryptoKey
 *
 * \param p         pointer to output buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the output buffer
 * \param key       the CryptoKey to write
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 */
static int tls_write_crypto_key(uint8_t ** p, const uint8_t * end, CryptoKey * key)
{
    return tls_write_binary(p, end, key->u.plaintext.keyMaterial, key->u.plaintext.keyLength);
}

/**
 * \brief           read a tls point into a CryptoKey
 *
 * \param p         pointer to input buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the input buffer
 * \param key       the CryptoKey to fill
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 *
 * \retval MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL
 *                  the input buffer is too small
 * \retval MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE
 *                  the point is not in uncompressed form
 */
static int tls_read_crypto_key(const uint8_t ** p, const uint8_t * end, CryptoKey * key)
{
    /* check that the point is uncompressed */
    if ((end < *p) || (end - *p < 2))
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }
    if ((*p)[1] != 0x04)
    {
        return (MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE);
    }

    return tls_read_binary(p, end, key->u.plaintext.keyMaterial, key->u.plaintext.keyLength);
}

/**
 * \brief           write the curve info into the buffer
 *
 * \param p         pointer to output buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the output buffer
 * \param group_id  the mbedtls group id write
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 *
 * \retval MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL
 *                  the input buffer is too small
 * \retval MBEDTLS_ERR_ECP_BAD_INPUT_DATA
 *                  the group id did not name a valid curve
 */
static int tls_write_curve_info(uint8_t ** p, const uint8_t * end, mbedtls_ecp_group_id group_id)
{
    const mbedtls_ecp_curve_info * curve_info;

    if ((end < *p) || (end - *p < 3))
    {
        return (MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);
    }

    curve_info = mbedtls_ecp_curve_info_from_grp_id(group_id);

    if (NULL == curve_info)
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* write that we use named curves */
    (*p)[0] = MBEDTLS_ECP_TLS_NAMED_CURVE;
    (*p)[1] = curve_info->tls_id >> 8;
    (*p)[2] = curve_info->tls_id & 0xFF;
    *p += 3;

    return (0);
}

/**
 * \brief           read and verify the curve info from the buffer
 *
 * \param p         pointer to input buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the input buffer
 * \param group_id  the mbedtls group id to load to check a match
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 *
 * \retval MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL
 *                  the input buffer is too small
 * \retval MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE
 *                  the tls curve identifiers do not match
 * \retval MBEDTLS_ERR_ECP_BAD_INPUT_DATA
 *                  the curve info was not for a named curve or we do not have
 *                  that named curve
 */
static int tls_verify_curve_info(const uint8_t ** p, const uint8_t * end, mbedtls_ecp_group_id group_id)
{
    uint16_t curve_name_id;
    const mbedtls_ecp_curve_info * curve_info;

    if ((end < *p) || (end - *p < 3))
    {
        return (MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);
    }

    if ((*p)[0] != MBEDTLS_ECP_TLS_NAMED_CURVE)
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }
    curve_name_id = (((uint16_t)(*p)[1] << 8)) | (((uint16_t)(*p)[2]));
    *p += 3;

    curve_info = mbedtls_ecp_curve_info_from_grp_id(group_id);

    if (NULL == curve_info)
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    if (curve_name_id != curve_info->tls_id)
    {
        return (MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE);
    }

    return (0);
}

/**
 * \brief           Fill the a point with a random function
 *
 * \param private_key
 *                  key to fill with random data
 * \param f_rng     random number function
 * \param p_rng     context for the random function
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 */
static int gen_private_key(unsigned char * private_key, int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    unsigned int i;
    unsigned int j   = 0;
    unsigned int sum = 0U;

    do
    {
        unsigned int * pkey = (unsigned int *) private_key;
        f_rng(p_rng, private_key, NISTP256_CURVE_LENGTH_BYTES);

        /* check if private_key is equal to zero */
        for (i = 0; i < (NISTP256_CURVE_LENGTH_BYTES / sizeof(unsigned int)); i++)
        {
            sum |= pkey[i];
        }
        j++;
    } while (0U == sum && (j < 3));

    return (0U != sum ? 0 : -1);
}

/**
 * \brief           Generate the second round of keys for this node and peer
 *
 * \param ctx       Context to use
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 */
static int ecjpake_alt_generate_round2_keys(mbedtls_ecjpake_context * ctx)
{
    ECJPAKE_OperationRoundTwoGenerateKeys roundTwoGenerateKeys;
    int ret;

    if (ctx->roundTwoGenerated)
    {
        return (0);
    }

    ECJPAKE_OperationRoundTwoGenerateKeys_init(&roundTwoGenerateKeys);
    roundTwoGenerateKeys.curve                = &ECCParams_NISTP256;
    roundTwoGenerateKeys.myPrivateKey2        = &ctx->myPrivateCryptoKey2;
    roundTwoGenerateKeys.myPublicKey1         = &ctx->myPublicCryptoKey1;
    roundTwoGenerateKeys.myPublicKey2         = &ctx->myPublicCryptoKey2;
    roundTwoGenerateKeys.theirPublicKey1      = &ctx->theirPublicCryptoKey1;
    roundTwoGenerateKeys.theirPublicKey2      = &ctx->theirPublicCryptoKey2;
    roundTwoGenerateKeys.preSharedSecret      = &ctx->preSharedSecretCryptoKey;
    roundTwoGenerateKeys.theirNewGenerator    = &ctx->theirGeneratorKey;
    roundTwoGenerateKeys.myNewGenerator       = &ctx->myGeneratorKey;
    roundTwoGenerateKeys.myCombinedPrivateKey = &ctx->myCombinedPrivateKey;
    roundTwoGenerateKeys.myCombinedPublicKey  = &ctx->myCombinedPublicKey;
    roundTwoGenerateKeys.myPrivateV           = &ctx->myPrivateCryptoV3;
    roundTwoGenerateKeys.myPublicV            = &ctx->myPublicCryptoV3;

    ECJPAKE_ALT_CHK(ECJPAKE_roundTwoGenerateKeys(ctx->handle, &roundTwoGenerateKeys));

    ctx->roundTwoGenerated = true;

cleanup:
    return (ret);
}

void mbedtls_ecjpake_init(mbedtls_ecjpake_context * ctx)
{
    ECJPAKE_Params params;

    if (ctx == NULL)
    {
        return;
    }

    ctx->md_info      = NULL;
    ctx->point_format = MBEDTLS_ECP_PF_UNCOMPRESSED;

    ECJPAKE_Params_init(&params);

    ctx->handle = ECJPAKE_open(0, &params);
}

void mbedtls_ecjpake_free(mbedtls_ecjpake_context * ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    ctx->md_info = NULL;

    ECJPAKE_close(ctx->handle);
    ctx->handle = NULL;
}

static void big_num_reverse(uint8_t * arr, size_t len)
{
    unsigned int left  = 0;
    unsigned int right = len - 1;
    uint8_t temp;

    while (left < right)
    {
        temp       = arr[left];
        arr[left]  = arr[right];
        arr[right] = temp;

        ++left;
        --right;
    }
}

/**
 * \brief           write a 4 byte length and binary output
 *
 * \param p         pointer to output buffer, will be changed by a successful
 *                  call
 * \param end       pointer to the end of the output buffer
 * \param bin       value to write
 * \param len       length to write
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 */
static int ecjpake_write_len_binary(uint8_t ** p, const uint8_t * end, const uint8_t * bin, size_t len)
{
    if ((end < *p) || ((size_t)(end - *p) < 4 + len))
    {
        return (MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);
    }

    memcpy(*p + 4, bin, len);

    (*p)[0] = (unsigned char) ((len >> 24) & 0xFF);
    (*p)[1] = (unsigned char) ((len >> 16) & 0xFF);
    (*p)[2] = (unsigned char) ((len >> 8) & 0xFF);
    (*p)[3] = (unsigned char) ((len) &0xFF);

    *p += 4 + len;

    return (0);
}

/**
 * \brief           Create the EC-JPAKE hash for each round
 *
 * \param md_info   message digest info to use
 * \param G         1st point to digest
 * \param V         2nd point to digest
 * \param X         3rd point to digest
 * \param id        string id to digest
 * \param hash      output buffer for digest
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 */
static int ecjpake_hash(const mbedtls_md_info_t * md_info, CryptoKey * G, CryptoKey * V, CryptoKey * X, const char * id,
                        uint8_t * hash)
{
    int ret;
    uint8_t buf[ECJPAKE_HASH_BUF_LEN];
    uint8_t * p         = buf;
    const uint8_t * end = buf + sizeof(buf);
    const size_t id_len = strlen(id);

    /*
     * Write 'lenG || G || lenV || V || lenX || X || lenID || ID' to the
     * temporary buffer
     */
    ECJPAKE_ALT_CHK(ecjpake_write_len_binary(&p, end, G->u.plaintext.keyMaterial, G->u.plaintext.keyLength));
    ECJPAKE_ALT_CHK(ecjpake_write_len_binary(&p, end, V->u.plaintext.keyMaterial, V->u.plaintext.keyLength));
    ECJPAKE_ALT_CHK(ecjpake_write_len_binary(&p, end, X->u.plaintext.keyMaterial, X->u.plaintext.keyLength));
    ECJPAKE_ALT_CHK(ecjpake_write_len_binary(&p, end, (uint8_t *) id, id_len));

    /*
     * Compute hash
     *
     * XXX: possible speedup by digesting the buffers directly instead of
     * creating a temp buffer
     */
    ECJPAKE_ALT_CHK(mbedtls_md(md_info, buf, p - buf, hash));

cleanup:
    return (ret);
}

/**
 * \brief           Read and validate the ZKP based on this public key.
 *
 * \param ctx       Context to use
 * \param generator_key
 *                  Generator point to use
 * \param public_key
 *                  Public key use
 * \param p         Pointer to current place in buffer, will point to the next
 *                  character after a successful call
 * \param end       Pointer to the end of the buffer
 *
 * \return          0 if successful,
 *                  a negative error code otherwise
 */
static int ecjpake_zkp_read(mbedtls_ecjpake_context * ctx, CryptoKey * generator_key, CryptoKey * public_key, const uint8_t ** p,
                            const uint8_t * end)
{
    int ret;
    CryptoKey v;
    uint8_t v_material[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    uint8_t r[NISTP256_CURVE_LENGTH_BYTES];
    uint8_t hash[NISTP256_CURVE_LENGTH_BYTES];

    ECJPAKE_OperationVerifyZKP operation_verify_zkp;

    CryptoKeyPlaintext_initKey(&v, v_material, sizeof(v_material));

    /* read the Ephemeral public key V */
    ECJPAKE_ALT_CHK(tls_read_crypto_key(p, end, &v));

    /* read the Schnorr signature r */
    ECJPAKE_ALT_CHK(tls_read_binary(p, end, r, sizeof(r)));

    /* calculate the hash */
    ECJPAKE_ALT_CHK(ecjpake_hash(ctx->md_info, generator_key, &v, public_key, ID_PEER, hash));

    /* verify the r and hash based on V */
    ECJPAKE_OperationVerifyZKP_init(&operation_verify_zkp);
    operation_verify_zkp.curve          = &ECCParams_NISTP256;
    operation_verify_zkp.theirGenerator = generator_key;
    operation_verify_zkp.theirPublicKey = public_key;
    operation_verify_zkp.theirPublicV   = &v;
    operation_verify_zkp.hash           = hash;
    operation_verify_zkp.r              = r;

    ECJPAKE_ALT_CHK(ECJPAKE_verifyZKP(ctx->handle, &operation_verify_zkp));

cleanup:
    return (ret);
}

/**
 * \brief           Check if a CryptoKey is zero
 *
 * \param G         CryptoKey to use
 *
 * \return          0 CryptoKey is not zero
 *                  MBEDTLS_ERR_ECP_INVALID_KEY if CryptoKey is zero
 */
static int check_CryptoKey_is_zero(CryptoKey * G)
{
    unsigned int i;
    /* skip beginning 0x04 byte */
    for (i = 1; i < G->u.plaintext.keyLength; i++)
    {
        if (G->u.plaintext.keyMaterial[i] != 0U)
        {
            /* the point is not all zero */
            return (0);
        }
    }
    /* the for loop completed, they were all zero */
    return (MBEDTLS_ERR_ECP_INVALID_KEY);
}

int mbedtls_ecjpake_setup(mbedtls_ecjpake_context * ctx, mbedtls_ecjpake_role role, mbedtls_md_type_t hash,
                          mbedtls_ecp_group_id curve, const uint8_t * secret, size_t len)
{
    int ret = 0;

    ctx->roundTwoGenerated = false;
    ctx->role              = role;

    if ((ctx->md_info = mbedtls_md_info_from_type(hash)) == NULL)
    {
        return (MBEDTLS_ERR_MD_FEATURE_UNAVAILABLE);
    }

    ctx->curve = curve;

    /* copy NISTP256 generator for hash functions */
    ctx->nistP256Generator[0] = 0x04;
    memcpy(&(ctx->nistP256Generator[1]), ECCParams_NISTP256.generatorX, (NISTP256_CURVE_LENGTH_BYTES * 2));
    big_num_reverse(&(ctx->nistP256Generator[1]), NISTP256_CURVE_LENGTH_BYTES);
    big_num_reverse(&(ctx->nistP256Generator[NISTP256_CURVE_LENGTH_BYTES + 1]), NISTP256_CURVE_LENGTH_BYTES);
    CryptoKeyPlaintext_initKey(&ctx->nistP256GeneratorCryptoKey, ctx->nistP256Generator, sizeof(ctx->nistP256Generator));

    /* Pre-shared secret */
    memcpy(ctx->preSharedSecretKeyingMaterial, secret, len);

    CryptoKeyPlaintext_initKey(&ctx->preSharedSecretCryptoKey, ctx->preSharedSecretKeyingMaterial, len);

    CryptoKeyPlaintext_initKey(&ctx->myPrivateCryptoKey1, ctx->myPrivateKeyMaterial1, sizeof(ctx->myPrivateKeyMaterial1));
    CryptoKeyPlaintext_initKey(&ctx->myPrivateCryptoKey2, ctx->myPrivateKeyMaterial2, sizeof(ctx->myPrivateKeyMaterial2));
    CryptoKeyPlaintext_initKey(&ctx->myPrivateCryptoV1, ctx->myPrivateVMaterial1, sizeof(ctx->myPrivateVMaterial1));
    CryptoKeyPlaintext_initKey(&ctx->myPrivateCryptoV2, ctx->myPrivateVMaterial2, sizeof(ctx->myPrivateVMaterial2));
    CryptoKeyPlaintext_initKey(&ctx->myPrivateCryptoV3, ctx->myPrivateVMaterial3, sizeof(ctx->myPrivateVMaterial3));

    CryptoKeyPlaintext_initBlankKey(&ctx->myPublicCryptoKey1, ctx->myPublicKeyMaterial1, sizeof(ctx->myPublicKeyMaterial1));
    CryptoKeyPlaintext_initBlankKey(&ctx->myPublicCryptoKey2, ctx->myPublicKeyMaterial2, sizeof(ctx->myPublicKeyMaterial2));
    CryptoKeyPlaintext_initBlankKey(&ctx->myPublicCryptoV1, ctx->myPublicVMaterial1, sizeof(ctx->myPublicVMaterial1));
    CryptoKeyPlaintext_initBlankKey(&ctx->myPublicCryptoV2, ctx->myPublicVMaterial2, sizeof(ctx->myPublicVMaterial2));
    CryptoKeyPlaintext_initBlankKey(&ctx->myPublicCryptoV3, ctx->myPublicVMaterial3, sizeof(ctx->myPublicVMaterial3));
    CryptoKeyPlaintext_initBlankKey(&ctx->myCombinedPrivateKey, ctx->myCombinedPrivateKeyMaterial1,
                                    sizeof(ctx->myCombinedPrivateKeyMaterial1));
    CryptoKeyPlaintext_initBlankKey(&ctx->myCombinedPublicKey, ctx->myCombinedPublicKeyMaterial1,
                                    sizeof(ctx->myCombinedPublicKeyMaterial1));
    CryptoKeyPlaintext_initBlankKey(&ctx->myGeneratorKey, ctx->myGenerator, sizeof(ctx->myGenerator));

    CryptoKeyPlaintext_initBlankKey(&ctx->theirPublicCryptoKey1, ctx->theirPublicKeyMaterial1,
                                    sizeof(ctx->theirPublicKeyMaterial1));
    CryptoKeyPlaintext_initBlankKey(&ctx->theirPublicCryptoKey2, ctx->theirPublicKeyMaterial2,
                                    sizeof(ctx->theirPublicKeyMaterial2));
    CryptoKeyPlaintext_initBlankKey(&ctx->theirCombinedPublicKey, ctx->theirCombinedPublicKeyMaterial1,
                                    sizeof(ctx->theirCombinedPublicKeyMaterial1));
    CryptoKeyPlaintext_initBlankKey(&ctx->theirGeneratorKey, ctx->theirGenerator, sizeof(ctx->theirGenerator));

    return (ret);
}

int mbedtls_ecjpake_check(const mbedtls_ecjpake_context * ctx)
{
    if (ctx->md_info == NULL || ctx->curve == MBEDTLS_ECP_DP_NONE || ctx->handle == NULL)
    {
        return (MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    return (0);
}

int mbedtls_ecjpake_read_round_one(mbedtls_ecjpake_context * ctx, const unsigned char * buf, size_t len)
{
    int ret;
    const unsigned char * p   = buf;
    const unsigned char * end = buf + len;

    /* read their combined key material (Xc or Xs) */
    ECJPAKE_ALT_CHK(tls_read_crypto_key(&p, end, &ctx->theirPublicCryptoKey1));

    /* verify that the point is not zero */
    ECJPAKE_ALT_CHK(check_CryptoKey_is_zero(&ctx->theirPublicCryptoKey1));

    /* verify the proof (ZKP(Xc) or ZKP(Xs)) */
    ECJPAKE_ALT_CHK(ecjpake_zkp_read(ctx, &ctx->nistP256GeneratorCryptoKey, &ctx->theirPublicCryptoKey1, &p, end));

    /* read their combined key material (Xc or Xs) */
    ECJPAKE_ALT_CHK(tls_read_crypto_key(&p, end, &ctx->theirPublicCryptoKey2));

    /* verify that the point is not zero */
    ECJPAKE_ALT_CHK(check_CryptoKey_is_zero(&ctx->theirPublicCryptoKey2));

    /* verify the proof (ZKP(Xc) or ZKP(Xs)) */
    ECJPAKE_ALT_CHK(ecjpake_zkp_read(ctx, &ctx->nistP256GeneratorCryptoKey, &ctx->theirPublicCryptoKey2, &p, end));

cleanup:
    return (ret);
}

int mbedtls_ecjpake_write_round_one(mbedtls_ecjpake_context * ctx, uint8_t * buf, size_t len, size_t * olen,
                                    int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    uint8_t * p         = buf;
    const uint8_t * end = buf + len;
    uint8_t hash[NISTP256_CURVE_LENGTH_BYTES];
    uint8_t r[NISTP256_CURVE_LENGTH_BYTES];

    /* Generate round one keys */
    ECJPAKE_OperationRoundOneGenerateKeys roundOneGenerateKeys;
    ECJPAKE_OperationGenerateZKP operationGenerateZKP;

    /* Generate private keys */
    ECJPAKE_ALT_CHK(gen_private_key(ctx->myPrivateKeyMaterial1, f_rng, p_rng));
    ECJPAKE_ALT_CHK(gen_private_key(ctx->myPrivateKeyMaterial2, f_rng, p_rng));
    ECJPAKE_ALT_CHK(gen_private_key(ctx->myPrivateVMaterial1, f_rng, p_rng));
    ECJPAKE_ALT_CHK(gen_private_key(ctx->myPrivateVMaterial2, f_rng, p_rng));
    ECJPAKE_ALT_CHK(gen_private_key(ctx->myPrivateVMaterial3, f_rng, p_rng));

    ECJPAKE_OperationRoundOneGenerateKeys_init(&roundOneGenerateKeys);
    roundOneGenerateKeys.curve         = &ECCParams_NISTP256;
    roundOneGenerateKeys.myPrivateKey1 = &ctx->myPrivateCryptoKey1;
    roundOneGenerateKeys.myPrivateKey2 = &ctx->myPrivateCryptoKey2;
    roundOneGenerateKeys.myPublicKey1  = &ctx->myPublicCryptoKey1;
    roundOneGenerateKeys.myPublicKey2  = &ctx->myPublicCryptoKey2;
    roundOneGenerateKeys.myPrivateV1   = &ctx->myPrivateCryptoV1;
    roundOneGenerateKeys.myPrivateV2   = &ctx->myPrivateCryptoV2;
    roundOneGenerateKeys.myPublicV1    = &ctx->myPublicCryptoV1;
    roundOneGenerateKeys.myPublicV2    = &ctx->myPublicCryptoV2;

    ECJPAKE_ALT_CHK(ECJPAKE_roundOneGenerateKeys(ctx->handle, &roundOneGenerateKeys));

    /* write X1 */
    ECJPAKE_ALT_CHK(tls_write_crypto_key(&p, end, &ctx->myPublicCryptoKey1));

    ECJPAKE_ALT_CHK(ecjpake_hash(ctx->md_info, &ctx->nistP256GeneratorCryptoKey, &ctx->myPublicCryptoV1, &ctx->myPublicCryptoKey1,
                                 ID_MINE, hash));

    /* generate round one ZKPs */
    ECJPAKE_OperationGenerateZKP_init(&operationGenerateZKP);
    operationGenerateZKP.curve        = &ECCParams_NISTP256;
    operationGenerateZKP.myPrivateKey = &ctx->myPrivateCryptoKey1;
    operationGenerateZKP.myPrivateV   = &ctx->myPrivateCryptoV1;
    operationGenerateZKP.hash         = hash;
    operationGenerateZKP.r            = r;

    ECJPAKE_ALT_CHK(ECJPAKE_generateZKP(ctx->handle, &operationGenerateZKP));

    /* write ZKP for X1 (V1 and r1) */
    ECJPAKE_ALT_CHK(tls_write_crypto_key(&p, end, &ctx->myPublicCryptoV1));

    /* write r */
    ECJPAKE_ALT_CHK(tls_write_binary(&p, end, r, NISTP256_CURVE_LENGTH_BYTES));

    ECJPAKE_ALT_CHK(ecjpake_hash(ctx->md_info, &ctx->nistP256GeneratorCryptoKey, &ctx->myPublicCryptoV2, &ctx->myPublicCryptoKey2,
                                 ID_MINE, hash));

    ECJPAKE_OperationGenerateZKP_init(&operationGenerateZKP);
    operationGenerateZKP.curve        = &ECCParams_NISTP256;
    operationGenerateZKP.myPrivateKey = &ctx->myPrivateCryptoKey2;
    operationGenerateZKP.myPrivateV   = &ctx->myPrivateCryptoV2;
    operationGenerateZKP.hash         = hash;
    operationGenerateZKP.r            = r;

    ECJPAKE_ALT_CHK(ECJPAKE_generateZKP(ctx->handle, &operationGenerateZKP));

    /* write X2 */
    ECJPAKE_ALT_CHK(tls_write_crypto_key(&p, end, &ctx->myPublicCryptoKey2));

    /* write ZKP for X2 */
    ECJPAKE_ALT_CHK(tls_write_crypto_key(&p, end, &ctx->myPublicCryptoV2));

    /* write r */
    ECJPAKE_ALT_CHK(tls_write_binary(&p, end, r, NISTP256_CURVE_LENGTH_BYTES));

    *olen = p - buf;

cleanup:
    return (ret);
}

int mbedtls_ecjpake_read_round_two(mbedtls_ecjpake_context * ctx, const unsigned char * buf, size_t len)
{
    int ret;
    const uint8_t * p   = buf;
    const uint8_t * end = buf + len;

    ECJPAKE_ALT_CHK(ecjpake_alt_generate_round2_keys(ctx));

    if (ctx->role == MBEDTLS_ECJPAKE_CLIENT)
    {
        ECJPAKE_ALT_CHK(tls_verify_curve_info(&p, end, ctx->curve));
    }

    /* read their combined key material (Xc or Xs) */
    ECJPAKE_ALT_CHK(tls_read_crypto_key(&p, end, &ctx->theirCombinedPublicKey));

    /* verify that the point is not zero */
    ECJPAKE_ALT_CHK(check_CryptoKey_is_zero(&ctx->theirCombinedPublicKey));

    /* verify the proof (ZKP(Xc) or ZKP(Xs)) */
    ECJPAKE_ALT_CHK(ecjpake_zkp_read(ctx, &ctx->theirGeneratorKey, &ctx->theirCombinedPublicKey, &p, end));

cleanup:
    return (ret);
}

int mbedtls_ecjpake_write_round_two(mbedtls_ecjpake_context * ctx, unsigned char * buf, size_t len, size_t * olen,
                                    int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    uint8_t hash[NISTP256_CURVE_LENGTH_BYTES];
    uint8_t r[NISTP256_CURVE_LENGTH_BYTES];
    uint8_t * p         = buf;
    const uint8_t * end = buf + len;

    ECJPAKE_OperationGenerateZKP operationGenerateZKP;

    ECJPAKE_ALT_CHK(ecjpake_alt_generate_round2_keys(ctx));

    ECJPAKE_ALT_CHK(
        ecjpake_hash(ctx->md_info, &ctx->myGeneratorKey, &ctx->myPublicCryptoV3, &ctx->myCombinedPublicKey, ID_MINE, hash));

    ECJPAKE_OperationGenerateZKP_init(&operationGenerateZKP);
    operationGenerateZKP.curve        = &ECCParams_NISTP256;
    operationGenerateZKP.myPrivateKey = &ctx->myCombinedPrivateKey;
    operationGenerateZKP.myPrivateV   = &ctx->myPrivateCryptoV3;
    operationGenerateZKP.hash         = hash;
    operationGenerateZKP.r            = r;

    ECJPAKE_ALT_CHK(ECJPAKE_generateZKP(ctx->handle, &operationGenerateZKP));

    if (ctx->role == MBEDTLS_ECJPAKE_SERVER)
    {
        /* write curve info */
        ECJPAKE_ALT_CHK(tls_write_curve_info(&p, end, ctx->curve));
    }

    /* write public key X */
    ECJPAKE_ALT_CHK(tls_write_crypto_key(&p, end, &ctx->myCombinedPublicKey));

    /* write ZKP for X (V and r) */
    ECJPAKE_ALT_CHK(tls_write_crypto_key(&p, end, &ctx->myPublicCryptoV3));

    /* write r */
    ECJPAKE_ALT_CHK(tls_write_binary(&p, end, r, NISTP256_CURVE_LENGTH_BYTES));

    *olen = p - buf;

cleanup:
    return (ret);
}

int mbedtls_ecjpake_derive_secret(mbedtls_ecjpake_context * ctx, unsigned char * buf, size_t len, size_t * olen,
                                  int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    unsigned char md_len;
    CryptoKey sharedSecretCryptoKey;
    uint8_t sharedSecretKeyingMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];

    ECJPAKE_OperationComputeSharedSecret computeSharedSecret;

    md_len = mbedtls_md_get_size(ctx->md_info);
    if (len < md_len)
    {
        return (MBEDTLS_ERR_ECP_BUFFER_TOO_SMALL);
    }

    CryptoKeyPlaintext_initKey(&sharedSecretCryptoKey, sharedSecretKeyingMaterial1, sizeof(sharedSecretKeyingMaterial1));

    /* Generate shared secret */
    ECJPAKE_OperationComputeSharedSecret_init(&computeSharedSecret);
    computeSharedSecret.curve                  = &ECCParams_NISTP256;
    computeSharedSecret.myCombinedPrivateKey   = &ctx->myCombinedPrivateKey;
    computeSharedSecret.theirCombinedPublicKey = &ctx->theirCombinedPublicKey;
    computeSharedSecret.theirPublicKey2        = &ctx->theirPublicCryptoKey2;
    computeSharedSecret.myPrivateKey2          = &ctx->myPrivateCryptoKey2;
    computeSharedSecret.sharedSecret           = &sharedSecretCryptoKey;

    ECJPAKE_ALT_CHK(ECJPAKE_computeSharedSecret(ctx->handle, &computeSharedSecret));

    ECJPAKE_ALT_CHK(mbedtls_md(ctx->md_info, sharedSecretKeyingMaterial1 + 1, NISTP256_CURVE_LENGTH_BYTES, buf));

    *olen = md_len;

cleanup:
    return (ret);
}

#undef ID_MINE
#undef ID_PEER

#endif /* MBEDTLS_ECJPAKE_ALT */
