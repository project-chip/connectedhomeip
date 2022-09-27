/*
 * Copyright 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "SecLib.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include "fsl_common.h"

#if defined(FSL_FEATURE_SOC_LTC_COUNT) && (FSL_FEATURE_SOC_LTC_COUNT > 0)
#include "fsl_ltc.h"
#endif
#if defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0)
#include "fsl_caam.h"
#endif
#if defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0)
#include "fsl_cau3.h"
#endif
#if defined(FSL_FEATURE_SOC_DCP_COUNT) && (FSL_FEATURE_SOC_DCP_COUNT > 0)
#include "fsl_dcp.h"
#endif
#if defined(FSL_FEATURE_SOC_HASHCRYPT_COUNT) && (FSL_FEATURE_SOC_HASHCRYPT_COUNT > 0)
#include "fsl_hashcrypt.h"
#endif
#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#if defined CPU_JN518X
#include "fsl_rng.h"
#else
#include "fsl_trng.h"
#endif
#elif defined(FSL_FEATURE_SOC_RNG_COUNT) && (FSL_FEATURE_SOC_RNG_COUNT > 0)
#include "fsl_rnga.h"
#elif defined(FSL_FEATURE_SOC_LPC_RNG1_COUNT) && (FSL_FEATURE_SOC_LPC_RNG1_COUNT > 0)
#include "fsl_rng.h"
#endif

#define CLEAN_RETURN(value)                                                                                                        \
    {                                                                                                                              \
        ret = value;                                                                                                               \
        goto cleanup;                                                                                                              \
    }

/******************************************************************************/
/*************************** CAAM *********************************************/
/******************************************************************************/
#if defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0) && defined(CRYPTO_USE_DRIVER_CAAM)
static caam_handle_t s_caamHandle = { .jobRing = kCAAM_JobRing0 };
#endif

/******************************************************************************/
/*************************** CAU3 *********************************************/
/******************************************************************************/
#if defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0)
static cau3_handle_t s_cau3Handle = { .taskDone = MBEDTLS_CAU3_COMPLETION_SIGNAL, .keySlot = kCAU3_KeySlot0 };
#endif

/******************************************************************************/
/**************************** DCP *********************************************/
/******************************************************************************/
#if defined(FSL_FEATURE_SOC_DCP_COUNT) && (FSL_FEATURE_SOC_DCP_COUNT > 0)
static dcp_handle_t s_dcpHandle = { .channel = kDCP_Channel0, .keySlot = kDCP_KeySlot0, .swapConfig = kDCP_NoSwap };
#endif

/******************************************************************************/
/************************* Key slot management ********************************/
/******************************************************************************/
#if (defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0)) || (defined(MBEDTLS_FREESCALE_DCP_AES))
static const void * s_mbedtlsCtx[4] = { 0 };

static void crypto_attach_ctx_to_key_slot(const void * ctx, uint8_t keySlot)
{
    s_mbedtlsCtx[keySlot] = ctx;
}

static void crypto_detach_ctx_from_key_slot(const void * ctx)
{
    for (int i = 0; i < 4; i++)
    {
        if (ctx == s_mbedtlsCtx[i])
        {
            s_mbedtlsCtx[i] = NULL;
            break;
        }
    }
}

static bool crypto_key_is_loaded(const void * ctx)
{
    bool ret = false;
    for (int i = 0; i < 4; i++)
    {
        if (ctx == s_mbedtlsCtx[i])
        {
            ret = true;
            break;
        }
    }
    return ret;
}
#endif

#if defined(MBEDTLS_SHA1_ALT) || defined(MBEDTLS_SHA256_ALT)
/* Implementation that should never be optimized out by the compiler */
static void mbedtls_zeroize(void * v, size_t n)
{
    volatile unsigned char * p = v;
    while (n--)
        *p++ = 0;
}
#endif /* MBEDTLS_SHA1_ALT || MBEDTLS_SHA256_ALT */

/******************************************************************************/
/******************** CRYPTO_InitHardware **************************************/
/******************************************************************************/
/*!
 * @brief Application init for various Crypto blocks.
 *
 * This function is provided to be called by MCUXpresso SDK applications.
 * It calls basic init for Crypto Hw acceleration and Hw entropy modules.
 */
void CRYPTO_InitHardware(void)
{
#if defined(FSL_FEATURE_SOC_LTC_COUNT) && (FSL_FEATURE_SOC_LTC_COUNT > 0)
    /* Initialize LTC driver.
     * This enables clocking and resets the module to a known state. */
    LTC_Init(LTC0);
#endif
#if defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0) && defined(CRYPTO_USE_DRIVER_CAAM)
    /* Initialize CAAM driver. */
    caam_config_t caamConfig;

    CAAM_GetDefaultConfig(&caamConfig);
    caamConfig.jobRingInterface[0] = &s_jrif0;
    caamConfig.jobRingInterface[1] = &s_jrif1;
    CAAM_Init(CAAM, &caamConfig);
#endif
#if defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0)
    /* Initialize CAU3 */
    CAU3_Init(CAU3);
#endif
#if defined(FSL_FEATURE_SOC_DCP_COUNT) && (FSL_FEATURE_SOC_DCP_COUNT > 0)
    /* Initialize DCP */
    dcp_config_t dcpConfig;

    DCP_GetDefaultConfig(&dcpConfig);
    DCP_Init(DCP, &dcpConfig);
#endif
#if defined(FSL_FEATURE_SOC_CASPER_COUNT) && (FSL_FEATURE_SOC_CASPER_COUNT > 0)
    /* Initialize CASPER */
    CASPER_Init(CASPER);
#endif
#if defined(FSL_FEATURE_SOC_HASHCRYPT_COUNT) && (FSL_FEATURE_SOC_HASHCRYPT_COUNT > 0)
    /* Initialize HASHCRYPT */
    HASHCRYPT_Init(HASHCRYPT);
#endif
    { /* Init RNG module.*/
#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#if defined CPU_JN518X
#if defined(RNG)
#define TRNG0 RNG
#endif
#else
#if defined(TRNG)
#define TRNG0 TRNG
#endif
#endif
        trng_config_t trngConfig;

        TRNG_GetDefaultConfig(&trngConfig);
        /* Set sample mode of the TRNG ring oscillator to Von Neumann, for better random data.*/
        /* Initialize TRNG */
        TRNG_Init(TRNG0, &trngConfig);
#elif defined(FSL_FEATURE_SOC_RNG_COUNT) && (FSL_FEATURE_SOC_RNG_COUNT > 0)
        RNGA_Init(RNG);
        RNGA_Seed(RNG, SIM->UIDL);
#endif
    }
}

/******************************************************************************/
/*************************** DES **********************************************/
/******************************************************************************/

#if defined(MBEDTLS_DES_C)

#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_MMCAU_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES) ||           \
    defined(MBEDTLS_FREESCALE_CAU3_DES)

#include "mbedtls/des.h"

#if defined(MBEDTLS_FREESCALE_MMCAU_DES)
const unsigned char parityLookup[128] = { 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0,
                                          0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1,
                                          0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1,
                                          1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
                                          0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
#endif

#if defined(MBEDTLS_FREESCALE_MMCAU_DES) || defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES)
/*
 * DES key schedule (56-bit, encryption)
 */
int mbedtls_des_setkey_enc(mbedtls_des_context * ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE])
{
#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES)
    memcpy(ctx->sk, key, MBEDTLS_DES_KEY_SIZE);
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    int i;
    unsigned char * sk_b = (unsigned char *) ctx->sk;

    /* fix key parity, if needed */
    for (i = 0; i < MBEDTLS_DES_KEY_SIZE; i++)
    {
        sk_b[i] = ((key[i] & 0xFE) | parityLookup[key[i] >> 1]);
    }
#endif
    ctx->mode = MBEDTLS_DES_ENCRYPT;

    return (0);
}

/*
 * DES key schedule (56-bit, decryption)
 */
int mbedtls_des_setkey_dec(mbedtls_des_context * ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE])
{
#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES)
    memcpy(ctx->sk, key, MBEDTLS_DES_KEY_SIZE);
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    int i;
    unsigned char * sk_b = (unsigned char *) ctx->sk;

    /* fix key parity, if needed */
    for (i = 0; i < MBEDTLS_DES_KEY_SIZE; i++)
    {
        sk_b[i] = ((key[i] & 0xFE) | parityLookup[key[i] >> 1]);
    }
#endif
    ctx->mode = MBEDTLS_DES_DECRYPT;

    return (0);
}
#endif /* MBEDTLS_FREESCALE_MMCAU_DES || MBEDTLS_FREESCALE_LTC_DES || MBEDTLS_FREESCALE_CAAM_DES */

/*
 * Triple-DES key schedule (112-bit, encryption)
 */
int mbedtls_des3_set2key_enc(mbedtls_des3_context * ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE * 2])
{
#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES) || defined(MBEDTLS_FREESCALE_CAU3_DES)
    memcpy(ctx->sk, key, MBEDTLS_DES_KEY_SIZE * 2);
    memcpy(&ctx->sk[4], key, MBEDTLS_DES_KEY_SIZE); /* K3 = K1 */
#if defined(MBEDTLS_FREESCALE_CAU3_DES)
    crypto_detach_ctx_from_key_slot(ctx);
#endif
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    int i;
    unsigned char * sk_b = (unsigned char *) ctx->sk;

    /* fix key parity, if needed */
    for (i = 0; i < MBEDTLS_DES_KEY_SIZE * 2; i++)
    {
        sk_b[i] = ((key[i] & 0xFE) | parityLookup[key[i] >> 1]);
    }
    for (i = MBEDTLS_DES_KEY_SIZE * 2; i < MBEDTLS_DES_KEY_SIZE * 3; i++)
    {
        sk_b[i] = ((key[i - MBEDTLS_DES_KEY_SIZE * 2] & 0xFE) | parityLookup[key[i - MBEDTLS_DES_KEY_SIZE * 2] >> 1]);
    }
#endif
    ctx->mode = MBEDTLS_DES_ENCRYPT;

    return (0);
}

/*
 * Triple-DES key schedule (112-bit, decryption)
 */
int mbedtls_des3_set2key_dec(mbedtls_des3_context * ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE * 2])
{
#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES) || defined(MBEDTLS_FREESCALE_CAU3_DES)
    memcpy(ctx->sk, key, MBEDTLS_DES_KEY_SIZE * 2);
    memcpy(&ctx->sk[4], key, MBEDTLS_DES_KEY_SIZE); /* K3 = K1 */
#if defined(MBEDTLS_FREESCALE_CAU3_DES)
    crypto_detach_ctx_from_key_slot(ctx);
#endif
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    int i;
    unsigned char * sk_b = (unsigned char *) ctx->sk;

    /* fix key parity, if needed */
    for (i = 0; i < MBEDTLS_DES_KEY_SIZE * 2; i++)
    {
        sk_b[i] = ((key[i] & 0xFE) | parityLookup[key[i] >> 1]);
    }
    for (i = MBEDTLS_DES_KEY_SIZE * 2; i < MBEDTLS_DES_KEY_SIZE * 3; i++)
    {
        sk_b[i] = ((key[i - MBEDTLS_DES_KEY_SIZE * 2] & 0xFE) | parityLookup[key[i - MBEDTLS_DES_KEY_SIZE * 2] >> 1]);
    }
#endif
    ctx->mode = MBEDTLS_DES_DECRYPT;

    return (0);
}

/*
 * Triple-DES key schedule (168-bit, encryption)
 */
int mbedtls_des3_set3key_enc(mbedtls_des3_context * ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE * 3])
{
#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES) || defined(MBEDTLS_FREESCALE_CAU3_DES)
    memcpy(ctx->sk, key, MBEDTLS_DES_KEY_SIZE * 3);
#if defined(MBEDTLS_FREESCALE_CAU3_DES)
    crypto_detach_ctx_from_key_slot(ctx);
#endif
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    int i;
    unsigned char * sk_b = (unsigned char *) ctx->sk;

    /* fix key parity, if needed */
    for (i = 0; i < MBEDTLS_DES_KEY_SIZE * 3; i++)
    {
        sk_b[i] = ((key[i] & 0xFE) | parityLookup[key[i] >> 1]);
    }
#endif
    ctx->mode = MBEDTLS_DES_ENCRYPT;

    return (0);
}

/*
 * Triple-DES key schedule (168-bit, decryption)
 */
int mbedtls_des3_set3key_dec(mbedtls_des3_context * ctx, const unsigned char key[MBEDTLS_DES_KEY_SIZE * 3])
{
#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES) || defined(MBEDTLS_FREESCALE_CAU3_DES)
    memcpy(ctx->sk, key, MBEDTLS_DES_KEY_SIZE * 3);
#if defined(MBEDTLS_FREESCALE_CAU3_DES)
    crypto_detach_ctx_from_key_slot(ctx);
#endif
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    int i;
    unsigned char * sk_b = (unsigned char *) ctx->sk;

    /* fix key parity, if needed */
    for (i = 0; i < MBEDTLS_DES_KEY_SIZE * 3; i++)
    {
        sk_b[i] = ((key[i] & 0xFE) | parityLookup[key[i] >> 1]);
    }
#endif
    ctx->mode = MBEDTLS_DES_DECRYPT;
    return (0);
}

#if defined(MBEDTLS_FREESCALE_LTC_DES) || defined(MBEDTLS_FREESCALE_MMCAU_DES) || defined(MBEDTLS_FREESCALE_CAAM_DES)
/*
 * DES-ECB block encryption/decryption
 */
int mbedtls_des_crypt_ecb(mbedtls_des_context * ctx, const unsigned char input[8], unsigned char output[8])
{
    uint8_t * key = (uint8_t *) ctx->sk;
#if defined(MBEDTLS_FREESCALE_LTC_DES)
    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        LTC_DES_EncryptEcb(LTC_INSTANCE, input, output, 8, key);
    }
    else
    {
        LTC_DES_DecryptEcb(LTC_INSTANCE, input, output, 8, key);
    }
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        MMCAU_DES_EncryptEcb(input, key, output);
    }
    else
    {
        MMCAU_DES_DecryptEcb(input, key, output);
    }
#elif defined(MBEDTLS_FREESCALE_CAAM_DES)
    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        CAAM_DES_EncryptEcb(CAAM_INSTANCE, &s_caamHandle, input, output, 8, key);
    }
    else
    {
        CAAM_DES_DecryptEcb(CAAM_INSTANCE, &s_caamHandle, input, output, 8, key);
    }
