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
 *          Contains non-inline method definitions for the
 *          GenericPlatformManagerImpl_FreeRTOS<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_IPP
#define GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_IPP

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/PlatformManager.h>
#include <Weave/DeviceLayer/FreeRTOS/GenericPlatformManagerImpl_FreeRTOS.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_FreeRTOS<> template inherits.
#include <Weave/DeviceLayer/internal/GenericPlatformManagerImpl.ipp>


namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;

template<class ImplClass>
WEAVE_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_InitWeaveStack(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    vTaskSetTimeOutState(&mNextTimerBaseTime);
    mNextTimerDurationTicks = 0;
    mEventLoopTask = NULL;
    mWeaveTimerActive = false;

    mWeaveStackLock = xSemaphoreCreateMutex();
    if (mWeaveStackLock == NULL)
    {
        WeaveLogError(DeviceLayer, "Failed to create Weave stack lock");
        ExitNow(err = WEAVE_ERROR_NO_MEMORY);
    }

    mWeaveEventQueue = xQueueCreate(WEAVE_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(WeaveDeviceEvent));
    if (mWeaveEventQueue == NULL)
    {
        WeaveLogError(DeviceLayer, "Failed to allocate Weave event queue");
        ExitNow(err = WEAVE_ERROR_NO_MEMORY);
    }

    // Call up to the base class _InitWeaveStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitWeaveStack();
    SuccessOrExit(err);

exit:
    return err;
}

template<class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_LockWeaveStack(void)
{
    xSemaphoreTake(mWeaveStackLock, portMAX_DELAY);
}

template<class ImplClass>
bool GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_TryLockWeaveStack(void)
{
    return xSemaphoreTake(mWeaveStackLock, 0) == pdTRUE;
}

template<class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_UnlockWeaveStack(void)
{
    xSemaphoreGive(mWeaveStackLock);
}

template<class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_PostEvent(const WeaveDeviceEvent * event)
{
    if (mWeaveEventQueue != NULL)
    {
        if (!xQueueSend(mWeaveEventQueue, event, 1))
        {
            WeaveLogError(DeviceLayer, "Failed to post event to Weave Platform event queue");
        }
    }
}

template<class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_RunEventLoop(void)
{
    WEAVE_ERROR err;
    WeaveDeviceEvent event;

    VerifyOrDie(mEventLoopTask == NULL);

    // Capture the task handle.
    mEventLoopTask = xTaskGetCurrentTaskHandle();

    // Lock the Weave stack.
    Impl()->LockWeaveStack();

    while (true)
    {
        TickType_t waitTime;

        // If one or more Weave timers are active...
        if (mWeaveTimerActive)
        {
            // Adjust the base time and remaining duration for the next scheduled timer based on the
            // amount of time that has elapsed since it was started.
            // IF the timer's expiration time has already arrived...
            if (xTaskCheckForTimeOut(&mNextTimerBaseTime, &mNextTimerDurationTicks) == pdTRUE)
            {
                // Reset the 'timer active' flag.  This will be set to true again by _StartWeaveTimer()
                // if there are further timers beyond the expired one that are still active.
                mWeaveTimerActive = false;

                // Call into the system layer to dispatch the callback functions for all timers
                // that have expired.
                err = SystemLayer.HandlePlatformTimer();
                if (err != WEAVE_SYSTEM_NO_ERROR)
                {
                    WeaveLogError(DeviceLayer, "Error handling Weave timers: %s", ErrorStr(err));
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

        // Otherwise no Weave timers are active, so wait indefinitely for an event to arrive on the event
        // queue.
        else
        {
            waitTime = portMAX_DELAY;
        }

        // Unlock the Weave stack, allowing other threads to enter Weave while the event loop thread is sleeping.
        Impl()->UnlockWeaveStack();

        BaseType_t eventReceived = xQueueReceive(mWeaveEventQueue, &event, waitTime);

        // Lock the Weave stack.
        Impl()->LockWeaveStack();

        // If an event was received, dispatch it.  Continue receiving events from the queue and
        // dispatching them until the queue is empty.
        while (eventReceived == pdTRUE)
        {
            Impl()->DispatchEvent(&event);

            eventReceived = xQueueReceive(mWeaveEventQueue, &event, 0);
        }
    }
}

template<class ImplClass>
WEAVE_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StartEventLoopTask(void)
{
    BaseType_t res;

    res = xTaskCreate(EventLoopTaskMain,
                WEAVE_DEVICE_CONFIG_WEAVE_TASK_NAME,
                WEAVE_DEVICE_CONFIG_WEAVE_TASK_STACK_SIZE / sizeof(StackType_t),
                this,
                WEAVE_DEVICE_CONFIG_WEAVE_TASK_PRIORITY,
                NULL);

    return (res == pdPASS) ? WEAVE_NO_ERROR : WEAVE_ERROR_NO_MEMORY;
}

template<class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::EventLoopTaskMain(void * arg)
{
    WeaveLogDetail(DeviceLayer, "Weave task running");
    static_cast<GenericPlatformManagerImpl_FreeRTOS<ImplClass>*>(arg)->Impl()->RunEventLoop();
}

template<class ImplClass>
WEAVE_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StartWeaveTimer(uint32_t aMilliseconds)
{
    mWeaveTimerActive = true;
    vTaskSetTimeOutState(&mNextTimerBaseTime);
    mNextTimerDurationTicks = pdMS_TO_TICKS(aMilliseconds);

    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (xTaskGetCurrentTaskHandle() != mEventLoopTask)
    {
        WeaveDeviceEvent event;
        event.Type = DeviceEventType::kNoOp;
        Impl()->PostEvent(&event);
    }

    return WEAVE_NO_ERROR;
}

template<class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::PostEventFromISR(const WeaveDeviceEvent * event, BaseType_t & yieldRequired)
{
    yieldRequired = pdFALSE;

    if (mWeaveEventQueue != NULL)
    {
        if (!xQueueSendFromISR(mWeaveEventQueue, event, &yieldRequired))
        {
            WeaveLogError(DeviceLayer, "Failed to post event to Weave Platform event queue");
        }
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_IPP
