/******************************************************************************

 @file sha256_alt.h

 @brief SHA256 specific alternate implementation items.

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


#ifndef MBEDTLS_SHA256_ALT_H
#define MBEDTLS_SHA256_ALT_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls-config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_SHA256_ALT)
#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/SHA2.h>
#include <ti/drivers/sha2/SHA2CC26X2.h>
/**
 * \brief          SHA-256 context structure
 */
typedef struct
{
    SHA2_Handle hndl;             /*!< A handle that is returned by the SHA driver  */
    SHA2_Config config;           /*!< structure containing SHA2 driver specific implementation  */
    SHA2CC26X2_Object object;     /*!< Pointer to a driver specific data object */
}
mbedtls_sha256_context;

#ifdef __cplusplus
}
#endif
#endif /* MBEDTLS_SHA256_ALT */

#endif /* MBEDTLS_SHA256_ALT_H */
