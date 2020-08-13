/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "FreeRTOSNewlibLockSupport_test.h"

#include <FreeRTOS.h>
#include <assert.h>
#include <semphr.h>
#include <stdbool.h>
#include <sys/lock.h>

#ifndef __SINGLE_THREAD__
__LOCK_INIT(static, test_lock);
__LOCK_INIT_RECURSIVE(static, test_lock_recursive);

struct __lock
{
    SemaphoreHandle_t semaphore;
};

struct __lock __lock_test_lock;
struct __lock __lock_test_lock_recursive;

__attribute__((constructor)) static void init_static_lock_test_mutexes(void)
{
    __lock_test_lock.semaphore           = xSemaphoreCreateMutex();
    __lock_test_lock_recursive.semaphore = xSemaphoreCreateRecursiveMutex();
}

void freertos_newlib_lock_test()
{
    __lock_acquire(test_lock);
    bool acquired = __lock_try_acquire(test_lock);
    ASSERT(!acquired);
    __lock_release(test_lock);
    acquired = __lock_try_acquire(test_lock);
    ASSERT(acquired);
    __lock_release(test_lock);

    __lock_acquire_recursive(test_lock_recursive);
    __lock_acquire_recursive(test_lock_recursive);
    acquired = __lock_try_acquire_recursive(test_lock_recursive);
    ASSERT(acquired);
    __lock_release_recursive(test_lock_recursive);
    __lock_release_recursive(test_lock_recursive);
    __lock_release_recursive(test_lock_recursive);

    _LOCK_T dynamic_lock;
    __lock_init(dynamic_lock);
    __lock_acquire(dynamic_lock);
    acquired = __lock_try_acquire(dynamic_lock);
    ASSERT(!acquired);
    __lock_release(dynamic_lock);
    acquired = __lock_try_acquire(dynamic_lock);
    ASSERT(acquired);
    __lock_release(dynamic_lock);

    _LOCK_T dynamic_lock_recursive;
    __lock_init_recursive(dynamic_lock_recursive);
    __lock_acquire_recursive(dynamic_lock_recursive);
    acquired = __lock_try_acquire_recursive(dynamic_lock_recursive);
    ASSERT(acquired);
    __lock_release_recursive(dynamic_lock);
    acquired = __lock_try_acquire_recursive(dynamic_lock_recursive);
    ASSERT(acquired);
    __lock_release_recursive(dynamic_lock_recursive);
}
#else
void freertos_newlib_lock_test() {}
#endif
