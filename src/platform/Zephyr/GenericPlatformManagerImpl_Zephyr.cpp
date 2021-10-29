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

#include <platform/PlatformManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/Zephyr/GenericPlatformManagerImpl_Zephyr.h>

#include <system/SystemError.h>
#include <system/SystemLayer.h>

#define DEFAULT_MIN_SLEEP_PERIOD (60 * 60 * 24 * 30) // Month [sec]

namespace chip {
namespace DeviceLayer {
namespace Internal {

CHIP_ERROR GenericPlatformManagerImpl_Zephyr::InitChipStackInner(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    k_mutex_init(&mChipStackLock);

    k_msgq_init(&mChipEventQueue, reinterpret_cast<char *>(&mChipEventRingBuffer), sizeof(ChipDeviceEvent),
                CHIP_DEVICE_CONFIG_MAX_EVENT_QUEUE_SIZE);

    mShouldRunEventLoop = false;

    // Call up to the base class _InitChipStack() to perform the bulk of the initialization.
    err = GenericPlatformManagerImpl::InitChipStackInner();
    SuccessOrExit(err);

exit:
    return err;
}

void GenericPlatformManagerImpl_Zephyr::LockChipStack(void)
{
    k_mutex_lock(&mChipStackLock, K_FOREVER);
}

bool GenericPlatformManagerImpl_Zephyr::TryLockChipStack(void)
{
    return k_mutex_lock(&mChipStackLock, K_NO_WAIT) == 0;
}

void GenericPlatformManagerImpl_Zephyr::UnlockChipStack(void)
{
    k_mutex_unlock(&mChipStackLock);
}

CHIP_ERROR GenericPlatformManagerImpl_Zephyr::StartChipTimer(System::Clock::Timeout delay)
{
    // Let Systemlayer.PrepareEvents() handle timers.
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericPlatformManagerImpl_Zephyr::StopEventLoopTask(void)
{
    mShouldRunEventLoop = false;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericPlatformManagerImpl_Zephyr::ShutdownInner(void)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR GenericPlatformManagerImpl_Zephyr::PostEvent(const ChipDeviceEvent * event)
{
    // For some reasons mentioned in https://github.com/zephyrproject-rtos/zephyr/issues/22301
    // k_msgq_put takes `void*` instead of `const void*`. Nonetheless, it should be safe to
    // const_cast here and there are components in Zephyr itself which do the same.
    int status = k_msgq_put(&mChipEventQueue, const_cast<ChipDeviceEvent *>(event), K_NO_WAIT);
    if (status != 0)
    {
        ChipLogError(DeviceLayer, "Failed to post event to CHIP Platform event queue");
        return System::MapErrorZephyr(status);
    }
    SystemLayerSocketsLoop().Signal(); // Trigger wake on CHIP thread
    return CHIP_NO_ERROR;
}

void GenericPlatformManagerImpl_Zephyr::ProcessDeviceEvents()
{
    ChipDeviceEvent event;

    while (k_msgq_get(&mChipEventQueue, &event, K_NO_WAIT) == 0)
        DispatchEvent(&event);
}

void GenericPlatformManagerImpl_Zephyr::RunEventLoop(void)
{
    LockChipStack();

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

        UnlockChipStack();
        SystemLayerSocketsLoop().WaitForEvents();
        LockChipStack();

        SystemLayerSocketsLoop().HandleEvents();

        ProcessDeviceEvents();
    }
    SystemLayerSocketsLoop().EventLoopEnds();

    UnlockChipStack();
}

void GenericPlatformManagerImpl_Zephyr::EventLoopTaskMain(void * thisPtr, void *, void *)
{
    ChipLogDetail(DeviceLayer, "CHIP task running");
    static_cast<GenericPlatformManagerImpl_Zephyr *>(thisPtr)->RunEventLoop();
}

CHIP_ERROR GenericPlatformManagerImpl_Zephyr::StartEventLoopTask(void)
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
