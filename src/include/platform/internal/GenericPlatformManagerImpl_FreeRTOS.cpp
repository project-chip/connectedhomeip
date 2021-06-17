/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Contains non-inline method definitions for the
 *          GenericPlatformManagerImpl_FreeRTOS<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_CPP

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.h>

#include <lib/support/CodeUtils.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_FreeRTOS<> template inherits.
#include <platform/internal/GenericPlatformManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    vTaskSetTimeOutState(&mNextTimerBaseTime);
    mNextTimerDurationTicks = 0;
    mEventLoopTask          = NULL;
    mChipTimerActive        = false;

    mChipStackLock = xSemaphoreCreateMutex();
    if (mChipStackLock == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to create CHIP stack lock");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
    }

#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE) && CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE
    mChipEventQueue =
        xQueueCreateStatic(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), mEventQueueBuffer, &mEventQueueStruct);
#else
    mChipEventQueue = xQueueCreate(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent));
#endif
    if (mChipEventQueue == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to allocate CHIP event queue");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
    }

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_LockChipStack(void)
{
    xSemaphoreTake(mChipStackLock, portMAX_DELAY);
}

template <class ImplClass>
bool GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_TryLockChipStack(void)
{
    return xSemaphoreTake(mChipStackLock, 0) == pdTRUE;
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_UnlockChipStack(void)
{
    xSemaphoreGive(mChipStackLock);
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    if (mChipEventQueue != NULL)
    {
        if (!xQueueSend(mChipEventQueue, event, 1))
        {
            ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        }
    }
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_RunEventLoop(void)
{
    CHIP_ERROR err;
    ChipDeviceEvent event;

    VerifyOrDie(mEventLoopTask != NULL);

    // Lock the CHIP stack.
    Impl()->LockChipStack();

    while (true)
    {
        TickType_t waitTime;

        // If one or more CHIP timers are active...
        if (mChipTimerActive)
        {
            // Adjust the base time and remaining duration for the next scheduled timer based on the
            // amount of time that has elapsed since it was started.
            // IF the timer's expiration time has already arrived...
            if (xTaskCheckForTimeOut(&mNextTimerBaseTime, &mNextTimerDurationTicks) == pdTRUE)
            {
                // Reset the 'timer active' flag.  This will be set to true again by _StartChipTimer()
                // if there are further timers beyond the expired one that are still active.
                mChipTimerActive = false;

                // Call into the system layer to dispatch the callback functions for all timers
                // that have expired.
                err = SystemLayer.HandlePlatformTimer();
                if (err != CHIP_SYSTEM_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Error handling CHIP timers: %s", ErrorStr(err));
                }

                // When processing the event queue below, do not wait if the queue is empty.  Instead
                // immediately loop around and process timers again
                waitTime = 0;
            }

            // If there is still time before the next timer expires, arrange to wait on the event queue
            // until that timer expires.
            else
            {
                waitTime = mNextTimerDurationTicks;
            }
        }

        // Otherwise no CHIP timers are active, so wait indefinitely for an event to arrive on the event
        // queue.
        else
        {
            waitTime = portMAX_DELAY;
        }

        // Unlock the CHIP stack, allowing other threads to enter CHIP while the event loop thread is sleeping.
        Impl()->UnlockChipStack();

        BaseType_t eventReceived = xQueueReceive(mChipEventQueue, &event, waitTime);

        // Lock the CHIP stack.
        Impl()->LockChipStack();

        // If an event was received, dispatch it.  Continue receiving events from the queue and
        // dispatching them until the queue is empty.
        while (eventReceived == pdTRUE)
        {
            Impl()->DispatchEvent(&event);

            eventReceived = xQueueReceive(mChipEventQueue, &event, 0);
        }
    }
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StartEventLoopTask(void)
{
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
    mEventLoopTask = xTaskCreateStatic(EventLoopTaskMain, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME, ArraySize(mEventLoopStack), this,
                                       CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, mEventLoopStack, &mventLoopTaskStruct);
#else
    xTaskCreate(EventLoopTaskMain, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE / sizeof(StackType_t),
                this, CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, &mEventLoopTask);
#endif

    return (mEventLoopTask != NULL) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::EventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_FreeRTOS<ImplClass> *>(arg)->Impl()->RunEventLoop();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StartChipTimer(uint32_t aMilliseconds)
{
    mChipTimerActive = true;
    vTaskSetTimeOutState(&mNextTimerBaseTime);
    mNextTimerDurationTicks = pdMS_TO_TICKS(aMilliseconds);

    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (xTaskGetCurrentTaskHandle() != mEventLoopTask)
    {
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kNoOp;
        Impl()->PostEvent(&event);
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::PostEventFromISR(const ChipDeviceEvent * event, BaseType_t & yieldRequired)
{
    yieldRequired = pdFALSE;

    if (mChipEventQueue != NULL)
    {
        if (!xQueueSendFromISR(mChipEventQueue, event, &yieldRequired))
        {
            ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        }
    }
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_Shutdown(void)
{
    VerifyOrDieWithMsg(false, DeviceLayer, "Shutdown is not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StopEventLoopTask(void)
{
    VerifyOrDieWithMsg(false, DeviceLayer, "StopEventLoopTask is not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_CPP
