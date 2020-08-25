/*
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

/**
 *    @file
 *          Provides an implementation of the ThreadStackManager object for
 *          nRF Connect SDK platforms.
 */

#ifndef THREAD_STACK_MANAGER_IMPL_H
#define THREAD_STACK_MANAGER_IMPL_H

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/Zephyr/GenericThreadStackManagerImpl_Zephyr.h>

#include <openthread/thread.h>
#include <platform/nrfconnect/BLEManagerImpl.h>

namespace chip {
namespace DeviceLayer {

class ThreadStackManager;
class ThreadStackManagerImpl;

/**
 * Concrete implementation of the ThreadStackManager singleton object for nRF Connect platforms.
 */
class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public Internal::GenericThreadStackManagerImpl_Zephyr<ThreadStackManagerImpl>
{
    // Allow the ThreadStackManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend class ThreadStackManager;

    // Allow the generic implementation base classes to call helper methods on
    // this class.
    friend Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;
    friend Internal::GenericThreadStackManagerImpl_Zephyr<ThreadStackManagerImpl>;

private:
    // ===== Members for internal use by the following friends.

    friend ThreadStackManager & ::chip::DeviceLayer::ThreadStackMgr(void);
    friend ThreadStackManagerImpl & ::chip::DeviceLayer::ThreadStackMgrImpl(void);

    static ThreadStackManagerImpl sInstance;

    // ===== Private members for use by this class only.

    ThreadStackManagerImpl() = default;
};

/**
 * Returns the public interface of the ThreadStackManager singleton object.
 *
 * chip applications should use this to access features of the ThreadStackManager object
 * that are common to all platforms.
 */
inline ThreadStackManager & ThreadStackMgr(void)
{
    return ThreadStackManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the ThreadStackManager singleton object.
 *
 * chip applications can use this to gain access to features of the ThreadStackManager
 * that are specific to nRF Connect platforms.
 */
inline ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip

#endif // THREAD_STACK_MANAGER_IMPL_H
