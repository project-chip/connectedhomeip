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
#include <chip/osal.h>
#include "os_hw.h"

chip_os_error_t chip_os_sem_init(struct chip_os_sem * sem, uint16_t tokens)
{
    if (!sem)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    sem->handle = xSemaphoreCreateCounting(128, tokens);
    assert(sem->handle);

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_sem_take(struct chip_os_sem * sem, chip_os_time_t timeout)
{
    BaseType_t woken;
    BaseType_t ret;

    if (!sem)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    assert(sem->handle);

    if (chip_hw_in_isr())
    {
        assert(timeout == 0);
        ret = xSemaphoreTakeFromISR(sem->handle, &woken);
        portYIELD_FROM_ISR(woken);
    }
    else
    {
        ret = xSemaphoreTake(sem->handle, timeout);
    }

    return ret == pdPASS ? CHIP_OS_OK : CHIP_OS_TIMEOUT;
}

chip_os_error_t chip_os_sem_give(struct chip_os_sem * sem)
{
    BaseType_t ret;
    BaseType_t woken;

    if (!sem)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    assert(sem->handle);

    if (chip_hw_in_isr())
    {
        ret = xSemaphoreGiveFromISR(sem->handle, &woken);
        portYIELD_FROM_ISR(woken);
    }
    else
    {
        ret = xSemaphoreGive(sem->handle);
    }

    assert(ret == pdPASS);
    return CHIP_OS_OK;
}
