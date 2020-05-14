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

#include <chip/osal.h>
#include "os_hw.h"

static void chip_os_timer_cb(TimerHandle_t freertosTimer)
{
    struct chip_os_timer * timer = pvTimerGetTimerID(freertosTimer);
    assert(timer);

    chip_os_timer_stop(timer);

    timer->func(timer->arg);
}

chip_os_error_t chip_os_timer_init(struct chip_os_timer * timer, chip_os_timer_fn * tm_cb, void * tm_arg)
{
    memset(timer, 0, sizeof(*timer));
    timer->func   = tm_cb;
    timer->arg    = tm_arg;
    timer->handle = xTimerCreate("timer", 1, pdFALSE, timer, chip_os_timer_cb);

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_timer_start(struct chip_os_timer * timer, chip_os_time_t ticks)
{
    BaseType_t woken1, woken2, woken3;

    if (ticks < 0)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    if (ticks == 0)
    {
        ticks = 1;
    }

    if (chip_hw_in_isr())
    {
        xTimerStopFromISR(timer->handle, &woken1);
        xTimerChangePeriodFromISR(timer->handle, ticks, &woken2);
        xTimerResetFromISR(timer->handle, &woken3);

        portYIELD_FROM_ISR(woken1 || woken2 || woken3);
    }
    else
    {
        xTimerStop(timer->handle, portMAX_DELAY);
        xTimerChangePeriod(timer->handle, ticks, portMAX_DELAY);
        xTimerReset(timer->handle, portMAX_DELAY);
    }

    return CHIP_OS_OK;
}

chip_os_time_t chip_os_timer_remaining_ticks(struct chip_os_timer * timer, chip_os_time_t now)
{
    chip_os_time_t rt;
    uint32_t exp;

    exp = xTimerGetExpiryTime(timer->handle);

    if (exp > now)
    {
        rt = exp - now;
    }
    else
    {
        rt = 0;
    }

    return rt;
}
