/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      This file provides the Mbedtls platform aternative implementation.
 *      It contains custom setup() and teardown() functions.
 */

#include "mbedtls/platform.h"
#include "mbedtls/threading.h"

#ifdef MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT

#include <string.h>

#ifdef MBEDTLS_PLATFORM_NV_SEED_ALT
static int mbedtls_platform_nv_seed_read(unsigned char * buf, size_t buf_len)
{
    if (buf == NULL)
    {
        return (-1);
    }
    memset(buf, 0xA5, buf_len);
    return 0;
}

static int mbedtls_platform_nv_seed_write(unsigned char * buf, size_t buf_len)
{
    return 0;
}
#endif /* MBEDTLS_PLATFORM_NV_SEED_ALT */

int mbedtls_platform_setup(mbedtls_platform_context * ctx)
{
    (void) ctx;
    int ret = 0;

#ifdef MBEDTLS_PLATFORM_NV_SEED_ALT
    ret = mbedtls_platform_set_nv_seed(mbedtls_platform_nv_seed_read, mbedtls_platform_nv_seed_write);
    if (ret)
    {
        return ret;
    }
#endif /* MBEDTLS_PLATFORM_NV_SEED_ALT */
#ifdef MBEDTLS_THREADING_ALT
    mbedtls_threading_set_cmsis_rtos();
#endif /* MBEDTLS_THREADING_ALT */
    return ret;
}

void mbedtls_platform_teardown(mbedtls_platform_context * ctx)
{
    (void) ctx;
}

#endif /* MBEDTLS_PLATFORM_SETUP_TEARDOWN_ALT */
