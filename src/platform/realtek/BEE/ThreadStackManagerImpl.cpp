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
 *          Provides an implementation of the ThreadStackManager object for the
 *          Realtek Bee platform using the Realtek Bee library and the OpenThread
 *          stack.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.hpp>
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp>
#else
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#endif
#include "os_task.h"
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <mem_config.h>
#include <openthread/heap.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platforms/openthread-system.h>

#if DLPS_EN
#ifdef __cplusplus
extern "C" {
#endif

void BEE_RadioExternalWakeup(void);

#ifdef __cplusplus
}
#endif
#endif

extern void otSysInit(int argc, char * argv[]);

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

void ThreadStackManagerImpl::_LockThreadStack(void)
{
    xSemaphoreTake(sInstance.mThreadStackLock, portMAX_DELAY);
#if DLPS_EN
    // Wake up the radio before accessing the Thread stack to ensure it's
    // responsive, as part of the Deep Low Power State (DLPS) management.
    BEE_RadioExternalWakeup();
#endif
}

bool ThreadStackManagerImpl::_TryLockThreadStack(void)
{
    return xSemaphoreTake(sInstance.mThreadStackLock, 0) == pdTRUE;
}

void ThreadStackManagerImpl::_UnlockThreadStack(void)
{
    xSemaphoreGive(sInstance.mThreadStackLock);
}

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mThreadTask = NULL;

    ChipLogProgress(DeviceLayer, "ThreadStackManagerImpl::InitThreadStack");
    // Initialize the OpenThread platform layer
    otSysInit(0, NULL);

    // Initialize the generic implementation base classes.
    ChipLogProgress(DeviceLayer, "GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit");
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    ChipLogProgress(DeviceLayer, "GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>::DoInit");
    err = GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>::DoInit(otInst);
#else
    ChipLogProgress(DeviceLayer, "GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit");
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(otInst);
#endif

exit:
    return err;
}

#if USE_FREERTOS_NATIVE_API
void ThreadStackManagerImpl::SignalThreadActivityPending()
{
    if (mThreadTask != NULL)
    {
        xTaskNotifyGive(mThreadTask);
    }
}

BaseType_t ThreadStackManagerImpl::SignalThreadActivityPendingFromISR()
{
    BaseType_t yieldRequired = pdFALSE;

    if (mThreadTask != NULL)
    {
        vTaskNotifyGiveFromISR(mThreadTask, &yieldRequired);
    }

    return yieldRequired;
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    if (mThreadTask != NULL)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    xTaskCreate(ThreadTaskMain, CHIP_DEVICE_CONFIG_THREAD_TASK_NAME,
                CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE / sizeof(StackType_t), this, CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY,
                &mThreadTask);

    if (mThreadTask == NULL)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return CHIP_NO_ERROR;
}

void ThreadStackManagerImpl::ExecuteThreadTask(void)
{
#if defined(FEATURE_TRUSTZONE_ENABLE) && (FEATURE_TRUSTZONE_ENABLE == 1)
    os_alloc_secure_ctx(1024);
#endif

    while (true)
    {
        LockThreadStack();
        ProcessThreadActivity();
        UnlockThreadStack();

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

#else // USE_FREERTOS_NATIVE_API

void ThreadStackManagerImpl::SignalThreadActivityPending()
{
    if (mThreadTask != NULL)
    {
        os_task_notify_give(mThreadTask);
    }
}

void ThreadStackManagerImpl::SignalThreadActivityPendingFromISR()
{
    if (mThreadTask != NULL)
    {
        os_task_notify_give(mThreadTask);
    }
}

CHIP_ERROR ThreadStackManagerImpl::_StartThreadTask()
{
    if (os_task_create(&mThreadTask, CHIP_DEVICE_CONFIG_THREAD_TASK_NAME, ThreadTaskMain, this,
                       CHIP_DEVICE_CONFIG_THREAD_TASK_STACK_SIZE, CHIP_DEVICE_CONFIG_THREAD_TASK_PRIORITY))
    {
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_NO_MEMORY;
}

void ThreadStackManagerImpl::ExecuteThreadTask(void)
{
#if defined(FEATURE_TRUSTZONE_ENABLE) && (FEATURE_TRUSTZONE_ENABLE == 1)
    os_alloc_secure_ctx(1024);
#endif
    uint32_t notify;
    while (true)
    {
        LockThreadStack();
        ProcessThreadActivity();
        UnlockThreadStack();

        os_task_notify_take(1, 0xffffffff, &notify);
    }
}
#endif // USE_FREERTOS_NATIVE_API

void ThreadStackManagerImpl::GetExtAddress(otExtAddress & aExtAddr)
{
    const otExtAddress * extAddr;
    LockThreadStack();
    extAddr = otLinkGetExtendedAddress(OTInstance());
    UnlockThreadStack();

    memcpy(aExtAddr.m8, extAddr->m8, OT_EXT_ADDRESS_SIZE);
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return sInstance.mThreadStackLock != NULL;
}

void ThreadStackManagerImpl::ThreadTaskMain(void * arg)
{
    reinterpret_cast<ThreadStackManagerImpl *>(arg)->ExecuteThreadTask();
}

} // namespace DeviceLayer
} // namespace chip

using namespace ::chip::DeviceLayer;

/**
 * Glue function called directly by the OpenThread stack when tasklet processing work
 * is pending.
 */
extern "C" void otTaskletsSignalPending(otInstance * p_instance)
{
    ThreadStackMgrImpl().SignalThreadActivityPending();
}

/**
 * Glue function called directly by the OpenThread stack when system event processing work
 * is pending.
 */
extern "C" void otSysEventSignalPending(void)
{
#if USE_FREERTOS_NATIVE_API
    BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
    portYIELD_FROM_ISR(yieldRequired);
#else
    ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
#endif
}

extern "C" void * otPlatCAlloc(size_t aNum, size_t aSize)
{
    return CHIPPlatformMemoryCalloc(aNum, aSize);
}

extern "C" void otPlatFree(void * aPtr)
{
    CHIPPlatformMemoryFree(aPtr);
}
