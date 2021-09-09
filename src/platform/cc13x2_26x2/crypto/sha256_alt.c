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

#include <mbedtls/sha256.h>

#if defined(MBEDTLS_SHA256_ALT)

#include <assert.h>
#include <string.h>

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>

/*!
 *  @brief Hardware-specific configuration attributes
 *
 *  SHA2CC26X2 hardware attributes are used by the SHA2_Config struct.
 */
const SHA2CC26X2_HWAttrs sha2CC26X2HWAttrs_sha = {
    .intPriority = ~0,
};

/**
 * \brief          Initialize SHA-256 context
 *
 * \param ctx      SHA-256 context to be initialized
 */
void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    SHA2_Params sha2Params;

    memset(ctx, 0, sizeof(mbedtls_sha256_context));
    SHA2_Params_init(&sha2Params);

    sha2Params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;

    ctx->config.object  = &ctx->object;
    ctx->config.hwAttrs = &sha2CC26X2HWAttrs_sha;

    ctx->hndl = SHA2_construct(&ctx->config, &sha2Params);
}

/**
 * \brief          Clear SHA-256 context
 *
 * \param ctx      SHA-256 context to be cleared
 */
void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (NULL == ctx)
    {
        return;
    }

    if (ctx->hndl != NULL)
    {
        SHA2_close(ctx->hndl);

        memset(ctx, 0, sizeof(mbedtls_sha256_context));
    }
}

/**
 * \brief          SHA-256 context setup
 *
 * \param ctx      context to be initialized
 * \param is224    0 = use SHA256, 1 = use SHA224
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED on failure to open driver
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    SHA2_HashType type;

    // open the driver here because no-one called init
    if (ctx->hndl == NULL)
    {
        SHA2_Params sha2Params;

        SHA2_Params_init(&sha2Params);

        sha2Params.returnBehavior = SHA2_RETURN_BEHAVIOR_POLLING;

        ctx->config.object  = &ctx->object;
        ctx->config.hwAttrs = &sha2CC26X2HWAttrs_sha;

        ctx->hndl = SHA2_construct(&ctx->config, &sha2Params);

        if (NULL == ctx->hndl)
        {
            return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
        }
    }
    else
    {
        SHA2_reset(ctx->hndl);
    }

    if (is224)
    {
        type = SHA2_HASH_TYPE_224;
    }
    else
    {
        type = SHA2_HASH_TYPE_256;
    }

    SHA2_setHashType(ctx->hndl, type);

    return 0;
}

/**
 * \brief          Clone (the state of) a SHA-256 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 *
 * \note This does not make any assumption on the state of the context being
 * passed into it. The destination context should be initialized before this
 * function is called. However, it has been noted that some uses of this
 * function do not initialize the context to avoid allocating hardware
 * resources. While this is undefined behavior, only internal buffers to the
 * SHA2 structure are copied.
 */
void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    dst->object.hashType       = src->object.hashType;
    dst->object.bytesInBuffer  = src->object.bytesInBuffer;
    dst->object.bytesProcessed = src->object.bytesProcessed;
    memcpy(dst->object.digest, src->object.digest, sizeof(dst->object.digest));
    memcpy(dst->object.hmacDigest, src->object.hmacDigest, sizeof(dst->object.hmacDigest));
    memcpy(dst->object.buffer, src->object.buffer, sizeof(dst->object.buffer));
}

/**
 * \brief          SHA-256 final digest
 *
 * \param ctx      SHA-256 context
 * \param output   SHA-224/256 checksum result
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    int_fast16_t result;

    result = SHA2_finalize(ctx->hndl, output);

    if (SHA2_STATUS_SUCCESS != result)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

/**
 * \brief          SHA-256 process buffer
 *
 * \param ctx      SHA-256 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    int_fast16_t result;

    // Process data in chunks. The driver buffers incomplete blocks internally.
    result = SHA2_addData(ctx->hndl, input, ilen);

    if (SHA2_STATUS_SUCCESS != result)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

/**
 * \brief          SHA-256 start/intermediate blocks
 *
 * \param ctx      SHA-256 context
 * \param data     64-byte input data block
 *
 * \retval         0                                  on success
 * \retval         MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED on driver failure
 */
int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    int_fast16_t result;

    // Process data in chunks. The driver buffers incomplete blocks internally.
    result = SHA2_addData(ctx->hndl, data, SHA2_BLOCK_SIZE_BYTES_256);

    if (SHA2_STATUS_SUCCESS != result)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    else
    {
        return 0;
    }
}

#endif /* MBEDTLS_SHA256_ALT */
