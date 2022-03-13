/*
 *  Test utilites
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

#include "test_utils.h"
#include "common.h"
#include "mbedtls/platform.h"


void mbedtls_printbuf( const char *comment, const void *buf, size_t len )
{
    mbedtls_printf( "%s [%u]:", comment, ( unsigned int )len );
    for ( size_t i = 0; i < len; i++ )
    {
        mbedtls_printf( " %02x", ( ( unsigned char * )buf )[i] );
    }
    mbedtls_printf( "\n" );
}

void mbedtls_printbuf_c( const char *comment, const void *buf, size_t len )
{

    mbedtls_printf( "%s [%u]: ", comment, ( unsigned int )len );
    for ( size_t i = 0; len && i < len - 1; i++ )
    {
        mbedtls_printf( "0x%02x, ", ( ( unsigned char * )buf )[i] );
    }
    if ( len )
    {
        mbedtls_printf( "0x%02x\n", ( ( unsigned char* )buf )[len - 1] );
    }
    else
    {
        mbedtls_printf( "\n" );
    }
}
