/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Open IOT SDK platform adaptation file
 */

#include "OpenIoTSDKArchUtils.h"
#include <time.h>
#ifdef __cplusplus
extern "C" {
#endif

static uint32_t tick_h = 0U;
static uint32_t tick_l = 0U;

// GetTick() needs to be called at least twice per tick overflow to work properly.
uint64_t GetTick(void)
{
    uint32_t tick = osKernelGetTickCount();

    if (tick < tick_l)
    {
        tick_h++;
    }
    tick_l = tick;

    return (((uint64_t) tick_h << 32) | tick_l);
}

/* Time to kernel ticks */
uint32_t sec2tick(uint32_t sec)
{
    return (sec * osKernelGetTickFreq());
}

uint32_t ms2tick(uint32_t ms)
{
    return (uint32_t) (((uint64_t) ms * (uint64_t) osKernelGetTickFreq()) / 1000U);
}

uint32_t us2tick(uint32_t usec)
{
    // round division up because our tick is so long this might become 0
    // we need the timer to sleep at least one tick as it otherwise breaks expectations
    return (uint32_t) (((uint64_t) usec * osKernelGetTickFreq() + (1000000 - 1)) / 1000000);
}

/* Kernel ticks to time */
uint64_t tick2sec(uint64_t tick)
{
    if (osKernelGetTickFreq() == 0U)
    {
        return 0;
    }

    return (tick / osKernelGetTickFreq());
}

uint64_t tick2ms(uint64_t tick)
{
    if (osKernelGetTickFreq() == 0U)
    {
        return 0;
    }

    return ((tick * 1000U) / osKernelGetTickFreq());
}

uint64_t tick2us(uint64_t tick)
{
    if (osKernelGetTickFreq() == 0U)
    {
        return 0;
    }

    return ((tick * 1000U * 1000U) / osKernelGetTickFreq());
}

void sleep(uint32_t sec)
{
    if (sec)
    {
        osDelay(sec2tick(sec));
    }
}

void usleep(uint32_t usec)
{
    if (usec)
    {
        osDelay(us2tick(usec));
    }
}

time_t time(time_t * __timer)
{
    time_t seconds = (time_t) (tick2sec(GetTick()));
    if (__timer)
    {
        *__timer = seconds;
    }
    return seconds;
}

#ifdef __cplusplus
}
#endif
