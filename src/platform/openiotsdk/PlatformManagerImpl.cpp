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

#include "OpenIoTSDKArchUtils.h"
#include "platform/internal/CHIPDeviceLayerInternal.h"
#include <platform/internal/testing/ConfigUnitTest.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.ipp>
#include <platform/openiotsdk/DiagnosticDataProviderImpl.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace chip::System::Clock;

namespace chip {
namespace DeviceLayer {

struct ScopedLock
{
    ScopedLock(osMutexId_t & lockable) : _lockable(lockable) { osMutexAcquire(_lockable, osWaitForever); }
    ScopedLock(const ScopedLock &)             = delete;
    ScopedLock & operator=(const ScopedLock &) = delete;
    ~ScopedLock() { osMutexRelease(_lockable); }

private:
    osMutexId_t & _lockable;
};

namespace {
LayerImpl & SystemLayerImpl()
{
    return static_cast<LayerImpl &>(DeviceLayer::SystemLayer());
}
} // anonymous namespace

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    // Members are initialized by the stack
    osMutexAttr_t mut_att = { .attr_bits = osMutexRecursive };

    // Reinitialize the Mutexes
    if (mChipStackMutex == nullptr)
    {
        mChipStackMutex = osMutexNew(&mut_att);
    }

    if (mEventTaskMutex == nullptr)
    {
        mEventTaskMutex = osMutexNew(nullptr);
    }

    if (mPlatformFlags == nullptr)
    {
        mPlatformFlags = osEventFlagsNew(nullptr);
    }

    if (mQueue == nullptr)
    {
        mQueue = osMessageQueueNew(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), nullptr);
    }
    else
    {
        osMessageQueueReset(mQueue);
    }

    if (!mChipStackMutex || !mEventTaskMutex || !mPlatformFlags || !mQueue)
    {
        osMutexDelete(mChipStackMutex);
        osMutexDelete(mEventTaskMutex);
        osEventFlagsDelete(mPlatformFlags);
        osMessageQueueDelete(mQueue);
        mChipStackMutex = mEventTaskMutex = mPlatformFlags = mQueue = nullptr;

        return CHIP_ERROR_INTERNAL;
    }

    ReturnLogErrorOnFailure(PlatformTimerInit());