#endif
    return (0);
}
#endif /* MBEDTLS_FREESCALE_LTC_DES || MBEDTLS_FREESCALE_MMCAU_DES || MBEDTLS_FREESCALE_CAAM_DES */

/*
 * 3DES-ECB block encryption/decryption
 */
int mbedtls_des3_crypt_ecb(mbedtls_des3_context * ctx, const unsigned char input[8], unsigned char output[8])
{
    uint8_t * key = (uint8_t *) ctx->sk;
#if defined(MBEDTLS_FREESCALE_LTC_DES)
    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        LTC_DES3_EncryptEcb(LTC_INSTANCE, input, output, 8, key, key + 8, key + 16);
    }
    else
    {
        LTC_DES3_DecryptEcb(LTC_INSTANCE, input, output, 8, key, key + 8, key + 16);
    }
#elif defined(MBEDTLS_FREESCALE_MMCAU_DES)
    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        MMCAU_DES_EncryptEcb(input, key, output);
        MMCAU_DES_DecryptEcb(output, key + 8, output);
        MMCAU_DES_EncryptEcb(output, key + 16, output);
    }
    else
    {
        MMCAU_DES_DecryptEcb(input, key + 16, output);
        MMCAU_DES_EncryptEcb(output, key + 8, output);
        MMCAU_DES_DecryptEcb(output, key, output);
    }
#elif defined(MBEDTLS_FREESCALE_CAAM_DES)
    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        CAAM_DES3_EncryptEcb(CAAM_INSTANCE, &s_caamHandle, input, output, 8, key, key + 8, key + 16);
    }
    else
    {
        CAAM_DES3_DecryptEcb(CAAM_INSTANCE, &s_caamHandle, input, output, 8, key, key + 8, key + 16);
    }
#elif defined(MBEDTLS_FREESCALE_CAU3_DES)
    if (!crypto_key_is_loaded(ctx))
    {
        CAU3_TDES_SetKey(CAU3, &s_cau3Handle, key, 24);
        crypto_attach_ctx_to_key_slot(ctx, s_cau3Handle.keySlot);
    }

    if (ctx->mode == MBEDTLS_DES_ENCRYPT)
    {
        CAU3_TDES_Encrypt(CAU3, &s_cau3Handle, input, output);
    }
    else
    {
        CAU3_TDES_Decrypt(CAU3, &s_cau3Handle, input, output);
    }
#endif
    return (0);
}

#if defined(MBEDTLS_CIPHER_MODE_CBC)
/*
 * DES-CBC buffer encryption/decryption
 */
#if defined(MBEDTLS_FREESCALE_LTC_DES)
int mbedtls_des_crypt_cbc(mbedtls_des_context * ctx, int mode, size_t length, unsigned char iv[8], const unsigned char * input,
                          unsigned char * output)
{
    unsigned char temp[8];
    uint8_t * key = (uint8_t *) ctx->sk;

    if (length % 8)
        return (MBEDTLS_ERR_DES_INVALID_INPUT_LENGTH);

    if (mode == MBEDTLS_DES_ENCRYPT)
    {
        LTC_DES_EncryptCbc(LTC_INSTANCE, input, output, length, iv, key);
        memcpy(iv, output + length - 8, 8);
    }
    else /* MBEDTLS_DES_DECRYPT */
    {
        memcpy(temp, input + length - 8, 8);
        LTC_DES_DecryptCbc(LTC_INSTANCE, input, output, length, iv, key);
        memcpy(iv, temp, 8);
    }
    return (0);
}

/*
 * 3DES-CBC buffer encryption/decryption
 */
int mbedtls_des3_crypt_cbc(mbedtls_des3_context * ctx, int mode, size_t length, unsigned char iv[8], const unsigned char * input,
                           unsigned char * output)
{
    unsigned char temp[8];
    uint8_t * key = (uint8_t *) ctx->sk;

    if (length % 8)
        return (MBEDTLS_ERR_DES_INVALID_INPUT_LENGTH);

    if (mode == MBEDTLS_DES_ENCRYPT)
    {
        LTC_DES3_EncryptCbc(LTC_INSTANCE, input, output, length, iv, key, key + 8, key + 16);
        memcpy(iv, output + length - 8, 8);
    }
    else /* MBEDTLS_DES_DECRYPT */
    {
        memcpy(temp, input + length - 8, 8);
        LTC_DES3_DecryptCbc(LTC_INSTANCE, input, output, length, iv, key, key + 8, key + 16);
        memcpy(iv, temp, 8);
    }

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_CAAM_DES)
int mbedtls_des_crypt_cbc(mbedtls_des_context * ctx, int mode, size_t length, unsigned char iv[8], const unsigned char * input,
                          unsigned char * output)
{
    unsigned char temp[8];
    uint8_t * key = (uint8_t *) ctx->sk;

    if (length % 8)
        return (MBEDTLS_ERR_DES_INVALID_INPUT_LENGTH);

    if (mode == MBEDTLS_DES_ENCRYPT)
    {
        CAAM_DES_EncryptCbc(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, key);
        memcpy(iv, output + length - 8, 8);
    }
    else /* MBEDTLS_DES_DECRYPT */
    {
        memcpy(temp, input + length - 8, 8);
        CAAM_DES_DecryptCbc(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, key);
        memcpy(iv, temp, 8);
    }
    return (0);
}

/*
 * 3DES-CBC buffer encryption/decryption
 */
int mbedtls_des3_crypt_cbc(mbedtls_des3_context * ctx, int mode, size_t length, unsigned char iv[8], const unsigned char * input,
                           unsigned char * output)
{
    unsigned char temp[8];
    uint8_t * key = (uint8_t *) ctx->sk;

    if (length % 8)
        return (MBEDTLS_ERR_DES_INVALID_INPUT_LENGTH);

    if (mode == MBEDTLS_DES_ENCRYPT)
    {
        CAAM_DES3_EncryptCbc(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, key, key + 8, key + 16);
        memcpy(iv, output + length - 8, 8);
    }
    else /* MBEDTLS_DES_DECRYPT */
    {
        memcpy(temp, input + length - 8, 8);
        CAAM_DES3_DecryptCbc(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, key, key + 8, key + 16);
        memcpy(iv, temp, 8);
    }

    return (0);
}

#endif /* MBEDTLS_FREESCALE_LTC_DES */
#endif /* MBEDTLS_CIPHER_MODE_CBC */

#endif /*MBEDTLS_FREESCALE_LTC_DES || MBEDTLS_FREESCALE_MMCAU_DES || MBEDTLS_FREESCALE_CAAM_DES*/

#endif /* MBEDTLS_DES_C */

/******************************************************************************/
/*************************** AES **********************************************/
/******************************************************************************/

#if defined(MBEDTLS_AES_C)

#if defined(MBEDTLS_FREESCALE_LTC_AES) || defined(MBEDTLS_FREESCALE_MMCAU_AES) || defined(MBEDTLS_FREESCALE_LPC_AES) ||            \
    defined(MBEDTLS_FREESCALE_CAU3_AES) || defined(MBEDTLS_FREESCALE_CAAM_AES) || defined(MBEDTLS_FREESCALE_DCP_AES)

#include "mbedtls/aes.h"

/*
 * AES key schedule (encryption)
 */
int mbedtls_aes_setkey_enc(mbedtls_aes_context * ctx, const unsigned char * key, unsigned int keybits)
{
    uint32_t * RK;

#ifdef MBEDTLS_AES_ALT_NO_192
    if (keybits == 192u)
    {
        return (MBEDTLS_ERR_AES_FEATURE_UNAVAILABLE);
    }
#endif

#ifdef MBEDTLS_AES_ALT_NO_256
    if (keybits == 256u)
    {
        return (MBEDTLS_ERR_AES_FEATURE_UNAVAILABLE);
    }
#endif

#if defined(MBEDTLS_FREESCALE_LTC_AES) || defined(MBEDTLS_FREESCALE_LPC_AES) || defined(MBEDTLS_FREESCALE_CAU3_AES) ||             \
    defined(MBEDTLS_FREESCALE_CAAM_AES) || defined(MBEDTLS_FREESCALE_DCP_AES)
    const unsigned char * key_tmp = key;
    ctx->rk = RK = ctx->buf;
    memcpy(RK, key_tmp, keybits / 8);

#if defined(MBEDTLS_FREESCALE_CAU3_AES) || defined(MBEDTLS_FREESCALE_DCP_AES)
    crypto_detach_ctx_from_key_slot(ctx);
#endif /* MBEDTLS_FREESCALE_CAU3_AES || MBEDTLS_FREESCALE_DCP_AES */

    switch (keybits)
    { /* Set keysize in bytes.*/
    case 128:
        ctx->nr = 16;
        break;
    case 192:
        ctx->nr = 24;
        break;
    case 256:
        ctx->nr = 32;
        break;
    default:
        return (MBEDTLS_ERR_AES_INVALID_KEY_LENGTH);
    }
#elif defined(MBEDTLS_FREESCALE_MMCAU_AES)
    ctx->rk = RK = ctx->buf;

    switch (keybits)
    {
    case 128:
        ctx->nr = 10;
        break;
    case 192:
        ctx->nr = 12;
        break;
    case 256:
        ctx->nr = 14;
        break;
    default:
        return (MBEDTLS_ERR_AES_INVALID_KEY_LENGTH);
    }

    MMCAU_AES_SetKey(key, keybits / 8, (uint8_t *) RK);
#endif

    return (0);
}

/*
 * AES key schedule (decryption)
 */
int mbedtls_aes_setkey_dec(mbedtls_aes_context * ctx, const unsigned char * key, unsigned int keybits)
{
    uint32_t * RK;

#ifdef MBEDTLS_AES_ALT_NO_192
    if (keybits == 192u)
    {
        return (MBEDTLS_ERR_AES_FEATURE_UNAVAILABLE);
    }
#endif

#ifdef MBEDTLS_AES_ALT_NO_256
    if (keybits == 256u)
    {
        return (MBEDTLS_ERR_AES_FEATURE_UNAVAILABLE);
    }
#endif

    ctx->rk = RK = ctx->buf;

#if defined(MBEDTLS_FREESCALE_LTC_AES) || defined(MBEDTLS_FREESCALE_LPC_AES) || defined(MBEDTLS_FREESCALE_CAU3_AES) ||             \
    defined(MBEDTLS_FREESCALE_CAAM_AES) || defined(MBEDTLS_FREESCALE_DCP_AES)
    const unsigned char * key_tmp = key;
    memcpy(RK, key_tmp, keybits / 8);

#if defined(MBEDTLS_FREESCALE_CAU3_AES) || defined(MBEDTLS_FREESCALE_DCP_AES)
    crypto_detach_ctx_from_key_slot(ctx);
#endif /* MBEDTLS_FREESCALE_CAU3_AES || MBEDTLS_FREESCALE_DCP_AES */

    switch (keybits)
    {
    case 128:
        ctx->nr = 16;
        break;
    case 192:
        ctx->nr = 24;
        break;
    case 256:
        ctx->nr = 32;
        break;
    default:
        return (MBEDTLS_ERR_AES_INVALID_KEY_LENGTH);
    }
#elif defined(MBEDTLS_FREESCALE_MMCAU_AES)
    ctx->rk = RK = ctx->buf;

    switch (keybits)
    {
    case 128:
        ctx->nr = 10;
        break;
    case 192:
        ctx->nr = 12;
        break;
    case 256:
        ctx->nr = 14;
        break;
    default:
        return (MBEDTLS_ERR_AES_INVALID_KEY_LENGTH);
    }

    MMCAU_AES_SetKey(key, keybits / 8, (uint8_t *) RK);
#endif

    return 0;
}

/*
 * AES-ECB block encryption
 */
int mbedtls_internal_aes_encrypt(mbedtls_aes_context * ctx, const unsigned char input[16], unsigned char output[16])
{
    uint8_t * key;

    key = (uint8_t *) ctx->rk;
#if defined(MBEDTLS_FREESCALE_LTC_AES)
    LTC_AES_EncryptEcb(LTC_INSTANCE, input, output, 16, key, ctx->nr);
#elif defined(MBEDTLS_FREESCALE_MMCAU_AES)
    MMCAU_AES_EncryptEcb(input, key, ctx->nr, output);
#elif defined(MBEDTLS_FREESCALE_CAU3_AES)
    if (!crypto_key_is_loaded(ctx))
    {
        CAU3_AES_SetKey(CAU3, &s_cau3Handle, key, ctx->nr);
        crypto_attach_ctx_to_key_slot(ctx, s_cau3Handle.keySlot);
    }
    CAU3_AES_Encrypt(CAU3, &s_cau3Handle, input, output);
#elif defined(MBEDTLS_FREESCALE_LPC_AES)
    AES_128_Encrypt(input, key, output);
#elif defined(MBEDTLS_FREESCALE_CAAM_AES)
    CAAM_AES_EncryptEcb(CAAM_INSTANCE, &s_caamHandle, input, output, 16, key, ctx->nr);
#elif defined(MBEDTLS_FREESCALE_DCP_AES)
    if (!crypto_key_is_loaded(ctx))
    {
        DCP_AES_SetKey(DCP, &s_dcpHandle, key, ctx->nr);
        crypto_attach_ctx_to_key_slot(ctx, s_dcpHandle.keySlot);
    }
    DCP_AES_EncryptEcb(DCP, &s_dcpHandle, input, output, 16);
#endif

    return (0);
}

/*
 * AES-ECB block decryption
 */
