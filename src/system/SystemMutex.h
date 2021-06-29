/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
 *      This file declares the abstraction of mutual exclusion locks
 *      offered by the target platform.
 */

#pragma once

// Include configuration headers
#include <system/SystemConfig.h>

// Include dependent headers
#include <system/SystemError.h>

#include <support/DLLUtil.h>

#if !CHIP_SYSTEM_CONFIG_NO_LOCKING

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
#include <pthread.h>
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#else
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#endif
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

#if CHIP_SYSTEM_CONFIG_MBED_LOCKING
#include <rtos/Mutex.h>
#endif // CHIP_SYSTEM_CONFIG_MBED_LOCKING

namespace chip {
namespace System {

/**
 *  @class Mutex
 *
 *  @brief
 *      This class represents a simple mutual exclusion lock used on platforms with preemptively scheduled multi-threaded
 *      programming environments, for example, POSIX threads and FreeRTOS. The lock is non-recursive, and may not be used
 *      in a hardware interrupt context. The constructor and destructor are defined as null functions to facilitate using
 *      objects with \c static storage duration and uninitialized memory. Use \c Init method to initialize. The copy/move
 *      operators are not provided.
 *
 */
class DLL_EXPORT Mutex
{
public:
    Mutex();
    ~Mutex();

    static Error Init(Mutex & aMutex);

    void Lock();   /**< Acquire the mutual exclusion lock, blocking the current thread indefinitely if necessary. */
    void Unlock(); /**< Release the mutual exclusion lock (can block on some systems until scheduler completes). */

private:
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    pthread_mutex_t mPOSIXMutex;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t mFreeRTOSSemaphoreObj;
#endif // (configSUPPORT_STATIC_ALLOCATION == 1)
    volatile SemaphoreHandle_t mFreeRTOSSemaphore = nullptr;
    volatile int mInitialized                     = 0;
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

#if CHIP_SYSTEM_CONFIG_MBED_LOCKING
    rtos::Mutex mMbedMutex;
#endif // CHIP_SYSTEM_CONFIG_MBED_LOCKING

    Mutex(const Mutex &) = delete;
    Mutex & operator=(const Mutex &) = delete;
};

inline Mutex::Mutex() {}

inline Mutex::~Mutex() {}

#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
inline void Mutex::Lock()
{
    pthread_mutex_lock(&this->mPOSIXMutex);
}

inline void Mutex::Unlock()
{
    pthread_mutex_unlock(&this->mPOSIXMutex);
}
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
inline void Mutex::Unlock(void)
{
    xSemaphoreGive(this->mFreeRTOSSemaphore);
}
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

#if CHIP_SYSTEM_CONFIG_MBED_LOCKING
inline Error Mutex::Init(Mutex & aMutex)
{
    // The mutex is initialized when constructed and generates
    // a runtime error in case of failure.
    return CHIP_SYSTEM_NO_ERROR;
}

inline void Mutex::Lock()
{
    return mMbedMutex.lock();
}

inline void Mutex::Unlock(void)
{
    return mMbedMutex.unlock();
}
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

} // namespace System
} // namespace chip

#endif // !CHIP_SYSTEM_CONFIG_NO_LOCKING