    mRunEventLoop.store(false);
    mInitialized = true;

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    ReturnLogErrorOnFailure(GenericPlatformManagerImpl<ImplClass>::_InitChipStack());

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_LockChipStack()
{
    osMutexAcquire(mChipStackMutex, osWaitForever);
}

bool PlatformManagerImpl::_TryLockChipStack()
{
    if (osMutexAcquire(mChipStackMutex, 0U) == osOK)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PlatformManagerImpl::_UnlockChipStack()
{
    osMutexRelease(mChipStackMutex);
}

CHIP_ERROR PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * eventPtr)
{
    if (!mInitialized)
    {
        ChipLogError(DeviceLayer, "_PostEvent: stack not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    osStatus_t status = osMessageQueuePut(mQueue, eventPtr, 0, 0);
    osEventFlagsSet(mPlatformFlags, kPostEventFlag);
    return (status == osOK) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
}

void PlatformManagerImpl::HandlePostEvent()
{
    /* handle an event */
    ChipDeviceEvent event;
    uint32_t count = osMessageQueueGetCount(mQueue);
    while (count)
    {
        if (osMessageQueueGet(mQueue, &event, nullptr, 0) != osOK)
        {
            break;
        }

        DispatchEvent(&event);
        count--;
    }
}

void PlatformManagerImpl::HandleTimerEvent(void)
{
    CHIP_ERROR err = SystemLayerImpl().HandlePlatformTimer();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandlePlatformTimer %ld", err.AsInteger());
    }
}

void PlatformManagerImpl::_RunEventLoop()
{
    uint32_t flags = 0;

    if (!mInitialized)
    {
        ChipLogError(DeviceLayer, "_PostEvent: stack not initialized");
        return;
    }

    {
        ScopedLock lock(mEventTaskMutex);

        bool expectedValue = false;
        if (!mRunEventLoop.compare_exchange_strong(expectedValue /* expected */, true /* desired */))
        {
            ChipLogError(DeviceLayer, "Error trying to run the event loop while it is already running");
            return;
        }

        // Look if a task ID has already been assigned or not.
        // If not, it means we run in the thread that called RunEventLoop
        if (!mEventTask)
        {
            ChipLogDetail(DeviceLayer, "Run CHIP event loop on external thread");
            mEventTask = osThreadGetId();
        }
        else
        {
            osEventFlagsSet(mPlatformFlags, kTaskHasEventLoopRunFlag);
        }
    }

    LockChipStack();

    while (mRunEventLoop.load())
    {
        UnlockChipStack();
        flags = osEventFlagsWait(mPlatformFlags, kPostEventFlag | kTimerEventFlag, osFlagsWaitAny, osWaitForever);
        LockChipStack();

        // In case of error we still need to know the value of flags we're not waiting for
        if (flags & osFlagsError)
        {
            flags = osEventFlagsGet(mPlatformFlags);
        }

        if (flags & kTimerEventFlag)
        {
            HandleTimerEvent();
        }

        if (flags & kPostEventFlag)
        {
            HandlePostEvent();
        }
    }

    UnlockChipStack();

    osEventFlagsSet(mPlatformFlags, kTaskHasEventLoopStopFlag);
    mEventTask = nullptr;
}

void PlatformManagerImpl::EventLoopTask(void * arg)
{
    (void) arg;
    PlatformMgrImpl().RunEventLoop();
    osThreadTerminate(osThreadGetId());
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    if (!mInitialized)
    {
        ChipLogError(DeviceLayer, "_StartEventLoopTask: stack not initialized");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    {
        ScopedLock lock(mEventTaskMutex);

        const osThreadAttr_t tread_attr = {
            .name       = CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
            .stack_size = CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
            .priority   = osPriorityNormal,

        };

        mEventTask = osThreadNew(EventLoopTask, NULL, &tread_attr);
        if (mEventTask == nullptr)
        {
            ChipLogError(DeviceLayer, "Create event loop thread failed");
            return CHIP_ERROR_INTERNAL;
        }
    }

    if (osEventFlagsWait(mPlatformFlags, kTaskHasEventLoopRunFlag, osFlagsWaitAny, osWaitForever) & osFlagsError)
    {
        ChipLogError(DeviceLayer, "Start event loop thread failed");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    {
        ScopedLock lock(mEventTaskMutex);

        // Early return if the event loop is not running
        if (!mRunEventLoop.load())
        {
            return CHIP_NO_ERROR;
        }

        // Indicate that the event loop store
        mRunEventLoop.store(false);
    }

    osEventFlagsSet(mPlatformFlags, kPostEventFlag);

    // If the thread running the event loop is different from the caller
    // then wait it to finish
    if (mEventTask != nullptr && mEventTask != osThreadGetId())
    {
        if (osEventFlagsWait(mPlatformFlags, kTaskHasEventLoopStopFlag, osFlagsWaitAny, ms2tick(1000)) & osFlagsError)
        {
            ChipLogError(DeviceLayer, "Stop event loop thread failed");
            return CHIP_ERROR_INTERNAL;
        }
    }

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::SetEventFlags(uint32_t flags)
{
    osEventFlagsSet(mPlatformFlags, flags);
}

void PlatformManagerImpl::TimerCallback(void * arg)
{
    PlatformMgrImpl().SetEventFlags(kTimerEventFlag);
}

CHIP_ERROR PlatformManagerImpl::PlatformTimerInit()
{
    if (mTimer != nullptr)
    {
        return CHIP_NO_ERROR;
    }

    mTimer = osTimerNew(TimerCallback, osTimerOnce, NULL, NULL);

    if (!mTimer)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::PlatformTimerDeinit()
{
    if (mTimer == nullptr)
    {
        return CHIP_NO_ERROR;
    }

    osTimerDelete(mTimer);
    mTimer = nullptr;

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(System::Clock::Timeout duration)
{
    if (duration.count() == 0)
    {
        TimerCallback(0);
    }
    else
    {
        Milliseconds32 msec = std::chrono::duration_cast<Milliseconds32>(duration);
        auto res            = osTimerStart(mTimer, ms2tick(msec.count()));
        if (res)
        {
            ChipLogError(DeviceLayer, "osTimerStart failed %d", res);
            return CHIP_ERROR_INTERNAL;
        }
    }

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown()
{
    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    (void) _StopEventLoopTask();

    osMutexDelete(mChipStackMutex);
    osMutexDelete(mEventTaskMutex);
    osEventFlagsDelete(mPlatformFlags);
    osMessageQueueDelete(mQueue);
    PlatformTimerDeinit();
    mChipStackMutex = nullptr;
    mPlatformFlags  = nullptr;
    mEventTaskMutex = nullptr;
    mQueue          = nullptr;
    mInitialized    = false;

    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip
