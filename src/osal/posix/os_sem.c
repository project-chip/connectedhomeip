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
#include <errno.h>
#include <semaphore.h>

#include <chip/osal.h>
#include "os_utils.h"

#ifdef __APPLE__

chip_os_error_t chip_os_sem_init(struct chip_os_sem * sem, uint16_t tokens)
{
    sem->lock = dispatch_semaphore_create(tokens);
    return (sem->lock == NULL) ? CHIP_OS_ENOMEM : CHIP_OS_OK;
}

chip_os_error_t chip_os_sem_give(struct chip_os_sem * sem)
{
    int woke = dispatch_semaphore_signal(sem->lock);
    return CHIP_OS_OK;
}

chip_os_error_t chip_os_sem_take(struct chip_os_sem * sem, chip_os_time_t timeout)
{
    int expired = dispatch_semaphore_wait(sem->lock, timeout);
    return (expired) ? CHIP_OS_TIMEOUT : CHIP_OS_OK;
}

#else

chip_os_error_t chip_os_sem_init(struct chip_os_sem * sem, uint16_t tokens)
{
    int ret = sem_init(&sem->lock, 0, tokens);
    SuccessOrExit(ret);
exit:
    return map_posix_to_osal_error(ret);
}

chip_os_error_t chip_os_sem_give(struct chip_os_sem * sem)
{
    int ret = sem_post(&sem->lock);
    SuccessOrExit(ret);
exit:
    return map_posix_to_osal_error(ret);
}

chip_os_error_t chip_os_sem_take(struct chip_os_sem * sem, chip_os_time_t timeout)
{
    int err = CHIP_OS_OK;
    struct timespec wait;

    if (timeout == CHIP_OS_TIME_FOREVER)
    {
        err = sem_wait(&sem->lock);
    }
    else
    {
        err = clock_gettime(CLOCK_REALTIME, &wait);
        if (err)
        {
            return CHIP_OS_ERROR;
        }

        wait.tv_sec += timeout / 1000;
        wait.tv_nsec += (timeout % 1000) * 1000000;
        err = sem_timedwait(&sem->lock, &wait);
        if (err && errno == ETIMEDOUT)
        {
            return CHIP_OS_TIMEOUT;
        }
    }

    return (err) ? CHIP_OS_ERROR : CHIP_OS_OK;
}

#endif // __APPLE__
