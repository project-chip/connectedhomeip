/*
 *  mbed Microcontroller Library
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  Copyright (C) 2019 Cypress Semiconductor Corporation
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * \file ecp.h
 * \version 1.0
 *
 * \brief This file provides an API for Elliptic Curves over GF(P) (ECP).
 *
 * The use of ECP in cryptography and TLS is defined in
 * <em>Standards for Efficient Cryptography Group (SECG): SEC1
 * Elliptic Curve Cryptography</em> and
 * <em>RFC-4492: Elliptic Curve Cryptography (ECC) Cipher Suites
 * for Transport Layer Security (TLS)</em>.
 *
 * <em>RFC-2409: The Internet Key Exchange (IKE)</em> defines ECP
 * group types.
 *
 */

#ifndef ECP_ALT_H
#define ECP_ALT_H

#include "mbedtls/bignum.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(MBEDTLS_ECP_ALT)

/*
 * default mbed TLS elliptic curve arithmetic implementation
 *
 * (in case MBEDTLS_ECP_ALT is defined then the developer has to provide an
 * alternative implementation for the whole module and it will replace this
 * one.)
 */

typedef struct mbedtls_ecp_group {
    mbedtls_ecp_group_id id;    /*!< An internal group identifier. */
    mbedtls_mpi P;              /*!< The prime modulus of the base field. */
    mbedtls_mpi A;              /*!< For Short Weierstrass: \p A in the equation. For
                                     Montgomery curves: <code>(A + 2) / 4</code>. */
    mbedtls_mpi B;              /*!< For Short Weierstrass: \p B in the equation.
                                     For Montgomery curves: unused. */
    mbedtls_ecp_point G;        /*!< The generator of the subgroup used. */
    mbedtls_mpi N;              /*!< The order of \p G. */
    size_t pbits;               /*!< The number of bits in \p P.*/
    size_t nbits;               /*!< For Short Weierstrass: The number of bits in \p P.
                                     For Montgomery curves: the number of bits in the
                                     private keys. */
    /* End of public fields */

    unsigned int MBEDTLS_PRIVATE(h);             /*!< \internal 1 if the constants are static. */
    int(*MBEDTLS_PRIVATE(modp))(mbedtls_mpi *);  /*!< The function for fast pseudo-reduction
                                                    mod \p P (see above).*/
    int(*MBEDTLS_PRIVATE(t_pre))(mbedtls_ecp_point *, void *);   /*!< Unused. */
    int(*MBEDTLS_PRIVATE(t_post))(mbedtls_ecp_point *, void *);  /*!< Unused. */
    void *MBEDTLS_PRIVATE(t_data);               /*!< Unused. */
    mbedtls_ecp_point *MBEDTLS_PRIVATE(T);       /*!< Pre-computed points for ecp_mul_comb(). */
    size_t MBEDTLS_PRIVATE(T_size);              /*!< The number of dynamic allocated pre-computed points. */
}
mbedtls_ecp_group;

typedef struct hw_ecp_point
{
    uint32_t x[16];          /*!< The X coordinate of the ECP point. */
    uint32_t y[16];          /*!< The Y coordinate of the ECP point. */
    uint32_t z[16];          /*!< The Z coordinate of the ECP point. */
} hw_ecp_point;

typedef struct hw_ecp_group
{
    uint32_t *N;    // modular, also called p
    uint32_t *A;    // curve parameter a
    uint32_t *B;    // curve parameter b
    uint32_t *n;    // order of G
    hw_ecp_point G; // base point

    uint32_t key_bits;
    uint16_t mode;
} hw_ecp_group;

#endif /* MBEDTLS_ECP_ALT */

#ifdef __cplusplus
}
#endif

#endif /* ecp_alt.h */
