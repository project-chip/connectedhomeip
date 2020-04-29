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

#include <errno.h>
#include <semaphore.h>

#include <chip/osal.h>

chip_os_error_t chip_os_sem_init(struct chip_os_sem * sem, uint16_t tokens)
{
    if (!sem)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    sem_init(&sem->lock, 0, tokens);

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_sem_give(struct chip_os_sem * sem)
{
    int err;

    if (!sem)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    err = sem_post(&sem->lock);

    return (err) ? CHIP_OS_ERROR : CHIP_OS_OK;
}

chip_os_error_t chip_os_sem_take(struct chip_os_sem * sem, uint32_t timeout)
{
    int err = 0;
    struct timespec wait;

    if (!sem)
    {
        return CHIP_OS_INVALID_PARAM;
    }

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

uint16_t chip_os_sem_get_count(struct chip_os_sem * sem)
{
    int count;

    assert(sem);
    assert(&sem->lock);
    sem_getvalue(&sem->lock, &count);

    return count;
}
