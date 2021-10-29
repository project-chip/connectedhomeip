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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include "events/EventQueue.h"
#include "rtos/ConditionVariable.h"
#include "rtos/Mutex.h"
#include "rtos/Thread.h"

#ifdef TARGET_MCU_STM32L4
// [ MBED HACK ]
// We have to undefine |SUCCESS| here to prevent compilation error due to
// conflict with |ErrorStatus| enum type values defined in CMSIS/stm32l4xx.h
// which is included by mstd_atormic header.
// This problem is only related for when tests are build and nlunit-test.h is used.
#undef SUCCESS
#endif
#include <mstd_atomic>
#include <platform/CHIPDeviceConfig.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.h>
#include <sys/select.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {

class GapEventHandler;
class CHIPService;
} // namespace Internal

/**
 * Concrete implementation of the PlatformManager singleton object for the nRF Connect SDK platforms.
 */
class PlatformManagerImpl final : public Internal::GenericPlatformManagerImpl
{
    // Members for select() loop
    int mMaxFd;
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;
    timeval mNextTimeout;

public:
    // ===== Platform-specific members that may be accessed directly by the application.

    /* none so far */

private:
    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR InitChipStackInner(void) override;
    CHIP_ERROR ShutdownInner() override;
    void LockChipStack() override;
    bool TryLockChipStack() override;
    void UnlockChipStack() override;
    CHIP_ERROR PostEvent(const ChipDeviceEvent * event) override;
    void RunEventLoop() override;
    CHIP_ERROR StartEventLoopTask() override;
    CHIP_ERROR StopEventLoopTask() override;
    CHIP_ERROR StartChipTimer(System::Clock::Timeout duration) override;

    void ProcessDeviceEvents();

    // ===== Members for internal use by the following friends.

    friend class Internal::BLEManagerImpl;
    friend class ConnectivityManagerImpl;
    friend class Internal::GapEventHandler;
    friend class Internal::CHIPService;

    // ===== Members for internal use.
    static CHIP_ERROR TranslateOsStatus(osStatus status);
    bool IsLoopActive();

    bool mInitialized = false;
    rtos::Thread mLoopTask{ osPriorityNormal, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
                            /* memory provided */ nullptr, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME };
    osThreadId_t mChipTaskId = 0;
    rtos::Mutex mThisStateMutex;
    rtos::ConditionVariable mEventLoopCond{ mThisStateMutex };
    rtos::Mutex mChipStackMutex;
    static const size_t event_size = EVENTS_EVENT_SIZE + sizeof(void *) + sizeof(ChipDeviceEvent *);
    events::EventQueue mQueue      = { event_size * CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE };
    mstd::atomic<bool> mShouldRunEventLoop;
    bool mEventLoopHasStopped = false;
    bool mEventLoopHasRun     = false;
};

} // namespace DeviceLayer
} // namespace chip
