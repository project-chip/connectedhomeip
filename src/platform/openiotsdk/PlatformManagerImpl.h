/*
 *
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

/**
 *    @file
 *          Provides the implementation of the Device Layer Platform Manager class
 *          for Open IOT SDK platform.
 */

#pragma once

#include "cmsis_os2.h"
#include "mbedtls/platform.h"
#include <platform/CHIPDeviceConfig.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for the nRF Connect SDK platforms.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

    // Allow the generic implementation base class to call helper methods on
    // this class.
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Internal::GenericPlatformManagerImpl<PlatformManagerImpl>;
#endif
public:
    // ===== Platform-specific members that may be accessed directly by the application.

    /* none so far */

private:
    static constexpr uint32_t kTaskRunningEventFlag = 1 << 0;
    static constexpr uint32_t kTaskStopEventFlag    = 1 << 1;
    static constexpr uint32_t kPostEventFlag        = 1 << 2;
    static constexpr uint32_t kTimerEventFlag       = 1 << 3;

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR _InitChipStack(void);
    void _LockChipStack();
    bool _TryLockChipStack();
    void _UnlockChipStack();

    CHIP_ERROR _PostEvent(const ChipDeviceEvent * event);

    void _RunEventLoop();
    CHIP_ERROR _StartEventLoopTask();
    CHIP_ERROR _StopEventLoopTask();

    CHIP_ERROR _StartChipTimer(System::Clock::Timeout duration);
    void _Shutdown();

    void SetEventFlags(uint32_t flags);
    void HandleTimerEvent(void);
    void HandlePostEvent(void);

    static void EventLoopTask(void * arg);
    static void TimerCallback(void * arg);

    void RunEventLoopInternal(void);

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);
    friend class ConnectivityManagerImpl;
    friend class GapEventHandler;
    friend class CHIPService;

    using PlatformManager::PostEvent;
    using PlatformManager::PostEventOrDie;
    static PlatformManagerImpl sInstance;

    osEventFlagsId_t mPlatformFlags = nullptr;
    osMutexId_t mChipStackMutex     = nullptr;
    osMutexId_t mEventTaskMutex     = nullptr;
    osMessageQueueId_t mQueue       = nullptr;
    osTimerId_t mTimer              = nullptr;
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the Mbed platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl()
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
