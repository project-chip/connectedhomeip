/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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
#include "STM32FreeRtosHooks.h"
#include "FreeRTOS.h"
#include "mbedtls/memory_buffer_alloc.h"

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#define MBEDTLS_HEAP_SIZE 15000
#include "mbedtls/pk.h"
#include "mbedtls/platform.h"
#include "mbedtls/sha1.h"
#include "mbedtls/sha256.h"
#include "mbedtls/x509_crt.h"

#include "mbedtls/threading.h"
#include "threading_alt.h"

#include <string.h>

static uint8_t mdedtls_heap[MBEDTLS_HEAP_SIZE];

static void freertos_mbedtls_heap_init(void);

static inline void mutex_init(mbedtls_threading_mutex_t * mutex)
{
    mutex->mutex = xSemaphoreCreateMutex();

    if (mutex->mutex != NULL)
    {
        mutex->is_valid = 1;
    }
    else
    {
        mutex->is_valid = 0;
    }
}

static inline void mutex_free(mbedtls_threading_mutex_t * mutex)
{
    if (mutex->is_valid == 1)
    {
        vSemaphoreDelete(mutex->mutex);
        mutex->is_valid = 0;
    }
}

static inline int mutex_lock(mbedtls_threading_mutex_t * mutex)
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if (mutex->is_valid == 1)
    {
        if (xSemaphoreTake(mutex->mutex, portMAX_DELAY))
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
        }
    }

    return ret;
}

static inline int mutex_unlock(mbedtls_threading_mutex_t * mutex)
{
    int ret = MBEDTLS_ERR_THREADING_BAD_INPUT_DATA;

    if (mutex->is_valid == 1)
    {
        if (xSemaphoreGive(mutex->mutex))
        {
            ret = 0;
        }
        else
        {
            ret = MBEDTLS_ERR_THREADING_MUTEX_ERROR;
        }
    }

    return ret;
}

void freertos_mbedtls_mutex_init(void)
{
    // Configure mbedtls to use mutexes from FreeRTOS
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
}

static void freertos_mbedtls_heap_init(void)
{
    mbedtls_memory_buffer_alloc_init(mdedtls_heap, sizeof(mdedtls_heap));
}

void freertos_mbedtls_init(void)
{
    freertos_mbedtls_mutex_init();
    freertos_mbedtls_heap_init();
}
