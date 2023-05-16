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

#include <string.h>

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>

/*!
 *  Hardware-specific configuration attributes, may be replaced with SysConfig
 *  ti_drivers.h structure.
 */
const SHA2CC26X2_HWAttrs sha2CC26X2HWAttrs_sha = {
    .intPriority = ~0,
};

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

/*
 * Deviation from documentation in sha256.h.
 *
 * It has been noticed that some code using mbedtls does not call `init` before
 * starting a SHA operation. Rather these implementations assume the `ctx`
 * structure may be zero'ed out as initialization. To support this usage the
 * driver instance is assumed to be uninitialized if the context's `hndl` is a
 * NULL pointer. Start will initialize the driver and assign the handle to the
 * context in this case.
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
 * Deviation from documentation in sha256.h.
 *
 * It has been noticed that some code using mbedtls does not call `init` on the
 * contexts being passed into this function. This seems to be done to avoid
 * allocating hardware to cloned context instances. To support this use-case,
 * only internal buffers to the SHA2 structure are copied without checking the
 * driver handles.
 */
void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    dst->object.hashType       = src->object.hashType;
    dst->object.bytesInBuffer  = src->object.bytesInBuffer;
    dst->object.bytesProcessed = src->object.bytesProcessed;
    memcpy(dst->object.buffer, src->object.buffer, sizeof(dst->object.buffer));
    // copy the digest to support cloning after finalization has happened
    memcpy(dst->object.digest, src->object.digest, sizeof(dst->object.digest));
}

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
