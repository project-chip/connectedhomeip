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

/**
 *    @file
 *          Provides an definitions of CHIP OSAL data structures for portability
 *          to POSIX platforms.
 */

#ifndef CHIP_OS_TIME_H
#define CHIP_OS_TIME_H

#include <time.h>

#ifdef __APPLE__

#include <dispatch/dispatch.h>

// Apple MacOS
#define CHIP_OS_TIME_FOREVER DISPATCH_TIME_FOREVER
#define CHIP_OS_TIME_NO_WAIT 0
#define CHIP_OS_TICKS_PER_SEC 1000000

typedef uint64_t chip_os_time_t;
typedef int64_t chip_os_stime_t;

#else

// Linux and stock POSIX
#define CHIP_OS_TIME_FOREVER INT32_MAX
#define CHIP_OS_TIME_NO_WAIT 0
#define CHIP_OS_TICKS_PER_SEC 1000

typedef uint32_t chip_os_time_t;
typedef int32_t chip_os_stime_t;

#endif

struct chip_os_timer
{
    chip_os_timer_fn * tm_cb;
    chip_os_time_t tm_ticks;
#ifdef __APPLE__
    dispatch_source_t tm_timer;
#else
    timer_t tm_timer;
#endif
    bool tm_active;
    void * tm_arg;
};

#endif // CHIP_OS_TIME_H
