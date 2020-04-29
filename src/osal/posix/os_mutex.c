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
#include <pthread.h>

#include <chip/osal.h>

chip_os_error_t chip_os_mutex_init(struct chip_os_mutex * mu)
{
    if (!mu)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    pthread_mutexattr_init(&mu->attr);
    pthread_mutexattr_settype(&mu->attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&mu->lock, &mu->attr);

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_mutex_give(struct chip_os_mutex * mu)
{
    if (!mu)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    if (pthread_mutex_unlock(&mu->lock))
    {
        return CHIP_OS_BAD_MUTEX;
    }

    return CHIP_OS_OK;
}

chip_os_error_t chip_os_mutex_take(struct chip_os_mutex * mu, uint32_t timeout)
{
    int err;

    if (!mu)
    {
        return CHIP_OS_INVALID_PARAM;
    }

    if (timeout == CHIP_OS_TIME_FOREVER)
    {
        err = pthread_mutex_lock(&mu->lock);
    }
    else
    {
        err = clock_gettime(CLOCK_REALTIME, &mu->wait);
        if (err)
        {
            return CHIP_OS_ERROR;
        }

        mu->wait.tv_sec += timeout / 1000;
        mu->wait.tv_nsec += (timeout % 1000) * 1000000;

        err = pthread_mutex_timedlock(&mu->lock, &mu->wait);
        if (err == ETIMEDOUT)
        {
            return CHIP_OS_TIMEOUT;
        }
    }

    return (err) ? CHIP_OS_ERROR : CHIP_OS_OK;
}
