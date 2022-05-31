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

#include "platform/internal/CHIPDeviceLayerInternal.h"
#include <platform/internal/testing/ConfigUnitTest.h>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.ipp>
#include <platform/openiotsdk/DiagnosticDataProviderImpl.h>
#include <platform/openiotsdk/SystemTimeSupport.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    // Members are initialized by the stack

    // Reinitialize the Mutexes
    mChipStackMutex = osMutexNew(nullptr);
    mEventTaskMutex = osMutexNew(nullptr);
    mPlatformFlags  = osEventFlagsNew(nullptr);
    mQueue          = osMessageQueueNew(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), nullptr);

    if (!mChipStackMutex || !mEventTaskMutex || !mPlatformFlags || !mQueue)
    {
        osMutexDelete(mChipStackMutex);
        osMutexDelete(mEventTaskMutex);
        osEventFlagsDelete(mPlatformFlags);
        osMessageQueueDelete(mQueue);
        mChipStackMutex = mEventTaskMutex = mPlatformFlags = mQueue = nullptr;

        return CHIP_ERROR_INTERNAL;
    }

    SetConfigurationMgr(&ConfigurationManagerImpl::GetDefaultInstance());
    SetDiagnosticDataProvider(&DiagnosticDataProviderImpl::GetDefaultInstance());

    auto err = System::Clock::InitClock_RealTime();
    SuccessOrExit(err);

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
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

    CHIP_ERROR ret = (status == osOK) ? CHIP_NO_ERROR : CHIP_ERROR_INTERNAL;
    return ret;
}

void PlatformManagerImpl::ProcessDeviceEvents()
{
    ChipDeviceEvent event;
    osStatus_t status = osMessageQueueGet(mQueue, &event, nullptr, osWaitForever);
    if (status == osOK)
    {
        LockChipStack();
        DispatchEvent(&event);
        UnlockChipStack();
    }
}

void PlatformManagerImpl::_RunEventLoop()
{
    // this mutex only needed to guard against multiple launches
    {
        osMutexAcquire(mEventTaskMutex, osWaitForever);

        if (FLAG_EVENT_TASK_RUNNING & osEventFlagsGet(mPlatformFlags))
        {
            // already running
            osMutexRelease(mEventTaskMutex);
            return;
        }

        osEventFlagsSet(mPlatformFlags, FLAG_EVENT_TASK_RUNNING);

        osMutexRelease(mEventTaskMutex);
    }

    RunEventLoopInternal();
}

void PlatformManagerImpl::RunEventLoopInternal()
{
    while ((osEventFlagsGet(mPlatformFlags) & FLAG_STOP_EVENT_TASK) == 0)
    {
        ProcessDeviceEvents();
    }

    osEventFlagsClear(mPlatformFlags, FLAG_STOP_EVENT_TASK | FLAG_EVENT_TASK_RUNNING);
}

void PlatformManagerImpl::RunEventLoopTask(void * arg)
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

        if (FLAG_EVENT_TASK_RUNNING & osEventFlagsGet(mPlatformFlags))
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_BUSY;
        }

        const osThreadAttr_t tread_attr = {
            .name       = CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
            .stack_size = CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
            .priority   = osPriorityNormal,

        };
        // this thread is self terminating
        osThreadId_t mEventTask = osThreadNew(PlatformManagerImpl::RunEventLoopTask, NULL, &tread_attr);

        if (mEventTask == nullptr)
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_INTERNAL;
        }

        osEventFlagsSet(mPlatformFlags, FLAG_EVENT_TASK_RUNNING);

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
        if ((FLAG_EVENT_TASK_RUNNING & flags) == 0)
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_INCORRECT_STATE;
        }

        if (FLAG_STOP_EVENT_TASK & flags)
        {
            osMutexRelease(mEventTaskMutex);
            return CHIP_ERROR_INCORRECT_STATE;
        }

        osEventFlagsSet(mPlatformFlags, FLAG_STOP_EVENT_TASK);

        osMutexRelease(mEventTaskMutex);
    }

    {
        // wake up the main loop by sending a NOP so that the flags get checked and loop can exit
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kNoOp;
        _PostEvent(&event);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(System::Clock::Timeout duration)
{
    // Let LayerSocketsLoop::PrepareSelect() handle timers.
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
        if ((FLAG_EVENT_TASK_RUNNING & flags) == 0)
        {
            break;
        }
        osDelay(1);
    }

    osMutexDelete(mChipStackMutex);
    osMutexDelete(mEventTaskMutex);
    osEventFlagsDelete(mPlatformFlags);
    osMessageQueueDelete(mQueue);
    mChipStackMutex = nullptr;
    mPlatformFlags  = nullptr;
    mEventTaskMutex = nullptr;
    mQueue          = nullptr;

    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

// ===== Members for internal use by the following friends.

PlatformManagerImpl PlatformManagerImpl::sInstance;

} // namespace DeviceLayer
} // namespace chip
