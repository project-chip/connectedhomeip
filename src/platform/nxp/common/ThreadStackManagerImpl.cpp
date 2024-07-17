/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *          Provides an implementation of the ThreadStackManager object for
 *          NXP platforms using the NXP SDK and the OpenThread stack.
 *
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.hpp>
#if CHIP_SYSTEM_CONFIG_USE_LWIP
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp>
#else
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp>
#endif

#include <lib/support/CHIPPlatformMemory.h>

#include "openthread-system.h"
#include "ot_platform_common.h"

/*
 * Empty content for otPlatCliUartProcess, as the openthread CLI
 * is managed by the matter cli if enabled.
 * An empty content is required as system.c from ot_nxp is always
 * calling this function. This could be removed once system.c will
 * be reworked to support more dynamically the openthread cli.
 */
extern "C" void otPlatCliUartProcess(void) {}

namespace chip {
namespace DeviceLayer {

using namespace ::chip::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

CHIP_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_INIT_OT_PLAT_ALARM
    /* Initialize the OpenThread Alarm module to make sure that if calling otInstance,
     * it can schedule events
     */
    otPlatAlarmInit();
#endif

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
#if CHIP_SYSTEM_CONFIG_USE_LWIP
    err = GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>::DoInit(NULL);
#else
    err = GenericThreadStackManagerImpl_OpenThread<ThreadStackManagerImpl>::DoInit(NULL);
#endif
    SuccessOrExit(err);

exit:
    return err;
}

#if CHIP_DEVICE_CONFIG_PROCESS_BLE_IN_THREAD
void ThreadStackManagerImpl::ProcessThreadActivity()
{
    /* reuse thread task for ble processing.
     * by doing this, we avoid allocating a new stack for short-lived
     * BLE processing (e.g.: only during Matter commissioning)
     */
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    auto * bleManager = &chip::DeviceLayer::Internal::BLEMgrImpl();
    bleManager->DoBleProcessing();
#endif

    otTaskletsProcess(OTInstance());
    otSysProcessDrivers(OTInstance());
}
#endif

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
    return CHIPPlatformMemoryFree(aPtr);
}

extern "C" void * otPlatRealloc(void * p, size_t aSize)
{
    return CHIPPlatformMemoryRealloc(p, aSize);
}
