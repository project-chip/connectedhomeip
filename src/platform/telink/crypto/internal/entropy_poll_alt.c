/*
 *  Hardware entropy
 *
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

#include "common.h"

#if defined( MBEDTLS_ENTROPY_HARDWARE_ALT )

#include "trng.h"
#include "multithread.h"
#include <string.h>


int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen )
{
    ( void ) data;

    if ( output != NULL && len != 0 && olen != NULL )
    {
        mbedtls_entropy_lock();
        *olen = 0;
        extern unsigned int g_rnd_m_w;
        while ( len / sizeof( g_rnd_m_w ) != 0 )
        {
            trng_init();
            *( ( unsigned int * )output ) = g_rnd_m_w;
            output += sizeof( g_rnd_m_w );
            len -= sizeof( g_rnd_m_w );
            *olen += sizeof( g_rnd_m_w );
        }
        if ( len != 0 )
        {
            trng_init();
            memcpy( output, &g_rnd_m_w, len );
            *olen += len;
        }
        mbedtls_entropy_unlock();
    }
    return 0;
}

#endif /* MBEDTLS_ENTROPY_HARDWARE_ALT */
