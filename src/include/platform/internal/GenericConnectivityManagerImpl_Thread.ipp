/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          Provides an generic implementation of ConnectivityManager features
 *          for use on platforms that support Thread.
 */

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_CPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_CPP

#include <lib/support/CodeUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

template <class ImplClass>
void GenericConnectivityManagerImpl_Thread<ImplClass>::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Define some short-hands for various interesting event conditions.
    const bool threadConnChanged = (event->Type == DeviceEventType::kThreadConnectivityChange &&
                                    event->ThreadConnectivityChange.Result != kConnectivity_NoChange);
    const bool threadAddrChanged = (event->Type == DeviceEventType::kThreadStateChange && event->ThreadStateChange.AddressChanged);
    const bool threadNetDataChanged =
        (event->Type == DeviceEventType::kThreadStateChange && event->ThreadStateChange.NetDataChanged);

    if (threadConnChanged && event->ThreadConnectivityChange.Result == kConnectivity_Established)
    {
        ThreadStackMgrImpl().OnThreadAttachFinished();
    }

    // If any of the above events has occurred, assess whether there's been a change in
    // service connectivity via Thread.
    if (threadConnChanged || threadAddrChanged || threadNetDataChanged)
    {
        UpdateServiceConnectivity();
    }
}

template <class ImplClass>
ConnectivityManager::ThreadMode GenericConnectivityManagerImpl_Thread<ImplClass>::_GetThreadMode()
{
    if (mFlags.Has(Flags::kIsApplicationControlled))
    {
        return ConnectivityManager::kThreadMode_ApplicationControlled;
    }

    return ThreadStackMgrImpl().IsThreadEnabled() ? ConnectivityManager::kThreadMode_Enabled
                                                  : ConnectivityManager::kThreadMode_Disabled;
}

template <class ImplClass>
CHIP_ERROR GenericConnectivityManagerImpl_Thread<ImplClass>::_SetThreadMode(ConnectivityManager::ThreadMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val == ConnectivityManager::kThreadMode_Enabled || val == ConnectivityManager::kThreadMode_Disabled ||
                     val == ConnectivityManager::kThreadMode_ApplicationControlled,
                 err = CHIP_ERROR_INVALID_ARGUMENT);

    if (val == ConnectivityManager::kThreadMode_ApplicationControlled)
    {
        mFlags.Set(Flags::kIsApplicationControlled);
    }
    else
    {
        mFlags.Clear(Flags::kIsApplicationControlled);

        err = ThreadStackMgrImpl().SetThreadEnabled(val == ConnectivityManager::kThreadMode_Enabled);
        SuccessOrExit(err);
    }

exit:
    return err;
}

template <class ImplClass>
void GenericConnectivityManagerImpl_Thread<ImplClass>::UpdateServiceConnectivity()
{
    constexpr bool haveServiceConnectivity = false;

    // If service connectivity via Thread has changed, post an event signaling the change.
    if (mFlags.Has(Flags::kHaveServiceConnectivity) != haveServiceConnectivity)
    {
        ChipLogProgress(DeviceLayer, "ConnectivityManager: Service connectivity via Thread %s",
                        (haveServiceConnectivity) ? "ESTABLISHED" : "LOST");

        mFlags.Set(Flags::kHaveServiceConnectivity, haveServiceConnectivity);

        {
            ChipDeviceEvent event;
            event.Clear();
            event.Type = DeviceEventType::kServiceConnectivityChange;
            event.ServiceConnectivityChange.ViaThread.Result =
                (haveServiceConnectivity) ? kConnectivity_Established : kConnectivity_Lost;
            event.ServiceConnectivityChange.Overall.Result = event.ServiceConnectivityChange.ViaThread.Result;
            CHIP_ERROR status                              = PlatformMgr().PostEvent(&event);
            if (status != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to post thread connectivity change: %" CHIP_ERROR_FORMAT, status.Format());
            }
        }
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_CPP
