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

#ifndef CHIP_OS_TYPES_H
#define CHIP_OS_TYPES_H

#include <pthread.h>
#include <semaphore.h>

/* The highest and lowest task priorities */
#define OS_TASK_PRI_HIGHEST (sched_get_priority_max(SCHED_RR))
#define OS_TASK_PRI_LOWEST (sched_get_priority_min(SCHED_RR))

typedef uint32_t chip_os_time_t;
typedef int32_t chip_os_stime_t;

typedef int chip_os_base_t;
typedef int chip_os_stack_t;

struct chip_os_queue
{
    void * q;
    chip_os_signal_fn * sig_cb;
    void * sig_arg;
};

struct chip_os_timer
{
    chip_os_timer_fn * tm_cb;
    uint32_t tm_ticks;
    timer_t tm_timer;
    bool tm_active;
    void * tm_arg;
};

struct chip_os_mutex
{
    pthread_mutex_t lock;
    pthread_mutexattr_t attr;
    struct timespec wait;
};

struct chip_os_sem
{
    sem_t lock;
};

struct chip_os_task
{
    pthread_t handle;
    pthread_attr_t attr;
    struct sched_param param;
    const char * name;
};

typedef void * (*chip_os_task_func_t)(void *);

#endif // CHIP_OS_TYPES_H
