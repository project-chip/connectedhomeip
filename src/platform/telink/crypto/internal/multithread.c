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

#include <kernel.h>

K_MUTEX_DEFINE(mbedtls_entropy_mutex);
K_MUTEX_DEFINE(mbedtls_ecp_mutex);
K_MUTEX_DEFINE(mbedtls_aes_mutex);

void mbedtls_entropy_lock(void)
{

    if (k_mutex_lock(&mbedtls_entropy_mutex, K_FOREVER) != 0)
    {
        printk("mbedtls_entropy_lock fail\n");
    }
}

void mbedtls_entropy_unlock(void)
{

    if (k_mutex_unlock(&mbedtls_entropy_mutex) != 0)
    {
        printk("mbedtls_entropy_unlock fail\n");
    }
}

void mbedtls_ecp_lock(void)
{

    if (k_mutex_lock(&mbedtls_ecp_mutex, K_FOREVER) != 0)
    {
        printk("mbedtls_ecp_lock fail\n");
    }
}

void mbedtls_ecp_unlock(void)
{

    if (k_mutex_unlock(&mbedtls_ecp_mutex) != 0)
    {
        printk("mbedtls_ecp_unlock fail\n");
    }
}

void mbedtls_aes_lock(void)
{

    if (k_mutex_lock(&mbedtls_aes_mutex, K_FOREVER) != 0)
    {
        printk("mbedtls_aes_lock fail\n");
    }
}

void mbedtls_aes_unlock(void)
{

    if (k_mutex_unlock(&mbedtls_aes_mutex) != 0)
    {
        printk("mbedtls_aes_unlock fail\n");
    }
}
