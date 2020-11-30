/******************************************************************************

 @file aes_alt.c

 @brief AES implientation for TI chip

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

#include <openthread/config.h>

#include "aes_alt.h"
#include "mbedtls/aes.h"

#if defined(MBEDTLS_AES_ALT)

#include <assert.h>
#include <string.h>

#include <ti/devices/DeviceFamily.h>
#include <ti/drivers/AESECB.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

/**
 * number of active contexts, used for power on/off of the crypto core
 */
static unsigned int ref_num = 0;

static AESECB_Handle AESECB_handle = NULL;

/**
 * @brief Initialize AES context
 *
 * @param [in,out] ctx AES context to be initialized
 */
void mbedtls_aes_init(mbedtls_aes_context * ctx)
{
    AESECB_Params AESECBParams;

    if (ref_num++ == 0)
    {
        AESECB_Params_init(&AESECBParams);
        AESECBParams.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;
        AESECB_handle               = AESECB_open(0, &AESECBParams);
        assert(AESECB_handle != 0);
    }
}

/**
 * @brief          Clear AES context
 *
 * \param ctx      AES context to be cleared
 */
void mbedtls_aes_free(mbedtls_aes_context * ctx)
{
    if (--ref_num == 0)
    {
        AESECB_close(AESECB_handle);

        AESECB_handle = NULL;
    }

    memset((void *) ctx, 0x00, sizeof(ctx));
}

/**
 * \brief          AES key schedule (encryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      encryption key
 * \param keybits  must be 128, 192 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_enc(mbedtls_aes_context * ctx, const unsigned char * key, unsigned int keybits)
{
    int_fast16_t statusCrypto = 0;

    /* Initialize AES key */
    memcpy(ctx->keyMaterial, key, (keybits >> 3));
    statusCrypto = CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t *) ctx->keyMaterial, (keybits >> 3));
    assert(statusCrypto == 0);

    return (int) statusCrypto;
}

/**
 * \brief          AES key schedule (decryption)
 *
 * \param ctx      AES context to be initialized
 * \param key      decryption key
 * \param keybits  must be 128, 192 or 256
 *
 * \return         0 if successful, or MBEDTLS_ERR_AES_INVALID_KEY_LENGTH
 */
int mbedtls_aes_setkey_dec(mbedtls_aes_context * ctx, const unsigned char * key, unsigned int keybits)
{
    int_fast16_t statusCrypto;

    /* Initialize AES key */
    statusCrypto = CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t *) key, (keybits >> 3));
    assert(statusCrypto == 0);

    return (int) statusCrypto;
}

/**
 * \brief          AES-ECB block encryption/decryption
 *
 * \param ctx      AES context
 * \param mode     MBEDTLS_AES_ENCRYPT or MBEDTLS_AES_DECRYPT
 * \param input    16-byte input block
 * \param output   16-byte output block
 *
 * \return         0 if successful
 */

int mbedtls_aes_crypt_ecb(mbedtls_aes_context * ctx, int mode, const unsigned char input[16], unsigned char output[16])
{
    int statusCrypto;
    AESECB_Operation operationOneStepEncrypt;

    /* run it through the authentication + encryption, pass the ccmLVal = 2 */
    AESECB_Operation_init(&operationOneStepEncrypt);

    operationOneStepEncrypt.key         = &ctx->cryptoKey;
    operationOneStepEncrypt.inputLength = 16;
    operationOneStepEncrypt.input       = (uint8_t *) input;
    operationOneStepEncrypt.output      = (uint8_t *) output;

    statusCrypto = AESECB_oneStepEncrypt(AESECB_handle, &operationOneStepEncrypt);
    assert(statusCrypto == 0);

    return statusCrypto;
}
#endif
