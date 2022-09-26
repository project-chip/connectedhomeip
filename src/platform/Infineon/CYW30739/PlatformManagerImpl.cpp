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

#include <platform/Infineon/CYW30739/DiagnosticDataProviderImpl.h>
#include <platform/PlatformManager.h>
#include <platform/internal/GenericPlatformManagerImpl.ipp>

#include <crypto/CHIPCryptoPAL.h>
#include <hal/wiced_memory.h>
#include <wiced_platform.h>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wiced_result_t result;

    // Initialize the configuration system.
    err = PersistedStorage::KeyValueStoreMgrImpl().Init();
    SuccessOrExit(err);

    /* Create the thread object. */
    mThread = wiced_rtos_create_thread();
    VerifyOrExit(mThread != nullptr, err = CHIP_ERROR_NO_MEMORY);

    /* Initialize the event flags. */
    mEventFlags = wiced_rtos_create_event_flags();
    VerifyOrExit(mEventFlags != nullptr, err = CHIP_ERROR_NO_MEMORY);

    result = wiced_rtos_init_event_flags(mEventFlags);
    VerifyOrExit(result == WICED_SUCCESS, err = CHIP_ERROR_NO_MEMORY);
    /* Initialize the event queue. */
    mEventQueue = wiced_rtos_create_queue();
    VerifyOrExit(mEventQueue != nullptr, err = CHIP_ERROR_NO_MEMORY);

    result = wiced_rtos_init_queue(mEventQueue, "EventQueue", sizeof(ChipDeviceEvent), CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);
    VerifyOrExit(result == WICED_SUCCESS, err = CHIP_ERROR_NO_MEMORY);

    /* Initialize the timer. */
    result = wiced_init_timer(&mTimer, TimerCallback, 0, WICED_MILLI_SECONDS_TIMER);
    VerifyOrExit(result == WICED_SUCCESS, err = CHIP_ERROR_INTERNAL);

    /* Initialize the mutex. */
    mMutex = wiced_rtos_create_mutex();
    VerifyOrExit(mMutex != nullptr, err = CHIP_ERROR_NO_MEMORY);

    result = wiced_rtos_init_mutex(mMutex);
    VerifyOrExit(result == WICED_SUCCESS, err = CHIP_ERROR_INTERNAL);

    ReturnErrorOnFailure(chip::Crypto::add_entropy_source(GetEntropy, NULL, 16));

    ReturnErrorOnFailure(GenericPlatformManagerImpl<ImplClass>::_InitChipStack());

exit:
    return err;
}

void PlatformManagerImpl::_RunEventLoop(void)
{
    ChipLogDetail(DeviceLayer, "Free RAM sizes: %lu\n", wiced_memory_get_free_bytes());

    while (true)
    {
        uint32_t flags_set          = 0;
        const wiced_result_t result = wiced_rtos_wait_for_event_flags(mEventFlags, 0xffffffff, &flags_set, WICED_TRUE,
                                                                      WAIT_FOR_ANY_EVENT, WICED_WAIT_FOREVER);
        if (result != WICED_SUCCESS)
        {
            ChipLogError(DeviceLayer, "wiced_rtos_wait_for_event_flags 0x%08x", result);
            continue;
        }

        if (flags_set & kTimerEventFlag)
        {
            HandleTimerEvent();
        }

        if (flags_set & kPostEventFlag)
        {
            HandlePostEvent();
        }
    }
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    wiced_result_t result;

    result = wiced_rtos_init_thread(mThread, CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME,
                                    EventLoopTaskMain, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE, this);
    VerifyOrExit(result == WICED_SUCCESS, err = CHIP_ERROR_NO_MEMORY);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_LockChipStack(void)
{
    const wiced_result_t result = wiced_rtos_lock_mutex(mMutex);
    VerifyOrReturn(result == WICED_SUCCESS, ChipLogError(DeviceLayer, "%s %x", __func__, result));
}

void PlatformManagerImpl::_UnlockChipStack(void)
{
    const wiced_result_t result = wiced_rtos_unlock_mutex(mMutex);
    VerifyOrReturn(result == WICED_SUCCESS || result == WICED_NOT_OWNED, ChipLogError(DeviceLayer, "%s %x", __func__, result));
}

CHIP_ERROR PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{
    const wiced_result_t result = wiced_rtos_push_to_queue(mEventQueue, const_cast<ChipDeviceEvent *>(event), WICED_NO_WAIT);
    if (WICED_SUCCESS != result)
    {
        ChipLogError(DeviceLayer, "wiced_rtos_push_to_queue %u", result);
        return CHIP_ERROR_INTERNAL;
    }

    SetEventFlags(kPostEventFlag);

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
        const wiced_result_t result = wiced_start_timer(&mTimer, durationMS.count());
        if (WICED_SUCCESS != result)
        {
            ChipLogError(DeviceLayer, "wiced_start_timer 0x%02x", result);
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_Shutdown() {}

void PlatformManagerImpl::SetEventFlags(uint32_t flags)
{
    assert(!wiced_rtos_check_for_stack_overflow());

    if (wiced_rtos_set_event_flags(mEventFlags, flags) != WICED_SUCCESS)
    {
        ChipLogError(DeviceLayer, "%s wiced_rtos_set_event_flags %08lx", __func__, flags);
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
    wiced_result_t result;
    ChipDeviceEvent event;

    /* Check the event queue. */
    if (wiced_rtos_is_queue_empty(mEventQueue))
    {
        return;
    }

    /* Pop one event from the event queue. */
    result = wiced_rtos_pop_from_queue(mEventQueue, &event, WICED_WAIT_FOREVER);

    if (WICED_SUCCESS != result)
    {
        ChipLogError(DeviceLayer, "wiced_rtos_pop_from_queue %u", result);
        return;
    }

    /* Process this event. */
    DispatchEvent(&event);

    /* Set another application thread event if the event queue is not empty. */
    if (!wiced_rtos_is_queue_empty(mEventQueue))
    {
        SetEventFlags(kPostEventFlag);
    }
}

void PlatformManagerImpl::EventLoopTaskMain(uint32_t arg)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    reinterpret_cast<PlatformManagerImpl *>(arg)->RunEventLoop();
}

void PlatformManagerImpl::TimerCallback(WICED_TIMER_PARAM_TYPE params)
{
    PlatformMgrImpl().SetEventFlags(kTimerEventFlag);
}

int PlatformManagerImpl::GetEntropy(void * data, unsigned char * output, size_t len, size_t * olen)
{
    const wiced_result_t result = wiced_platform_entropy_get(output, static_cast<uint16_t>(len));
    if (result != WICED_SUCCESS)
    {
        return -1;
    }
    *olen = len;
    return 0;
}

} // namespace DeviceLayer
} // namespace chip
