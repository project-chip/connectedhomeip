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
#include <platform/internal/GenericPlatformManagerImpl.ipp>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    vTaskSetTimeOutState(&mNextTimerBaseTime);
    mNextTimerDurationTicks = 0;
    mChipTimerActive        = false;

    if (mChipStackLock == NULL)
    {
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE) && CHIP_CONFIG_FREERTOS_USE_STATIC_SEMAPHORE
        mChipStackLock = xSemaphoreCreateMutexStatic(&mChipStackLockMutex);
#else
        mChipStackLock  = xSemaphoreCreateMutex();
#endif

        if (mChipStackLock == NULL)
        {
            ChipLogError(DeviceLayer, "Failed to create CHIP stack lock");
            ExitNow(err = CHIP_ERROR_NO_MEMORY);
        }
    }

    if (mChipEventQueue == NULL)
    {
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE) && CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE
        mChipEventQueue = xQueueCreateStatic(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), mEventQueueBuffer,
                                             &mEventQueueStruct);
#else
        mChipEventQueue = xQueueCreate(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent));
#endif
        if (mChipEventQueue == NULL)
        {
            ChipLogError(DeviceLayer, "Failed to allocate CHIP main event queue");
            ExitNow(err = CHIP_ERROR_NO_MEMORY);
        }
    }
    else
    {
        // Clear out any events that might be stuck in the queue, so we start
        // with a clean slate, as if we had just re-created the queue.
        xQueueReset(mChipEventQueue);
    }

    mShouldRunEventLoop.store(false);

#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    if (mBackgroundEventQueue == NULL)
    {
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE) && CHIP_CONFIG_FREERTOS_USE_STATIC_QUEUE
        mBackgroundEventQueue = xQueueCreateStatic(CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent),
                                                   mBackgroundQueueBuffer, &mBackgroundQueueStruct);
#else
        mBackgroundEventQueue = xQueueCreate(CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent));
#endif
        if (mBackgroundEventQueue == NULL)
        {
            ChipLogError(DeviceLayer, "Failed to allocate CHIP background event queue");
            ExitNow(err = CHIP_ERROR_NO_MEMORY);
        }
    }
    else
    {
        xQueueReset(mBackgroundEventQueue);
    }

    mShouldRunBackgroundEventLoop.store(false);
#endif

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