int mbedtls_internal_aes_decrypt(mbedtls_aes_context * ctx, const unsigned char input[16], unsigned char output[16])
{
    uint8_t * key;

    key = (uint8_t *) ctx->rk;
#if defined(MBEDTLS_FREESCALE_LTC_AES)
    LTC_AES_DecryptEcb(LTC_INSTANCE, input, output, 16, key, ctx->nr, kLTC_EncryptKey);
#elif defined(MBEDTLS_FREESCALE_MMCAU_AES)
    MMCAU_AES_DecryptEcb(input, key, ctx->nr, output);
#elif defined(MBEDTLS_FREESCALE_CAU3_AES)
    if (!crypto_key_is_loaded(ctx))
    {
        CAU3_AES_SetKey(CAU3, &s_cau3Handle, key, ctx->nr);
        crypto_attach_ctx_to_key_slot(ctx, s_cau3Handle.keySlot);
    }
    CAU3_AES_Decrypt(CAU3, &s_cau3Handle, input, output);
#elif defined(MBEDTLS_FREESCALE_LPC_AES)
    AES_128_Decrypt(input, key, output);
#elif defined(MBEDTLS_FREESCALE_CAAM_AES)
    CAAM_AES_DecryptEcb(CAAM_INSTANCE, &s_caamHandle, input, output, 16, key, ctx->nr);
#elif defined(MBEDTLS_FREESCALE_DCP_AES)
    if (!crypto_key_is_loaded(ctx))
    {
        DCP_AES_SetKey(DCP, &s_dcpHandle, key, ctx->nr);
        crypto_attach_ctx_to_key_slot(ctx, s_dcpHandle.keySlot);
    }
    DCP_AES_DecryptEcb(DCP, &s_dcpHandle, input, output, 16);
#endif

    return (0);
}

#if defined(MBEDTLS_CIPHER_MODE_CBC)
/*
 * AES-CBC buffer encryption/decryption
 */
#if defined(MBEDTLS_FREESCALE_LTC_AES)
int mbedtls_aes_crypt_cbc(mbedtls_aes_context * ctx, int mode, size_t length, unsigned char iv[16], const unsigned char * input,
                          unsigned char * output)
{
    uint8_t * key    = (uint8_t *) ctx->rk;
    uint32_t keySize = ctx->nr;

    if (length % 16)
        return (MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH);

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        uint8_t tmp[16];
        memcpy(tmp, input + length - 16, 16);
        LTC_AES_DecryptCbc(LTC_INSTANCE, input, output, length, iv, key, keySize, kLTC_EncryptKey);
        memcpy(iv, tmp, 16);
    }
    else
    {
        LTC_AES_EncryptCbc(LTC_INSTANCE, input, output, length, iv, key, keySize);
        memcpy(iv, output + length - 16, 16);
    }

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_LPC_AES)
int mbedtls_aes_crypt_cbc(mbedtls_aes_context * ctx, int mode, size_t length, unsigned char iv[16], const unsigned char * input,
                          unsigned char * output)
{
    uint8_t * key;
    size_t keySize;

    if (length % 16)
        return (MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH);

    key = (uint8_t *) ctx->rk;
    keySize = (size_t) ctx->nr;

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        uint8_t tmp[16];
        memcpy(tmp, input + length - 16, 16);
        AES_128_CBC_Decrypt_And_Depad(tmp, length, iv, key, output);
        memcpy(iv, tmp, 16);
    }
    else
    {
        AES_128_CBC_Encrypt(input, length, iv, key, output);
        memcpy(iv, output + length - 16, 16);
    }

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_CAAM_AES)
int mbedtls_aes_crypt_cbc(mbedtls_aes_context * ctx, int mode, size_t length, unsigned char iv[16], const unsigned char * input,
                          unsigned char * output)
{
    uint8_t * key = (uint8_t *) ctx->rk;
    uint32_t keySize = ctx->nr;

    if (length % 16)
        return (MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH);

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        uint8_t tmp[16];
        memcpy(tmp, input + length - 16, 16);
        CAAM_AES_DecryptCbc(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, key, keySize);
        memcpy(iv, tmp, 16);
    }
    else
    {
        CAAM_AES_EncryptCbc(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, key, keySize);
        memcpy(iv, output + length - 16, 16);
    }

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_DCP_AES)
int mbedtls_aes_crypt_cbc(mbedtls_aes_context * ctx, int mode, size_t length, unsigned char iv[16], const unsigned char * input,
                          unsigned char * output)
{
    uint8_t * key;

    if (length % 16)
        return (MBEDTLS_ERR_AES_INVALID_INPUT_LENGTH);

    key = (uint8_t *) ctx->rk;
    if (!crypto_key_is_loaded(ctx))
    {
        DCP_AES_SetKey(DCP, &s_dcpHandle, key, ctx->nr);
        crypto_attach_ctx_to_key_slot(ctx, s_dcpHandle.keySlot);
    }

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        uint8_t tmp[16];
        memcpy(tmp, input + length - 16, 16);
        DCP_AES_DecryptCbc(DCP, &s_dcpHandle, input, output, length, iv);
        memcpy(iv, tmp, 16);
    }
    else
    {
        DCP_AES_EncryptCbc(DCP, &s_dcpHandle, input, output, length, iv);
        memcpy(iv, output + length - 16, 16);
    }

    return (0);
}
#endif
#endif /* MBEDTLS_CIPHER_MODE_CBC */

#if defined(MBEDTLS_CIPHER_MODE_CFB)
#if defined(MBEDTLS_FREESCALE_LPC_AES)
/*
 * AES-CFB128 buffer encryption/decryption
 */
int mbedtls_aes_crypt_cfb128(mbedtls_aes_context * ctx, int mode, size_t length, size_t * iv_off, unsigned char iv[16],
                             const unsigned char * input, unsigned char * output)
{
    uint8_t * key;
    size_t keySize;

    key     = (uint8_t *) ctx->rk;
    keySize = (size_t) ctx->nr;
    AES_SetKey(AES_INSTANCE, key, keySize);

    if (mode == MBEDTLS_AES_DECRYPT)
    {
        AES_DecryptCfb(AES_INSTANCE, input, output, length, iv);
    }
    else
    {
        AES_EncryptCfb(AES_INSTANCE, input, output, length, iv);
    }

    return (0);
}

/*
 * AES-CFB8 buffer encryption/decryption
 */
int mbedtls_aes_crypt_cfb8(mbedtls_aes_context * ctx, int mode, size_t length, unsigned char iv[16], const unsigned char * input,
                           unsigned char * output)
{
    int status;
    unsigned char c;
    unsigned char ov[17];

    while (length--)
    {
        memcpy(ov, iv, 16);
        status = mbedtls_aes_crypt_ecb(ctx, MBEDTLS_AES_ENCRYPT, iv, iv);
        if (status != 0)
        {
            return status;
        }

        if (mode == MBEDTLS_AES_DECRYPT)
            ov[16] = *input;

        c = *output++ = (unsigned char) (iv[0] ^ *input++);

        if (mode == MBEDTLS_AES_ENCRYPT)
            ov[16] = c;

        memcpy(iv, ov + 1, 16);
    }

    return (0);
}
#endif /* MBEDTLS_FREESCALE_LPC_AES */
#endif /* MBEDTLS_CIPHER_MODE_CFB */

#if defined(MBEDTLS_CIPHER_MODE_CTR)
/*
 * AES-CTR buffer encryption/decryption
 */
#if defined(MBEDTLS_FREESCALE_LTC_AES)
int mbedtls_aes_crypt_ctr(mbedtls_aes_context * ctx, size_t length, size_t * nc_off, unsigned char nonce_counter[16],
                          unsigned char stream_block[16], const unsigned char * input, unsigned char * output)
{
    uint8_t * key;
    uint32_t keySize;

    key     = (uint8_t *) ctx->rk;
    keySize = ctx->nr;
    LTC_AES_CryptCtr(LTC_INSTANCE, input, output, length, nonce_counter, key, keySize, stream_block, (uint32_t *) nc_off);

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_LPC_AES)
int mbedtls_aes_crypt_ctr(mbedtls_aes_context * ctx, size_t length, size_t * nc_off, unsigned char nonce_counter[16],
                          unsigned char stream_block[16], const unsigned char * input, unsigned char * output)
{
    uint8_t * key;
    size_t keySize;

    key = (uint8_t *) ctx->rk;
    keySize = (size_t) ctx->nr;

    AES_128_CTR(input, length, nonce_counter, key, output);

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_CAAM_AES)
int mbedtls_aes_crypt_ctr(mbedtls_aes_context * ctx, size_t length, size_t * nc_off, unsigned char nonce_counter[16],
                          unsigned char stream_block[16], const unsigned char * input, unsigned char * output)
{
    uint8_t * key;
    uint32_t keySize;

    key = (uint8_t *) ctx->rk;
    keySize = ctx->nr;

    CAAM_AES_CryptCtr(CAAM_INSTANCE, &s_caamHandle, input, output, length, nonce_counter, key, keySize, stream_block, nc_off);

    return (0);
}
#endif
#endif /* MBEDTLS_CIPHER_MODE_CTR */

#if defined(MBEDTLS_CIPHER_CMAC_ALT) && defined(MBEDTLS_CMAC_C)

#include "mbedtls/cipher.h"
#include "mbedtls/cmac.h"

#if defined(MBEDTLS_FREESCALE_CAU3_CIPHER_CMAC)
int mbedtls_cipher_cmac(const mbedtls_cipher_info_t * cipher_info, const unsigned char * key, size_t keylen,
                        const unsigned char * input, size_t ilen, unsigned char * output)
{
    mbedtls_cipher_context_t ctx;
    int ret;

    if (cipher_info == NULL || key == NULL || input == NULL || output == NULL)
        return (MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA);

    mbedtls_cipher_init(&ctx);

    if ((ret = mbedtls_cipher_setup(&ctx, cipher_info)) != 0)
        goto exit;

    ret = mbedtls_cipher_cmac_starts(&ctx, key, keylen);
    if (ret != 0)
        goto exit;

    /* AES-CMAC-128 is directly supported by CAU3 firmware */
    if (cipher_info->type == MBEDTLS_CIPHER_AES_128_ECB)
    {
        status_t status;
        uint8_t mac[16];

        status = CAU3_AES_SetKey(CAU3, &s_cau3Handle, key, keylen / 8u);
        if (status != kStatus_Success)
        {
            ret = MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA;
            goto exit;
        }
        status = CAU3_AES_Cmac(CAU3, &s_cau3Handle, input, ilen, mac);
        if (status != kStatus_Success)
        {
            ret = MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA;
            goto exit;
        }
        memcpy(output, mac, 16);
    }
#if defined(MBEDTLS_CIPHER_CMAC_TDES_ENABLED) || defined(MBEDTLS_CIPHER_CMAC_AES_256_ENABLED)
    else if (cipher_info->type == MBEDTLS_CIPHER_AES_192_ECB)
    {
        /* CAU3 initial firmware does not support AES 192 */
        ret = MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA;
        goto exit;
    }
    else
    {
        /* AES-CMAC-256 and TDES-CMAC.
         * If both MBEDTLS_DES_C and MBEDTLS_CIPHER_CMAC_WANTS_AES_256 are undefined,
         * this does not compile
         */
        ret = mbedtls_cipher_cmac_update(&ctx, input, ilen);
        if (ret != 0)
            goto exit;

        ret = mbedtls_cipher_cmac_finish(&ctx, output);
    }
#else
    else
    {
        ret = MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA;
        goto exit;
    }
#endif /* MBEDTLS_CIPHER_CMAC_TDES_ENABLED || MBEDTLS_CIPHER_CMAC_AES_256_ENABLED */

exit:
    mbedtls_cipher_free(&ctx);

    return (ret);
}
#endif /* MBEDTLS_FREESCALE_CAU3_CIPHER_CMAC */
#endif /* MBEDTLS_CIPHER_CMAC_ALT && MBEDTLS_CMAC_C */

#if defined(MBEDTLS_CCM_C)

#include "mbedtls/ccm.h"

#define CCM_ENCRYPT 0
#define CCM_DECRYPT 1

/*
 * Authenticated encryption or decryption
 */
#if defined(MBEDTLS_FREESCALE_LTC_AES)
static int ccm_auth_crypt(mbedtls_ccm_context * ctx, int mode, size_t length, const unsigned char * iv, size_t iv_len,
                          const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                          unsigned char * tag, size_t tag_len)
{
    status_t status;
    const uint8_t * key;
    uint8_t keySize;
    mbedtls_aes_context * aes_ctx;

    aes_ctx = (mbedtls_aes_context *) ctx->cipher_ctx.cipher_ctx;
    key     = (uint8_t *) aes_ctx->rk;
    keySize = aes_ctx->nr;
    if (mode == CCM_ENCRYPT)
    {
        status = LTC_AES_EncryptTagCcm(LTC_INSTANCE, input, output, length, iv, iv_len, add, add_len, key, keySize, tag, tag_len);
    }
    else
    {
        status = LTC_AES_DecryptTagCcm(LTC_INSTANCE, input, output, length, iv, iv_len, add, add_len, key, keySize, tag, tag_len);
    }

    if (status == kStatus_InvalidArgument)
    {
        return MBEDTLS_ERR_CCM_BAD_INPUT;
    }
    else if (status != kStatus_Success)
    {
        return MBEDTLS_ERR_CCM_AUTH_FAILED;
    }

    return (0);
}
#elif defined(MBEDTLS_FREESCALE_CAAM_AES)
static int ccm_auth_crypt(mbedtls_ccm_context * ctx, int mode, size_t length, const unsigned char * iv, size_t iv_len,
                          const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                          unsigned char * tag, size_t tag_len)
{
    status_t status;
    const uint8_t * key;
    uint8_t keySize;
    mbedtls_aes_context * aes_ctx;

    aes_ctx = (mbedtls_aes_context *) ctx->cipher_ctx.cipher_ctx;
    key = (uint8_t *) aes_ctx->rk;
    keySize = aes_ctx->nr;
    if (mode == CCM_ENCRYPT)
    {
        status = CAAM_AES_EncryptTagCcm(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, iv_len, add, add_len, key, keySize,
                                        tag, tag_len);
    }
    else
    {
        status = CAAM_AES_DecryptTagCcm(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, iv_len, add, add_len, key, keySize,
                                        tag, tag_len);
    }

    if (status == kStatus_InvalidArgument)
    {
        return MBEDTLS_ERR_CCM_BAD_INPUT;
    }
    else if (status != kStatus_Success)
    {
        return MBEDTLS_ERR_CCM_AUTH_FAILED;
    }

    return (0);
}
#endif /* MBEDTLS_FREESCALE_LTC_AES */

#if defined(MBEDTLS_FREESCALE_LTC_AES) || defined(MBEDTLS_FREESCALE_CAAM_AES)
/*
 * Authenticated encryption
 */
int mbedtls_ccm_encrypt_and_tag(mbedtls_ccm_context * ctx, size_t length, const unsigned char * iv, size_t iv_len,
                                const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                                unsigned char * tag, size_t tag_len)
{
    return (ccm_auth_crypt(ctx, CCM_ENCRYPT, length, iv, iv_len, add, add_len, input, output, tag, tag_len));
}

