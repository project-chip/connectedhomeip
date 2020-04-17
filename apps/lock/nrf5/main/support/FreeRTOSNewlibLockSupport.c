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

#include <sys/lock.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

/*
 * Global mutex objects used by newlib.
 */

SemaphoreHandle_t __lock___sinit_recursive_mutex;
SemaphoreHandle_t __lock___sfp_recursive_mutex;
SemaphoreHandle_t __lock___atexit_recursive_mutex;
SemaphoreHandle_t __lock___at_quick_exit_mutex;
SemaphoreHandle_t __lock___malloc_recursive_mutex;
SemaphoreHandle_t __lock___env_recursive_mutex;
SemaphoreHandle_t __lock___tz_mutex;
SemaphoreHandle_t __lock___dd_hash_mutex;
SemaphoreHandle_t __lock___arc4random_mutex;

/**
 * Global "constructor" function for initializing newlib mutexes at system start.
 *
 * This is called automatically at system start by the C runtime.
 */
__attribute__((constructor))
static void InitNewlibMutexes(void)
{
#if USE_STATIC_NEWLIB_MUTEXES

    static StaticSemaphore_t sSemBuf_sinit_recursive_mutex;
    static StaticSemaphore_t sSemBuf_sfp_recursive_mutex;
    static StaticSemaphore_t sSemBuf_atexit_recursive_mutex;
    static StaticSemaphore_t sSemBuf_at_quick_exit_mutex;
    static StaticSemaphore_t sSemBuf_env_recursive_mutex;
    static StaticSemaphore_t sSemBuf_tz_mutex;
    static StaticSemaphore_t sSemBuf_dd_hash_mutex;
    static StaticSemaphore_t sSemBuf_arc4random_mutex;

    __lock___sinit_recursive_mutex  = xSemaphoreCreateRecursiveMutexStatic(&sSemBuf_sinit_recursive_mutex);
    __lock___sfp_recursive_mutex    = xSemaphoreCreateRecursiveMutexStatic(&sSemBuf_sfp_recursive_mutex);
    __lock___atexit_recursive_mutex = xSemaphoreCreateRecursiveMutexStatic(&sSemBuf_atexit_recursive_mutex);
    __lock___at_quick_exit_mutex    = xSemaphoreCreateMutexStatic(&sSemBuf_at_quick_exit_mutex);
    __lock___env_recursive_mutex    = xSemaphoreCreateRecursiveMutexStatic(&sSemBuf_env_recursive_mutex);
    __lock___tz_mutex               = xSemaphoreCreateMutexStatic(&sSemBuf_tz_mutex);
    __lock___dd_hash_mutex          = xSemaphoreCreateMutexStatic(&sSemBuf_dd_hash_mutex);
    __lock___arc4random_mutex       = xSemaphoreCreateMutexStatic(&sSemBuf_arc4random_mutex);

#else /* USE_STATIC_NEWLIB_MUTEXES */

    __lock___sinit_recursive_mutex  = xSemaphoreCreateRecursiveMutex();
    __lock___sfp_recursive_mutex    = xSemaphoreCreateRecursiveMutex();
    __lock___atexit_recursive_mutex = xSemaphoreCreateRecursiveMutex();
    __lock___at_quick_exit_mutex    = xSemaphoreCreateMutex();
    __lock___env_recursive_mutex    = xSemaphoreCreateRecursiveMutex();
    __lock___tz_mutex               = xSemaphoreCreateMutex();
    __lock___dd_hash_mutex          = xSemaphoreCreateMutex();
    __lock___arc4random_mutex       = xSemaphoreCreateMutex();

#endif /* USE_STATIC_NEWLIB_MUTEXES */
}

/*
 * Overrides for newlib's retargetable locking functions.
 */

void __retarget_lock_init(_LOCK_T * lock)
{
    *lock = (_LOCK_T)xSemaphoreCreateMutex();
}

void __retarget_lock_init_recursive(_LOCK_T * lock)
{
    *lock = (_LOCK_T)xSemaphoreCreateRecursiveMutex();
}

void __retarget_lock_close(_LOCK_T lock)
{
    vSemaphoreDelete((SemaphoreHandle_t)lock);
}

void __retarget_lock_close_recursive(_LOCK_T lock)
{
    vSemaphoreDelete((SemaphoreHandle_t)lock);
}

void __retarget_lock_acquire(_LOCK_T lock)
{
    TickType_t waitTicks = (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) ? 0 : portMAX_DELAY;
    xSemaphoreTake((SemaphoreHandle_t)lock, waitTicks);
}

void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    TickType_t waitTicks = (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) ? 0 : portMAX_DELAY;
    xSemaphoreTakeRecursive((SemaphoreHandle_t)lock, waitTicks);
}

int __retarget_lock_try_acquire(_LOCK_T lock)
{
    return xSemaphoreTake((SemaphoreHandle_t)lock, 0) == pdTRUE ? 1 : 0;
}

int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return xSemaphoreTakeRecursive((SemaphoreHandle_t)lock, 0) == pdTRUE ? 1 : 0;
}

void __retarget_lock_release(_LOCK_T lock)
{
    xSemaphoreGive((SemaphoreHandle_t)lock);
}

void __retarget_lock_release_recursive(_LOCK_T lock)
{
    xSemaphoreGiveRecursive((SemaphoreHandle_t)lock);
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

