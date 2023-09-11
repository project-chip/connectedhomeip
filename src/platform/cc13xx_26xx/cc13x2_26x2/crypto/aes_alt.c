/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2020 Texas Instruments Incorporated
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "aes_alt.h"
#include "mbedtls/aes.h"

#if defined(MBEDTLS_AES_ALT)

#include <string.h>

#include "ti_drivers_config.h"

#include <ti/drivers/AESECB.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>

/*
 * number of active contexts, used for power on/off of the crypto core
 */
static unsigned int ref_num = 0;

static AESECB_Handle AESECB_handle = NULL;

void mbedtls_aes_init(mbedtls_aes_context * ctx)
{
    AESECB_Params AESECBParams;

    if (ref_num == 0)
    {
        AESECB_Params_init(&AESECBParams);
        AESECBParams.returnBehavior = AESECB_RETURN_BEHAVIOR_POLLING;
        AESECB_handle               = AESECB_open(CONFIG_AESECB_1, &AESECBParams);

        // handle will be NULL if open failed, subsequent calls will fail with a generic HW error
    }
    ref_num++;
}

void mbedtls_aes_free(mbedtls_aes_context * ctx)
{
    if (ref_num > 0)
    {
        ref_num--;
        if (ref_num == 0)
        {
            AESECB_close(AESECB_handle);

            AESECB_handle = NULL;
        }
    }

    memset((void *) ctx, 0x00, sizeof(ctx));
}

int mbedtls_aes_setkey_enc(mbedtls_aes_context * ctx, const unsigned char * key, unsigned int keybits)
{
    int_fast16_t statusCrypto;
    size_t keylen = keybits / 8U; // 8 bits in a byte

    if (keylen > sizeof(ctx->keyMaterial))
    {
        return MBEDTLS_ERR_AES_INVALID_KEY_LENGTH;
    }

    /* Initialize AES key */
    memcpy(ctx->keyMaterial, key, keylen);
    statusCrypto = CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t *) ctx->keyMaterial, keylen);

    if (CryptoKey_STATUS_SUCCESS != statusCrypto)
    {
        return MBEDTLS_ERR_AES_HW_ACCEL_FAILED;
    }

    return 0;
}

int mbedtls_aes_setkey_dec(mbedtls_aes_context * ctx, const unsigned char * key, unsigned int keybits)
{
    int_fast16_t statusCrypto;
    size_t keylen = keybits / 8U; // 8 bits in a byte

    if (keylen > sizeof(ctx->keyMaterial))
    {
        return MBEDTLS_ERR_AES_INVALID_KEY_LENGTH;
    }

    /* Initialize AES key */
    statusCrypto = CryptoKeyPlaintext_initKey(&ctx->cryptoKey, (uint8_t *) key, keylen);

    if (CryptoKey_STATUS_SUCCESS != statusCrypto)
    {
        return MBEDTLS_ERR_AES_HW_ACCEL_FAILED;
    }

    return 0;
}

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

    if (CryptoKey_STATUS_SUCCESS != statusCrypto)
    {
        return MBEDTLS_ERR_AES_HW_ACCEL_FAILED;
    }

    return 0;
}
#endif
