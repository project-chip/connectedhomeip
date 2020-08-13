/*
 *
 *    Copyright (c) 2019 Google LLC.
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
 *          Implementation of newlib's retargetable locking functions and global
 *          locks for use on FreeRTOS systems.
 *
 */

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include <stdlib.h>
#include <sys/lock.h>

struct __lock
{
    SemaphoreHandle_t semaphore;
};

/*
 * Global mutex objects used by newlib.
 */

struct __lock __lock___sinit_recursive_mutex;
struct __lock __lock___sfp_recursive_mutex;
struct __lock __lock___atexit_recursive_mutex;
struct __lock __lock___at_quick_exit_mutex;
struct __lock __lock___malloc_recursive_mutex;
struct __lock __lock___env_recursive_mutex;
struct __lock __lock___tz_mutex;
struct __lock __lock___dd_hash_mutex;
struct __lock __lock___arc4random_mutex;

/**
 * Global "constructor" function for initializing newlib mutexes at system start.
 *
 * This is called automatically at system start by the C runtime.
 */
__attribute__((constructor)) static void InitNewlibMutexes(void)
{
    __lock___sinit_recursive_mutex.semaphore  = xSemaphoreCreateRecursiveMutex();
    __lock___sfp_recursive_mutex.semaphore    = xSemaphoreCreateRecursiveMutex();
    __lock___atexit_recursive_mutex.semaphore = xSemaphoreCreateRecursiveMutex();
    __lock___at_quick_exit_mutex.semaphore    = xSemaphoreCreateMutex();
    __lock___env_recursive_mutex.semaphore    = xSemaphoreCreateRecursiveMutex();
    __lock___tz_mutex.semaphore               = xSemaphoreCreateMutex();
    __lock___dd_hash_mutex.semaphore          = xSemaphoreCreateMutex();
    __lock___arc4random_mutex.semaphore       = xSemaphoreCreateMutex();
}

/*
 * Overrides for newlib's retargetable locking functions.
 */

void __retarget_lock_init(_LOCK_T * lock_ptr)
{
    _LOCK_T lock = malloc(sizeof(*lock));
    ASSERT(lock);
    *lock_ptr       = lock;
    lock->semaphore = xSemaphoreCreateMutex();
}

void __retarget_lock_init_recursive(_LOCK_T * lock_ptr)
{
    _LOCK_T lock = malloc(sizeof(*lock));
    ASSERT(lock);
    *lock_ptr       = lock;
    lock->semaphore = xSemaphoreCreateRecursiveMutex();
}

void __retarget_lock_close(_LOCK_T lock)
{
    vSemaphoreDelete(lock->semaphore);
    free(lock);
}

void __retarget_lock_close_recursive(_LOCK_T lock)
{
    vSemaphoreDelete(lock->semaphore);
    free(lock);
}

void __retarget_lock_acquire(_LOCK_T lock)
{
    TickType_t waitTicks = (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) ? 0 : portMAX_DELAY;
    xSemaphoreTake(lock->semaphore, waitTicks);
}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    TickType_t waitTicks = (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) ? 0 : portMAX_DELAY;
    xSemaphoreTakeRecursive(lock->semaphore, waitTicks);
}

int __retarget_lock_try_acquire(_LOCK_T lock)
{
    return xSemaphoreTake(lock->semaphore, 0) == pdTRUE ? 1 : 0;
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return xSemaphoreTakeRecursive(lock->semaphore, 0) == pdTRUE ? 1 : 0;
}

void __retarget_lock_release(_LOCK_T lock)
{
    xSemaphoreGive(lock->semaphore);
}

void __retarget_lock_release_recursive(_LOCK_T lock)
{
    xSemaphoreGiveRecursive(lock->semaphore);
}

/*
 * Overrides for newlib's malloc locking functions.
 *
 * These are overridden specially to use critical sections rather than FreeRTOS semaphores
 * to improve speed.
 */

void __malloc_lock(struct _reent * r)
{
    taskENTER_CRITICAL();
}

void __malloc_unlock(struct _reent * r)
{
    taskEXIT_CRITICAL();
}