/*
 * Authenticated decryption
 */
int mbedtls_ccm_auth_decrypt(mbedtls_ccm_context * ctx, size_t length, const unsigned char * iv, size_t iv_len,
                             const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                             const unsigned char * tag, size_t tag_len)
{
    unsigned char tagCopy[16];
    unsigned char * actTag = NULL;
    if (tag)
    {
        memcpy(tagCopy, tag, tag_len);
        actTag = tagCopy;
    }
    return (ccm_auth_crypt(ctx, CCM_DECRYPT, length, iv, iv_len, add, add_len, input, output, actTag, tag_len));
}
#endif /* MBEDTLS_FREESCALE_LTC_AES || MBEDTLS_FREESCALE_CAAM_AES */
#endif /* MBEDTLS_CCM_C */

#if defined(MBEDTLS_GCM_C)
#if defined(MBEDTLS_FREESCALE_LTC_AES_GCM)

#include "mbedtls/gcm.h"

int mbedtls_gcm_crypt_and_tag(mbedtls_gcm_context * ctx, int mode, size_t length, const unsigned char * iv, size_t iv_len,
                              const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                              size_t tag_len, unsigned char * tag)
{
    status_t status;
    uint8_t * key;
    uint32_t keySize;
    mbedtls_aes_context * aes_ctx;

    ctx->len     = length;
    ctx->add_len = add_len;
    aes_ctx      = (mbedtls_aes_context *) ctx->cipher_ctx.cipher_ctx;
    key          = (uint8_t *) aes_ctx->rk;
    keySize      = aes_ctx->nr;
    if (mode == MBEDTLS_GCM_ENCRYPT)
    {
        status = LTC_AES_EncryptTagGcm(LTC_INSTANCE, input, output, length, iv, iv_len, add, add_len, key, keySize, tag, tag_len);
    }
    else
    {
        status = LTC_AES_DecryptTagGcm(LTC_INSTANCE, input, output, length, iv, iv_len, add, add_len, key, keySize, tag, tag_len);
    }

    if (status == kStatus_InvalidArgument)
    {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }
    else if (status != kStatus_Success)
    {
        return MBEDTLS_ERR_GCM_AUTH_FAILED;
    }

    return 0;
}

int mbedtls_gcm_auth_decrypt(mbedtls_gcm_context * ctx, size_t length, const unsigned char * iv, size_t iv_len,
                             const unsigned char * add, size_t add_len, const unsigned char * tag, size_t tag_len,
                             const unsigned char * input, unsigned char * output)
{
    unsigned char tag_copy[16];
    unsigned char * actTag = NULL;
    if (tag)
    {
        memcpy(tag_copy, tag, tag_len);
        actTag = tag_copy;
    }
    return (mbedtls_gcm_crypt_and_tag(ctx, MBEDTLS_GCM_DECRYPT, length, iv, iv_len, add, add_len, input, output, tag_len, actTag));
}

#elif defined(MBEDTLS_FREESCALE_LPC_AES_GCM)

#include "mbedtls/gcm.h"

int mbedtls_gcm_crypt_and_tag(mbedtls_gcm_context * ctx, int mode, size_t length, const unsigned char * iv, size_t iv_len,
                              const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                              size_t tag_len, unsigned char * tag)
{
    status_t status;
    uint8_t * key;
    size_t keySize;
    mbedtls_aes_context * aes_ctx;

    ctx->len = length;
    ctx->add_len = add_len;
    aes_ctx = (mbedtls_aes_context *) ctx->cipher_ctx.cipher_ctx;
    key = (uint8_t *) aes_ctx->rk;
    keySize = (size_t) aes_ctx->nr;

    status = AES_SetKey(AES_INSTANCE, key, keySize);
    if (status != kStatus_Success)
    {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }

    if (mode == MBEDTLS_GCM_ENCRYPT)
    {
        status = AES_EncryptTagGcm(AES_INSTANCE, input, output, length, iv, iv_len, add, add_len, tag, tag_len);
    }
    else
    {
        status = AES_DecryptTagGcm(AES_INSTANCE, input, output, length, iv, iv_len, add, add_len, tag, tag_len);
    }

    if (status == kStatus_InvalidArgument)
    {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }
    else if (status != kStatus_Success)
    {
        return MBEDTLS_ERR_GCM_AUTH_FAILED;
    }

    return 0;
}

int mbedtls_gcm_auth_decrypt(mbedtls_gcm_context * ctx, size_t length, const unsigned char * iv, size_t iv_len,
                             const unsigned char * add, size_t add_len, const unsigned char * tag, size_t tag_len,
                             const unsigned char * input, unsigned char * output)
{
    unsigned char tag_copy[16];

    memcpy(tag_copy, tag, tag_len);
    return (
        mbedtls_gcm_crypt_and_tag(ctx, MBEDTLS_GCM_DECRYPT, length, iv, iv_len, add, add_len, input, output, tag_len, tag_copy));
}

#elif defined(MBEDTLS_FREESCALE_CAAM_AES_GCM)

#include "mbedtls/gcm.h"

int mbedtls_gcm_crypt_and_tag(mbedtls_gcm_context * ctx, int mode, size_t length, const unsigned char * iv, size_t iv_len,
                              const unsigned char * add, size_t add_len, const unsigned char * input, unsigned char * output,
                              size_t tag_len, unsigned char * tag)
{
    status_t status;
    uint8_t * key;
    uint32_t keySize;
    mbedtls_aes_context * aes_ctx;

    ctx->len = length;
    ctx->add_len = add_len;
    aes_ctx = (mbedtls_aes_context *) ctx->cipher_ctx.cipher_ctx;
    key = (uint8_t *) aes_ctx->rk;
    keySize = aes_ctx->nr;
    if (mode == MBEDTLS_GCM_ENCRYPT)
    {
        status = CAAM_AES_EncryptTagGcm(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, iv_len, add, add_len, key, keySize,
                                        tag, tag_len);
    }
    else
    {
        status = CAAM_AES_DecryptTagGcm(CAAM_INSTANCE, &s_caamHandle, input, output, length, iv, iv_len, add, add_len, key, keySize,
                                        tag, tag_len);
    }

    if (status == kStatus_InvalidArgument)
    {
        return MBEDTLS_ERR_GCM_BAD_INPUT;
    }
    else if (status != kStatus_Success)
    {
        return MBEDTLS_ERR_GCM_AUTH_FAILED;
    }

    return 0;
}

int mbedtls_gcm_auth_decrypt(mbedtls_gcm_context * ctx, size_t length, const unsigned char * iv, size_t iv_len,
                             const unsigned char * add, size_t add_len, const unsigned char * tag, size_t tag_len,
                             const unsigned char * input, unsigned char * output)
{
    unsigned char tag_copy[16];
    unsigned char * actTag = NULL;
    if (tag)
    {
        memcpy(tag_copy, tag, tag_len);
        actTag = tag_copy;
    }
    return (mbedtls_gcm_crypt_and_tag(ctx, MBEDTLS_GCM_DECRYPT, length, iv, iv_len, add, add_len, input, output, tag_len, actTag));
}
#endif
#endif /* MBEDTLS_GCM_C */

#endif /* MBEDTLS_FREESCALE_LTC_AES || MBEDTLS_FREESCALE_MMCAU_AES || MBEDTLS_FREESCALE_LPC_AES */

#endif /* MBEDTLS_AES_C */

/******************************************************************************/
/*************************** PKHA *********************************************/
/******************************************************************************/

#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA) || defined(MBEDTLS_FREESCALE_CAAM_PKHA)

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_calloc calloc
#define mbedtls_free free
#endif

static void ltc_reverse_array(uint8_t * src, size_t src_len)
{
    int i;

    for (i = 0; i < src_len / 2; i++)
    {
        uint8_t tmp;

        tmp                  = src[i];
        src[i]               = src[src_len - 1 - i];
        src[src_len - 1 - i] = tmp;
    }
}

#if defined(MBEDTLS_BIGNUM_C)

#include "mbedtls/bignum.h"

#if defined(MBEDTLS_FREESCALE_CAU3_PKHA)
#define LTC_PKHA_ModAdd CAU3_PKHA_ModAdd
#define LTC_PKHA_ModSub1 CAU3_PKHA_ModSub1
#define LTC_PKHA_ModMul CAU3_PKHA_ModMul
#define LTC_PKHA_ModRed CAU3_PKHA_ModRed
#define LTC_PKHA_ModExp CAU3_PKHA_ModExp
#define LTC_PKHA_GCD CAU3_PKHA_ModGcd
#define LTC_PKHA_ModInv CAU3_PKHA_ModInv
#define LTC_PKHA_PrimalityTest CAU3_PKHA_PrimalityTest
#define LTC_INSTANCE ((CAU3_Type *) CAU3_BASE)

#define kLTC_PKHA_IntegerArith kCAU3_PKHA_IntegerArith
#define kLTC_PKHA_NormalValue kCAU3_PKHA_NormalValue
#define kLTC_PKHA_TimingEqualized kCAU3_PKHA_TimingEqualized

#define cau3_reverse_array ltc_reverse_array
#define cau3_get_from_mbedtls_mpi ltc_get_from_mbedtls_mpi
#endif

#if defined(MBEDTLS_FREESCALE_LTC_PKHA)
typedef uint16_t pkha_size_t;
#else
typedef size_t pkha_size_t;
#endif

#if defined(MBEDTLS_MPI_ADD_ABS_ALT)

/* Access to original version of mbedtls_mpi_add_abs function. */
int mbedtls_mpi_add_abs_orig(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B);

