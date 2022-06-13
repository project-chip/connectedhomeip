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

void sleep(uint32_t sec)
{
    uint32_t ms = (sec * 1000);

    if (ms)
    {
        osDelay(ms2tick(ms));
    }
}
