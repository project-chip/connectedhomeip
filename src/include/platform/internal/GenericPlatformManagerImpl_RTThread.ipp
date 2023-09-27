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
 *          GenericPlatformManagerImpl_RTThread<> template.
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_RTTHREAD_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_RTTHREAD_CPP

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_RTThread.h>
#include <lib/support/CodeUtils.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_RTThread<> template inherits.
#include <platform/internal/GenericPlatformManagerImpl.ipp>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_RTThread<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize any necessary data structures and resources here

    // Create a mutex for the CHIP stack lock
    mChipStackLock = rt_mutex_create("CHIPStackLock", RT_IPC_FLAG_FIFO);
    if (mChipStackLock == RT_NULL)
    {
        ChipLogError(DeviceLayer, "Failed to create CHIP stack lock");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
    }

    // Create an event queue for CHIP events
    mChipEventQueue = rt_mq_create("CHIPEventQueue", sizeof(ChipDeviceEvent), CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE,
                                   RT_IPC_FLAG_FIFO);
    if (mChipEventQueue == RT_NULL)
    {
        ChipLogError(DeviceLayer, "Failed to allocate CHIP event queue");
        ExitNow(err = CHIP_ERROR_NO_MEMORY);
    }

    mShouldRunEventLoop.store(false);

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

exit:
    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl_RTThread<ImplClass>::_LockChipStack(void)
{
    rt_mutex_take(mChipStackLock, RT_WAITING_FOREVER);
}

template <class ImplClass>
bool GenericPlatformManagerImpl_FreeRTOS<ImplClass>::_TryLockChipStack(void)
{
    return rt_mutex_trytake(mChipStackLock) == RT_EOK;
}

template <class ImplClass>
void GenericPlatformManagerImpl_RTThread<ImplClass>::_UnlockChipStack(void)
{
    rt_mutex_release(mChipStackLock);
}

#if CHIP_STACK_LOCK_TRACKING_ENABLED
template <class ImplClass>
bool GenericPlatformManagerImpl_RTThread<ImplClass>::_IsChipStackLockedByCurrentThread() const
{
    // TODO
}
#endif // CHIP_STACK_LOCK_TRACKING_ENABLED

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_RTThread<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    if (mChipEventQueue == RT_NULL)
    {
        return CHIP_ERROR_INTERNAL;
    }

    rt_err_t result = rt_mq_send(&mChipEventQueue, event, sizeof(ChipDeviceEvent));
    if (result != RT_EOK)
    {
        ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        return CHIP_ERROR(chip::ChipError::Range::kOS, result);
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_RTThread<ImplClass>::_RunEventLoop(void)
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
        rt_tick_t waitTime;

        // If one or more CHIP timers are active...
        if (mChipTimerActive)
        {
            // Adjust the base time and remaining duration for the next scheduled timer based on the
            // amount of time that has elapsed since it was started.
            // IF the timer's expiration time has already arrived...
            if (rt_tick_get() >= mNextTimerBaseTime + mNextTimerDurationTicks)
            {
                // Reset the 'timer active' flag. This will be set to true again by _StartChipTimer()
                // if there are further timers beyond the expired one that are still active.
                mChipTimerActive = false;

                // Call into the system layer to dispatch the callback functions for all timers
                // that have expired.
                err = static_cast<System::LayerImplRTThread &>(DeviceLayer::SystemLayer()).HandlePlatformTimer();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Error handling CHIP timers: %" CHIP_ERROR_FORMAT, err.Format());
                }

                // When processing the event queue below, do not wait if the queue is empty. Instead
                // immediately loop around and process timers again
                waitTime = 0;
            }
            else
            {
                waitTime = mNextTimerBaseTime + mNextTimerDurationTicks - rt_tick_get();
            }
        }
        else
        {
            // Otherwise no CHIP timers are active, so wait indefinitely for an event to arrive on the event
            // queue.
            waitTime = RT_WAITING_FOREVER;
        }

        rt_size_t receivedSize;
        rt_err_t result = rt_mq_recv(&mChipEventQueue, &event, sizeof(ChipDeviceEvent), RT_WAITING_FOREVER);
        if (result == RT_EOK)
        {
            Impl()->DispatchEvent(&event);
        }
    }
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_RTThread<ImplClass>::_StartEventLoopTask(void)
{
    rt_thread_t thread = rt_thread_create("CHIPEventLoopTask", EventLoopTaskMain, RT_NULL, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE,
                                         CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, RT_TICK_PER_SECOND);
    if (thread == RT_NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    rt_thread_startup(thread);

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_RTThread<ImplClass>::EventLoopTaskMain(void * arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_RTThread<ImplClass> *>(arg)->Impl()->RunEventLoop();
    // TODO: At this point, should we not
    // vTaskDelete(static_cast<GenericPlatformManagerImpl_RTThread<ImplClass> *>(arg)->mEventLoopTask)?
    // Or somehow get our caller to do it once this thread is joined?
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_RTThread<ImplClass>::_StartChipTimer(System::Clock::Timeout delay)
{
    mChipTimerActive = true;
    mNextTimerBaseTime = rt_tick_get();
    // mNextTimerDurationTicks = RT_TICK_PER_SECOND * static_cast<uint32_t>(delay.ToMilliseconds());
    mNextTimerDurationTicks = RT_TICK_PER_SECOND * static_cast<uint32_t>(delay.count());

    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (rt_thread_self() != mEventLoopThread)
    {
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kNoOp;
        ReturnErrorOnFailure(Impl()->PostEvent(&event));
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_RTThread<ImplClass>::PostEventFromISR(const ChipDeviceEvent * event, rt_base_t & yieldRequired)
{
    yieldRequired = RT_FALSE;

    rt_interrupt_enter();   
    if (mChipEventQueue != RT_NULL)
    {
        rt_err_t result = rt_mq_send(&mChipEventQueue, &event, sizeof(ChipDeviceEvent));
        if (result != RT_EOK)
        {
            ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        }
    }
    rt_interrupt_leave();
}

template <class ImplClass>
void GenericPlatformManagerImpl_RTThread<ImplClass>::_Shutdown(void)
{
    GenericPlatformManagerImpl<ImplClass>::_Shutdown();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_RTThread<ImplClass>::_StopEventLoopTask(void)
{
    mShouldRunEventLoop.store(false);
    return CHIP_NO_ERROR;
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericPlatformManagerImpl_RTThread<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_RTTHREAD_CPP