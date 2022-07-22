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
 *          Qorvo QPG platform using the Qorvo QPG library and the OpenThread
 *          stack.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.cpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.cpp>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

#include <openthread/heap.h>
// Qorvo OpenThread functions
extern "C" {
#include "alarm_qorvo.h"
#include "radio_qorvo.h"
#include "random_qorvo.h"
}

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

CHIP_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize Low level QPG OpenThread glue
    qorvoAlarmInit();
    qorvoRandomInit();
    qorvoRadioInit();

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(otInst);
    SuccessOrExit(err);

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
    BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
    portYIELD_FROM_ISR(yieldRequired);
}

extern "C" void * otPlatCAlloc(size_t aNum, size_t aSize)
{
    return CHIPPlatformMemoryCalloc(aNum, aSize);
}

extern "C" void otPlatFree(void * aPtr)
{
    CHIPPlatformMemoryFree(aPtr);
}
