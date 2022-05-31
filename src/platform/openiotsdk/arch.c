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

#include "cmsis_os2.h"
#include <time.h>
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
static uint32_t ms2tick(uint32_t ms)
{
    uint32_t tick_freq = osKernelGetTickFreq();
    if (tick_freq == 1000U) {
        // No scaling for 1ms ticks
        tick_freq = 0U;
    }

    if (ms == 0U) {
        return (osWaitForever);
    }
    if (tick_freq != 0U) {
        ms = (ms * tick_freq) / 1000U;
    }
    return (ms);
}

static uint32_t us2tick(uint32_t usec)
{
    uint32_t tick_freq = osKernelGetTickFreq();
    if (usec == 0U) {
        return osWaitForever;
    }
    if (tick_freq != 0U) {
        usec = (uint32_t)(((uint64_t)usec * osKernelGetTickFreq()) / 1000000);
    }
    return usec;
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