/*
 * Unsigned addition: X = |A| + |B|  (HAC 14.7)
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_add_abs(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);
    pkha_size_t sizeN = FREESCALE_PKHA_INT_MAX_BYTES;

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    /*
     * Perform HW acceleration only if the size in bytes is less than maximum.
     * Since modular add is used below, the result would be wrong
     * if the real sum of operands exceeded LTC maximum number value.
     */
    if ((sizeA < sizeN) && (sizeB < sizeN))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * N    = mbedtls_calloc(4, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrA = N + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == N)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        memset(N, 0xFF, sizeN);

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(B, ptrB, sizeB);
        ltc_reverse_array(ptrB, sizeB);

        ret = (int) LTC_PKHA_ModAdd(LTC_INSTANCE, ptrA, sizeA, ptrB, sizeB, N, sizeN, ptrC, &sizeC, kLTC_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrC, sizeC);
        mbedtls_mpi_read_binary(X, ptrC, sizeC);
        X->s = 1;
    cleanup:
        if (N)
        {
            mbedtls_free(N);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_add_abs_orig(X, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_add_abs(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);
    pkha_size_t sizeN = FREESCALE_PKHA_INT_MAX_BYTES;

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    /*
     * Perform HW acceleration only if the size in bytes is less than maximum.
     * Since modular add is used below, the result would be wrong
     * if the real sum of operands exceeded CAAM maximum number value.
     */
    if ((sizeA < sizeN) && (sizeB < sizeN))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * N = mbedtls_calloc(4, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrA = N + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == N)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        memset(N, 0xFF, sizeN);

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        mbedtls_mpi_write_binary(B, ptrB, sizeB);

        ret = (int) CAAM_PKHA_ModAdd(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrB, sizeB, N, sizeN, ptrC, &sizeC,
                                     kCAAM_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(X, ptrC, sizeC);
        X->s = 1;
    cleanup:
        if (N)
        {
            mbedtls_free(N);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_add_abs_orig(X, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_ADD_ABS_ALT */

#if defined(MBEDTLS_MPI_SUB_ABS_ALT)

/* Access to original version of mbedtls_mpi_sub_abs function. */
int mbedtls_mpi_sub_abs_orig(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B);

/*
 * Unsigned subtraction: X = |A| - |B|  (HAC 14.9)
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_sub_abs(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);
    pkha_size_t sizeN = FREESCALE_PKHA_INT_MAX_BYTES;

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    /*
     * Perform HW acceleration only if |A| >= |B|. Since modular subtraction is used below,
     * the result would be wrong if the real sum of operands exceeded maximum.
     */
    if ((sizeA <= sizeN) && (sizeB <= sizeN) && (mbedtls_mpi_cmp_abs(A, B) >= 0))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * N    = mbedtls_calloc(4, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrA = N + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == N)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        memset(N, 0xFF, sizeN);

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(B, ptrB, sizeB);
        ltc_reverse_array(ptrB, sizeB);

        ret = (int) LTC_PKHA_ModSub1(LTC_INSTANCE, ptrA, sizeA, ptrB, sizeB, N, sizeN, ptrC, &sizeC);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrC, sizeC);
        mbedtls_mpi_read_binary(X, ptrC, sizeC);
        X->s = 1;
    cleanup:
        if (N)
        {
            mbedtls_free(N);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_sub_abs_orig(X, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_sub_abs(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);
    pkha_size_t sizeN = FREESCALE_PKHA_INT_MAX_BYTES;

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    /*
     * Perform HW acceleration only if |A| >= |B|. Since modular subtraction is used below,
     * the result would be wrong if the real sum of operands exceeded maximum.
     */
    if ((sizeA <= sizeN) && (sizeB <= sizeN) && (mbedtls_mpi_cmp_abs(A, B) >= 0))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * N = mbedtls_calloc(4, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrA = N + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == N)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        memset(N, 0xFF, sizeN);

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        mbedtls_mpi_write_binary(B, ptrB, sizeB);

        ret = (int) CAAM_PKHA_ModSub1(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrB, sizeB, N, sizeN, ptrC, &sizeC);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(X, ptrC, sizeC);
        X->s = 1;
    cleanup:
        if (N)
        {
            mbedtls_free(N);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_sub_abs_orig(X, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_SUB_ABS_ALT */

#if defined(MBEDTLS_MPI_MUL_MPI_ALT)

/* Access to original version of mbedtls_mpi_mul_mpi function. */
int mbedtls_mpi_mul_mpi_orig(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B);

/*
 * Baseline multiplication: X = A * B  (HAC 14.12)
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_mul_mpi(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);
    pkha_size_t sizeN = FREESCALE_PKHA_INT_MAX_BYTES;

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    /*
     * Should be "if ((sizeA + sizeB) <= sizeN)", but if the multiplication result
     * would be maximum LTC number (the same value as the modulus N below),
     * zero would be returned instead, which is wrong value.
     */
    if ((sizeA + sizeB) < sizeN)
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        int sign = A->s * B->s;

        uint8_t * N    = mbedtls_calloc(4, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrA = N + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == N)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        memset(N, 0xFF, sizeN);

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(B, ptrB, sizeB);
        ltc_reverse_array(ptrB, sizeB);

        /*
         * Modular multiplication operation is used here. Since the modulus N is larger
         * than the expected result of A * B, the effect is normal multiplication.
         * TODO use PKHA MUL_IM_OM instead.
         */
        ret = (int) LTC_PKHA_ModMul(LTC_INSTANCE, ptrA, sizeA, ptrB, sizeB, N, sizeN, ptrC, &sizeC, kLTC_PKHA_IntegerArith,
                                    kLTC_PKHA_NormalValue, kLTC_PKHA_NormalValue, kLTC_PKHA_TimingEqualized);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrC, sizeC);
        mbedtls_mpi_read_binary(X, ptrC, sizeC);
        X->s = sign;
    cleanup:
        if (N)
        {
            mbedtls_free(N);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_mul_mpi_orig(X, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_mul_mpi(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);
    pkha_size_t sizeN = FREESCALE_PKHA_INT_MAX_BYTES;

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    /*
     * Should be "if ((sizeA + sizeB) <= sizeN)", but if the multiplication result
     * would be maximum CAAM number (the same value as the modulus N below),
     * zero would be returned instead, which is wrong value.
     */
    if ((sizeA + sizeB) < sizeN)
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        int sign = A->s * B->s;

        uint8_t * N = mbedtls_calloc(4, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrA = N + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == N)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        memset(N, 0xFF, sizeN);

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        mbedtls_mpi_write_binary(B, ptrB, sizeB);

        /*
         * Modular multiplication operation is used here. Since the modulus N is larger
         * than the expected result of A * B, the effect is normal multiplication.
         * TODO use PKHA MUL_IM_OM instead.
         */
        ret = (int) CAAM_PKHA_ModMul(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrB, sizeB, N, sizeN, ptrC, &sizeC,
                                     kCAAM_PKHA_IntegerArith, kCAAM_PKHA_NormalValue, kCAAM_PKHA_NormalValue,
                                     kCAAM_PKHA_TimingEqualized);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(X, ptrC, sizeC);
        X->s = sign;
    cleanup:
        if (N)
        {
            mbedtls_free(N);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_mul_mpi_orig(X, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_MUL_MPI_ALT */

#if defined(MBEDTLS_MPI_MOD_MPI_ALT)

/* Access to original version of mbedtls_mpi_mod_mpi function. */
int mbedtls_mpi_mod_mpi_orig(mbedtls_mpi * R, const mbedtls_mpi * A, const mbedtls_mpi * B);

/*
 * Modulo: R = A mod B
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_mod_mpi(mbedtls_mpi * R, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeA <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeB <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        int sign       = A->s;
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(B, ptrB, sizeB);
        ltc_reverse_array(ptrB, sizeB);

        ret = (int) LTC_PKHA_ModRed(LTC_INSTANCE, ptrA, sizeA, ptrB, sizeB, ptrC, &sizeC, kLTC_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrC, sizeC);
        mbedtls_mpi_read_binary(R, ptrC, sizeC);
        R->s = sign;

        while (mbedtls_mpi_cmp_int(R, 0) < 0)
            mbedtls_mpi_add_mpi(R, R, B); /* MBEDTLS_MPI_CHK( mbedtls_mpi_add_mpi( R, R, B ) ); */

        while (mbedtls_mpi_cmp_mpi(R, B) >= 0)
            mbedtls_mpi_sub_mpi(R, R, B); /* MBEDTLS_MPI_CHK( mbedtls_mpi_sub_mpi( R, R, B ) ); cleanup:*/
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_mod_mpi_orig(R, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_mod_mpi(mbedtls_mpi * R, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeA <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeB <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        int sign = A->s;
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        mbedtls_mpi_write_binary(B, ptrB, sizeB);

        ret = (int) CAAM_PKHA_ModRed(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrB, sizeB, ptrC, &sizeC, kCAAM_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(R, ptrC, sizeC);
        R->s = sign;

        while (mbedtls_mpi_cmp_int(R, 0) < 0)
            mbedtls_mpi_add_mpi(R, R, B); /* MBEDTLS_MPI_CHK( mbedtls_mpi_add_mpi( R, R, B ) ); */

        while (mbedtls_mpi_cmp_mpi(R, B) >= 0)
            mbedtls_mpi_sub_mpi(R, R, B); /* MBEDTLS_MPI_CHK( mbedtls_mpi_sub_mpi( R, R, B ) ); cleanup:*/
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_mod_mpi_orig(R, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_MOD_MPI_ALT */

#if defined(MBEDTLS_MPI_EXP_MOD_ALT)

/* Access to original version of mbedtls_mpi_exp_mod function. */
int mbedtls_mpi_exp_mod_orig(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * E, const mbedtls_mpi * N,
                             mbedtls_mpi * _RR);

/*
 * Sliding-window exponentiation: X = A^E mod N  (HAC 14.85)
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_exp_mod(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * E, const mbedtls_mpi * N, mbedtls_mpi * _RR)
{
    int ret;
    pkha_size_t sizeE = mbedtls_mpi_size(E);
    pkha_size_t sizeN = mbedtls_mpi_size(N);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeE <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeN <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif                    /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        mbedtls_mpi * AA; // TODO rename etc.

        /*
         * If number is greater than modulus, we must first reduce it due to LTC requirement
         * on modular exponentiaton that it needs number less than modulus.
         * We can take advantage of modular arithmetic rule that: A^B mod C = ( (A mod C)^B ) mod C.
         * So we do (A mod N) first and if the size of A in bytes fits into LTC, it will be done in LTC
         * (here LTC does not give size requirement on A versus N), otherwise it will be done in SW
         * and since the size of N fits into LTC, the result of (A mod N) will also fit into LTC.
         * Then we can do modular exponentiation in LTC.
         */
        if (mbedtls_mpi_cmp_mpi(A, N) >= 0)
        {
            /* A >= N, perform X = (A mod N). */
            ret = mbedtls_mpi_mod_mpi(X, A, N);

            if (ret != kStatus_Success)
                return (MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

            /* Exponenciation will be performed with X. */
            AA = X;
        }
        else
        {
            /* Exponentiation will be performed with original A. */
            AA = (mbedtls_mpi *) A;
        }

        pkha_size_t sizeA = mbedtls_mpi_size(AA);
        uint8_t * ptrA    = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrE    = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrN    = ptrE + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(AA, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(E, ptrE, sizeE);
        ltc_reverse_array(ptrE, sizeE);

        mbedtls_mpi_write_binary(N, ptrN, sizeN);
        ltc_reverse_array(ptrN, sizeN);

        ret = (int) LTC_PKHA_ModExp(LTC_INSTANCE, ptrA, sizeA, ptrN, sizeN, ptrE, sizeE, ptrN, &sizeN, kLTC_PKHA_IntegerArith,
                                    kLTC_PKHA_NormalValue, kLTC_PKHA_TimingEqualized);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrN, sizeN);
        mbedtls_mpi_read_binary(X, ptrN, sizeN);
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_exp_mod_orig(X, A, E, N, _RR);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_exp_mod(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * E, const mbedtls_mpi * N, mbedtls_mpi * _RR)
{
    int ret;
    pkha_size_t sizeE = mbedtls_mpi_size(E);
    pkha_size_t sizeN = mbedtls_mpi_size(N);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeE <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeN <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        mbedtls_mpi * AA; // TODO rename etc.

        /*
         * If number is greater than modulus, we must first reduce it due to CAAM requirement
         * on modular exponentiaton that it needs number less than modulus.
         * We can take advantage of modular arithmetic rule that: A^B mod C = ( (A mod C)^B ) mod C.
         * So we do (A mod N) first and if the size of A in bytes fits into CAAM, it will be done in CAAM
         * (here CAAM does not give size requirement on A versus N), otherwise it will be done in SW
         * and since the size of N fits into CAAM, the result of (A mod N) will also fit into CAAM.
         * Then we can do modular exponentiation in CAAM.
         */
        if (mbedtls_mpi_cmp_mpi(A, N) >= 0)
        {
            /* A >= N, perform X = (A mod N). */
            ret = mbedtls_mpi_mod_mpi(X, A, N);

            if (ret != kStatus_Success)
                return (MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

            /* Exponenciation will be performed with X. */
            AA = X;
        }
        else
        {
            /* Exponentiation will be performed with original A. */
            AA = (mbedtls_mpi *) A;
        }

        pkha_size_t sizeA = mbedtls_mpi_size(AA);
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrE = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrN = ptrE + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(AA, ptrA, sizeA);
        mbedtls_mpi_write_binary(E, ptrE, sizeE);
        mbedtls_mpi_write_binary(N, ptrN, sizeN);

        ret = (int) CAAM_PKHA_ModExp(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrN, sizeN, ptrE, sizeE, ptrN, &sizeN,
                                     kCAAM_PKHA_IntegerArith, kCAAM_PKHA_NormalValue, kCAAM_PKHA_TimingEqualized);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(X, ptrN, sizeN);
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_exp_mod_orig(X, A, E, N, _RR);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_EXP_MOD_ALT */

#if defined(MBEDTLS_MPI_GCD_ALT)

/* Access to original version of mbedtls_mpi_gcd function. */
int mbedtls_mpi_gcd_orig(mbedtls_mpi * G, const mbedtls_mpi * A, const mbedtls_mpi * B);

/*
 * Greatest common divisor: G = gcd(A, B)  (HAC 14.54)
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_gcd(mbedtls_mpi * G, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeA <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeB <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(B, ptrB, sizeB);
        ltc_reverse_array(ptrB, sizeB);

        if (mbedtls_mpi_cmp_mpi(A, B) >= 0)
        {
            ret = (int) LTC_PKHA_ModRed(LTC_INSTANCE, ptrA, sizeA, ptrB, sizeB, ptrA, &sizeA, kLTC_PKHA_IntegerArith);

            if (ret != kStatus_Success)
                CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }

        ret = (int) LTC_PKHA_GCD(LTC_INSTANCE, ptrA, sizeA, ptrB, sizeB, ptrC, &sizeC, kLTC_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrC, sizeC);
        mbedtls_mpi_read_binary(G, ptrC, sizeC);
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_gcd_orig(G, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_gcd(mbedtls_mpi * G, const mbedtls_mpi * A, const mbedtls_mpi * B)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeB = mbedtls_mpi_size(B);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeA <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeB <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrB = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrB + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        mbedtls_mpi_write_binary(B, ptrB, sizeB);

        if (mbedtls_mpi_cmp_mpi(A, B) >= 0)
        {
            ret = (int) CAAM_PKHA_ModRed(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrB, sizeB, ptrA, &sizeA,
                                         kCAAM_PKHA_IntegerArith);

            if (ret != kStatus_Success)
                CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }

        ret = (int) CAAM_PKHA_ModGcd(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrB, sizeB, ptrC, &sizeC, kCAAM_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(G, ptrC, sizeC);
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_gcd_orig(G, A, B);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_GCD_ALT */

#if defined(MBEDTLS_MPI_INV_MOD_ALT)

/* Access to original version of mbedtls_mpi_inv_mod function. */
int mbedtls_mpi_inv_mod_orig(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * N);

/*
 * Modular inverse: X = A^-1 mod N  (HAC 14.61 / 14.64)
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_inv_mod(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * N)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeN = mbedtls_mpi_size(N);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeA <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeN <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrN = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrN + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        /* N cannot be negative */
        if (N->s < 0 || mbedtls_mpi_cmp_int(N, 0) == 0)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_BAD_INPUT_DATA);
        }

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        ltc_reverse_array(ptrA, sizeA);

        mbedtls_mpi_write_binary(N, ptrN, sizeN);
        ltc_reverse_array(ptrN, sizeN);

        if (mbedtls_mpi_cmp_mpi(A, N) >= 0)
        {
            ret = (int) LTC_PKHA_ModRed(LTC_INSTANCE, ptrA, sizeA, ptrN, sizeN, ptrA, &sizeA, kLTC_PKHA_IntegerArith);

            if (ret != kStatus_Success)
                CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }

        ret = (int) LTC_PKHA_ModInv(LTC_INSTANCE, ptrA, sizeA, ptrN, sizeN, ptrC, &sizeC, kLTC_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        ltc_reverse_array(ptrC, sizeC);
        mbedtls_mpi_read_binary(X, ptrC, sizeC);
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_inv_mod_orig(X, A, N);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_inv_mod(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * N)
{
    pkha_size_t sizeA = mbedtls_mpi_size(A);
    pkha_size_t sizeN = mbedtls_mpi_size(N);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if ((sizeA <= FREESCALE_PKHA_INT_MAX_BYTES) && (sizeN <= FREESCALE_PKHA_INT_MAX_BYTES))
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        pkha_size_t sizeC;
        uint8_t * ptrA = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
        uint8_t * ptrN = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;
        uint8_t * ptrC = ptrN + FREESCALE_PKHA_INT_MAX_BYTES;
        if (NULL == ptrA)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        /* N cannot be negative */
        if (N->s < 0 || mbedtls_mpi_cmp_int(N, 0) == 0)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_BAD_INPUT_DATA);
        }

        mbedtls_mpi_write_binary(A, ptrA, sizeA);
        mbedtls_mpi_write_binary(N, ptrN, sizeN);

        if (mbedtls_mpi_cmp_mpi(A, N) >= 0)
        {
            ret = (int) CAAM_PKHA_ModRed(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrN, sizeN, ptrA, &sizeA,
                                         kCAAM_PKHA_IntegerArith);

            if (ret != kStatus_Success)
                CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }

        ret = (int) CAAM_PKHA_ModInv(CAAM_INSTANCE, &s_caamHandle, ptrA, sizeA, ptrN, sizeN, ptrC, &sizeC, kCAAM_PKHA_IntegerArith);

        if (ret != kStatus_Success)
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);

        mbedtls_mpi_read_binary(X, ptrC, sizeC);
    cleanup:
        if (ptrA)
        {
            mbedtls_free(ptrA);
        }
        return (ret);
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_inv_mod_orig(X, A, N);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_INV_MOD_ALT */

#if defined(MBEDTLS_MPI_IS_PRIME_ALT)

/* Access to original version of mbedtls_mpi_is_prime function. */
int mbedtls_mpi_is_prime_orig(const mbedtls_mpi * X, int (*f_rng)(void *, unsigned char *, size_t), void * p_rng);

/*
 * Pseudo-primality test: small factors, then Miller-Rabin
 */
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int mbedtls_mpi_is_prime(const mbedtls_mpi * X, int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    pkha_size_t sizeX = mbedtls_mpi_size(X);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if (sizeX <= FREESCALE_PKHA_INT_MAX_BYTES)
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        int random;
        bool result    = false;
        uint8_t * ptrX = mbedtls_calloc(1, FREESCALE_PKHA_INT_MAX_BYTES);
        if (NULL == ptrX)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(X, ptrX, FREESCALE_PKHA_INT_MAX_BYTES);
        ltc_reverse_array(ptrX, FREESCALE_PKHA_INT_MAX_BYTES);

        // Get the random seed number
        f_rng(p_rng, (unsigned char *) (&random), sizeof(random));

        ret = (int) LTC_PKHA_PrimalityTest(LTC_INSTANCE, (unsigned char *) &random, sizeof(random), (const uint8_t *) "1", 1u, ptrX,
                                           sizeX, &result);

        if (ret != kStatus_Success)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }

        if (result == false)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }
    cleanup:
        if (ptrX)
        {
            mbedtls_free(ptrX);
        }
        return ret;
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_is_prime_orig(X, f_rng, p_rng);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int mbedtls_mpi_is_prime(const mbedtls_mpi * X, int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    pkha_size_t sizeX = mbedtls_mpi_size(X);

#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    if (sizeX <= FREESCALE_PKHA_INT_MAX_BYTES)
    {
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
        int ret;
        int random;
        bool result = false;
        uint8_t * ptrX = mbedtls_calloc(1, FREESCALE_PKHA_INT_MAX_BYTES);
        if (NULL == ptrX)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
        }

        mbedtls_mpi_write_binary(X, ptrX, FREESCALE_PKHA_INT_MAX_BYTES);

        // Get the random seed number
        f_rng(p_rng, (unsigned char *) (&random), sizeof(random));

        ret = (int) CAAM_PKHA_PrimalityTest(CAAM_INSTANCE, &s_caamHandle, (unsigned char *) &random, sizeof(random),
                                            (const uint8_t *) "1", 1u, ptrX, sizeX, &result);

        if (ret != kStatus_Success)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }

        if (result == false)
        {
            CLEAN_RETURN(MBEDTLS_ERR_MPI_NOT_ACCEPTABLE);
        }
    cleanup:
        if (ptrX)
        {
            mbedtls_free(ptrX);
        }
        return ret;
#if defined(FREESCALE_PKHA_LONG_OPERANDS_ENABLE)
    }
    else
    {
        return mbedtls_mpi_is_prime_orig(X, f_rng, p_rng);
    }
#endif /* FREESCALE_PKHA_LONG_OPERANDS_ENABLE */
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA || MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_MPI_IS_PRIME_ALT */

#endif /* MBEDTLS_BIGNUM_C */

#if defined(MBEDTLS_ECP_C)

#include "mbedtls/ecp.h"

#define LTC_MAX_ECC (512)
#define CAAM_MAX_ECC (528)
#define CAU3_MAX_ECC (512)

typedef enum
{
    kBigEndian    = 0U,
    kLittleEndian = 1U
} endian_t;

/* convert from mbedtls_mpi to LTC or CAAM integer, as array of bytes of size sz.
 * if mbedtls_mpi has less bytes than sz, add zero bytes at most significant byte positions.
 * This is when for example modulus is 32 bytes (P-256 curve)
 * and mbedtls_mpi has only 31 bytes, we add leading zeroes
 * so that result array has 32 bytes, same as modulus (sz).
 */
#if defined(MBEDTLS_ECP_MUL_COMB_ALT) || defined(MBEDTLS_ECP_ADD_ALT)
static int get_and_extend_mbedtls_mpi(uint8_t * dst, const mbedtls_mpi * a, size_t sz, endian_t endian)
{
    size_t szbin;
    int offset;
    int ret;

    /* check how many bytes are in the mbedtls_mpi */
    szbin = mbedtls_mpi_size(a);

    /* compute offset from dst */
    offset = sz - szbin;
    if (offset < 0)
        offset = 0;
    if (offset > sz)
        offset = sz;

    /* add leading zeroes */
    if (offset)
        memset(dst, 0, offset);

    /* convert mbedtls_mpi to array of bytes */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(a, dst + offset, szbin));

    /* reverse array for LTC direct use */
    if (endian == kLittleEndian)
        ltc_reverse_array(dst, sz);
cleanup:
    return (ret);
}

#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
static int ltc_get_from_mbedtls_mpi(uint8_t * dst, const mbedtls_mpi * a, size_t sz)
{
    return get_and_extend_mbedtls_mpi(dst, a, sz, kLittleEndian);
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
static int caam_get_from_mbedtls_mpi(uint8_t * dst, const mbedtls_mpi * a, size_t sz)
{
    return get_and_extend_mbedtls_mpi(dst, a, sz, kBigEndian);
}
#endif /* MBEDTLS_FREESCALE_LTC_PKHA */
#endif /* MBEDTLS_ECP_MUL_COMB_ALT || MBEDTLS_ECP_ADD_ALT */

/*
 * Multiplication using the comb method,
 * for curves in short Weierstrass form
 */
#if defined(MBEDTLS_ECP_MUL_COMB_ALT)
#if defined(MBEDTLS_FREESCALE_LTC_PKHA)
int ecp_mul_comb(mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_mpi * m, const mbedtls_ecp_point * P,
                 int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    bool is_inf;
    size_t size;
    size_t size_bin;
    int sign = m->s;

    ltc_pkha_ecc_point_t A;
    ltc_pkha_ecc_point_t result;

    /* Allocate 7 elements with size of (LTC_MAX_ECC / 8) plus ptrE with size of FREESCALE_PKHA_INT_MAX_BYTES */
    uint8_t * ptrAX     = mbedtls_calloc((7 * (LTC_MAX_ECC / 8)) + FREESCALE_PKHA_INT_MAX_BYTES, 1);
    uint8_t * ptrAY     = ptrAX + (LTC_MAX_ECC / 8);
    uint8_t * ptrRX     = ptrAY + (LTC_MAX_ECC / 8);
    uint8_t * ptrRY     = ptrRX + (LTC_MAX_ECC / 8);
    uint8_t * ptrN      = ptrRY + (LTC_MAX_ECC / 8);
    uint8_t * ptrParamA = ptrN + (LTC_MAX_ECC / 8);
    uint8_t * ptrParamB = ptrParamA + (LTC_MAX_ECC / 8);
    uint8_t * ptrE      = ptrParamB + (LTC_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    A.X      = ptrAX;
    A.Y      = ptrAY;
    result.X = ptrRX;
    result.Y = ptrRY;
    size     = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (LTC_MAX_ECC / 8) || (mbedtls_mpi_get_bit(&grp->N, 0) != 1))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(A.X, &P->X, size));
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(A.Y, &P->Y, size));
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(ptrParamA, &grp->A, size));
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(ptrParamB, &grp->B, size));

    /* scalar multiplier integer of any size */
    size_bin = mbedtls_mpi_size(m);
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(m, ptrE, size_bin));
    ltc_reverse_array(ptrE, size_bin);

    /* modulus */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&grp->P, ptrN, size));
    ltc_reverse_array(ptrN, size);

    /* Multiply */
    LTC_PKHA_ECC_PointMul(LTC_INSTANCE, &A, ptrE, size_bin, ptrN, NULL, ptrParamA, ptrParamB, size, kLTC_PKHA_TimingEqualized,
                          kLTC_PKHA_IntegerArith, &result, &is_inf);
    /* Convert result */
    ltc_reverse_array(ptrRX, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    ltc_reverse_array(ptrRY, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->Y, ptrRY, size));
    /* if the integer multiplier is negative, the computation happens with abs() value
     * and the result (x,y) is changed to (x, -y)
     */
    R->Y.s = sign;
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int ecp_mul_comb(mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_mpi * m, const mbedtls_ecp_point * P,
                 int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    size_t size;
    size_t size_bin;
    int sign = m->s;

    caam_pkha_ecc_point_t A;
    caam_pkha_ecc_point_t result;

    /* Allocate 7 elements with size of (CAAM_MAX_ECC / 8) plus ptrE with size of FREESCALE_PKHA_INT_MAX_BYTES */
    uint8_t * ptrAX = mbedtls_calloc((7 * (CAAM_MAX_ECC / 8)) + FREESCALE_PKHA_INT_MAX_BYTES, 1);
    uint8_t * ptrAY = ptrAX + (CAAM_MAX_ECC / 8);
    uint8_t * ptrRX = ptrAY + (CAAM_MAX_ECC / 8);
    uint8_t * ptrRY = ptrRX + (CAAM_MAX_ECC / 8);
    uint8_t * ptrN = ptrRY + (CAAM_MAX_ECC / 8);
    uint8_t * ptrParamA = ptrN + (CAAM_MAX_ECC / 8);
    uint8_t * ptrParamB = ptrParamA + (CAAM_MAX_ECC / 8);
    uint8_t * ptrE = ptrParamB + (CAAM_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    A.X = ptrAX;
    A.Y = ptrAY;
    result.X = ptrRX;
    result.Y = ptrRY;
    size = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (CAAM_MAX_ECC / 8) || (mbedtls_mpi_get_bit(&grp->N, 0) != 1))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(A.X, &P->X, size));
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(A.Y, &P->Y, size));
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(ptrParamA, &grp->A, size));
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(ptrParamB, &grp->B, size));

    /* scalar multiplier integer of any size */
    size_bin = mbedtls_mpi_size(m);
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(m, ptrE, size_bin));

    /* modulus */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&grp->P, ptrN, size));

    /* Multiply */
    CAAM_PKHA_ECC_PointMul(CAAM_INSTANCE, &s_caamHandle, &A, ptrE, size_bin, ptrN, NULL, ptrParamA, ptrParamB, size,
                           kCAAM_PKHA_TimingEqualized, kCAAM_PKHA_IntegerArith, &result);
    /* Convert result */
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->Y, ptrRY, size));
    /* if the integer multiplier is negative, the computation happens with abs() value
     * and the result (x,y) is changed to (x, -y)
     */
    R->Y.s = sign;
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#elif defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int ecp_mul_comb(mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_mpi * m, const mbedtls_ecp_point * P,
                 int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    status_t status;
    size_t size;
    size_t size_bin;
    int sign = m->s;

    cau3_pkha_ecc_point_t A;
    cau3_pkha_ecc_point_t result;

    /* Allocate 7 elements with size of (CAU3_MAX_ECC / 8) plus ptrE with size of FREESCALE_PKHA_INT_MAX_BYTES */
    uint8_t * ptrAX = mbedtls_calloc((7 * (CAU3_MAX_ECC / 8)) + FREESCALE_PKHA_INT_MAX_BYTES, 1);
    uint8_t * ptrAY = ptrAX + (CAU3_MAX_ECC / 8);
    uint8_t * ptrRX = ptrAY + (CAU3_MAX_ECC / 8);
    uint8_t * ptrRY = ptrRX + (CAU3_MAX_ECC / 8);
    uint8_t * ptrN = ptrRY + (CAU3_MAX_ECC / 8);
    uint8_t * ptrParamA = ptrN + (CAU3_MAX_ECC / 8);
    uint8_t * ptrParamB = ptrParamA + (CAU3_MAX_ECC / 8);
    uint8_t * ptrE = ptrParamB + (CAU3_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    A.X = ptrAX;
    A.Y = ptrAY;
    result.X = ptrRX;
    result.Y = ptrRY;
    size = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (CAU3_MAX_ECC / 8) || (mbedtls_mpi_get_bit(&grp->N, 0) != 1))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(A.X, &P->X, size));
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(A.Y, &P->Y, size));
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(ptrParamA, &grp->A, size));
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(ptrParamB, &grp->B, size));

    /* scalar multiplier integer of any size */
    size_bin = mbedtls_mpi_size(m);
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(m, ptrE, size_bin));
    cau3_reverse_array(ptrE, size_bin);

    /* modulus */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&grp->P, ptrN, size));
    cau3_reverse_array(ptrN, size);

    /* Multiply */
    status = CAU3_PKHA_ECC_PointMul(CAU3, &A, ptrE, size_bin, ptrN, NULL, ptrParamA, ptrParamB, size, kCAU3_PKHA_TimingEqualized,
                                    kCAU3_PKHA_IntegerArith, &result);

    if (status != kStatus_Success)
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert result */
    cau3_reverse_array(ptrRX, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    cau3_reverse_array(ptrRY, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->Y, ptrRY, size));
    /* if the integer multiplier is negative, the computation happens with abs() value
     * and the result (x,y) is changed to (x, -y)
     */
    R->Y.s = sign;
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#endif /* MBEDTLS_FREESCALE_LTC_PKHA */
#endif /* MBEDTLS_ECP_MUL_COMB_ALT */

