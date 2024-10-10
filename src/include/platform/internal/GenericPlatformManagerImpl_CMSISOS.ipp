/*
 *
 *    Copyright (c) 2021-2024 Project CHIP Authors
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
 *          GenericPlatformManagerImpl_CMSISOS<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_CMSISOS_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_CMSISOS_CPP

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_CMSISOS.h>

#include <lib/support/CodeUtils.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_CMSISOS<> template inherits.
#include <platform/internal/GenericPlatformManagerImpl.ipp>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_InitChipStack(void)
{
    mNextTimerBaseTime      = osKernelGetTickCount();
    mNextTimerDurationTicks = 0;
    mChipTimerActive        = false;

    // We support calling Shutdown followed by InitChipStack, because some tests
    // do that.  To keep things simple for existing consumers, we do not
    // destroy our lock and queue at shutdown, but rather check whether they
    // already exist here before trying to create them.
    if (mChipStackLock == nullptr)
    {
        mChipStackLock = osMutexNew(&mChipStackMutexAttr);
        VerifyOrReturnError(mChipStackLock != nullptr, CHIP_ERROR_NO_MEMORY,
                            ChipLogError(DeviceLayer, "Failed to create CHIP stack lock"));
    }

    if (mChipEventQueue == nullptr)
    {
        mChipEventQueue = osMessageQueueNew(CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), mEventQueueAttrPtr);
        VerifyOrReturnError(mChipEventQueue != nullptr, CHIP_ERROR_NO_MEMORY,
                            ChipLogError(DeviceLayer, "Failed to allocate CHIP main event queue"));
    }
    else
    {
        // Clear out any events that might be stuck in the queue, so we start
        // with a clean slate, as if we had just re-created the queue.
        osMessageQueueReset(mChipEventQueue);
    }

    mShouldRunEventLoop.store(false);

#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    if (mBackgroundEventQueue == nullptr)
    {
        mBackgroundEventQueue =
            osMessageQueueNew(CHIP_DEVICE_CONFIG_BG_MAX_EVENT_QUEUE_SIZE, sizeof(ChipDeviceEvent), mBgQueueAttrPtr);
        VerifyOrReturnError(mBackgroundEventQueue != nullptr, CHIP_ERROR_NO_MEMORY,
                            ChipLogError(DeviceLayer, "Failed to allocate CHIP background event queue"));
    }
    else
    {
        osMessageQueueReset(mBackgroundEventQueue);
    }

    mShouldRunBackgroundEventLoop.store(false);
#endif

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    return GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
}

template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::_LockChipStack(void)
{
    osMutexAcquire(mChipStackLock, osWaitForever);
}

template <class ImplClass>
bool GenericPlatformManagerImpl_CMSISOS<ImplClass>::_TryLockChipStack(void)
{
    return osMutexAcquire(mChipStackLock, 0) == osOK;
}

template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::_UnlockChipStack(void)
{
    osMutexRelease(mChipStackLock);
}

#if CHIP_STACK_LOCK_TRACKING_ENABLED
template <class ImplClass>
bool GenericPlatformManagerImpl_CMSISOS<ImplClass>::_IsChipStackLockedByCurrentThread() const
{
    // If we have not started our event loop yet, return true because in that
    // case we can't be racing against the (not yet started) event loop.
    //
    // Similarly, if mChipStackLock has not been created yet, might as well
    // return true.
    return (mEventLoopTask == nullptr) || (mChipStackLock == nullptr) || (osMutexGetOwner(mChipStackLock) == osThreadGetId());
}
#endif // CHIP_STACK_LOCK_TRACKING_ENABLED

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    VerifyOrReturnError(mChipEventQueue != nullptr, CHIP_ERROR_UNINITIALIZED);

    osStatus_t status = osMessageQueuePut(mChipEventQueue, event, osPriorityNormal, 1);
    if (status != osOK)
    {
        ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        return CHIP_ERROR(chip::ChipError::Range::kOS, status);
    }
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::_RunEventLoop(void)
{
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
        uint32_t waitTimeInTicks;

        // If one or more CHIP timers are active...
        if (mChipTimerActive)
        {
            // Adjust the base time and remaining duration for the next scheduled timer based on the
            // amount of time that has elapsed since it was started.
            // When the timer's expiration time elapses, Handle the platform Timer
            // else wait for a queue event for timer remaining time.
            waitTimeInTicks = SyncNextChipTimerHandling();
            if (waitTimeInTicks == 0)
            {
                // Reset the 'timer active' flag.  This will be set to true again by _StartChipTimer()
                // if there are further timers beyond the expired one that are still active.
                mChipTimerActive = false;

                // Call into the system layer to dispatch the callback functions for all timers
                // that have expired.
                // TODO We use the same SystemLayer implementation as FreeRTOS, Nothing in it is freeRTOS specific. We should
                // it.
                CHIP_ERROR err = static_cast<System::LayerImplFreeRTOS &>(DeviceLayer::SystemLayer()).HandlePlatformTimer();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Error handling CHIP timers: %" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        else
        {
            // No CHIP timers are active, so we wait indefinitely for an event to arrive on the event
            // queue.
            waitTimeInTicks = osWaitForever;
        }

        // Unlock the CHIP stack, allowing other threads to enter CHIP while
        // the event loop thread is sleeping.
        StackUnlock unlock;
        ChipDeviceEvent event;
        osStatus_t eventReceived = osMessageQueueGet(mChipEventQueue, &event, nullptr, waitTimeInTicks);

        // If an event was received, dispatch it and continue until the queue is empty.
        while (eventReceived == osOK)
        {
            StackLock lock;
            Impl()->DispatchEvent(&event);
            StackUnlock unlock;
            eventReceived = osMessageQueueGet(mChipEventQueue, &event, nullptr, 0);
        }
    }
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_StartEventLoopTask(void)
{
    mEventLoopTask = osThreadNew(EventLoopTaskMain, this, &mEventLoopTaskAttr);
    return (mEventLoopTask != nullptr) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
}

template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::EventLoopTaskMain(void * pvParameter)
{
    ChipLogDetail(DeviceLayer, "CHIP event task running");
    static_cast<GenericPlatformManagerImpl_CMSISOS<ImplClass> *>(pvParameter)->Impl()->RunEventLoop();
}

/**
 * @brief Calculate the elapsed time of the active chip platform timer since it has been started,
 *        as set in mNextTimerBaseTime, and adjust its remaining time with the mNextTimerDurationTicks member
 *
 * @return The next Timer remaining time in ticks
 */
