/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#ifndef K32W061_MBEDTLS_CONFIG_H
#define K32W061_MBEDTLS_CONFIG_H

#if defined(MBEDTLS_ECP_WINDOW_SIZE)
#undef MBEDTLS_ECP_WINDOW_SIZE
#define MBEDTLS_ECP_WINDOW_SIZE 4 /**< Maximum window size used */
#endif

#if defined(MBEDTLS_ECP_FIXED_POINT_OPTIM)
#undef MBEDTLS_ECP_FIXED_POINT_OPTIM
#define MBEDTLS_ECP_FIXED_POINT_OPTIM 1 /**< Enable fixed-point speed-up */
#endif

/**
 * \def MBEDTLS_AES_ALT
 *
 * Enable hardware acceleration for the AES block cipher
 *
 * See MBEDTLS_AES_C for more information.
 */
#define MBEDTLS_AES_ALT

#if defined(MBEDTLS_AES_ALT)
/**************************** KSDK ********************************************/
#include "fsl_device_registers.h"

/* Enable LTC use in library if there is LTC on chip. */
#if defined(FSL_FEATURE_SOC_LTC_COUNT) && (FSL_FEATURE_SOC_LTC_COUNT > 0)
#include "fsl_ltc.h"

#define LTC_INSTANCE LTC0 /* LTC base register.*/

#if FSL_FEATURE_LTC_HAS_SHA
#define MBEDTLS_FREESCALE_LTC_SHA1   /* Enable use of LTC SHA.*/
#define MBEDTLS_FREESCALE_LTC_SHA256 /* Enable use of LTC SHA256.*/
#endif
#if defined(FSL_FEATURE_LTC_HAS_DES) && FSL_FEATURE_LTC_HAS_DES
#define MBEDTLS_FREESCALE_LTC_DES /* Enable use of LTC DES.*/
#endif
#define MBEDTLS_FREESCALE_LTC_AES /* Enable use of LTC AES.*/
#if defined(FSL_FEATURE_LTC_HAS_GCM) && FSL_FEATURE_LTC_HAS_GCM
#define MBEDTLS_FREESCALE_LTC_AES_GCM /* Enable use of LTC AES GCM.*/
#endif
#if defined(FSL_FEATURE_LTC_HAS_PKHA) && FSL_FEATURE_LTC_HAS_PKHA
#define MBEDTLS_FREESCALE_LTC_PKHA /* Enable use of LTC PKHA.*/
#define FREESCALE_PKHA_INT_MAX_BYTES 256
#endif
#endif

/* Enable MMCAU use in library if there is MMCAU on chip. */
#if defined(FSL_FEATURE_SOC_MMCAU_COUNT) && (FSL_FEATURE_SOC_MMCAU_COUNT > 0)
#include "fsl_mmcau.h"

#define MBEDTLS_FREESCALE_MMCAU_MD5    /* Enable use of MMCAU MD5.*/
#define MBEDTLS_FREESCALE_MMCAU_SHA1   /* Enable use of MMCAU SHA1.*/
#define MBEDTLS_FREESCALE_MMCAU_SHA256 /* Enable use of MMCAU SHA256.*/
#define MBEDTLS_FREESCALE_MMCAU_DES    /* Enable use of MMCAU DES, when LTC is disabled.*/
#define MBEDTLS_FREESCALE_MMCAU_AES    /* Enable use of MMCAU AES, when LTC is disabled.*/
#endif

/* Enable CAU3 use in library if there is CAU3 on chip. */
#if defined(FSL_FEATURE_SOC_CAU3_COUNT) && (FSL_FEATURE_SOC_CAU3_COUNT > 0)
#include "cau3_pkha.h"
#include "fsl_cau3.h"

#define MBEDTLS_CAU3_COMPLETION_SIGNAL CAU3_CC_CMD_EVT
#define MBEDTLS_SHA256_ALT_NO_224