#if CHIP_STACK_LOCK_TRACKING_ENABLED
template <class ImplClass>
bool GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_IsChipStackLockedByCurrentThread() const
{
    // We can't check for INCLUDE_xTaskGetCurrentTaskHandle because it's often
    // _not_ set, but xTaskGetCurrentTaskHandle works anyway because
    // configUSE_MUTEXES is set.  So in practice, xTaskGetCurrentTaskHandle can
    // be assumed to be available here.
#if INCLUDE_xSemaphoreGetMutexHolder != 1
#error Must either set INCLUDE_xSemaphoreGetMutexHolder = 1 in FreeRTOSConfig.h or set chip_stack_lock_tracking = "none" in Matter gn configuration.
#endif
    // If we have not started our event loop yet, return true because in that
    // case we can't be racing against the (not yet started) event loop.
    //
    // Similarly, if mChipStackLock has not been created yet, might as well
    // return true.
    return (mEventLoopTask == nullptr) || (mChipStackLock == nullptr) ||
        (xSemaphoreGetMutexHolder(mChipStackLock) == xTaskGetCurrentTaskHandle());
}
#endif // CHIP_STACK_LOCK_TRACKING_ENABLED

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    if (mChipEventQueue == NULL)
    {
        return CHIP_ERROR_INTERNAL;
    }
    BaseType_t status = xQueueSend(mChipEventQueue, event, 1);
    if (status != pdTRUE)
    {
        ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        return CHIP_ERROR(chip::ChipError::Range::kOS, status);
    }
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_RunEventLoop(void)
{
    CHIP_ERROR err;
    ChipDeviceEvent event;

    // Lock the CHIP stack.
    StackLock lock;

    bool oldShouldRunEventLoop = false;
    if (!mShouldRunEventLoop.compare_exchange_strong(oldShouldRunEventLoop /* expected */, true /* desired */))
    {
        ChipLogError(DeviceLayer, "Error trying to run the event loop while it is already running");
        return;
    }

    while (mShouldRunEventLoop.load())
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
                err = static_cast<System::LayerImplFreeRTOS &>(DeviceLayer::SystemLayer()).HandlePlatformTimer();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Error handling CHIP timers: %" CHIP_ERROR_FORMAT, err.Format());
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

        BaseType_t eventReceived = pdFALSE;
        {
            // Unlock the CHIP stack, allowing other threads to enter CHIP while
            // the event loop thread is sleeping.
            StackUnlock unlock;
            eventReceived = xQueueReceive(mChipEventQueue, &event, waitTime);
        }

        // If an event was received, dispatch it and continue until the queue is empty.
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
    mEventLoopTask = xTaskCreateStatic(EventLoopTaskMain, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME, MATTER_ARRAY_SIZE(mEventLoopStack),
                                       this, CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, mEventLoopStack, &mEventLoopTaskStruct);
#else
    xTaskCreate(EventLoopTaskMain, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE / sizeof(StackType_t),
                this, CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, &mEventLoopTask);
#endif
    return (mEventLoopTask != NULL) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::EventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP event task running");
    GenericPlatformManagerImpl_FreeRTOS<ImplClass> * platformManager =
        static_cast<GenericPlatformManagerImpl_FreeRTOS<ImplClass> *>(arg);
    platformManager->Impl()->RunEventLoop();
    vTaskDelete(NULL);
    platformManager->mEventLoopTask = NULL;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_PostBackgroundEvent(const ChipDeviceEvent * event)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    if (mBackgroundEventQueue == NULL)
    {
        return CHIP_ERROR_INTERNAL;
    }
    if (!(event->Type == DeviceEventType::kCallWorkFunct || event->Type == DeviceEventType::kNoOp))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    auto status = xQueueSendToBack(mBackgroundEventQueue, event, 1);
    if (status != pdTRUE)
    {
        ChipLogError(DeviceLayer, "Failed to post event to CHIP background event queue");
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
#else
    // Use foreground event loop for background events
    return _PostEvent(event);
#endif
}

template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_RunBackgroundEventLoop(void)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    bool oldShouldRunBackgroundEventLoop = false;
    if (!mShouldRunBackgroundEventLoop.compare_exchange_strong(oldShouldRunBackgroundEventLoop /* expected */, true /* desired */))
    {
        ChipLogError(DeviceLayer, "Error trying to run the background event loop while it is already running");
        return;
    }

    while (mShouldRunBackgroundEventLoop.load())
    {
        ChipDeviceEvent event;
        auto eventReceived = xQueueReceive(mBackgroundEventQueue, &event, portMAX_DELAY) == pdTRUE;
        while (eventReceived)
        {
            Impl()->DispatchEvent(&event);
            eventReceived = xQueueReceive(mBackgroundEventQueue, &event, portMAX_DELAY) == pdTRUE;
        }
    }
#else
    // Use foreground event loop for background events
#endif
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StartBackgroundEventLoopTask(void)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
#if defined(CHIP_CONFIG_FREERTOS_USE_STATIC_TASK) && CHIP_CONFIG_FREERTOS_USE_STATIC_TASK
    mBackgroundEventLoopTask = xTaskCreateStatic(
        BackgroundEventLoopTaskMain, CHIP_DEVICE_CONFIG_BG_TASK_NAME, MATTER_ARRAY_SIZE(mBackgroundEventLoopStack), this,
        CHIP_DEVICE_CONFIG_BG_TASK_PRIORITY, mBackgroundEventLoopStack, &mBackgroundEventLoopTaskStruct);
#else
    xTaskCreate(BackgroundEventLoopTaskMain, CHIP_DEVICE_CONFIG_BG_TASK_NAME,
                CHIP_DEVICE_CONFIG_BG_TASK_STACK_SIZE / sizeof(StackType_t), this, CHIP_DEVICE_CONFIG_BG_TASK_PRIORITY,
                &mBackgroundEventLoopTask);
#endif
    return (mBackgroundEventLoopTask != NULL) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
#else
    // Use foreground event loop for background events
    return CHIP_NO_ERROR;
#endif
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StopBackgroundEventLoopTask(void)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    bool oldShouldRunBackgroundEventLoop = true;
    if (mShouldRunBackgroundEventLoop.compare_exchange_strong(oldShouldRunBackgroundEventLoop /* expected */, false /* desired */))
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        xQueueSendToBack(mBackgroundEventQueue, &noop, 0);
    }
    return CHIP_NO_ERROR;
#else
    // Use foreground event loop for background events
    return CHIP_NO_ERROR;
#endif
}

#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
template <class ImplClass>
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::BackgroundEventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP background task running");
    GenericPlatformManagerImpl_FreeRTOS<ImplClass> * platformManager =
        static_cast<GenericPlatformManagerImpl_FreeRTOS<ImplClass> *>(arg);
    platformManager->Impl()->RunBackgroundEventLoop();
    vTaskDelete(NULL);
    platformManager->mBackgroundEventLoopTask = NULL;
}
#endif

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StartChipTimer(System::Clock::Timeout delay)
{
    mChipTimerActive = true;
    vTaskSetTimeOutState(&mNextTimerBaseTime);
    mNextTimerDurationTicks = pdMS_TO_TICKS(System::Clock::Milliseconds64(delay).count());

    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (xTaskGetCurrentTaskHandle() != mEventLoopTask)
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        ReturnErrorOnFailure(Impl()->PostEvent(&noop));
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
void GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_Shutdown(void)
{
    if (mChipEventQueue)
    {
        vQueueDelete(mChipEventQueue);
        mChipEventQueue = NULL;
    }
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    if (mBackgroundEventQueue)
    {
        vQueueDelete(mBackgroundEventQueue);
        mBackgroundEventQueue = NULL;
    }
#endif
    vSemaphoreDelete(mChipStackLock);
    mChipStackLock = NULL;
    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_StopEventLoopTask(void)
{
    mShouldRunEventLoop.store(false);
    return CHIP_NO_ERROR;
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_FREERTOS_CPP
