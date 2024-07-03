/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    All rights reserved.
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
 *          GenericThreadStackManagerImpl_FreeRTOS<> template.
 */

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include "../ThreadStackManagerImpl_CMSIS.h"
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
CHIP_ERROR ThreadStackManagerImpl_CMSIS<ImplClass>::DoInit(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    return err;
}

template <class ImplClass>
CHIP_ERROR ThreadStackManagerImpl_CMSIS<ImplClass>::_StartThreadTask(void)
{
    return CHIP_NO_ERROR;
}

template <class ImplClass>
void ThreadStackManagerImpl_CMSIS<ImplClass>::_LockThreadStack(void)
{
}

template <class ImplClass>
bool ThreadStackManagerImpl_CMSIS<ImplClass>::_TryLockThreadStack(void)
{
    return true;
}

template <class ImplClass>
void ThreadStackManagerImpl_CMSIS<ImplClass>::_UnlockThreadStack(void)
{
}

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
template <class ImplClass>
void ThreadStackManagerImpl_CMSIS<ImplClass>::_WaitOnSrpClearAllComplete()
{
    // Only 1 task can be blocked on a srpClearAll request
    // if (mSrpClearAllRequester == nullptr)
    // {
    //     //mSrpClearAllRequester = xTaskGetCurrentTaskHandle();
    //     // Wait on OnSrpClientNotification which confirms the slearing is done.
    //     // It will notify this current task with NotifySrpClearAllComplete.
    //     // However, we won't wait more than 2s.
    //     ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2000));
    //     mSrpClearAllRequester = nullptr;
    // }
}

template <class ImplClass>
void ThreadStackManagerImpl_CMSIS<ImplClass>::_NotifySrpClearAllComplete()
{
    // if (mSrpClearAllRequester)
    // {
    //     xTaskNotifyGive(mSrpClearAllRequester);
    // }
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

template <class ImplClass>
void ThreadStackManagerImpl_CMSIS<ImplClass>::SignalThreadActivityPending()
{
    // if (mThreadTask != NULL)
    // {
    //     xTaskNotifyGive(mThreadTask);
    // }
}

template <class ImplClass>
BaseType_t ThreadStackManagerImpl_CMSIS<ImplClass>::SignalThreadActivityPendingFromISR()
{
    BaseType_t yieldRequired = pdFALSE;

    // if (mThreadTask != NULL)
    // {
    //     vTaskNotifyGiveFromISR(mThreadTask, &yieldRequired);
    // }

    return yieldRequired;
}

template <class ImplClass>
void ThreadStackManagerImpl_CMSIS<ImplClass>::ThreadTaskMain(void * arg)
{
    ThreadStackManagerImpl_CMSIS<ImplClass> * self =
        static_cast<ThreadStackManagerImpl_CMSIS<ImplClass> *>(arg);

    ChipLogDetail(DeviceLayer, "Thread task running");

    while (true)
    {
        self->Impl()->LockThreadStack();
        self->Impl()->ProcessThreadActivity();
        self->Impl()->UnlockThreadStack();

        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
}

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
// NB: This must come after all templated class members are defined.
template class ThreadStackManagerImpl_CMSIS<ThreadStackManagerImpl>;

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
