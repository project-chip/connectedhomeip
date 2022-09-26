/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#pragma once

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.h>

#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include <queue.h>

extern "C" void otSysEventSignalPending(void);
extern "C" void otrLock(void);
extern "C" void otrUnlock(void);
extern "C" otInstance * otrGetInstance();

namespace chip {
namespace DeviceLayer {

class ThreadStackManager;
class ThreadStackManagerImpl;

/**
 * Concrete implementation of the ThreadStackManager singleton object for BL702 platforms
 * using the Bouffalolab SDK and the OpenThread stack.
 */
class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public Internal::GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>,
                                     public Internal::GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>
{
    // Allow the ThreadStackManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ThreadStackManager;

    // Allow the generic implementation base classes to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;
    friend Internal::GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>;
    friend Internal::GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>;
#endif

    // // Allow glue functions called by OpenThread to call helper methods on this
    // // class.
    friend void ::otSysEventSignalPending(void);
    friend void ::otrLock(void);
    friend void ::otrUnlock(void);
    friend void ::otTaskletsSignalPending(otInstance * otInst);

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    using ThreadStackManager::InitThreadStack;
    CHIP_ERROR InitThreadStack(otInstance * otInst);

private:
    // ===== Methods that implement the ThreadStackManager abstract interface.

    CHIP_ERROR _InitThreadStack(void);

    // ===== Members for internal use by the following friends.

    friend ThreadStackManager & ::chip::DeviceLayer::ThreadStackMgr(void);
    friend ThreadStackManagerImpl & ::chip::DeviceLayer::ThreadStackMgrImpl(void);

    static ThreadStackManagerImpl sInstance;

    static bool IsInitialized();

    // ===== Private members for use by this class only.

    ThreadStackManagerImpl() = default;
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
 * that are specific to BL702 platforms.
 */
inline ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
