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
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <system/SystemError.h>

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

#if CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING
#include <cmsis_os2.h>
#endif // CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING

#if CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING
#include <zephyr/kernel.h>
#endif

namespace chip {
namespace System {

// Enable thread safety attributes only with clang.
#if defined(__clang__) && (!defined(SWIG))
#define CHIP_TSA_ATTRIBUTE__(x) __attribute__((x))
#else
#define CHIP_TSA_ATTRIBUTE__(x)
#endif

#define CHIP_CAPABILITY(x) CHIP_TSA_ATTRIBUTE__(capability(x))
#define CHIP_SCOPED_CAPABILITY CHIP_TSA_ATTRIBUTE__(scoped_lockable)
#define CHIP_GUARDED_BY(x) CHIP_TSA_ATTRIBUTE__(guarded_by(x))
#define CHIP_PT_GUARDED_BY(x) CHIP_TSA_ATTRIBUTE__(pt_guarded_by(x))
#define CHIP_ACQUIRED_BEFORE(...) CHIP_TSA_ATTRIBUTE__(acquired_before(__VA_ARGS__))
#define CHIP_ACQUIRED_AFTER(...) CHIP_TSA_ATTRIBUTE__(acquired_after(__VA_ARGS__))
#define CHIP_REQUIRES(...) CHIP_TSA_ATTRIBUTE__(requires_capability(__VA_ARGS__))
#define CHIP_REQUIRES_SHARED(...) CHIP_TSA_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))
#define CHIP_ACQUIRE(...) CHIP_TSA_ATTRIBUTE__(acquire_capability(__VA_ARGS__))
#define CHIP_ACQUIRE_SHARED(...) CHIP_TSA_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))
#define CHIP_RELEASE(...) CHIP_TSA_ATTRIBUTE__(release_capability(__VA_ARGS__))
#define CHIP_RELEASE_SHARED(...) CHIP_TSA_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))
#define CHIP_RELEASE_GENERIC(...) CHIP_TSA_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))
#define CHIP_TRY_ACQUIRE(...) CHIP_TSA_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))
#define CHIP_TRY_ACQUIRE_SHARED(...) CHIP_TSA_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))
#define CHIP_EXCLUDES(...) CHIP_TSA_ATTRIBUTE__(locks_excluded(__VA_ARGS__))
#define CHIP_ASSERT_CAPABILITY(x) CHIP_TSA_ATTRIBUTE__(assert_capability(x))
#define CHIP_ASSERT_SHARED_CAPABILITY(x) CHIP_TSA_ATTRIBUTE__(assert_shared_capability(x))
#define CHIP_RETURN_CAPABILITY(x) CHIP_TSA_ATTRIBUTE__(lock_returned(x))
#define CHIP_NO_THREAD_SAFETY_ANALYSIS CHIP_TSA_ATTRIBUTE__(no_thread_safety_analysis)

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
 * @note
 *      This class is compatible with \c std::lock_guard and provides
 *      annotations for thread safety analysis.
 *
 */
class DLL_EXPORT CHIP_CAPABILITY("mutex") Mutex
{
public:
    Mutex() = default;

    static CHIP_ERROR Init(Mutex & aMutex);
#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
    inline bool isInitialized() { return mInitialized; }
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

    void Lock() CHIP_ACQUIRE();   /**< Acquire the mutual exclusion lock, blocking the current thread indefinitely if necessary. */
    void Unlock() CHIP_RELEASE(); /**< Release the mutual exclusion lock (can block on some systems until scheduler completes). */

    // Synonyms for compatibility with std::lock_guard.
    void lock() CHIP_ACQUIRE() { Lock(); }
    void unlock() CHIP_RELEASE() { Unlock(); }

private:
#if CHIP_SYSTEM_CONFIG_POSIX_LOCKING
    pthread_mutex_t mPOSIXMutex;
#endif // CHIP_SYSTEM_CONFIG_POSIX_LOCKING

#if CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING
#if (configSUPPORT_STATIC_ALLOCATION == 1)
    StaticSemaphore_t mFreeRTOSSemaphoreObj;
#endif // (configSUPPORT_STATIC_ALLOCATION == 1)
    volatile SemaphoreHandle_t mFreeRTOSSemaphore = nullptr;
    volatile bool mInitialized                    = 0;
#endif // CHIP_SYSTEM_CONFIG_FREERTOS_LOCKING

#if CHIP_SYSTEM_CONFIG_MBED_LOCKING
    rtos::Mutex mMbedMutex;
#endif // CHIP_SYSTEM_CONFIG_MBED_LOCKING

#if CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING
    osMutexId_t mCmsisRTOSMutex;
#endif // CHIP_SYSTEM_CONFIG_CMSIS_RTOS_LOCKING

#if CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING
    k_mutex mZephyrMutex;
#endif // CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING

    Mutex(const Mutex &) = delete;
    Mutex & operator=(const Mutex &) = delete;
};

#if CHIP_SYSTEM_CONFIG_NO_LOCKING
inline CHIP_ERROR Init(Mutex & aMutex)
{
    return CHIP_NO_ERROR;
}
inline void Mutex::Lock() {}
inline void Mutex::Unlock() {}
#endif // CHIP_SYSTEM_CONFIG_NO_LOCKING

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
inline CHIP_ERROR Mutex::Init(Mutex & aMutex)
{
    // The mutex is initialized when constructed and generates
    // a runtime error in case of failure.
    return CHIP_NO_ERROR;
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

#if CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING
inline CHIP_ERROR Mutex::Init(Mutex & aMutex)
{
    return System::MapErrorZephyr(k_mutex_init(&aMutex.mZephyrMutex));
}

inline void Mutex::Lock()
{
    VerifyOrDie(0 == k_mutex_lock(&mZephyrMutex, K_FOREVER));
}

inline void Mutex::Unlock(void)
{
    VerifyOrDie(0 == k_mutex_unlock(&mZephyrMutex));
}
#endif // CHIP_SYSTEM_CONFIG_ZEPHYR_LOCKING

} // namespace System
} // namespace chip
