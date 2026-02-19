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
    // If the platform timer is being updated by a thread other than the event loop thread,
    // trigger the event loop thread to recalculate its wait time by posting a no-op event
    // to the event queue.
    if (k_current_get() != &mChipThread)
    {
        ChipDeviceEvent noop{ .Type = DeviceEventType::kNoOp };
        ReturnErrorOnFailure(Impl()->PostEvent(&noop));
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

    while (mShouldRunEventLoop)
    {
        k_timeout_t timeout = K_FOREVER;
        bool eventReceived  = false;
        ChipDeviceEvent event;

        // Get the awaken time for the next timer
        std::optional<System::Clock::Timestamp> nextAwakenTime = systemLayer.GetNextAwakenTime();
        if (nextAwakenTime.has_value())
        {
            System::Clock::Timestamp now = System::SystemClock().GetMonotonicTimestamp();
            if (nextAwakenTime.value() > now)
            {
                // Calculate the timeout
                System::Clock::Timestamp delay = nextAwakenTime.value() - now;
                timeout                        = K_MSEC(delay.count());
            }
            else
            {
                // Some timer already expired, do not wait if event queue is empty
                timeout = K_NO_WAIT;
            }
        }

        Impl()->UnlockChipStack();
        eventReceived = k_msgq_get(&mChipEventQueue, &event, timeout) == 0;
        Impl()->LockChipStack();

        // Call into the system layer to dispatch the callback functions for all timers
        // that have expired.
        CHIP_ERROR err = systemLayer.HandlePlatformTimer();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Error handling CHIP timers: %" CHIP_ERROR_FORMAT, err.Format());
        }

        while (eventReceived)
        {
            SYSTEM_STATS_DECREMENT(System::Stats::kPlatformMgr_NumEvents);
            Impl()->DispatchEvent(&event);
            eventReceived = k_msgq_get(&mChipEventQueue, &event, K_NO_WAIT) == 0;
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
