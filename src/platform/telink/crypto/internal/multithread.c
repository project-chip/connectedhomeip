/*
 *  Multithread support for mbedTLS HW unit
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

#include "multithread.h"
#include "common.h"


/****************************************************************
 * If RTOS is used and cryptography stuffs are used
 * from more than one thread implement exclusive access
 * for HW crypto units depending on used operation system.
 * See documentation for your RTOS.
 ****************************************************************/

void mbedtls_entropy_lock( void )
{
}

void mbedtls_entropy_unlock( void )
{
}

void mbedtls_ecp_lock( void )
{
}

void mbedtls_ecp_unlock( void )
{
}

void mbedtls_aes_lock( void )
{
}

void mbedtls_aes_unlock( void )
{
}
