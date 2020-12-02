/*
 *
 *    Copyright (c) 2020 Google LLC.
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
 *
 */

#include "FreeRtosMbedtlsUtils.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <assert.h>

#include <stdio.h>
#include <string.h>

static inline void mutex_init(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    *p_mutex = xSemaphoreCreateMutex();
    assert(*p_mutex != NULL);
}

static inline void mutex_free(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    assert(*p_mutex != NULL);
    vSemaphoreDelete(*p_mutex);
}

static inline int mutex_lock(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    assert(*p_mutex != NULL);
    return xSemaphoreTake(*p_mutex, portMAX_DELAY) != pdTRUE;
}

static inline int mutex_unlock(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    assert(*p_mutex != NULL);
    return xSemaphoreGive(*p_mutex) != pdTRUE;
}

void freertos_mbedtls_mutex_init(void)
{
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
}

void freertos_mbedtls_mutex_free(void)
{
    mbedtls_threading_free_alt();
}

void *pvPortCallocRtos(size_t num, size_t size)
{
	size_t totalAllocSize = (size_t)(num * size);

    void *p = pvPortMalloc(totalAllocSize);

    if (p)
    {
        memset(p, 0, totalAllocSize);
    }

    return p;
}
