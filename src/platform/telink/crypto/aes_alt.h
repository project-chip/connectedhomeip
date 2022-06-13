/* aes_alt.h */
/*
 *  Copyright The Mbed TLS Contributors
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

#ifndef AES_ALT_H
#define AES_ALT_H

#if defined(MBEDTLS_AES_ALT)
// Regular implementation
//

/**
 * \brief The AES context-type definition.
 */
typedef struct mbedtls_aes_context
{
    int MBEDTLS_PRIVATE(nr);           /*!< The number of rounds. */
    uint32_t * MBEDTLS_PRIVATE(rk);    /*!< AES round keys. */
    uint32_t MBEDTLS_PRIVATE(buf)[68]; /*!< Unaligned data buffer. This buffer can
                                            hold 32 extra Bytes, which can be used for
                                            one of the following purposes:
                                            <ul><li>Alignment if VIA padlock is used.</li>
                                            <li>Simplifying key expansion in the 256-bit
                                                case by generating an extra round key.
                                            </li></ul> */
} mbedtls_aes_context;

#if defined(MBEDTLS_CIPHER_MODE_XTS)
/**
 * \brief The AES XTS context-type definition.
 */
typedef struct mbedtls_aes_xts_context
{
    mbedtls_aes_context MBEDTLS_PRIVATE(crypt); /*!< The AES context to use for AES block encryption or decryption. */
    mbedtls_aes_context MBEDTLS_PRIVATE(tweak); /*!< The AES context used for tweak computation. */
} mbedtls_aes_xts_context;

#endif /* MBEDTLS_CIPHER_MODE_XTS */

#endif /* MBEDTLS_AES_ALT */

#endif /* aes_alt.h */
