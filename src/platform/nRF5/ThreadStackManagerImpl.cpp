/*
 *
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
 *          Provides an implementation of the ThreadStackManager object for
 *          nRF52 platforms using the Nordic nRF5 SDK and the OpenThread
 *          stack.
 *
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/ThreadStackManager.h>
#include <Weave/DeviceLayer/OpenThread/OpenThreadUtils.h>

#include <Weave/DeviceLayer/FreeRTOS/GenericThreadStackManagerImpl_FreeRTOS.ipp>
#include <Weave/DeviceLayer/OpenThread/GenericThreadStackManagerImpl_OpenThread_LwIP.ipp>

#include "boards.h"
#include "nrf_log.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {

using namespace ::nl::Weave::DeviceLayer::Internal;

ThreadStackManagerImpl ThreadStackManagerImpl::sInstance;

WEAVE_ERROR ThreadStackManagerImpl::_InitThreadStack(void)
{
    return InitThreadStack(NULL);
}

WEAVE_ERROR ThreadStackManagerImpl::InitThreadStack(otInstance * otInst)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

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

void ThreadStackManagerImpl::_OnWoBLEAdvertisingStart(void)
{
    // If Thread-over-BLE is enabled, ensure that ToBLE advertising is stopped before
    // starting WoBLE advertising.  This is accomplished by disabling the OpenThread
    // IPv6 interface via a call to otIp6SetEnabled(false).
    //
    // On platforms where there is no native support for simultaneous BLE advertising
    // (e.g. Nordic’s SoftDevice), it is necessary to coordinate between the different
    // advertising modes a Weave device may employ.  This arises in particular when a
    // device supports both WoBLE and ToBLE, each of which requires a separate advertising
    // regime.  The OnWoBLEAdvertisingStart()/OnWoBLEAdvertisingStop() methods handle
    // the switching between the two modes.
    //
#if OPENTHREAD_CONFIG_ENABLE_TOBLE
    LockThreadStack();
    otIp6SetEnabled(OTInstance(), false);
    UnlockThreadStack();
#endif
}

void ThreadStackManagerImpl::_OnWoBLEAdvertisingStop(void)
{
    // If Thread-over-BLE is enabled, and a Thread provision exists, ensure that ToBLE
    // advertising is re-activated once WoBLE advertising stops.
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
} // namespace Weave
} // namespace nl

using namespace ::nl::Weave::DeviceLayer;

/**
 * Glue function called directly by the OpenThread stack when tasklet processing work
 * is pending.
 */
extern "C"
void otTaskletsSignalPending(otInstance * p_instance)
{
    ThreadStackMgrImpl().SignalThreadActivityPending();
}

/**
 * Glue function called directly by the OpenThread stack when system event processing work
 * is pending.
 */
extern "C"
void otSysEventSignalPending(void)
{
    BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
    portYIELD_FROM_ISR(yieldRequired);
}

