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

#include <time.h>

#include <chip/osal.h>

/**
 * Return ticks [ms] since system start as uint32_t.
 */
chip_os_time_t chip_os_time_get(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now))
    {
        return 0;
    }
    return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

chip_os_error_t chip_os_time_ms_to_ticks(uint32_t ms, chip_os_time_t * out_ticks)
{
    *out_ticks = ms;

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_time_ticks_to_ms(chip_os_time_t ticks, uint32_t * out_ms)
{
    *out_ms = ticks;

    return CHIP_OS_OK;
}

chip_os_time_t chip_os_time_ms_to_ticks32(uint32_t ms)
{
    return ms;
}

uint32_t chip_os_time_ticks_to_ms32(chip_os_time_t ticks)
{
    return ticks;
}

void chip_os_time_delay(chip_os_time_t ticks)
{
    struct timespec sleep_time;
    long ms    = chip_os_time_ticks_to_ms32(ticks);
    uint32_t s = ms / 1000;

    ms -= s * 1000;
    sleep_time.tv_sec  = s;
    sleep_time.tv_nsec = ms * 1000000;

    nanosleep(&sleep_time, NULL);
}
