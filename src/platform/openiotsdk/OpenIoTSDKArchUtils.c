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

#include <time.h>
#include "OpenIoTSDKArchUtils.h"
#ifdef __cplusplus
extern "C" {
#endif

static uint32_t tick_h      = 0U;
static uint32_t tick_l      = 0U;
static uint64_t tick_offset = 0U;

// GetTick() needs to be called at least twice per tick overflow to work properly.
uint64_t GetTick(void)
{
    uint32_t tick = osKernelGetTickCount();

    if (tick < tick_l)
    {
        tick_h = tick_h + 1;
    }
    tick_l = tick;

    return (((uint64_t) tick_h << 32) | tick_l) + tick_offset;
}

void SetTick(uint64_t newTick)
{
    tick_offset = newTick - GetTick();
}

/* Time to kernel ticks */
uint32_t ms2tick(uint32_t ms)
{
    if (ms == 0U) {
        return (osWaitForever);
    }

    uint32_t tick_freq = osKernelGetTickFreq();
    if (tick_freq == 1000U) {
        // No scaling for 1ms ticks
        tick_freq = 0U;
    }
    if (tick_freq != 0U) {
        ms = (ms * tick_freq) / 1000U;
    }
    return (ms);
}

uint32_t us2tick(uint32_t usec)
{
    if (usec == 0U) {
        return osWaitForever;
    }

    uint32_t tick_freq = osKernelGetTickFreq();

    // 0 tick_freq is a special case where 1 tick = 1 ms
    if (tick_freq == 0U) {
        tick_freq = 1000;
    }
    
    // round division up because our tick is so long this might become 0
    // we need the timer to sleep at least one tick as it otherwise breaks expectations
    uint32_t ticks = (uint32_t)(((uint64_t)usec * tick_freq + (1000000 - 1)) / 1000000);

    return ticks;
}

void sleep(uint32_t sec)
{
    uint32_t ms = (sec * 1000);

    if (ms)
    {
        osDelay(ms2tick(ms));
    }
}

void usleep(uint32_t usec)
{
    if (usec)
    {
        osDelay(us2tick(usec));
    }
}

time_t time(time_t *__timer)
{
    time_t seconds = (time_t)(GetTick() / osKernelGetTickFreq());
    if (__timer)
    {
        *__timer = seconds;
    }
    return seconds;
}

#ifdef __cplusplus
}
#endif
