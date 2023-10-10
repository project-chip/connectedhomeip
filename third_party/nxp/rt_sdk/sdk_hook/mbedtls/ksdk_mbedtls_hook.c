/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_trng.h"

#define TRGN_MAX_RETRY 10

int __wrap_mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen)
{
    status_t result = kStatus_Fail;
    int nbRetry     = 0;

    do
    {
        result = TRNG_GetRandomData(TRNG, output, len);
        if (result == kStatus_Success)
            break;
        nbRetry++;
    } while (nbRetry < TRGN_MAX_RETRY);

    if (result == kStatus_Success)
    {
        *olen = len;
        return 0;
    }
    else
    {
        return result;
    }
}
