/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the ThreadStackManager object
 *          for nRF52 platforms using the Nordic nRF5 SDK and the OpenThread
 *          stack.
 */

#ifndef THREAD_STACK_MANAGER_IMPL_H
#define THREAD_STACK_MANAGER_IMPL_H

#include <Weave/DeviceLayer/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.h>
#include <Weave/DeviceLayer/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.h>
#include <openthread/thread.h>
#include <openthread/tasklet.h>

extern "C" void otSysEventSignalPending(void);

namespace nl {
namespace Weave {
namespace DeviceLayer {

class ThreadStackManager;
class ThreadStackManagerImpl;
namespace Internal {
extern int GetEntropy_nRF5(uint8_t * buf, size_t bufSize);
}

/**
 * Concrete implementation of the ThreadStackManager singleton object for nRF52 platforms
 * using the Nordic nRF5 SDK and the OpenThread stack.
 */
class ThreadStackManagerImpl final
    : public ThreadStackManager,
      public Internal::GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>,
      public Internal::GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>
{
    // Allow the ThreadStackManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ThreadStackManager;

    // Allow the generic implementation base classes to call helper methods on
    // this class.
    friend Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;
    friend Internal::GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>;
    friend Internal::GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>;

    // Allow glue functions called by OpenThread to call helper methods on this
    // class.
    friend void ::otTaskletsSignalPending(otInstance * otInst);
    friend void ::otSysEventSignalPending(void);

public:

    // ===== Platform-specific members that may be accessed directly by the application.

    using ThreadStackManager::InitThreadStack;
    WEAVE_ERROR InitThreadStack(otInstance * otInst);
    void _OnWoBLEAdvertisingStart(void);
    void _OnWoBLEAdvertisingStop(void);

private:

    // ===== Methods that implement the ThreadStackManager abstract interface.

    WEAVE_ERROR _InitThreadStack(void);

    // ===== Members for internal use by the following friends.

    friend ThreadStackManager & ::nl::Weave::DeviceLayer::ThreadStackMgr(void);
    friend ThreadStackManagerImpl & ::nl::Weave::DeviceLayer::ThreadStackMgrImpl(void);
    friend int Internal::GetEntropy_nRF5(uint8_t * buf, size_t bufSize);

    static ThreadStackManagerImpl sInstance;

    static bool IsInitialized();

    // ===== Private members for use by this class only.

    ThreadStackManagerImpl() = default;
};

/**
 * Returns the public interface of the ThreadStackManager singleton object.
 *
 * Weave applications should use this to access features of the ThreadStackManager object
 * that are common to all platforms.
 */
inline ThreadStackManager & ThreadStackMgr(void)
{
    return ThreadStackManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ThreadStackManager singleton object.
 *
 * Weave applications can use this to gain access to features of the ThreadStackManager
 * that are specific to nRF52 platforms.
 */
inline ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return ThreadStackManagerImpl::sInstance;
}


} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // THREAD_STACK_MANAGER_IMPL_H
