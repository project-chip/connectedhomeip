/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          for the ESP32 platform.
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>

#include <openthread/instance.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {

class ThreadStackManager;
class ThreadStackManagerImpl;

/**
 * Concrate implementation of the ThreadStackManager singleton object for ESP32 platform.
 */
class ThreadStackManagerImpl final : public ThreadStackManager,
                                     public Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>
{
    friend class ThreadStackManager;

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>;
#endif
    friend void ::otTaskletsSignalPending(otInstance * otInst);

public:
    CHIP_ERROR _InitThreadStack();

protected:
    CHIP_ERROR _StartThreadTask();
    void _LockThreadStack();
    bool _TryLockThreadStack();
    void _UnlockThreadStack();
    void _ProcessThreadActivity();
    void _OnCHIPoBLEAdvertisingStart();
    void _OnCHIPoBLEAdvertisingStop();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    void _WaitOnSrpClearAllComplete();
    void _NotifySrpClearAllComplete();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    // ===== Methods that override the GenericThreadStackMa

private:
    friend ThreadStackManager & ::chip::DeviceLayer::ThreadStackMgr(void);
    friend ThreadStackManagerImpl & ::chip::DeviceLayer::ThreadStackMgrImpl(void);
    static ThreadStackManagerImpl sInstance;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    TaskHandle_t mSrpClearAllRequester = nullptr;
#endif

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
 * that are specific to ESP32 platform.
 */
inline ThreadStackManagerImpl & ThreadStackMgrImpl(void)
{
    return ThreadStackManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
