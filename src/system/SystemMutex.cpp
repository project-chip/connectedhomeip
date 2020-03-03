/*
 *
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file defines the abstraction of mutual exclusion locks
 *      offered by the target platform.
 */

// Include module header
#include <SystemMutex.h>

// Include common private header
#include "SystemLayerPrivate.h"

#if !WEAVE_SYSTEM_CONFIG_NO_LOCKING

// Include system headers
#include <errno.h>

namespace nl {
namespace Weave {
namespace System {

/**
 * Initialize the mutual exclusion lock instance.
 *
 *  @param[inout]   aThis   A zero-initialized object.
 *
 *  @retval         #WEAVE_SYSTEM_NO_ERROR                  The mutual exclusion lock is ready to use.
 *  @retval         #WEAVE_SYSTEM_ERROR_NO_MEMORY           Insufficient system memory to allocate the mutual exclusion lock.
 *  @retval         #WEAVE_SYSTEM_ERROR_UNEXPECTED_STATE    An unexpected system error encountered during initialization.
 */

#if WEAVE_SYSTEM_CONFIG_POSIX_LOCKING
NL_DLL_EXPORT Error Mutex::Init(Mutex& aThis)
{
    int lSysError = pthread_mutex_init(&aThis.mPOSIXMutex, NULL);
    Error lError;

    switch (lSysError)
    {
    case 0:
        lError = WEAVE_SYSTEM_NO_ERROR;
        break;

    case ENOMEM:
        lError = WEAVE_SYSTEM_ERROR_NO_MEMORY;
        break;

    default:
        lError = WEAVE_SYSTEM_ERROR_UNEXPECTED_STATE;
        break;
    }

    return lError;
}
#endif // WEAVE_SYSTEM_CONFIG_POSIX_LOCKING

#if WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING
NL_DLL_EXPORT Error Mutex::Init(Mutex& aThis)
{
restart:
    if (__sync_bool_compare_and_swap(&aThis.mInitialized, 0, 1))
    {
#if (configSUPPORT_STATIC_ALLOCATION == 1)
        aThis.mFreeRTOSSemaphore = xSemaphoreCreateMutexStatic(&aThis.mFreeRTOSSemaphoreObj);
#else
        aThis.mFreeRTOSSemaphore = xSemaphoreCreateMutex();
#endif
        if (aThis.mFreeRTOSSemaphore == NULL)
        {
            aThis.mInitialized = 0;

            return WEAVE_SYSTEM_ERROR_NO_MEMORY;
        }
    } else {
        while (aThis.mFreeRTOSSemaphore == NULL)
        {
            vTaskDelay(1);

            if (aThis.mInitialized == 0)
            {
                goto restart;
            }
        }
    }

    return WEAVE_SYSTEM_NO_ERROR;
}

NL_DLL_EXPORT void Mutex::Lock(void)
{
    xSemaphoreTake(this->mFreeRTOSSemaphore, portMAX_DELAY);
}
#endif // WEAVE_SYSTEM_CONFIG_FREERTOS_LOCKING

} // namespace System
} // namespace Weave
} // namespace nl

#endif // !WEAVE_SYSTEM_CONFIG_NO_LOCKING