template <class ImplClass>
uint32_t GenericPlatformManagerImpl_CMSISOS<ImplClass>::SyncNextChipTimerHandling()
{
    uint32_t elapsedTime   = 0;
    uint32_t timerBaseTime = mNextTimerBaseTime;
    uint32_t currentTime   = osKernelGetTickCount();
    if (currentTime < timerBaseTime)
    {
        // TickCount has wrapped around
        elapsedTime = (UINT32_MAX - timerBaseTime) + currentTime;
    }
    else
    {
        elapsedTime = currentTime - timerBaseTime;
    }

    if (elapsedTime < mNextTimerDurationTicks)
    {
        // We didn't timeout yet, adjust the remaining time
        mNextTimerDurationTicks -= elapsedTime;
        mNextTimerBaseTime = osKernelGetTickCount();
    }
    else
    {
        mNextTimerDurationTicks = 0;
    }
    return mNextTimerDurationTicks;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_PostBackgroundEvent(const ChipDeviceEvent * event)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    VerifyOrReturnError(mBackgroundEventQueue != nullptr, CHIP_ERROR_UNINITIALIZED);
    if (!(event->Type == DeviceEventType::kCallWorkFunct || event->Type == DeviceEventType::kNoOp))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    osStatus_t status = osMessageQueuePut(mBackgroundEventQueue, event, osPriorityNormal, 1);
    VerifyOrReturnError(status == osOk, CHIP_ERROR_NO_MEMORY,
                        ChipLogError(DeviceLayer, "Failed to post event to CHIP background event queue"));
    return CHIP_NO_ERROR;
#else
    // Use foreground event loop for background events
    return _PostEvent(event);
#endif
}

template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::_RunBackgroundEventLoop(void)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    bool oldShouldRunBackgroundEventLoop = false;
    VerifyOrReturn(
        mShouldRunBackgroundEventLoop.compare_exchange_strong(oldShouldRunBackgroundEventLoop /* expected */, true /* desired */),
        ChipLogError(DeviceLayer, "Error trying to run the background event loop while it is already running"));

    while (mShouldRunBackgroundEventLoop.load())
    {
        ChipDeviceEvent event;
        osStatus_t eventReceived = osMessageQueueGet(mBackgroundEventQueue, &event, NULL, osWaitForever);
        while (eventReceived == osOK)
        {
            Impl()->DispatchEvent(&event);
            eventReceived = osMessageQueueGet(mBackgroundEventQueue, &event, portMAX_DELAY);
        }
    }
#endif
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_StartBackgroundEventLoopTask(void)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    mBackgroundEventLoopTask = osThreadNew(BackgroundEventLoopTaskMain, this, &mBgEventLoopTaskAttr);
    return (mBackgroundEventLoopTask != NULL) ? CHIP_NO_ERROR : CHIP_ERROR_NO_MEMORY;
#else
    // Use foreground event loop for background events
    return CHIP_NO_ERROR;
#endif
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_StopBackgroundEventLoopTask(void)
{
#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
    bool oldShouldRunBackgroundEventLoop = true;
    if (mShouldRunBackgroundEventLoop.compare_exchange_strong(oldShouldRunBackgroundEventLoop /* expected */, false /* desired */))
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        osMessageQueuePut(mBackgroundEventQueue, &noop, osPriorityNormal, 0);
    }
    return CHIP_NO_ERROR;
#else
    // Use foreground event loop for background events
    return CHIP_NO_ERROR;
#endif
}

#if defined(CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING) && CHIP_DEVICE_CONFIG_ENABLE_BG_EVENT_PROCESSING
template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::BackgroundEventLoopTaskMain(void * pvParameter)
{
    ChipLogDetail(DeviceLayer, "CHIP background task running");
    static_cast<GenericPlatformManagerImpl_CMSISOS<ImplClass> *>(pvParameter)->Impl()->RunBackgroundEventLoop();
}
#endif

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_StartChipTimer(System::Clock::Timeout delay)
{
    mChipTimerActive        = true;
    mNextTimerBaseTime      = osKernelGetTickCount();
    mNextTimerDurationTicks = (System::Clock::Milliseconds64(delay).count() * osKernelGetTickFreq()) / 1000;

    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (osThreadGetId() != mEventLoopTask)
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        ReturnErrorOnFailure(Impl()->PostEvent(&noop));
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_CMSISOS<ImplClass>::_Shutdown(void)
{
    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_CMSISOS<ImplClass>::_StopEventLoopTask(void)
{
    mShouldRunEventLoop.store(false);
    return CHIP_NO_ERROR;
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericPlatformManagerImpl_CMSISOS<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_CMSISOS_CPP
