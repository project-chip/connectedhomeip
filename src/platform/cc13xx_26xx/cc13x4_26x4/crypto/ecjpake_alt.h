/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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

#pragma once

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls-config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#if defined(MBEDTLS_ECJPAKE_ALT)

#include "mbedtls/ecp.h"
#include "mbedtls/md.h"

#include <ti/drivers/ECJPAKE.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKey.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Length of parameters for the NIST p256r1.
 *
 * ECJ-PAKE is only defined for p256r1
 */
#define NISTP256_CURVE_LENGTH_BYTES (32)

/**
 * Size in bytes of the identifier at the beginning of the point format
 */
#define OCTET_STRING_OFFSET 1

/**
 * Size in bytes of a point expressed in the TLS point format
 *
 * id_byte || X coord || Y coord
 */
#define NISTP256_PUBLIC_KEY_LENGTH_BYTES (OCTET_STRING_OFFSET + (NISTP256_CURVE_LENGTH_BYTES * 2))

/**
 * EC J-PAKE context structure.
 */
typedef struct
{
    const mbedtls_md_info_t * md_info;
    mbedtls_ecp_group_id curve;
    mbedtls_ecjpake_role role;
    int point_format;

    bool roundTwoGenerated;
    /*
     * XXX: possible size reduction by moving ephemeral material to round
     * calculations.
     */
    unsigned char myPrivateKeyMaterial1[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateKeyMaterial2[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateVMaterial1[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateVMaterial2[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPrivateVMaterial3[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char myPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicKeyMaterial2[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicVMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicVMaterial2[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myPublicVMaterial3[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myCombinedPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myCombinedPrivateKeyMaterial1[NISTP256_CURVE_LENGTH_BYTES];
    unsigned char nistP256Generator[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char myGenerator[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirPublicKeyMaterial2[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirCombinedPublicKeyMaterial1[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char theirGenerator[NISTP256_PUBLIC_KEY_LENGTH_BYTES];
    unsigned char preSharedSecretKeyingMaterial[NISTP256_CURVE_LENGTH_BYTES];

    CryptoKey nistP256GeneratorCryptoKey;
    CryptoKey preSharedSecretCryptoKey;
    CryptoKey myPrivateCryptoKey1;
    CryptoKey myPrivateCryptoKey2;
    CryptoKey myPrivateCryptoV1;
    CryptoKey myPrivateCryptoV2;
    CryptoKey myPrivateCryptoV3;
    CryptoKey myCombinedPrivateKey;
    CryptoKey myPublicCryptoKey1;
    CryptoKey myPublicCryptoKey2;
    CryptoKey myPublicCryptoV1;
    CryptoKey myPublicCryptoV2;
    CryptoKey myPublicCryptoV3;
    CryptoKey myCombinedPublicKey;
    CryptoKey myGeneratorKey;
    CryptoKey theirPublicCryptoKey1;
    CryptoKey theirPublicCryptoKey2;
    CryptoKey theirCombinedPublicKey;
    CryptoKey theirGeneratorKey;

    ECJPAKE_Handle handle;
} mbedtls_ecjpake_context;

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_ECJPAKE_ALT */
