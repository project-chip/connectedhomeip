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

    /* Initialize the event flags. */
    result = lega_rtos_init_event_flags(&mEventFlags);
    VerifyOrExit(result == kNoErr, err = CHIP_ERROR_NO_MEMORY);

    /* Initialize the event queue. */
    result = lega_rtos_init_queue(&mEventQueue, "EventQueue", sizeof(ChipDeviceEvent), CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);
    VerifyOrExit(result == kNoErr, err = CHIP_ERROR_NO_MEMORY);

    /* Initialize the timer. */
    result = lega_rtos_init_timer(&mTimer, 1000, (timer_handler_t) TimerCallback, (void *) this);
    VerifyOrExit(result == kNoErr, err = CHIP_ERROR_INTERNAL);

    /* Initialize the mutex. */
    result = lega_rtos_init_mutex(&mChipMutex);
    VerifyOrExit(result == kNoErr, err = CHIP_ERROR_INTERNAL);

    result = lega_rtos_init_mutex(&mEventMutex);
    VerifyOrExit(result == kNoErr, err = CHIP_ERROR_INTERNAL);

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
    kTaskRunningEventFlag = 1;

    RunEventLoopInternal();
}

void PlatformManagerImpl::RunEventLoopInternal(void)
{
    while (true)
    {
        uint32_t flags_set = 0;

        OSStatus result = lega_rtos_wait_for_event_flags(&mEventFlags, kPostEventFlag | kTimerEventFlag | kTaskStopEventFlag,
                                                         &flags_set, TRUE, WAIT_FOR_ANY_EVENT, LEGA_WAIT_FOREVER);

        if (result != kNoErr)
        {
            ChipLogError(DeviceLayer, "lega_rtos_wait_for_event_flags 0x%08x", result);
            continue;
        }

        if (flags_set & kTaskStopEventFlag)
        {
            kTaskRunningEventFlag = 0;
            break;
        }

        if (flags_set & kTimerEventFlag)
        {
            HandleTimerEvent();
        }

        if (flags_set & kPostEventFlag)
        {
            HandlePostEvent();
        }

        if (kTaskRunningEventFlag == 0)
        {
            break;
        }
    }
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask(void)
{
    lega_task_config_t cfg;

    lega_rtos_lock_mutex(&mEventMutex, LEGA_WAIT_FOREVER);

    if (kTaskRunningEventFlag == 1)
    {
        lega_rtos_unlock_mutex(&mEventMutex);
        return CHIP_ERROR_BUSY;
    }

    MatterInitializer::Matter_Task_Config(&cfg);

    kTaskRunningEventFlag = 1;

    OSStatus result = lega_rtos_create_thread(&mThread, cfg.task_priority, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
                                              (lega_thread_function_t) EventLoopTaskMain, cfg.stack_size, (lega_thread_arg_t) this);

    if (result != kNoErr)
    {
        lega_rtos_unlock_mutex(&mEventMutex);
        return CHIP_ERROR_INTERNAL;
    }

    lega_rtos_unlock_mutex(&mEventMutex);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    lega_rtos_lock_mutex(&mEventMutex, LEGA_WAIT_FOREVER);

    if (kTaskRunningEventFlag == 0)
    {
        lega_rtos_unlock_mutex(&mEventMutex);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    PlatformMgrImpl().SetEventFlags(kTaskStopEventFlag);

    lega_rtos_unlock_mutex(&mEventMutex);

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_LockChipStack(void)
{
    OSStatus result = lega_rtos_lock_mutex(&mChipMutex, LEGA_WAIT_FOREVER);
    VerifyOrReturn(result == kNoErr, ChipLogError(DeviceLayer, "%s %x", __func__, result));
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

    PlatformMgrImpl().SetEventFlags(kPostEventFlag);

    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StartChipTimer(System::Clock::Timeout durationMS)
{
    if (durationMS.count() == 0)
    {
        TimerCallback(0);
    }
    else
    {
        lega_rtos_deinit_timer(&mTimer);
        lega_rtos_init_timer(&mTimer, durationMS.count(), (timer_handler_t) TimerCallback, (void *) this);
        OSStatus result = lega_rtos_start_timer(&mTimer);
        if (kNoErr != result)
        {
            ChipLogError(DeviceLayer, "wiced_start_timer 0x%02x", result);
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
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

    (void) _StopEventLoopTask();

    // the task thread is self terminating, we might have to wait if it's still processing
    while (true)
    {
        if (kTaskRunningEventFlag == 0)
        {
            break;
        }
        lega_rtos_delay_milliseconds(1);
    }

    ChipLogError(DeviceLayer, "StopEventLoopTask done.");

    lega_rtos_deinit_event_flags(&mEventFlags);
    lega_rtos_deinit_queue(&mEventQueue);
    lega_rtos_deinit_timer(&mTimer);
    lega_rtos_deinit_mutex(&mChipMutex);
    lega_rtos_deinit_mutex(&mEventMutex);

    Internal::GenericPlatformManagerImpl<PlatformManagerImpl>::_Shutdown();
#else
    Internal::GenericPlatformManagerImpl_FreeRTOS<PlatformManagerImpl>::_Shutdown();
#endif
}

#if CONFIG_ENABLE_ASR_LEGA_RTOS
void PlatformManagerImpl::SetEventFlags(uint32_t flags)
{
    if (lega_rtos_set_event_flags(&mEventFlags, flags) != kNoErr)
    {
        ChipLogError(DeviceLayer, "%s lega_rtos_set_event_flags %08lx", __func__, flags);
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

void PlatformManagerImpl::HandlePostEvent(void)
{
    OSStatus result;
    ChipDeviceEvent event;

    /* Check the event queue. */
    if (lega_rtos_is_queue_empty(&mEventQueue))
    {
        return;
    }

    /* Pop one event from the event queue. */
    result = lega_rtos_pop_from_queue(&mEventQueue, &event, LEGA_WAIT_FOREVER);

    if (kNoErr != result)
    {
        ChipLogError(DeviceLayer, "lega_rtos_pop_from_queue %u", result);
        return;
    }

    /* Process this event. */
    DispatchEvent(&event);

    /* Set another application thread event if the event queue is not empty. */
    if (!lega_rtos_is_queue_empty(&mEventQueue))
    {
        PlatformMgrImpl().SetEventFlags(kPostEventFlag);
    }
}

void PlatformManagerImpl::EventLoopTaskMain(uint32_t arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    PlatformMgrImpl().RunEventLoopInternal();
    lega_rtos_delete_thread(NULL);
}

void PlatformManagerImpl::TimerCallback(void * params)
{
    PlatformMgrImpl().SetEventFlags(kTimerEventFlag);
}
#endif

} // namespace DeviceLayer
} // namespace chip
