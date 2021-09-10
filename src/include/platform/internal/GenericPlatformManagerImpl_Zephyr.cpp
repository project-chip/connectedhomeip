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
#include <platform/internal/GenericPlatformManagerImpl.cpp>

#include <system/SystemLayer.h>

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericPlatformManagerImpl_Zephyr<PlatformManagerImpl>;

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_InitChipStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    k_mutex_init(&mChipStackLock);

    k_msgq_init(&mChipEventQueue, reinterpret_cast<char *>(&mChipEventRingBuffer), sizeof(ChipDeviceEvent),
                CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl<ImplClass>::_InitChipStack();
    SuccessOrExit(err);

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
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StartChipTimer(uint32_t aMilliseconds)
{
    // Let Systemlayer.PrepareEvents() handle timers.
    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StopEventLoopTask(void)
{
    VerifyOrDieWithMsg(false, DeviceLayer, "StopEventLoopTask is not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_Shutdown(void)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::_PostEvent(const ChipDeviceEvent * event)
{
    // For some reasons mentioned in https://github.com/zephyrproject-rtos/zephyr/issues/22301
    // k_msgq_put takes `void*` instead of `const void*`. Nonetheless, it should be safe to
    // const_cast here and there are components in Zephyr itself which do the same.
    if (k_msgq_put(&mChipEventQueue, const_cast<ChipDeviceEvent *>(event), K_NO_WAIT) == 0)
        SystemLayer.Signal(); // Trigger wake on CHIP thread
    else
        ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
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

    SystemLayer.EventLoopBegins();
    while (true)
    {
        SystemLayer.PrepareEvents();

        Impl()->UnlockChipStack();
        SystemLayer.WaitForEvents();
        Impl()->LockChipStack();

        SystemLayer.HandleEvents();

        ProcessDeviceEvents();
    }
    SystemLayer.EventLoopEnds();

    Impl()->UnlockChipStack();
}

template <class ImplClass>
void GenericPlatformManagerImpl_Zephyr<ImplClass>::EventLoopTaskMain(void * thisPtr, void *, void *)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_Zephyr<ImplClass> *>(thisPtr)->Impl()->RunEventLoop();
}

template <class ImplClass>
CHIP_ERROR GenericPlatformManagerImpl_Zephyr<ImplClass>::_StartEventLoopTask(void)
{
    const auto tid = k_thread_create(&mChipThread, mChipThreadStack, K_THREAD_STACK_SIZEOF(mChipThreadStack), EventLoopTaskMain,
                                     this, nullptr, nullptr, CHIP_DEVICE_CONFIG_CHIP_TASK_PRIORITY, 0, K_NO_WAIT);

#ifdef CONFIG_THREAD_NAME
    k_thread_name_set(tid, CHIP_DEVICE_CONFIG_CHIP_TASK_NAME);
#else
    IgnoreUnusedVariable(tid);
#endif

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_PLATFORM_MANAGER_IMPL_ZEPHYR_CPP
