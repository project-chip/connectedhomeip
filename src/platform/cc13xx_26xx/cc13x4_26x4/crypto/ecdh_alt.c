/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2021 Texas Instruments Incorporated
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

#if defined MBEDTLS_ECDH_COMPUTE_SHARED_ALT || defined MBEDTLS_ECDH_GEN_PUBLIC_ALT

#include <stdint.h>

#include <mbedtls/bignum.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/platform.h>
#include <mbedtls/platform_util.h>

#include "ti_drivers_config.h"

#include <ti/drivers/ECDH.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/ecdh/ECDHCC26X2.h>

/* Parameter validation macros based on platform_util.h */
#define ECDH_VALIDATE_RET(cond) MBEDTLS_INTERNAL_VALIDATE_RET(cond, MBEDTLS_ERR_ECP_BAD_INPUT_DATA)
#define ECDH_VALIDATE(cond) MBEDTLS_INTERNAL_VALIDATE(cond)

#if defined MBEDTLS_ECDH_GEN_PUBLIC_ALT
/*
 * Generate public key (restartable version)
 */
static int ecdh_gen_public_restartable(mbedtls_ecp_group * grp, mbedtls_mpi * d, mbedtls_ecp_point * Q,
                                       int (*f_rng)(void *, unsigned char *, size_t), void * p_rng,
                                       mbedtls_ecp_restart_ctx * rs_ctx)
{
    int ret         = 0;
    size_t plen     = grp->nbits / 8U;
    uint8_t * q_buf = (uint8_t *) mbedtls_calloc(1, (plen * 2U) + 1);
    uint8_t * d_buf = (uint8_t *) mbedtls_calloc(1, plen);
    CryptoKey myPrivateKey;
    CryptoKey myPublicKey;
    ECDH_Config config = { 0 };
    ECDH_Handle handle = NULL;
    ECDH_OperationGeneratePublicKey operationGeneratePublicKey;
    ECDHCC26X2_HWAttrs hwAttrs = { 0 };
    ECDHCC26X2_Object object   = { 0 };

    if (NULL == q_buf || NULL == d_buf)
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    hwAttrs.intPriority = (1 << 5);

    config.object  = (void *) &object;
    config.hwAttrs = (void *) &hwAttrs;

    handle = ECDH_construct(&config, NULL);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_ECP_HW_ACCEL_FAILED;
        goto cleanup;
    }

    CryptoKeyPlaintext_initKey(&myPrivateKey, d_buf, plen);
    CryptoKeyPlaintext_initKey(&myPublicKey, q_buf, (plen * 2U) + 1);

    if (0 != f_rng(p_rng, d_buf, plen))
    {
        ret = MBEDTLS_ERR_ECP_RANDOM_FAILED;
        goto cleanup;
    }

    ECDH_OperationGeneratePublicKey_init(&operationGeneratePublicKey);
    operationGeneratePublicKey.myPrivateKey = &myPrivateKey;
    operationGeneratePublicKey.myPublicKey  = &myPublicKey;

    /* Load the ROM curve params */
    switch (grp->id)
    {
    case MBEDTLS_ECP_DP_SECP224R1:
        operationGeneratePublicKey.curve = &ECCParams_NISTP224;
        break;

    case MBEDTLS_ECP_DP_SECP256R1:
        operationGeneratePublicKey.curve = &ECCParams_NISTP256;
        break;

    case MBEDTLS_ECP_DP_SECP384R1:
        operationGeneratePublicKey.curve = &ECCParams_NISTP384;
        break;

    case MBEDTLS_ECP_DP_SECP521R1:
        operationGeneratePublicKey.curve = &ECCParams_NISTP521;
        break;

    case MBEDTLS_ECP_DP_BP256R1:
        operationGeneratePublicKey.curve = &ECCParams_BrainpoolP256R1;
        break;

    case MBEDTLS_ECP_DP_BP384R1:
        operationGeneratePublicKey.curve = &ECCParams_BrainpoolP384R1;
        break;

    case MBEDTLS_ECP_DP_BP512R1:
        operationGeneratePublicKey.curve = &ECCParams_BrainpoolP512R1;
        break;

    case MBEDTLS_ECP_DP_CURVE25519:
        operationGeneratePublicKey.curve = &ECCParams_Curve25519;
        break;

    default:
        /* Possible extension to load an arbitrary curve */
        ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
        goto cleanup;
    }

    if (ECDH_STATUS_SUCCESS == ECDH_generatePublicKey(handle, &operationGeneratePublicKey))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(d, d_buf, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&(Q->X), q_buf + 1, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(&(Q->Y), q_buf + 1 + plen, plen));

cleanup:
    mbedtls_free(q_buf);
    mbedtls_free(d_buf);

    if (NULL != handle)
    {
        ECDH_close(handle);
    }

    return (ret);
}

/*
 * Generate public key
 */
int mbedtls_ecdh_gen_public(mbedtls_ecp_group * grp, mbedtls_mpi * d, mbedtls_ecp_point * Q,
                            int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    ECDH_VALIDATE_RET(grp != NULL);
    ECDH_VALIDATE_RET(d != NULL);
    ECDH_VALIDATE_RET(Q != NULL);
    ECDH_VALIDATE_RET(f_rng != NULL);
    return (ecdh_gen_public_restartable(grp, d, Q, f_rng, p_rng, NULL));
}
#endif /* MBEDTLS_ECDH_GEN_PUBLIC_ALT */

