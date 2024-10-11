/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          Provides an implementation of the ThreadStackManager object
 *          for EFR32 platforms using the Silicon Labs SDK and the OpenThread
 *          stack.
 */

#pragma once

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>

#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include "cmsis_os2.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
static constexpr uint32_t threadSrpClearAllFlags = 0x0001U;
#endif

extern "C" void otSysEventSignalPending(void);

namespace chip {
namespace DeviceLayer {

class ThreadStackManager;
class ThreadStackManagerImpl;
namespace Internal {
extern int GetEntropy_EFR32(uint8_t * buf, size_t bufSize);
}

/**
 * Concrete implementation of the ThreadStackManager singleton object for EFR32 platforms
 * using the Silicon Labs SDK and the OpenThread stack.
 */
class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>
{
    // Allow the ThreadStackManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ThreadStackManager;

    // Allow the generic implementation base classes to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;
#endif

    // Allow glue functions called by OpenThread to call helper methods on this
    // class.
    friend void ::otTaskletsSignalPending(otInstance * otInst);
    friend void ::otSysEventSignalPending(void);

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    using ThreadStackManager::InitThreadStack;
    CHIP_ERROR InitThreadStack(otInstance * otInst);
    void FactoryResetThreadStack(void);

private:
    // ===== Methods that implement the ThreadStackManager abstract interface.

    CHIP_ERROR _InitThreadStack(void);
    CHIP_ERROR _StartThreadTask(void);
    void _LockThreadStack(void);
    bool _TryLockThreadStack(void);
    void _UnlockThreadStack(void);

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    void _WaitOnSrpClearAllComplete();
    void _NotifySrpClearAllComplete();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    // ===== Members for internal use by the following friends.

    friend ThreadStackManager & ::chip::DeviceLayer::ThreadStackMgr(void);
    friend ThreadStackManagerImpl & ::chip::DeviceLayer::ThreadStackMgrImpl(void);
    friend int Internal::GetEntropy_EFR32(uint8_t * buf, size_t bufSize);

    static ThreadStackManagerImpl sInstance;

    static bool IsInitialized();

    // ===== Private members for use by this class only.

    ThreadStackManagerImpl() = default;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    osThreadId_t mSrpClearAllRequester = NULL;
#endif
};

/**
 * Returns the public interface of the ThreadStackManager singleton object.
 *
 * Chip applications should use this to access features of the ThreadStackManager object
 * that are common to all platforms.
 */
inline ThreadStackManager & ThreadStackMgr(void)
{
    return ThreadStackManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ThreadStackManager singleton object.
 *
 * Chip applications can use this to gain access to features of the ThreadStackManager
 * that are specific to EFR32 platforms.
 */
inline ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