/*
 * Curve types: internal for now, might be exposed later
 */
typedef enum
{
    ECP_TYPE_NONE = 0,
    ECP_TYPE_SHORT_WEIERSTRASS, /* y^2 = x^3 + a x + b      */
    ECP_TYPE_MONTGOMERY,        /* y^2 = x^3 + a x^2 + x    */
} ecp_curve_type;
/*
 * Get the type of a curve
 */
static inline ecp_curve_type ecp_get_type(const mbedtls_ecp_group * grp)
{
    if (grp->G.X.p == NULL)
        return (ECP_TYPE_NONE);

    if (grp->G.Y.p == NULL)
        return (ECP_TYPE_MONTGOMERY);
    else
        return (ECP_TYPE_SHORT_WEIERSTRASS);
}

/*
 * Addition: R = P + Q, result's coordinates normalized
 */
#if defined(MBEDTLS_ECP_ADD_ALT)
#if defined(MBEDTLS_FREESCALE_LTC_PKHA)
int ecp_add(const mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_ecp_point * P, const mbedtls_ecp_point * Q)
{
    int ret;
    size_t size;
    ltc_pkha_ecc_point_t A;
    ltc_pkha_ecc_point_t B;
    ltc_pkha_ecc_point_t result;

    uint8_t * ptrAX     = mbedtls_calloc(9, (LTC_MAX_ECC / 8));
    uint8_t * ptrAY     = ptrAX + (LTC_MAX_ECC / 8);
    uint8_t * ptrBX     = ptrAY + (LTC_MAX_ECC / 8);
    uint8_t * ptrBY     = ptrBX + (LTC_MAX_ECC / 8);
    uint8_t * ptrRX     = ptrBY + (LTC_MAX_ECC / 8);
    uint8_t * ptrRY     = ptrRX + (LTC_MAX_ECC / 8);
    uint8_t * ptrN      = ptrRY + (LTC_MAX_ECC / 8);
    uint8_t * ptrParamA = ptrN + (LTC_MAX_ECC / 8);
    uint8_t * ptrParamB = ptrParamA + (LTC_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    if (ecp_get_type(grp) != ECP_TYPE_SHORT_WEIERSTRASS)
        CLEAN_RETURN(MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE);

    A.X      = ptrAX;
    A.Y      = ptrAY;
    B.X      = ptrBX;
    B.Y      = ptrBY;
    result.X = ptrRX;
    result.Y = ptrRY;
    size     = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (LTC_MAX_ECC / 8) || (mbedtls_mpi_get_bit(&grp->P, 0) != 1))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(A.X, &P->X, size));
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(A.Y, &P->Y, size));
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(B.X, &Q->X, size));
    MBEDTLS_MPI_CHK(ltc_get_from_mbedtls_mpi(B.Y, &Q->Y, size));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&grp->P, ptrN, size));
    ltc_reverse_array(ptrN, size);
    /* Multiply */
    LTC_PKHA_ECC_PointAdd(LTC_INSTANCE, &A, &B, ptrN, NULL, ptrParamA, ptrParamB, size, kLTC_PKHA_IntegerArith, &result);
    /* Convert result */
    ltc_reverse_array(ptrRX, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    ltc_reverse_array(ptrRY, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->Y, ptrRY, size));
    R->X.s = P->X.s;
    R->Y.s = P->Y.s;
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#elif defined(MBEDTLS_FREESCALE_CAAM_PKHA)
int ecp_add(const mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_ecp_point * P, const mbedtls_ecp_point * Q)
{
    int ret;
    size_t size;
    caam_pkha_ecc_point_t A;
    caam_pkha_ecc_point_t B;
    caam_pkha_ecc_point_t result;

    uint8_t * ptrAX = mbedtls_calloc(9, (CAAM_MAX_ECC / 8));
    uint8_t * ptrAY = ptrAX + (CAAM_MAX_ECC / 8);
    uint8_t * ptrBX = ptrAY + (CAAM_MAX_ECC / 8);
    uint8_t * ptrBY = ptrBX + (CAAM_MAX_ECC / 8);
    uint8_t * ptrRX = ptrBY + (CAAM_MAX_ECC / 8);
    uint8_t * ptrRY = ptrRX + (CAAM_MAX_ECC / 8);
    uint8_t * ptrN = ptrRY + (CAAM_MAX_ECC / 8);
    uint8_t * ptrParamA = ptrN + (CAAM_MAX_ECC / 8);
    uint8_t * ptrParamB = ptrParamA + (CAAM_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    if (ecp_get_type(grp) != ECP_TYPE_SHORT_WEIERSTRASS)
        CLEAN_RETURN(MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE);

    A.X = ptrAX;
    A.Y = ptrAY;
    B.X = ptrBX;
    B.Y = ptrBY;
    result.X = ptrRX;
    result.Y = ptrRY;
    size = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (CAAM_MAX_ECC / 8) || (mbedtls_mpi_get_bit(&grp->P, 0) != 1))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(A.X, &P->X, size));
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(A.Y, &P->Y, size));
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(B.X, &Q->X, size));
    MBEDTLS_MPI_CHK(caam_get_from_mbedtls_mpi(B.Y, &Q->Y, size));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&grp->P, ptrN, size));

    /* Multiply */
    CAAM_PKHA_ECC_PointAdd(CAAM_INSTANCE, &s_caamHandle, &A, &B, ptrN, NULL, ptrParamA, ptrParamB, size, kCAAM_PKHA_IntegerArith,
                           &result);
    /* Convert result */
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->Y, ptrRY, size));
    R->X.s = P->X.s;
    R->Y.s = P->Y.s;
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#elif defined(MBEDTLS_FREESCALE_CAU3_PKHA)
int ecp_add(const mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_ecp_point * P, const mbedtls_ecp_point * Q)
{
    int ret;
    status_t status;
    size_t size;
    cau3_pkha_ecc_point_t A;
    cau3_pkha_ecc_point_t B;
    cau3_pkha_ecc_point_t result;

    uint8_t * ptrAX = mbedtls_calloc(9, (CAU3_MAX_ECC / 8));
    uint8_t * ptrAY = ptrAX + (CAU3_MAX_ECC / 8);
    uint8_t * ptrBX = ptrAY + (CAU3_MAX_ECC / 8);
    uint8_t * ptrBY = ptrBX + (CAU3_MAX_ECC / 8);
    uint8_t * ptrRX = ptrBY + (CAU3_MAX_ECC / 8);
    uint8_t * ptrRY = ptrRX + (CAU3_MAX_ECC / 8);
    uint8_t * ptrN = ptrRY + (CAU3_MAX_ECC / 8);
    uint8_t * ptrParamA = ptrN + (CAU3_MAX_ECC / 8);
    uint8_t * ptrParamB = ptrParamA + (CAU3_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    if (ecp_get_type(grp) != ECP_TYPE_SHORT_WEIERSTRASS)
        CLEAN_RETURN(MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE);

    A.X = ptrAX;
    A.Y = ptrAY;
    B.X = ptrBX;
    B.Y = ptrBY;
    result.X = ptrRX;
    result.Y = ptrRY;
    size = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (CAU3_MAX_ECC / 8) || (mbedtls_mpi_get_bit(&grp->P, 0) != 1))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(A.X, &P->X, size));
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(A.Y, &P->Y, size));
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(B.X, &Q->X, size));
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(B.Y, &Q->Y, size));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&grp->P, ptrN, size));
    cau3_reverse_array(ptrN, size);
    /* Multiply */
    status = CAU3_PKHA_ECC_PointAdd(CAU3, &A, &B, ptrN, NULL, ptrParamA, ptrParamB, size, kCAU3_PKHA_IntegerArith, &result);

    if (status != kStatus_Success)
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert result */
    cau3_reverse_array(ptrRX, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    cau3_reverse_array(ptrRY, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->Y, ptrRY, size));
    R->X.s = P->X.s;
    R->Y.s = P->Y.s;
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#endif /* MBEDTLS_FREESCALE_LTC_PKHA */

