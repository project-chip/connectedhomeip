/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/ASR/DiagnosticDataProviderImpl.h>
#include <platform/PlatformManager.h>
#if CONFIG_ENABLE_ASR_LEGA_RTOS
#include <platform/internal/GenericPlatformManagerImpl.ipp>
#else
#include <platform/internal/GenericPlatformManagerImpl_FreeRTOS.ipp>
#endif
#include <init_Matter.h>

namespace chip {
namespace DeviceLayer {

namespace Internal {
extern CHIP_ERROR InitLwIPCoreLock(void);
}

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
#if CONFIG_ENABLE_ASR_LEGA_RTOS
    CHIP_ERROR err = CHIP_NO_ERROR;
    OSStatus result;

    /* Initialize LwIP lock. */
    err = Internal::InitLwIPCoreLock();
    SuccessOrExit(err);

    if (mEventQueue == NULL)
    {
        /* Initialize the event queue. */
        result = lega_rtos_init_queue(&mEventQueue, "EventQueue", sizeof(ChipDeviceEvent), CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);
        VerifyOrExit(result == kNoErr, err = CHIP_ERROR_NO_MEMORY);
    }

    /* Initialize the timeout. */
    lega_rtos_set_timeout(&mNextTimerBaseTime);

    if (mChipMutex == NULL)
    {
        /* Initialize the mutex. */
        result = lega_rtos_init_mutex(&mChipMutex);
        VerifyOrExit(result == kNoErr, err = CHIP_ERROR_INTERNAL);
    }

    mChipTimerActive = false;
    mShouldRunEventLoop.store(false);

    ReturnErrorOnFailure(GenericPlatformManagerImpl<ImplClass>::_InitChipStack());
#else
    CHIP_ERROR err;

    // Make sure the LwIP core lock has been initialized
    err = Internal::InitLwIPCoreLock();
    SuccessOrExit(err);

    mStartTime = System::SystemClock().GetMonotonicTimestamp();

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);
#endif
exit:
    return err;
}

#if CONFIG_ENABLE_ASR_LEGA_RTOS

void PlatformManagerImpl::_RunEventLoop()
{
    RunEventLoopInternal();
}

void PlatformManagerImpl::RunEventLoopInternal(void)
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
        uint32_t waitTime;

        // If one or more CHIP timers are active...
        if (mChipTimerActive)
        {
            // Adjust the base time and remaining duration for the next scheduled timer based on the
            // amount of time that has elapsed since it was started.
            // IF the timer's expiration time has already arrived...
            if (lega_rtos_check_timeout(&mNextTimerBaseTime, &mNextTimerDurationTicks) == TRUE)
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
            waitTime = LEGA_WAIT_FOREVER;
        }

        OSStatus result;
        {
            // Unlock the CHIP stack, allowing other threads to enter CHIP while
            // the event loop thread is sleeping.
            StackUnlock unlock;
            result = lega_rtos_pop_from_queue(&mEventQueue, &event, waitTime);
        }

        // If an event was received, dispatch it and continue until the queue is empty.
        while (result == kNoErr)
        {
            DispatchEvent(&event);
            result = lega_rtos_pop_from_queue(&mEventQueue, &event, LEGA_NO_WAIT);
        }
    }
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask(void)
{
    lega_task_config_t cfg;

    MatterInitializer::Matter_Task_Config(&cfg);

    OSStatus result = lega_rtos_create_thread(&mThread, cfg.task_priority, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
                                              (lega_thread_function_t) EventLoopTaskMain, cfg.stack_size, (lega_thread_arg_t) this);

    if (result != kNoErr)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    mShouldRunEventLoop.store(false);

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_LockChipStack(void)
{
    OSStatus result = lega_rtos_lock_mutex(&mChipMutex, LEGA_WAIT_FOREVER);
    VerifyOrReturn(result == kNoErr, ChipLogError(DeviceLayer, "%s %x", __func__, result));
}

bool PlatformManagerImpl::_TryLockChipStack(void)
{
    if (lega_rtos_lock_mutex(&mChipMutex, LEGA_NO_WAIT) == kNoErr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PlatformManagerImpl::_UnlockChipStack(void)
{
    OSStatus result = lega_rtos_unlock_mutex(&mChipMutex);
    VerifyOrReturn(result == kNoErr, ChipLogError(DeviceLayer, "%s %x", __func__, result));
}

CHIP_ERROR PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{
    OSStatus result = lega_rtos_push_to_queue(&mEventQueue, const_cast<ChipDeviceEvent *>(event), LEGA_NO_WAIT);
    if (kNoErr != result)
    {
        ChipLogError(DeviceLayer, "lega_rtos_push_to_queue %u", result);
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(System::Clock::Timeout durationMS)
{
    mChipTimerActive = true;
    lega_rtos_set_timeout(&mNextTimerBaseTime);
    mNextTimerDurationTicks = (lega_tick_t) ms_to_tick(System::Clock::Milliseconds64(durationMS).count());
    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (lega_rtos_get_current_thread() != mThread)
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        ReturnErrorOnFailure(PostEvent(&noop));
    }

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::EventLoopTaskMain(uint32_t arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    PlatformMgrImpl().RunEventLoopInternal();
    lega_rtos_delete_thread(NULL);
}
#else
CHIP_ERROR PlatformManagerImpl::InitLwIPCoreLock(void)
{
    return Internal::InitLwIPCoreLock();
}
#endif

void PlatformManagerImpl::_Shutdown()
{
    uint64_t upTime = 0;

    if (GetDiagnosticDataProvider().GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalOperationalHours = 0;

        if (ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours) == CHIP_NO_ERROR)
        {
            ConfigurationMgr().StoreTotalOperationalHours(totalOperationalHours + static_cast<uint32_t>(upTime / 3600));
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get total operational hours of the Node");
        }
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to get current uptime since the Node’s last reboot");
    }
#if CONFIG_ENABLE_ASR_LEGA_RTOS
    //
    // Call up to the base class _Shutdown() to perform the actual stack de-initialization
    // and clean-up
    //
    Internal::GenericPlatformManagerImpl<PlatformManagerImpl>::_Shutdown();
#else
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
#endif
}

} // namespace DeviceLayer
} // namespace chip