#if defined MBEDTLS_ECDH_COMPUTE_SHARED_ALT
/*
 * Compute shared secret (SEC1 3.3.1)
 */
static int ecdh_compute_shared_restartable(mbedtls_ecp_group * grp, mbedtls_mpi * z, const mbedtls_ecp_point * Q,
                                           const mbedtls_mpi * d, int (*f_rng)(void *, unsigned char *, size_t), void * p_rng,
                                           mbedtls_ecp_restart_ctx * rs_ctx)
{
    int ret         = 0;
    size_t plen     = grp->nbits / 8U;
    uint8_t * q_buf = (uint8_t *) mbedtls_calloc(1, (plen * 2U) + 1);
    uint8_t * s_buf = (uint8_t *) mbedtls_calloc(1, (plen * 2U) + 1);
    uint8_t * d_buf = (uint8_t *) mbedtls_calloc(1, plen);
    CryptoKey myPrivateKey;
    CryptoKey theirPublicKey;
    CryptoKey sharedSecret;
    ECDH_Config config = { 0 };
    ECDH_Handle handle = NULL;
    ECDH_OperationComputeSharedSecret operationComputeSharedSecret;
    ECDHCC26X2_HWAttrs hwAttrs = { 0 };
    ECDHCC26X2_Object object   = { 0 };

    if (NULL == q_buf || NULL == s_buf || NULL == d_buf)
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    hwAttrs.intPriority = (1 << 5);

    config.object  = (void *) &object;
    config.hwAttrs = (void *) &hwAttrs;

    handle = ECDH_construct(&config, NULL);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_ECP_HW_ACCEL_FAILED;
        goto cleanup;
    }

    CryptoKeyPlaintext_initKey(&sharedSecret, s_buf, (2U * plen) + 1);

    /* The driver consumes network byte order. Copy this number and reverse them again. */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(d, d_buf, plen));
    CryptoKeyPlaintext_initKey(&myPrivateKey, d_buf, plen);

    /* The driver consumes an octet string in network byte order. Copy this
     * point and reverse them again. */
    q_buf[0] = 0x04;
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&(Q->X), q_buf + 1, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&(Q->Y), q_buf + 1 + plen, plen));
    CryptoKeyPlaintext_initKey(&theirPublicKey, q_buf, (2U * plen) + 1);

    ECDH_OperationComputeSharedSecret_init(&operationComputeSharedSecret);
    operationComputeSharedSecret.myPrivateKey   = &myPrivateKey;
    operationComputeSharedSecret.theirPublicKey = &theirPublicKey;
    operationComputeSharedSecret.sharedSecret   = &sharedSecret;

    /* Load the ROM curve params */
    switch (grp->id)
    {
    case MBEDTLS_ECP_DP_SECP224R1:
        operationComputeSharedSecret.curve = &ECCParams_NISTP224;
        break;

    case MBEDTLS_ECP_DP_SECP256R1:
        operationComputeSharedSecret.curve = &ECCParams_NISTP256;
        break;

    case MBEDTLS_ECP_DP_SECP384R1:
        operationComputeSharedSecret.curve = &ECCParams_NISTP384;
        break;

    case MBEDTLS_ECP_DP_SECP521R1:
        operationComputeSharedSecret.curve = &ECCParams_NISTP521;
        break;

    case MBEDTLS_ECP_DP_BP256R1:
        operationComputeSharedSecret.curve = &ECCParams_BrainpoolP256R1;
        break;

    case MBEDTLS_ECP_DP_BP384R1:
        operationComputeSharedSecret.curve = &ECCParams_BrainpoolP384R1;
        break;

    case MBEDTLS_ECP_DP_BP512R1:
        operationComputeSharedSecret.curve = &ECCParams_BrainpoolP512R1;
        break;

    case MBEDTLS_ECP_DP_CURVE25519:
        operationComputeSharedSecret.curve = &ECCParams_Curve25519;
        break;

    default:
        /* Possible extension to load an arbitrary curve */
        ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
        goto cleanup;
    }

    if (ECDH_STATUS_SUCCESS == ECDH_computeSharedSecret(handle, &operationComputeSharedSecret))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_ECP_BAD_INPUT_DATA;
    }

    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(z, s_buf + 1, plen));

cleanup:
    mbedtls_free(q_buf);
    mbedtls_free(d_buf);
    mbedtls_free(s_buf);

    if (NULL != handle)
    {
        ECDH_close(handle);
    }

    return (ret);
}

/*
 * Compute shared secret (SEC1 3.3.1)
 */
int mbedtls_ecdh_compute_shared(mbedtls_ecp_group * grp, mbedtls_mpi * z, const mbedtls_ecp_point * Q, const mbedtls_mpi * d,
                                int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    ECDH_VALIDATE_RET(grp != NULL);
    ECDH_VALIDATE_RET(Q != NULL);
    ECDH_VALIDATE_RET(d != NULL);
    ECDH_VALIDATE_RET(z != NULL);
    return (ecdh_compute_shared_restartable(grp, z, Q, d, f_rng, p_rng, NULL));
}
#endif /* MBEDTLS_ECDH_COMPUTE_SHARED_ALT */

#endif /* MBEDTLS_ECDH_COMPUTE_SHARED_ALT || MBEDTLS_ECDH_GEN_PUBLIC_ALT */
