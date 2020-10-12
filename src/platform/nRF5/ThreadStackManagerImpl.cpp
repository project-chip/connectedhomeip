/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          nRF52 platforms using the Nordic nRF5 SDK and the OpenThread
 *          stack.
 *
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.cpp>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.cpp>

#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>

#include "boards.h"
#include "nrf_log.h"

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

    // Initialize the generic implementation base classes.
    err = GenericThreadStackManagerImpl_FreeRTOS<ThreadStackManagerImpl>::DoInit();
    SuccessOrExit(err);
    err = GenericThreadStackManagerImpl_OpenThread_LwIP<ThreadStackManagerImpl>::DoInit(otInst);
    SuccessOrExit(err);

exit:
    return err;
}

bool ThreadStackManagerImpl::IsInitialized()
{
    return sInstance.mThreadStackLock != NULL;
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStart(void)
{
    // If Thread-over-BLE is enabled, ensure that ToBLE advertising is stopped before
    // starting CHIPoBLE advertising.  This is accomplished by disabling the OpenThread
    // IPv6 interface via a call to otIp6SetEnabled(false).
    //
    // On platforms where there is no native support for simultaneous BLE advertising
    // (e.g. Nordic’s SoftDevice), it is necessary to coordinate between the different
    // advertising modes a CHIP device may employ.  This arises in particular when a
    // device supports both CHIPoBLE and ToBLE, each of which requires a separate advertising
    // regime.  The OnCHIPoBLEAdvertisingStart()/OnCHIPoBLEAdvertisingStop() methods handle
    // the switching between the two modes.
    //
#if OPENTHREAD_CONFIG_ENABLE_TOBLE
    LockThreadStack();
    otIp6SetEnabled(OTInstance(), false);
    UnlockThreadStack();
#endif
}

void ThreadStackManagerImpl::_OnCHIPoBLEAdvertisingStop(void)
{
    // If Thread-over-BLE is enabled, and a Thread provision exists, ensure that ToBLE
    // advertising is re-activated once CHIPoBLE advertising stops.
    //
#if OPENTHREAD_CONFIG_ENABLE_TOBLE
    LockThreadStack();
    if (otThreadGetDeviceRole(OTInstance()) != OT_DEVICE_ROLE_DISABLED && otDatasetIsCommissioned(OTInstance()))
    {
        otIp6SetEnabled(OTInstance(), true);
    }
    UnlockThreadStack();
#endif
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
