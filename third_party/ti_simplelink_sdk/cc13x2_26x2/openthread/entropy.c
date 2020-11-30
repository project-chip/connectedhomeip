/******************************************************************************

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************

 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************


 *****************************************************************************/

/**
 * @file
 *   This file implements an entropy source based on TRNG.
 *
 */

#include <openthread/config.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib / prcm.h)
#include DeviceFamily_constructPath(driverlib / trng.h)

#include <openthread/platform/entropy.h>
#include <ti/drivers/TRNG.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

#include <assert.h>

TRNG_Handle TRNG_handle;

/**
 * @internal
 * @brief Fill an arbitrary area with the random data.
 *
 * @param aOutput area to place the random data
 * @param aLen size if the area to place random data
 *
 *
 * @return returns 0 if no error occurred, -1 if error.
 */
static int getRandom(uint8_t * aOutput, size_t aLen)
{
    int_fast16_t rtn;
    CryptoKey entropyKey;

    /*
     * prepare the data buffer
     */
    CryptoKeyPlaintext_initBlankKey(&entropyKey, aOutput, aLen);

    /* get entropy */
    rtn = TRNG_generateEntropy(TRNG_handle, &entropyKey);
    if (rtn != TRNG_STATUS_SUCCESS)
        return OT_ERROR_FAILED;

    return OT_ERROR_NONE;
}

/**
 * Function documented in system.h
 */
void platformRandomInit(void)
{
    otError error = OT_ERROR_NONE;
    TRNG_Params TRNGParams;

    /* Init the TRNG HW */
    TRNG_init();

    TRNG_Params_init(&TRNGParams);
    /* use the polling mode */
    TRNGParams.returnBehavior = TRNG_RETURN_BEHAVIOR_POLLING;
    TRNG_handle               = TRNG_open(0, &TRNGParams);

    if (NULL == TRNG_handle)
    {
        error = OT_ERROR_FAILED;
    }

    assert(error == OT_ERROR_NONE);

    /* suppress the compiling warning */
    (void) error;

    return;
}
/**
 * Function documented in system.h
 */
void platformRandomProcess(void)
{
    /* place holder */
}

/**
 * Function documented in platform/entropy.h
 */
otError otPlatEntropyGet(uint8_t * aOutput, uint16_t aOutputLength)
{
    otError error = OT_ERROR_NONE;

    if (NULL == aOutput)
    {
        return OT_ERROR_INVALID_ARGS;
    }

    if (getRandom(aOutput, aOutputLength) != 0)
    {
        error = OT_ERROR_FAILED;
    }

    return error;
}
