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
 *          GenericPlatformManagerImpl_Zephyr<> template.
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

#include <zephyr/sys/reboot.h>

#ifdef CONFIG_CHIP_CRYPTO_PSA
#include <psa/crypto.h>
#endif

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

System::LayerSocketsLoop & SystemLayerSocketsLoop()
{
    return static_cast<System::LayerSocketsLoop &>(DeviceLayer::SystemLayer());
}

K_WORK_DEFINE(sSignalWork, [](k_work *) { SystemLayerSocketsLoop().Signal(); });

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
    // Let Systemlayer.PrepareEvents() handle timers.
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StopEventLoopTask(void)
{
    mShouldRunEventLoop = false;
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

    // Wake CHIP thread to process the event. If the function is called from ISR, such as a Zephyr
    // timer handler, do not signal the thread directly because that involves taking a mutex, which
    // is forbidden in ISRs. Instead, submit a task to the system work queue to do the singalling.
    if (k_is_in_isr())
    {
        (void) k_work_submit(&sSignalWork);
    }
    else
    {
        SystemLayerSocketsLoop().Signal();
    }
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::ProcessDeviceEvents()
{
    ChipDeviceEvent event;

    while (k_msgq_get(&mChipEventQueue, &event, K_NO_WAIT) == 0)
        Impl()->DispatchEvent(&event);
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

    SystemLayerSocketsLoop().EventLoopBegins();
    while (mShouldRunEventLoop)
    {
        SystemLayerSocketsLoop().PrepareEvents();

        Impl()->UnlockChipStack();
        SystemLayerSocketsLoop().WaitForEvents();
        Impl()->LockChipStack();

        SystemLayerSocketsLoop().HandleEvents();

        ProcessDeviceEvents();
    }
    SystemLayerSocketsLoop().EventLoopEnds();

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
        return CHIP_ERROR_WELL_UNINITIALIZED;

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
