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

#include "os_hw.h"

#include <chip/osal.h>

chip_os_error_t chip_os_queue_get(struct chip_os_queue * queue, void * data, chip_os_time_t tmo)
{
    BaseType_t woken;
    BaseType_t ret;

    if (chip_hw_in_isr())
    {
        assert(tmo == 0);
        ret = xQueueReceiveFromISR(queue->handle, data, &woken);
        portYIELD_FROM_ISR(woken);
    }
    else
    {
        ret = xQueueReceive(queue->handle, data, tmo);
    }
    assert(ret == pdPASS || ret == errQUEUE_EMPTY);

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_queue_put(struct chip_os_queue * queue, void * data)
{
    BaseType_t woken;
    BaseType_t ret;

    if (chip_hw_in_isr())
    {
        ret = xQueueSendToBackFromISR(queue->handle, data, &woken);
        portYIELD_FROM_ISR(woken);
    }
    else
    {
        ret = xQueueSendToBack(queue->handle, data, portMAX_DELAY);
    }

    assert(ret == pdPASS);

    return CHIP_OS_OK;
}