#endif /* MBEDTLS_ECP_ADD_ALT */

#if defined(MBEDTLS_ECP_MUL_MXZ_ALT)
#if defined(MBEDTLS_FREESCALE_CAU3_PKHA)

/* curve25519 params - in little endian for CAU3 */
static const uint8_t s_curve25519_A24[] = { 0x42, 0xdb, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const uint8_t s_curve25519_N[] = { 0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f };

static const uint8_t s_curve25519_R2modN[] = { 0xa4, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int ecp_mul_mxz(mbedtls_ecp_group * grp, mbedtls_ecp_point * R, const mbedtls_mpi * m, const mbedtls_ecp_point * P,
                int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret;
    status_t status;
    size_t size;
    size_t size_bin;

    cau3_pkha_ecc_point_t A;
    cau3_pkha_ecc_point_t result;

    /* Allocate 2 elements with size of (CAU3_MAX_ECC / 8) plus ptrE with size of FREESCALE_PKHA_INT_MAX_BYTES */
    uint8_t * ptrAX = mbedtls_calloc((2 * (CAU3_MAX_ECC / 8)) + FREESCALE_PKHA_INT_MAX_BYTES, 1);
    uint8_t * ptrRX = ptrAX + (CAU3_MAX_ECC / 8);
    uint8_t * ptrE  = ptrRX + (CAU3_MAX_ECC / 8);
    if (NULL == ptrAX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    A.X      = ptrAX;
    result.X = ptrRX;
    size     = mbedtls_mpi_size(&grp->P);
    if (mbedtls_mpi_size(&P->X) > (CAAM_MAX_ECC / 8))
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert multi precision integers to arrays */
    MBEDTLS_MPI_CHK(cau3_get_from_mbedtls_mpi(A.X, &P->X, size));

    /* scalar multiplier integer of any size */
    size_bin = mbedtls_mpi_size(m);
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(m, ptrE, size_bin));
    cau3_reverse_array(ptrE, size_bin);

    /* Multiply */
    status = CAU3_PKHA_ECM_PointMul(CAU3, ptrE, size_bin, A.X, s_curve25519_A24, s_curve25519_N, s_curve25519_R2modN, size,
                                    kCAU3_PKHA_TimingEqualized, result.X);

    if (status != kStatus_Success)
    {
        CLEAN_RETURN(MBEDTLS_ERR_ECP_BAD_INPUT_DATA);
    }

    /* Convert result */
    cau3_reverse_array(ptrRX, size);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&R->X, ptrRX, size));
    MBEDTLS_MPI_CHK(mbedtls_mpi_lset(&R->Z, 1));

cleanup:
    if (ptrAX)
    {
        mbedtls_free(ptrAX);
    }
    return (ret);
}

#endif /* MBEDTLS_FREESCALE_CAU3_PKHA */
#endif /* MBEDTLS_ECP_MUL_MXZ_ALT */

#endif /* MBEDTLS_ECP_C */

#endif /* MBEDTLS_FREESCALE_LTC_PKHA */

#if defined(MBEDTLS_RSA_PUBLIC_ALT)
#if defined(MBEDTLS_FREESCALE_CASPER_PKHA)

#if defined(MBEDTLS_PLATFORM_C)
#include "mbedtls/platform.h"
#else
#include <stdio.h>
#define mbedtls_calloc calloc
#define mbedtls_free free
#endif

#include "mbedtls/bignum.h"
#include "mbedtls/rsa.h"

static void reverse_array(uint8_t * src, size_t src_len)
{
    int i;

    for (i = 0; i < src_len / 2; i++)
    {
        uint8_t tmp;

        tmp                  = src[i];
        src[i]               = src[src_len - 1 - i];
        src[src_len - 1 - i] = tmp;
    }
}
/*
 * Do an RSA public key operation
 */
static int mbedtls_mpi_exp_mod_shim(mbedtls_mpi * X, const mbedtls_mpi * A, const mbedtls_mpi * E,
                                    const mbedtls_mpi * N /*, mbedtls_mpi *_RR */)
{
    int ret        = MBEDTLS_ERR_MPI_ALLOC_FAILED;
    size_t sizeA   = mbedtls_mpi_size(A);
    size_t sizeN   = mbedtls_mpi_size(N);
    uint8_t * ptrX = mbedtls_calloc(3, FREESCALE_PKHA_INT_MAX_BYTES);
    uint8_t * ptrA = ptrX + FREESCALE_PKHA_INT_MAX_BYTES;
    uint8_t * ptrN = ptrA + FREESCALE_PKHA_INT_MAX_BYTES;

    if (NULL == ptrX)
    {
        CLEAN_RETURN(MBEDTLS_ERR_MPI_ALLOC_FAILED);
    }

    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(A, ptrA, sizeA));
    reverse_array(ptrA, sizeA);

    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(N, ptrN, sizeN));
    reverse_array(ptrN, sizeN);

    CASPER_ModExp(CASPER, ptrA, ptrN, sizeN / 4, E->p[0], ptrX);

    reverse_array(ptrX, sizeN);
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(X, ptrX, sizeN));
cleanup:
    if (ptrX != NULL)
    {
        mbedtls_free(ptrX);
    }

    return ret;
}

int mbedtls_rsa_public(mbedtls_rsa_context * ctx, const unsigned char * input, unsigned char * output)
{
    int ret;
    size_t olen;
    mbedtls_mpi T;

    mbedtls_mpi_init(&T);

#if defined(MBEDTLS_THREADING_C)
    if ((ret = mbedtls_mutex_lock(&ctx->mutex)) != 0)
        return (ret);
#endif

    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&T, input, ctx->len));

    if (mbedtls_mpi_cmp_mpi(&T, &ctx->N) >= 0)
    {
        ret = MBEDTLS_ERR_MPI_BAD_INPUT_DATA;
        goto cleanup;
    }

    olen = ctx->len;

    MBEDTLS_MPI_CHK(mbedtls_mpi_exp_mod_shim(&T, &T, &ctx->E, &ctx->N /*, &ctx->RN */));

    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&T, output, olen));

cleanup:
#if defined(MBEDTLS_THREADING_C)
    if (mbedtls_mutex_unlock(&ctx->mutex) != 0)
        return (MBEDTLS_ERR_THREADING_MUTEX_ERROR);
#endif

    mbedtls_mpi_free(&T);

    if (ret != 0)
        return (MBEDTLS_ERR_RSA_PUBLIC_FAILED + ret);

    return (0);
}

#endif /* MBEDTLS_FREESCALE_CASPER_PKHA */
#endif /* MBEDTLS_RSA_PUBLIC_ALT */

/******************************************************************************/
/*************************** MD5 **********************************************/
/******************************************************************************/

#if defined(MBEDTLS_MD5_C)

#if defined(MBEDTLS_FREESCALE_MMCAU_MD5)

#include "mbedtls/md5.h"

int mbedtls_internal_md5_process(mbedtls_md5_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = MMCAU_MD5_HashN(data, 1, ctx->state);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_MD5_HW_ACCEL_FAILED;
    }
    return 0;
}

#endif /* MBEDTLS_FREESCALE_MMCAU_MD5 */

#endif /* MBEDTLS_MD5_C */

/******************************************************************************/
/*************************** SHA1 *********************************************/
/******************************************************************************/

#if defined(MBEDTLS_SHA1_C)

#if defined(MBEDTLS_FREESCALE_LTC_SHA1)
#include "mbedtls/sha1.h"

