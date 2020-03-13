/*
 *
 *    <COPYRIGHT>
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

#ifndef GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_IPP
#define GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_IPP

#include <platform/internal/ChipDeviceLayerInternal.h>
#include <platform/internal/GenericConnectivityManagerImpl_Thread.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>;

template<class ImplClass>
void GenericConnectivityManagerImpl_Thread<ImplClass>::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Define some short-hands for various interesting event conditions.
    const bool threadConnChanged       = (event->Type == DeviceEventType::kThreadConnectivityChange &&
                                          event->ThreadConnectivityChange.Result != kConnectivity_NoChange);
    const bool threadAddrChanged       = (event->Type == DeviceEventType::kThreadStateChange &&
                                          event->ThreadStateChange.AddressChanged);
    const bool threadNetDataChanged    = (event->Type == DeviceEventType::kThreadStateChange &&
                                          event->ThreadStateChange.NetDataChanged);
    const bool fabricMembershipChanged = (event->Type == DeviceEventType::kFabricMembershipChange);

    // If the state of the Thread interface has changed, notify WARM accordingly.
    if (threadConnChanged)
    {
        Warm::ThreadInterfaceStateChange(event->ThreadConnectivityChange.Result == kConnectivity_Established
                ? Warm::kInterfaceStateUp : Warm::kInterfaceStateDown);
    }

    // If any of the above events has occurred, assess whether there's been a change in
    // service connectivity via Thread.
    if (threadConnChanged || threadAddrChanged || threadNetDataChanged || fabricMembershipChanged)
    {
        UpdateServiceConnectivity();
    }
}

template<class ImplClass>
ConnectivityManager::ThreadMode GenericConnectivityManagerImpl_Thread<ImplClass>::_GetThreadMode(void)
{
    if (GetFlag(mFlags, kFlag_IsApplicationControlled))
    {
        return ConnectivityManager::kThreadMode_ApplicationControlled;
    }

    return ThreadStackMgrImpl().IsThreadEnabled()
              ? ConnectivityManager::kThreadMode_Enabled
              : ConnectivityManager::kThreadMode_Disabled;
}

template<class ImplClass>
CHIP_ERROR GenericConnectivityManagerImpl_Thread<ImplClass>::_SetThreadMode(ConnectivityManager::ThreadMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val == ConnectivityManager::kThreadMode_Enabled ||
                 val == ConnectivityManager::kThreadMode_Disabled ||
                 val == ConnectivityManager::kThreadMode_ApplicationControlled, err = CHIP_ERROR_INVALID_ARGUMENT);

    if (val == ConnectivityManager::kThreadMode_ApplicationControlled)
    {
        SetFlag(mFlags, kFlag_IsApplicationControlled);
    }
    else
    {
        ClearFlag(mFlags, kFlag_IsApplicationControlled);

        err = ThreadStackMgrImpl().SetThreadEnabled(val == ConnectivityManager::kThreadMode_Enabled);
        SuccessOrExit(err);
    }

exit:
    return err;
}

template<class ImplClass>
void GenericConnectivityManagerImpl_Thread<ImplClass>::UpdateServiceConnectivity(void)
{
    bool haveServiceConnectivity = false;

    // Evaluate whether there is connectivity to the chip service subnet via the Thread network.
    //
    // If the device is a member of a fabric, then service connectivity is assessed by checking if the
    // local Thread stack has a route to the chip service subnet.  This route will typically be a /48
    // chip ULA route that has been advertised by one or more chip border router devices in the Thread
    // network.  If no such route exists, then it is likely that there are no functioning chip border
    // routers, and thus no route to the service via Thread.
    //
    // If the device is NOT a member of a fabric, then there can be no chip service connectivity via Thread.
    //
    if (FabricState.FabricId != kFabricIdNotSpecified)
    {
        const uint64_t fabricGlobalId = ChipFabricIdToIPv6GlobalId(FabricState.FabricId);
        IPAddress serviceAddr = IPAddress::MakeULA(fabricGlobalId, nl::chip::kChipSubnetId_Service, 1);
        haveServiceConnectivity = ThreadStackMgr().HaveRouteToAddress(serviceAddr);
    }

    // If service connectivity via Thread has changed, post an event signaling the change.
    if (GetFlag(mFlags, kFlag_HaveServiceConnectivity) != haveServiceConnectivity)
    {
        ChipLogProgress(DeviceLayer, "ConnectivityManager: Service connectivity via Thread %s", (haveServiceConnectivity) ? "ESTABLISHED" : "LOST");

        SetFlag(mFlags, kFlag_HaveServiceConnectivity, haveServiceConnectivity);

        {
            ChipDeviceEvent event;
            event.Clear();
            event.Type = DeviceEventType::kServiceConnectivityChange;
            event.ServiceConnectivityChange.ViaThread.Result = (haveServiceConnectivity) ? kConnectivity_Established : kConnectivity_Lost;
            event.ServiceConnectivityChange.ViaTunnel.Result = kConnectivity_NoChange;
            event.ServiceConnectivityChange.Overall.Result = (Impl()->HaveServiceConnectivityViaTunnel())
                    ? kConnectivity_NoChange
                    : event.ServiceConnectivityChange.ViaThread.Result;
            PlatformMgr().PostEvent(&event);
        }
    }
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // GENERIC_CONNECTIVITY_MANAGER_IMPL_THREAD_IPP
