/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <inet/UDPEndPointImplSockets.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/internal/BLEManager.h>
#include <platform/Zephyr/InetUtils.h>

#include <platform/internal/GenericConnectivityManagerImpl_UDP.ipp>

#if INET_CONFIG_ENABLE_TCP_ENDPOINT
#include <platform/internal/GenericConnectivityManagerImpl_TCP.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <platform/internal/GenericConnectivityManagerImpl_BLE.ipp>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/internal/GenericConnectivityManagerImpl_Thread.ipp>
#include <platform/OpenThread/OpenThreadUtils.h>
#endif

using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

namespace {
CHIP_ERROR JoinLeaveMulticastGroup(net_if * iface, const Inet::IPAddress & address, bool join)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (net_if_l2(iface) == &NET_L2_GET_NAME(OPENTHREAD))
    {
        const otIp6Address otAddress = ToOpenThreadIP6Address(address);
        const auto handler           = join ? otIp6SubscribeMulticastAddress : otIp6UnsubscribeMulticastAddress;
        otError error;

        ThreadStackMgr().LockThreadStack();
        error = handler(openthread_get_default_instance(), &otAddress);
        ThreadStackMgr().UnlockThreadStack();

        return MapOpenThreadError(error);
    }
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    // The following code should also be valid for other interface types, such as Ethernet,
    // but they are not officially supported, so for now enable it for Wi-Fi only.
    const in6_addr in6Addr = InetUtils::ToZephyrAddr(address);

    if (join)
    {
        net_if_mcast_addr * maddr = net_if_ipv6_maddr_add(iface, &in6Addr);

        if (maddr && !net_if_ipv6_maddr_is_joined(maddr))
        {
            net_if_ipv6_maddr_join(iface, maddr);
        }
    }
    else
    {
        VerifyOrReturnError(net_if_ipv6_maddr_rm(iface, &in6Addr), CHIP_ERROR_INVALID_ADDRESS);
    }
#endif

    return CHIP_NO_ERROR;
}
} // namespace

ConnectivityManagerImpl ConnectivityManagerImpl::sInstance;

CHIP_ERROR ConnectivityManagerImpl::_Init()
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_Init();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ReturnErrorOnFailure(InitWiFi());
#endif

    UDPEndPointImplSockets::SetMulticastGroupHandler([](InterfaceId interfaceId, const IPAddress & address, bool join) {
        if (interfaceId.IsPresent())
        {
            net_if * iface = InetUtils::GetInterface(interfaceId);
            VerifyOrReturnError(iface != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

            return JoinLeaveMulticastGroup(iface, address, join);
        }

        // If the interface is not specified, join or leave the multicast group on all interfaces.
        for (int i = 1; net_if * iface = net_if_get_by_index(i); i++)
        {
            ReturnErrorOnFailure(JoinLeaveMulticastGroup(iface, address, join));
        }

        return CHIP_NO_ERROR;
    });

    return CHIP_NO_ERROR;
}

void ConnectivityManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    // Forward the event to the generic base classes as needed.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    GenericConnectivityManagerImpl_Thread<ConnectivityManagerImpl>::_OnPlatformEvent(event);
#endif
}

} // namespace DeviceLayer
} // namespace chip
