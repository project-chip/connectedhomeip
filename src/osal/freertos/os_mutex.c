/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC
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

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include <chip/osal.h>
#include "os_hw.h"

chip_os_error_t chip_os_mutex_init(struct chip_os_mutex * mu)
{
    if (!mu)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    mu->handle = xSemaphoreCreateRecursiveMutex();
    assert(mu->handle);

    return (mu->handle == NULL) ? CHIP_OS_BAD_MUTEX : CHIP_OS_OK;
}

chip_os_error_t chip_os_mutex_take(struct chip_os_mutex * mu, chip_os_time_t timeout)
{
    BaseType_t ret;

    assert(!chip_hw_in_isr());

    if (!mu)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    assert(mu->handle);

    ret = xSemaphoreTakeRecursive(mu->handle, timeout);

    return ret == pdPASS ? CHIP_OS_OK : CHIP_OS_TIMEOUT;
}

chip_os_error_t chip_os_mutex_give(struct chip_os_mutex * mu)
{
    assert(!chip_hw_in_isr());

    if (!mu)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    assert(mu->handle);

    if (xSemaphoreGiveRecursive(mu->handle) != pdPASS)
    {
        return CHIP_OS_BAD_MUTEX;
    }

    return CHIP_OS_OK;
}