void mbedtls_sha1_init(mbedtls_sha1_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context * dst, const mbedtls_sha1_context * src)
{
    memcpy(dst, src, sizeof(mbedtls_sha1_context));
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret(mbedtls_sha1_context * ctx)
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Init(LTC_INSTANCE, ctx, kLTC_Sha1, NULL, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Update(ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Update(ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Finish(ctx, output, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_MMCAU_SHA1)

#include "mbedtls/sha1.h"

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = MMCAU_SHA1_HashN(data, 1, ctx->state);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_LPC_SHA1)
#include "mbedtls/sha1.h"

void mbedtls_sha1_init(mbedtls_sha1_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context * dst, const mbedtls_sha1_context * src)
{
    memcpy(dst, src, sizeof(mbedtls_sha1_context));
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret(mbedtls_sha1_context * ctx)
{
    status_t ret = kStatus_Fail;
    ret          = SHA_Init(SHA_INSTANCE, ctx, kSHA_Sha1);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = SHA_Update(SHA_INSTANCE, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = SHA_Update(SHA_INSTANCE, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    size_t outputSize = 20u;
    status_t ret      = kStatus_Fail;
    ret               = SHA_Finish(SHA_INSTANCE, ctx, output, &outputSize);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}
#elif defined(MBEDTLS_FREESCALE_CAAM_SHA1)
#include "mbedtls/sha1.h"

void mbedtls_sha1_init(mbedtls_sha1_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context * dst, const mbedtls_sha1_context * src)
{
    memcpy(dst, src, sizeof(mbedtls_sha1_context));
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret(mbedtls_sha1_context * ctx)
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Init(CAAM_INSTANCE, &s_caamHandle, ctx, kCAAM_Sha1, NULL, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Update(ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Update(ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Finish(ctx, output, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_CAU3_SHA1)
#include "mbedtls/sha1.h"

void mbedtls_sha1_init(mbedtls_sha1_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context * dst, const mbedtls_sha1_context * src)
{
    memcpy(dst, src, sizeof(mbedtls_sha1_context));
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret(mbedtls_sha1_context * ctx)
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Init(CAU3, ctx, kCAU3_Sha1);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Update(CAU3, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Update(CAU3, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Finish(CAU3, ctx, output, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_DCP_SHA1)
#include "mbedtls/sha1.h"

void mbedtls_sha1_init(mbedtls_sha1_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context * dst, const mbedtls_sha1_context * src)
{
    memcpy(dst, src, sizeof(mbedtls_sha1_context));
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret(mbedtls_sha1_context * ctx)
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Init(DCP, &s_dcpHandle, ctx, kDCP_Sha1);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Update(DCP, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Update(DCP, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Finish(DCP, ctx, output, NULL);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_HASHCRYPT_SHA1)
#include "mbedtls/sha1.h"

void mbedtls_sha1_init(mbedtls_sha1_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_free(mbedtls_sha1_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha1_context));
}

void mbedtls_sha1_clone(mbedtls_sha1_context * dst, const mbedtls_sha1_context * src)
{
    memcpy(dst, src, sizeof(mbedtls_sha1_context));
}

/*
 * SHA-1 context setup
 */
int mbedtls_sha1_starts_ret(mbedtls_sha1_context * ctx)
{
    status_t ret = kStatus_Fail;
    ret          = HASHCRYPT_SHA_Init(HASHCRYPT, ctx, kHASHCRYPT_Sha1);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = HASHCRYPT_SHA_Update(HASHCRYPT, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 process buffer
 */
int mbedtls_sha1_update_ret(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = HASHCRYPT_SHA_Update(HASHCRYPT, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-1 final digest
 */
int mbedtls_sha1_finish_ret(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    status_t ret      = kStatus_Fail;
    size_t outputSize = 20;
    ret               = HASHCRYPT_SHA_Finish(HASHCRYPT, ctx, output, &outputSize);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA1_HW_ACCEL_FAILED;
    }
    return 0;
}
#endif /* MBEDTLS_FREESCALE_LPC_SHA1 */
#if !defined(MBEDTLS_DEPRECATED_REMOVED) && defined(MBEDTLS_SHA1_ALT)
#include "mbedtls/sha1.h"

void mbedtls_sha1_starts(mbedtls_sha1_context * ctx)
{
    mbedtls_sha1_starts_ret(ctx);
}

void mbedtls_sha1_update(mbedtls_sha1_context * ctx, const unsigned char * input, size_t ilen)
{
    mbedtls_sha1_update_ret(ctx, input, ilen);
}

void mbedtls_sha1_finish(mbedtls_sha1_context * ctx, unsigned char output[20])
{
    mbedtls_sha1_finish_ret(ctx, output);
}

void mbedtls_sha1_process(mbedtls_sha1_context * ctx, const unsigned char data[64])
{
    mbedtls_internal_sha1_process(ctx, data);
}
#endif /* MBEDTLS_DEPRECATED_REMOVED */
#endif /* MBEDTLS_SHA1_C */

/******************************************************************************/
/*************************** SHA256********************************************/
/******************************************************************************/

#if defined(MBEDTLS_SHA256_C)

#if defined(MBEDTLS_FREESCALE_LTC_SHA256)
#include "mbedtls/sha256.h"

void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    memcpy(dst, src, sizeof(*dst));
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    status_t ret = kStatus_Fail;
    if (is224)
    {
        ret = LTC_HASH_Init(LTC_INSTANCE, ctx, kLTC_Sha224, NULL, 0);
    }
    else
    {
        ret = LTC_HASH_Init(LTC_INSTANCE, ctx, kLTC_Sha256, NULL, 0);
    }

    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Update(ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Update(ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    status_t ret = kStatus_Fail;
    ret          = LTC_HASH_Finish(ctx, output, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_MMCAU_SHA256)

#include "mbedtls/sha256.h"

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = MMCAU_SHA256_HashN(data, 1, ctx->state);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_CAU3_SHA256)

#include "mbedtls/sha256.h"

void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    memcpy(dst, src, sizeof(*dst));
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    status_t ret = kStatus_Fail;
    if (!is224) /* SHA-224 not supported at the moment */
    {
        ret = CAU3_HASH_Init(CAU3, ctx, kCAU3_Sha256);
    }
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Update(CAU3, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Update(CAU3, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    status_t ret = kStatus_Fail;
    ret          = CAU3_HASH_Finish(CAU3, ctx, output, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_LPC_SHA256)
#include "mbedtls/sha256.h"

void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    memcpy(dst, src, sizeof(*dst));
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    int status = MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    if (!is224) /* SHA-224 not supported */
    {
        SHA256_Init(ctx);
        status = 0;
    }
    return status;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    SHA256_HashUpdate(ctx, data, 64);
    return 0;
}

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    SHA256_HashUpdate(ctx, input, ilen);

    return 0;
}

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    SHA256_HashFinish(ctx, output);
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_CAAM_SHA256)
#include "mbedtls/sha256.h"

void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    memcpy(dst, src, sizeof(*dst));
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    status_t ret = kStatus_Fail;
    if (is224)
    {
        ret = CAAM_HASH_Init(CAAM_INSTANCE, &s_caamHandle, ctx, kCAAM_Sha224, NULL, 0);
    }
    else
    {
        ret = CAAM_HASH_Init(CAAM_INSTANCE, &s_caamHandle, ctx, kCAAM_Sha256, NULL, 0);
    }
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Update(ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Update(ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    status_t ret = kStatus_Fail;
    ret          = CAAM_HASH_Finish(ctx, output, 0);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_DCP_SHA256)
#include "mbedtls/sha256.h"

void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    memcpy(dst, src, sizeof(*dst));
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    status_t ret = kStatus_Fail;
    if (!is224)
    {
        ret = DCP_HASH_Init(DCP, &s_dcpHandle, ctx, kDCP_Sha256);
    }
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Update(DCP, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Update(DCP, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    status_t ret = kStatus_Fail;
    ret          = DCP_HASH_Finish(DCP, ctx, output, NULL);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

#elif defined(MBEDTLS_FREESCALE_HASHCRYPT_SHA256)
#include "mbedtls/sha256.h"

void mbedtls_sha256_init(mbedtls_sha256_context * ctx)
{
    memset(ctx, 0, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_free(mbedtls_sha256_context * ctx)
{
    if (ctx == NULL)
        return;

    mbedtls_zeroize(ctx, sizeof(mbedtls_sha256_context));
}

void mbedtls_sha256_clone(mbedtls_sha256_context * dst, const mbedtls_sha256_context * src)
{
    memcpy(dst, src, sizeof(*dst));
}

/*
 * SHA-256 context setup
 */
int mbedtls_sha256_starts_ret(mbedtls_sha256_context * ctx, int is224)
{
    status_t ret = kStatus_Fail;
    if (!is224) /* SHA-224 not supported */
    {
        ret = HASHCRYPT_SHA_Init(HASHCRYPT, ctx, kHASHCRYPT_Sha256);
    }
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

int mbedtls_internal_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    status_t ret = kStatus_Fail;
    ret          = HASHCRYPT_SHA_Update(HASHCRYPT, ctx, data, 64);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 process buffer
 */
int mbedtls_sha256_update_ret(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    status_t ret = kStatus_Fail;
    ret          = HASHCRYPT_SHA_Update(HASHCRYPT, ctx, input, ilen);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}

/*
 * SHA-256 final digest
 */
int mbedtls_sha256_finish_ret(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    status_t ret      = kStatus_Fail;
    size_t outputSize = 32;
    ret               = HASHCRYPT_SHA_Finish(HASHCRYPT, ctx, output, &outputSize);
    if (ret != kStatus_Success)
    {
        return MBEDTLS_ERR_SHA256_HW_ACCEL_FAILED;
    }
    return 0;
}
#endif /* MBEDTLS_FREESCALE_LTC_SHA256 */
#if !defined(MBEDTLS_DEPRECATED_REMOVED) && defined(MBEDTLS_SHA256_ALT)
#include "mbedtls/sha256.h"

void mbedtls_sha256_starts(mbedtls_sha256_context * ctx, int is224)
{
    mbedtls_sha256_starts_ret(ctx, is224);
}

void mbedtls_sha256_update(mbedtls_sha256_context * ctx, const unsigned char * input, size_t ilen)
{
    mbedtls_sha256_update_ret(ctx, input, ilen);
}

void mbedtls_sha256_finish(mbedtls_sha256_context * ctx, unsigned char output[32])
{
    mbedtls_sha256_finish_ret(ctx, output);
}

void mbedtls_sha256_process(mbedtls_sha256_context * ctx, const unsigned char data[64])
{
    mbedtls_internal_sha256_process(ctx, data);
}
#endif /* MBEDTLS_DEPRECATED_REMOVED */
#endif /* MBEDTLS_SHA256_C */

/* Entropy poll callback for a hardware source */
#if defined(MBEDTLS_ENTROPY_HARDWARE_ALT)

#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#if defined CPU_JN518X
#include "fsl_rng.h"
#else
#include "fsl_trng.h"
#endif
#elif defined(FSL_FEATURE_SOC_RNG_COUNT) && (FSL_FEATURE_SOC_RNG_COUNT > 0)
#include "fsl_rnga.h"
#elif defined(FSL_FEATURE_SOC_LPC_RNG_COUNT) && (FSL_FEATURE_SOC_LPC_RNG_COUNT > 0)
#include "fsl_rng.h"
#elif defined(FSL_FEATURE_SOC_LPC_RNG1_COUNT) && (FSL_FEATURE_SOC_LPC_RNG1_COUNT > 0)
#include "fsl_rng.h"
#endif

int mbedtls_hardware_poll(void * data, unsigned char * output, size_t len, size_t * olen)
{
    status_t result = kStatus_Success;

#if defined(FSL_FEATURE_SOC_TRNG_COUNT) && (FSL_FEATURE_SOC_TRNG_COUNT > 0)
#if defined CPU_JN518X
#ifndef TRNG0
#define TRNG0 RNG
#endif
#else
#ifndef TRNG0
#define TRNG0 TRNG
#endif
#endif
    result = TRNG_GetRandomData(TRNG0, output, len);
#elif defined(FSL_FEATURE_SOC_RNG_COUNT) && (FSL_FEATURE_SOC_RNG_COUNT > 0)
    result = RNGA_GetRandomData(RNG, (void *) output, len);
#elif defined(FSL_FEATURE_SOC_CAAM_COUNT) && (FSL_FEATURE_SOC_CAAM_COUNT > 0) && defined(CRYPTO_USE_DRIVER_CAAM)
    result = CAAM_RNG_GetRandomData(CAAM_INSTANCE, &s_caamHandle, kCAAM_RngStateHandle0, output, len, kCAAM_RngDataAny, NULL);
#elif defined(FSL_FEATURE_SOC_LPC_RNG_COUNT) && (FSL_FEATURE_SOC_LPC_RNG_COUNT > 0)
    uint32_t rn;
    size_t length;
    int i;

    length = len;

    while (length > 0)
    {
        rn = RNG_GetRandomData();

        if (length >= sizeof(uint32_t))
        {
            memcpy(output, &rn, sizeof(uint32_t));
            length -= sizeof(uint32_t);
            output += sizeof(uint32_t);
        }
        else
        {
            memcpy(output, &rn, length);
            output += length;
            len = 0U;
        }

        /* Discard next 32 random words for better entropy */
        for (i = 0; i < 32; i++)
        {
            RNG_GetRandomData();
        }
    }

    result = kStatus_Success;
#elif defined(FSL_FEATURE_SOC_LPC_RNG1_COUNT) && (FSL_FEATURE_SOC_LPC_RNG1_COUNT > 0)
    status_t status = kStatus_Fail;

    while (status != kStatus_Success)
    {
        status = RNG_GetRandomData(RNG, output, len);

        if (status == kStatus_Fail)
        {
            RNG_Init(RNG);
        }
    }

    result = status;
#endif
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

#endif

/******************************************************************************/
/*************************** FreeRTOS ********************************************/
/******************************************************************************/
#if USE_RTOS && defined(FSL_RTOS_FREE_RTOS) && defined(MBEDTLS_FREESCALE_FREERTOS_CALLOC_ALT)
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>

/*---------HEAP_3 calloc --------------------------------------------------*/

void * pvPortCalloc(size_t num, size_t size)
{
    void * pvReturn;

    vTaskSuspendAll();
    {
        pvReturn = calloc(num, size);
        traceMALLOC(pvReturn, xWantedSize);
    }
    (void) xTaskResumeAll();

#if (configUSE_MALLOC_FAILED_HOOK == 1)
    {
        if (pvReturn == NULL)
        {
            extern void vApplicationMallocFailedHook(void);
            vApplicationMallocFailedHook();
        }
    }
#endif

    return pvReturn;
}
#endif /* USE_RTOS && defined(FSL_RTOS_FREE_RTOS) && defined(MBEDTLS_FREESCALE_FREERTOS_CALLOC_ALT) */