#define MBEDTLS_FREESCALE_CAU3_AES    /* Enable use of CAU3 AES.*/
#define MBEDTLS_FREESCALE_CAU3_SHA256 /* Enable use of CAU3 SHA256.*/
#define MBEDTLS_FREESCALE_CAU3_PKHA   /* Enable use of CAU3 PKHA.*/
#define FREESCALE_PKHA_INT_MAX_BYTES 512
#endif

#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
/*
 * This FREESCALE_PKHA_LONG_OPERANDS_ENABLE macro can be defined.
 * In such a case both software and hardware algorithm for TFM is linked in.
 * The decision for which algorithm is used is determined at runtime
 * from size of inputs. If inputs and result can fit into LTC (see FREESCALE_PKHA_INT_MAX_BYTES)
 * then we call hardware algorithm, otherwise we call software algorithm.
 *
 * Note that mbedTLS algorithms break modular operations unefficiently into two steps.
 * First is normal operation, for example non-modular multiply, which can produce number
 * with greater size than operands. Second is modular reduction.
 * The implication of this is that if for example FREESCALE_PKHA_INT_MAX_BYTES is 256 (2048 bits),
 * RSA-2048 still requires the FREESCALE_PKHA_LONG_OPERANDS_ENABLE macro to be defined,
 * otherwise it fails at runtime.
 */
//#define FREESCALE_PKHA_LONG_OPERANDS_ENABLE
#endif

/* Enable AES use in library if there is AES on chip. */
#if defined(FSL_FEATURE_SOC_AES_COUNT) && (FSL_FEATURE_SOC_AES_COUNT > 0)
#include "fsl_aes.h"

#define AES_INSTANCE AES0             /* AES base register.*/
#define MBEDTLS_FREESCALE_LPC_AES     /* Enable use of LPC AES.*/
#define MBEDTLS_FREESCALE_LPC_AES_GCM /* Enable use of LPC AES GCM.*/

#endif

/* Enable SHA use in library if there is SHA on chip. */
#if defined(FSL_FEATURE_SOC_SHA_COUNT) && (FSL_FEATURE_SOC_SHA_COUNT > 0)
#include "fsl_sha.h"

#define SHA_INSTANCE SHA0            /* SHA base register.*/
#define MBEDTLS_FREESCALE_LPC_SHA1   /* Enable use of LPC SHA.*/
#define MBEDTLS_FREESCALE_LPC_SHA256 /* Enable use of LPC SHA256.*/

#endif

/* Define ALT MMCAU & LTC functions. Do not change it. */
#if defined(MBEDTLS_FREESCALE_MMCAU_DES) || defined(MBEDTLS_FREESCALE_LTC_DES)
#define MBEDTLS_DES_SETKEY_ENC_ALT
#define MBEDTLS_DES_SETKEY_DEC_ALT
#define MBEDTLS_DES_CRYPT_ECB_ALT
#define MBEDTLS_DES3_CRYPT_ECB_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LTC_DES)
#define MBEDTLS_DES_CRYPT_CBC_ALT
#define MBEDTLS_DES3_CRYPT_CBC_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LTC_AES) || defined(MBEDTLS_FREESCALE_MMCAU_AES) || defined(MBEDTLS_FREESCALE_LPC_AES) ||            \
    defined(MBEDTLS_FREESCALE_CAU3_AES)
