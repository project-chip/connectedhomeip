/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          GenericPlatformManagerImpl_Zephyr<> template which does not use select().
 */

#ifndef GENERIC_PLATFORM_MANAGER_IMPL_ZEPHYR_CPP
#define GENERIC_PLATFORM_MANAGER_IMPL_ZEPHYR_CPP

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericPlatformManagerImpl_Zephyr.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
// from which the GenericPlatformManagerImpl_Zephyr<> template inherits.
#include <platform/internal/GenericPlatformManagerImpl.ipp>

#include <system/SystemError.h>
#include <system/SystemLayer.h>
#include <system/SystemStats.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Semaphore used to wake up the event loop thread.
K_SEM_DEFINE(sEventSignal, 0, 1);

// Work item used to signal the event loop from ISR context, since giving a semaphore
// directly from ISR may not wake up the thread immediately in all cases.
K_WORK_DEFINE(sSignalWork, [](k_work *) { k_sem_give(&sEventSignal); });

// Timer handler to wake up the event loop after a small delay.
// This runs in ISR context, so we use the work queue to signal safely.
void WakeupTimerHandler(k_timer * timer)
{
    k_work_submit(&sSignalWork);
}

// Timer used to schedule a delayed wake-up of the event loop when _StartChipTimer is called.
// The delay allows the calling thread to complete its operation before the event loop
// processes the timer, avoiding race conditions where timers fire too early.
K_TIMER_DEFINE(sWakeupTimer, WakeupTimerHandler, nullptr);

// Delay before waking up the event loop after a timer is scheduled from another thread.
// 1ms is enough to let the calling thread finish its current operation.
constexpr uint32_t kWakeupDelayMs = 1;

} // anonymous namespace

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mInitialized)
        return err;

    k_mutex_init(&mChipStackLock);

    k_msgq_init(&mChipEventQueue, reinterpret_cast<char *>(&mChipEventRingBuffer), sizeof(ChipDeviceEvent),
                CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);

    mShouldRunEventLoop = false;

#ifdef CONFIG_CHIP_CRYPTO_PSA
    VerifyOrReturnError(psa_crypto_init() == PSA_SUCCESS, CHIP_ERROR_INTERNAL);
#endif

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

    mInitialized = true;

exit:
    return err;
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::_LockChipStack(void)
{
    k_mutex_lock(&mChipStackLock, K_FOREVER);
}

template <class ImplClass>
bool GenericPlatformManagerImpl_Zephyr<ImplClass>::_TryLockChipStack(void)
{
    return k_mutex_lock(&mChipStackLock, K_NO_WAIT) == 0;
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::_UnlockChipStack(void)
{
    k_mutex_unlock(&mChipStackLock);
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StartChipTimer(System::Clock::Timeout delay)
{
    // The timer is already recorded in the system layer's timer list.
    // If called from a different thread, schedule a delayed wake-up to let the event loop
    // process the timer. The delay allows the calling thread to complete its operation
    // before the event loop processes the timer, avoiding race conditions.
    if (k_current_get() != &mChipThread)
    {
        k_timer_start(&sWakeupTimer, K_MSEC(kWakeupDelayMs), K_NO_WAIT);
    }
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StopEventLoopTask(void)
{
    mShouldRunEventLoop = false;

    if (k_current_get() != &mChipThread)
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        ReturnErrorOnFailure(Impl()->PostEvent(&noop));
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::_Shutdown(void)
{
#ifdef CONFIG_REBOOT
    sys_reboot(SYS_REBOOT_WARM);
#else
    // NB: When this is implemented, |mInitialized| can be removed.
#endif
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    int status = k_msgq_put(&mChipEventQueue, event, K_NO_WAIT);
    if (status != 0)
    {
        ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        return System::MapErrorZephyr(status);
    }

    SYSTEM_STATS_INCREMENT(System::Stats::kPlatformMgr_NumEvents);

    // The k_poll in the event loop is configured with K_POLL_TYPE_MSGQ_DATA_AVAILABLE,
    // so it will automatically wake up when k_msgq_put adds data to the queue.
    if (k_is_in_isr())
    {
        (void) k_work_submit(&sSignalWork);
    }

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::_RunEventLoop(void)
{
    Impl()->LockChipStack();

    if (mShouldRunEventLoop)
    {
        ChipLogError(DeviceLayer, "Error trying to run the event loop while it is already running");
        return;
    }
    mShouldRunEventLoop = true;

    System::LayerImplZephyr & systemLayer = static_cast<System::LayerImplZephyr &>(DeviceLayer::SystemLayer());

    // Define poll events for message queue and signal semaphore.
    // This allows waking up when either an event is posted or a signal arrives.
    struct k_poll_event pollEvents[2];

    while (mShouldRunEventLoop)
    {
        k_timeout_t timeout = K_FOREVER;
        ChipDeviceEvent event;

        // Get the awaken time for the next timer
        std::optional<System::Clock::Timestamp> nextAwakenTime = systemLayer.GetNextAwakenTime();
        if (nextAwakenTime.has_value())
        {
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
            if (nextAwakenTime.value() > now)
            {
                // Calculate the timeout based on the next timer
                System::Clock::Timestamp delay = nextAwakenTime.value() - now;
                timeout                        = K_MSEC(delay.count());
            }
            else
            {
                // Some timer already expired, do not wait
                timeout = K_NO_WAIT;
            }
        }

        // Initialize poll events before each wait
        k_poll_event_init(&pollEvents[0], K_POLL_TYPE_MSGQ_DATA_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &mChipEventQueue);
        k_poll_event_init(&pollEvents[1], K_POLL_TYPE_SEM_AVAILABLE, K_POLL_MODE_NOTIFY_ONLY, &sEventSignal);

        Impl()->UnlockChipStack();

        // Wait for either:
        // 1. An event in the message queue
        // 2. A signal from ISR
        // 3. Timeout for the next timer
        k_poll(pollEvents, 2, timeout);

        // Reset the signal semaphore if it was given (consume the signal)
        k_sem_reset(&sEventSignal);

        Impl()->LockChipStack();

        // Call into the system layer to dispatch the callback functions for all timers
        // that have expired.
        CHIP_ERROR err = systemLayer.HandlePlatformTimer();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Error handling CHIP timers: %" CHIP_ERROR_FORMAT, err.Format());
        }

        // Process all pending events from the queue
        while (k_msgq_get(&mChipEventQueue, &event, K_NO_WAIT) == 0)
        {
            SYSTEM_STATS_DECREMENT(System::Stats::kPlatformMgr_NumEvents);
            Impl()->DispatchEvent(&event);
        }
    }

    Impl()->UnlockChipStack();
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::EventLoopTaskMain(void * thisPtr, void *, void *)
{
    ChipLogProgress(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_Zephyr<ImplClass> *>(thisPtr)->Impl()->RunEventLoop();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StartEventLoopTask(void)
{
    if (!mChipThreadStack)
        return CHIP_ERROR_UNINITIALIZED;

    const auto tid = k_thread_create(&mChipThread, mChipThreadStack, CHIP_DEVICE_CONFIG_CHIP_TASK_STACK_SIZE, EventLoopTaskMain,
                                     this, nullptr, nullptr, CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, 0, K_NO_WAIT);

#ifdef CONFIG_THREAD_NAME
    k_thread_name_set(tid, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME);
#else
    IgnoreUnusedVariable(tid);
#endif

    return CHIP_NO_ERROR;
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_ZEPHYR_CPP
