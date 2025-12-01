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

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined MBEDTLS_ECDSA_VERIFY_ALT || defined MBEDTLS_ECDSA_SIGN_ALT

/* NOTE: The TI drivers consume these points and big numbers in network byte
 *       order. This is in contrast to the mbedtls_mpi structures which story
 *       these numbers and points in little endian byte order. This file uses
 *       the mpi functions to re-write the buffers into network byte order.
 */

#include <stdint.h>

#include <mbedtls/bignum.h>
#include <mbedtls/ecdsa.h>
#include <mbedtls/platform.h>
#include <mbedtls/platform_util.h>

#include "ti_drivers_config.h"

#include <ti/drivers/ECDSA.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/ecdsa/ECDSACC26X2.h>

#ifdef MBEDTLS_ECDSA_VERIFY_ALT

/*
 * Verify ECDSA signature of hashed message
 */
int mbedtls_ecdsa_verify(mbedtls_ecp_group * grp, const unsigned char * buf, size_t blen, const mbedtls_ecp_point * Q,
                         const mbedtls_mpi * r, const mbedtls_mpi * s)
{
    int ret         = 0;
    size_t plen     = grp->nbits / 8U;
    uint8_t * r_buf = (uint8_t *) mbedtls_calloc(1, plen);
    uint8_t * s_buf = (uint8_t *) mbedtls_calloc(1, plen);
    uint8_t * q_buf = (uint8_t *) mbedtls_calloc(1, (plen * 2U) + 1);
    CryptoKey theirPublicKey;
    ECDSA_Config config = { 0 };
    ECDSA_Handle handle = NULL;
    ECDSA_OperationVerify operationVerify;
    ECDSACC26X2_HWAttrs hwAttrs = { 0 };
    ECDSACC26X2_Object object   = { 0 };

    if (NULL == r_buf || NULL == s_buf || NULL == q_buf)
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    hwAttrs.intPriority     = (1 << 5);
    hwAttrs.trngIntPriority = (1 << 5);

    config.object  = (void *) &object;
    config.hwAttrs = (void *) &hwAttrs;

    handle = ECDSA_construct(&config, NULL);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_ECP_HW_ACCEL_FAILED;
        goto cleanup;
    }

    /* The driver consumes an octet string in network byte order. Copy this
     * point and reverse them again. */
    q_buf[0] = 0x04;
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&(Q->X), q_buf + 1, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(&(Q->Y), q_buf + 1 + plen, plen));
    CryptoKeyPlaintext_initKey(&theirPublicKey, q_buf, (2U * plen) + 1);

    /* The driver also consumes the r and s in network byte order. Copy these
     * buffers and them reverse them again */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(r, r_buf, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(s, s_buf, plen));

    /* prepare the operation */
    ECDSA_OperationVerify_init(&operationVerify);
    operationVerify.theirPublicKey = &theirPublicKey;
    operationVerify.hash           = buf;
    operationVerify.r              = r_buf;
    operationVerify.s              = s_buf;

    /* Load the ROM curve params */
    switch (grp->id)
    {
    case MBEDTLS_ECP_DP_SECP224R1:
        operationVerify.curve = &ECCParams_NISTP224;
        break;

    case MBEDTLS_ECP_DP_SECP256R1:
        operationVerify.curve = &ECCParams_NISTP256;
        break;

    case MBEDTLS_ECP_DP_SECP384R1:
        operationVerify.curve = &ECCParams_NISTP384;
        break;

    case MBEDTLS_ECP_DP_SECP521R1:
        operationVerify.curve = &ECCParams_NISTP521;
        break;

    case MBEDTLS_ECP_DP_BP256R1:
        operationVerify.curve = &ECCParams_BrainpoolP256R1;
        break;

    case MBEDTLS_ECP_DP_BP384R1:
        operationVerify.curve = &ECCParams_BrainpoolP384R1;
        break;

    case MBEDTLS_ECP_DP_BP512R1:
        operationVerify.curve = &ECCParams_BrainpoolP512R1;
        break;

    case MBEDTLS_ECP_DP_CURVE25519:
        operationVerify.curve = &ECCParams_Curve25519;
        break;

    default:
        /* Possible extension to load an arbitrary curve */
        ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
        goto cleanup;
    }

    if (ECDSA_STATUS_SUCCESS == ECDSA_verify(handle, &operationVerify))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_ECP_VERIFY_FAILED;
    }

cleanup:
    mbedtls_free(r_buf);
    mbedtls_free(s_buf);
    mbedtls_free(q_buf);

    if (NULL != handle)
    {
        ECDSA_close(handle);
    }

    return ret;
}

