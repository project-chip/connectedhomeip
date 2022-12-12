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

namespace chip {
namespace DeviceLayer {

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    // Members are initialized by the stack
    osMutexAttr_t mut_att = { .attr_bits = osMutexRecursive };

    // Reinitialize the Mutexes
    mChipStackMutex = osMutexNew(&mut_att);
    mEventTaskMutex = osMutexNew(nullptr);
    mPlatformFlags  = osEventFlagsNew(nullptr);
    mQueue          = osMessageQueueNew(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), nullptr);
    mTimer          = osTimerNew(TimerCallback, osTimerOnce, NULL, NULL);

    if (!mChipStackMutex || !mEventTaskMutex || !mPlatformFlags || !mQueue || !mTimer)
    {
        osMutexDelete(mChipStackMutex);
        osMutexDelete(mEventTaskMutex);
        osEventFlagsDelete(mPlatformFlags);
        osMessageQueueDelete(mQueue);
        osTimerDelete(mTimer);
        mChipStackMutex = mEventTaskMutex = mPlatformFlags = mQueue = mTimer = nullptr;

        return CHIP_ERROR_INTERNAL;
    }

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    CHIP_ERROR err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
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
    osStatus_t status = osMessageQueuePut(mQueue, eventPtr, 0, 0);
    CHIP_ERROR ret    = (status == osOK) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    osEventFlagsSet(mPlatformFlags, kPostEventFlag);
    return ret;
}

void PlatformManagerImpl::HandlePostEvent()
{
    /* handle an event */
    ChipDeviceEvent event;
    osStatus_t status = osMessageQueueGet(mQueue, &event, nullptr, 0);
    if (status == osOK)
    {
        LockChipStack();
        DispatchEvent(&event);
        UnlockChipStack();
    }
}

void PlatformManagerImpl::HandleTimerEvent(void)
{
    const CHIP_ERROR err = static_cast<System::LayerImplFreeRTOS &>(DeviceLayer::SystemLayer()).HandlePlatformTimer();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandlePlatformTimer %ld", err.AsInteger());
    }
}

void PlatformManagerImpl::RunEventLoopInternal()
{
    uint32_t flags = 0;
    while (true)
    {
        flags = osEventFlagsWait(mPlatformFlags, kPostEventFlag | kTimerEventFlag | kTaskStopEventFlag,
                                 osFlagsWaitAny | osFlagsNoClear, ms2tick(1000));

        // in case of error we still need to know the value of flags we're not waiting for
        if (flags & osFlagsError)
        {
            flags = osEventFlagsGet(mPlatformFlags);
        }

        if (flags & kTaskStopEventFlag)
        {
            osEventFlagsClear(mPlatformFlags, kTaskStopEventFlag);
            osEventFlagsClear(mPlatformFlags, kTaskRunningEventFlag);
            break;
        }

        if (flags & kTimerEventFlag)
        {
            osEventFlagsClear(mPlatformFlags, kTimerEventFlag);
            HandleTimerEvent();
        }

        if (flags & kPostEventFlag)
        {
            HandlePostEvent();

            if (!osMessageQueueGetCount(mQueue))
            {
                osEventFlagsClear(mPlatformFlags, kPostEventFlag);
            }
        }

        if ((flags & kTaskRunningEventFlag) == 0)
        {
            break;
        }
    }
}

void PlatformManagerImpl::_RunEventLoop()
{
    osEventFlagsSet(mPlatformFlags, kTaskRunningEventFlag);

    RunEventLoopInternal();
}

void PlatformManagerImpl::EventLoopTask(void * arg)
{
    (void) arg;
    PlatformMgrImpl().RunEventLoopInternal();
    osThreadTerminate(osThreadGetId());
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    // this mutex only needed to guard against multiple launches
    {
        osMutexAcquire(mEventTaskMutex, osWaitForever);

        if (kTaskRunningEventFlag & osEventFlagsGet(mPlatformFlags))
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_BUSY;
        }

        const osThreadAttr_t tread_attr = {
            .name       = CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
            .stack_size = CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
            .priority   = osPriorityNormal,

        };

        osEventFlagsSet(mPlatformFlags, kTaskRunningEventFlag);

        // this thread is self terminating
        osThreadId_t mEventTask = osThreadNew(EventLoopTask, NULL, &tread_attr);

        if (mEventTask == nullptr)
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_INTERNAL;
        }

        osMutexRelease(mEventTaskMutex);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    // this mutex only needed to guard against multiple calls to stop
    {
        osMutexAcquire(mEventTaskMutex, osWaitForever);

        uint32_t flags = osEventFlagsGet(mPlatformFlags);
        if ((kTaskRunningEventFlag & flags) == 0)
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if (kTaskStopEventFlag & flags)
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_INCORRECT_STATE;
        }

        osEventFlagsSet(mPlatformFlags, kTaskStopEventFlag);

        osMutexRelease(mEventTaskMutex);
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

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(System::Clock::Timeout duration)
{
    if (duration.count() == 0)
    {
        TimerCallback(0);
    }
    else
    {
        auto res = osTimerStart(mTimer, ms2tick(duration.count()));
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

    // the task thread is self terminating, we might have to wait if it's still processing
    while (true)
    {
        uint32_t flags = osEventFlagsGet(mPlatformFlags);
        if ((kTaskRunningEventFlag & flags) == 0)
        {
            break;
        }
        osDelay(1);
    }

    osMutexDelete(mChipStackMutex);
    osMutexDelete(mEventTaskMutex);
    osEventFlagsDelete(mPlatformFlags);
    osMessageQueueDelete(mQueue);
    osTimerDelete(mTimer);
    mChipStackMutex = nullptr;
    mPlatformFlags  = nullptr;
    mEventTaskMutex = nullptr;
    mQueue          = nullptr;
    mTimer          = nullptr;

    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip
