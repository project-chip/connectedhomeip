/*
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.hpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>

#include <lib/support/CHIPPlatformMemory.h>

#include <openthread/platform/entropy.h>

#include <mbedtls/platform.h>

#include <openthread_port.h>
#include <utils_list.h>

extern "C" {
#include <ot_utils_ext.h>
}

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;
extern "C" void (*ot_otrNotifyEvent_ptr)(ot_system_event_t sevent);

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(otInst);
    SuccessOrExit(err);

    mbedtls_platform_set_calloc_free(pvPortCalloc, vPortFree);

exit:
    return err;
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return sInstance.mThreadStackLock != NULL;
}

} // namespace DeviceLayer
} // namespace chip

using namespace ::chip::DeviceLayer;

ot_system_event_t ot_system_event_var = OT_SYSTEM_EVENT_NONE;

void otSysProcessDrivers(otInstance * aInstance)
{
    ot_system_event_t sevent = otrGetNotifyEvent();

    ot_alarmTask(sevent);
    ot_radioTask(sevent);
}

extern "C" void otTaskletsSignalPending(otInstance * p_instance)
{
    otSysEventSignalPending();
}

extern "C" void otSysEventSignalPending(void)
{
    if (xPortIsInsideInterrupt())
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
    else
    {
        ThreadStackMgrImpl().SignalThreadActivityPending();
    }
}

extern "C" otInstance * otrGetInstance()
{
    return ThreadStackMgrImpl().OTInstance();
}

extern "C" ot_system_event_t otrGetNotifyEvent(void)
{
    ot_system_event_t sevent = OT_SYSTEM_EVENT_NONE;

    taskENTER_CRITICAL();
    sevent              = ot_system_event_var;
    ot_system_event_var = OT_SYSTEM_EVENT_NONE;
    taskEXIT_CRITICAL();

    return sevent;
}

extern "C" void otrNotifyEvent(ot_system_event_t sevent)
{
    uint32_t tag        = otrEnterCrit();
    ot_system_event_var = (ot_system_event_t) (ot_system_event_var | sevent);
    otrExitCrit(tag);

    otSysEventSignalPending();
}