#endif /* MBEDTLS_ECDSA_VERIFY_ALT */

#ifdef MBEDTLS_ECDSA_SIGN_ALT

/*
 * Compute ECDSA signature of a hashed message
 */
int mbedtls_ecdsa_sign(mbedtls_ecp_group * grp, mbedtls_mpi * r, mbedtls_mpi * s, const mbedtls_mpi * d, const unsigned char * buf,
                       size_t blen, int (*f_rng)(void *, unsigned char *, size_t), void * p_rng)
{
    int ret         = 0;
    size_t plen     = grp->nbits / 8U;
    uint8_t * r_buf = (uint8_t *) mbedtls_calloc(1, plen);
    uint8_t * s_buf = (uint8_t *) mbedtls_calloc(1, plen);
    uint8_t * d_buf = (uint8_t *) mbedtls_calloc(1, plen);
    CryptoKey myPrivateKey;
    ECDSA_Config config = { 0 };
    ECDSA_Handle handle = NULL;
    ECDSA_OperationSign operationSign;
    ECDSACC26X2_HWAttrs hwAttrs = { 0 };
    ECDSACC26X2_Object object   = { 0 };

    if (NULL == r_buf || NULL == s_buf || NULL == d_buf)
    {
        ret = MBEDTLS_ERR_ECP_ALLOC_FAILED;
        goto cleanup;
    }

    hwAttrs.intPriority     = (1 << 5);
    hwAttrs.trngIntPriority = (1 << 5);

    config.object  = (void *) &object;
    config.hwAttrs = (void *) &hwAttrs;

    handle = ECDSA_construct(&config, NULL);

    if (NULL == handle)
    {
        ret = MBEDTLS_ERR_ECP_HW_ACCEL_FAILED;
        goto cleanup;
    }

    /* The driver consumes numbers in network byte order */
    MBEDTLS_MPI_CHK(mbedtls_mpi_write_binary(d, d_buf, plen));

    CryptoKeyPlaintext_initKey(&myPrivateKey, d_buf, plen);
    ECDSA_OperationSign_init(&operationSign);
    operationSign.myPrivateKey = &myPrivateKey;
    operationSign.hash         = buf;
    operationSign.r            = r_buf;
    operationSign.s            = s_buf;

    /* Load the ROM curve params */
    switch (grp->id)
    {
    case MBEDTLS_ECP_DP_SECP224R1:
        operationSign.curve = &ECCParams_NISTP224;
        break;

    case MBEDTLS_ECP_DP_SECP256R1:
        operationSign.curve = &ECCParams_NISTP256;
        break;

    case MBEDTLS_ECP_DP_SECP384R1:
        operationSign.curve = &ECCParams_NISTP384;
        break;

    case MBEDTLS_ECP_DP_SECP521R1:
        operationSign.curve = &ECCParams_NISTP521;
        break;

    case MBEDTLS_ECP_DP_BP256R1:
        operationSign.curve = &ECCParams_BrainpoolP256R1;
        break;

    case MBEDTLS_ECP_DP_BP384R1:
        operationSign.curve = &ECCParams_BrainpoolP384R1;
        break;

    case MBEDTLS_ECP_DP_BP512R1:
        operationSign.curve = &ECCParams_BrainpoolP512R1;
        break;

    case MBEDTLS_ECP_DP_CURVE25519:
        operationSign.curve = &ECCParams_Curve25519;
        break;

    default:
        /* Possible extension to load an arbitrary curve */
        ret = MBEDTLS_ERR_ECP_FEATURE_UNAVAILABLE;
        goto cleanup;
    }

    if (ECDSA_STATUS_SUCCESS == ECDSA_sign(handle, &operationSign))
    {
        ret = 0;
    }
    else
    {
        ret = MBEDTLS_ERR_ECP_VERIFY_FAILED;
    }

    /* The driver produces r and s in network byte order. copy into mbedtls mpi
     * format. This incurs an extra byte reversal when written to ASN1. */
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(r, r_buf, plen));
    MBEDTLS_MPI_CHK(mbedtls_mpi_read_binary(s, s_buf, plen));

cleanup:
    mbedtls_free(r_buf);
    mbedtls_free(s_buf);
    mbedtls_free(d_buf);

    if (NULL != handle)
    {
        ECDSA_close(handle);
    }

    return ret;
}
#endif /* MBEDTLS_ECDSA_SIGN_ALT */

#endif /* defined MBEDTLS_ECDSA_VERIFY_ALT || defined MBEDTLS_ECDSA_SIGN_ALT */
