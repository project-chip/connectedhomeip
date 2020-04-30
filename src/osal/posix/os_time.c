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

#ifdef __APPLE__
// OS X does not have clock_gettime, use clock_get_time

#include <mach/clock.h>
#include <mach/mach.h>

chip_os_time_t chip_os_time_get(void)
{
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return mts.tv_sec * 1000.0 + mts.tv_nsec / 1000000.;
}

#else
// True POSIX Implementation

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

#endif // __APPLE__

chip_os_time_t chip_os_time_ms_to_ticks(chip_os_time_t ms)
{
    return (ms * CHIP_OS_TICKS_PER_SEC) / 1000;
}

chip_os_time_t chip_os_time_ticks_to_ms(chip_os_time_t ticks)
{
    return (ticks * 1000) / CHIP_OS_TICKS_PER_SEC;
}