#define MBEDTLS_AES_SETKEY_ENC_ALT
#define MBEDTLS_AES_SETKEY_DEC_ALT
#define MBEDTLS_AES_ENCRYPT_ALT
#define MBEDTLS_AES_DECRYPT_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LTC_AES)
#define MBEDTLS_AES_CRYPT_CBC_ALT
#define MBEDTLS_AES_CRYPT_CTR_ALT
#define MBEDTLS_CCM_CRYPT_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LTC_AES_GCM) || defined(MBEDTLS_FREESCALE_LPC_AES_GCM)
#define MBEDTLS_GCM_CRYPT_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LTC_PKHA) || defined(MBEDTLS_FREESCALE_CAU3_PKHA)
#define MBEDTLS_MPI_ADD_ABS_ALT
#define MBEDTLS_MPI_SUB_ABS_ALT
#define MBEDTLS_MPI_MUL_MPI_ALT
#define MBEDTLS_MPI_MOD_MPI_ALT
#define MBEDTLS_MPI_EXP_MOD_ALT
#define MBEDTLS_MPI_GCD_ALT
#define MBEDTLS_MPI_INV_MOD_ALT
#define MBEDTLS_MPI_IS_PRIME_ALT
#if defined(MBEDTLS_FREESCALE_LTC_PKHA)
#define MBEDTLS_ECP_MUL_COMB_ALT
#define MBEDTLS_ECP_ADD_ALT
#endif
#endif
#if defined(MBEDTLS_FREESCALE_LTC_SHA1) || defined(MBEDTLS_FREESCALE_LPC_SHA1)
#define MBEDTLS_SHA1_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LTC_SHA256) || defined(MBEDTLS_FREESCALE_LPC_SHA256)
#define MBEDTLS_SHA256_ALT
/*
 * LPC SHA module does not support SHA-224.
 *
 * Since mbed TLS does not provide separate APIs for SHA-224 and SHA-256
 * and SHA-224 is not widely used, this implementation provides HW accelerated SHA-256 only
 * and SHA-224 is not available at all (calls will fail).
 *
 * To use SHA-224 on LPC, do not define MBEDTLS_SHA256_ALT and both SHA-224 and SHA-256 will use
 * original mbed TLS software implementation.
 */
#if defined(MBEDTLS_FREESCALE_LPC_SHA256)
#define MBEDTLS_SHA256_ALT_NO_224
#endif
#endif
#if defined(MBEDTLS_FREESCALE_MMCAU_MD5)
#define MBEDTLS_MD5_PROCESS_ALT
#endif
#if defined(MBEDTLS_FREESCALE_MMCAU_SHA1)
#define MBEDTLS_SHA1_PROCESS_ALT
#endif
#if defined(MBEDTLS_FREESCALE_MMCAU_SHA256)
#define MBEDTLS_SHA256_PROCESS_ALT
#endif
#if defined(MBEDTLS_FREESCALE_CAU3_SHA256)
#define MBEDTLS_SHA256_PROCESS_ALT
#endif
#if defined(MBEDTLS_FREESCALE_CAU3_AES)
#define MBEDTLS_AES_ALT_NO_192
#endif
#if defined(MBEDTLS_FREESCALE_LTC_AES)
#if !defined(FSL_FEATURE_LTC_HAS_AES192) || !FSL_FEATURE_LTC_HAS_AES192
#define MBEDTLS_AES_ALT_NO_192
#endif
#if !defined(FSL_FEATURE_LTC_HAS_AES256) || !FSL_FEATURE_LTC_HAS_AES256
#define MBEDTLS_AES_ALT_NO_256
#endif
#endif
#if defined(MBEDTLS_FREESCALE_LPC_AES)
#define MBEDTLS_AES_CRYPT_CBC_ALT
#define MBEDTLS_AES_CRYPT_CFB_ALT
#define MBEDTLS_AES_CRYPT_CTR_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LPC_SHA1)
#define MBEDTLS_SHA1_PROCESS_ALT
#endif
#if defined(MBEDTLS_FREESCALE_LPC_SHA256)
#define MBEDTLS_SHA256_PROCESS_ALT
#endif

#if USE_RTOS && defined(FSL_RTOS_FREE_RTOS)
#include "FreeRTOS.h"

#define MBEDTLS_PLATFORM_MEMORY

#endif /* USE_RTOS*/
/**************************** KSDK end ****************************************/
#endif /* MBEDTLS_AES_ALT || MBEDTLS_SHA256_ALT */

#endif // K32W061_MBEDTLS_CONFIG_H
