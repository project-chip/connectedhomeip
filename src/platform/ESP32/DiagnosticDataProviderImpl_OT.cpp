/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMemString.h>
#include <openthread/ip6.h>
#include <platform/ESP32/DiagnosticDataProviderImpl.h>
#include <platform/ESP32/ThreadStackManagerImpl.h>
#include <platform/ThreadStackManager.h>

#include <cstring>

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * interface = new NetworkInterface();
    VerifyOrReturnError(interface != nullptr, CHIP_ERROR_NO_MEMORY);

    Platform::CopyString(interface->Name, "ot");
    interface->name = CharSpan::fromCharString(interface->Name);
    interface->type = InterfaceTypeEnum::kThread;
    interface->offPremiseServicesReachableIPv4.SetNull();
    interface->offPremiseServicesReachableIPv6.SetNull();

    static_assert(OT_EXT_ADDRESS_SIZE <= sizeof(interface->MacAddress), "Unexpected extended address size");
    if (ThreadStackMgr().GetPrimary802154MACAddress(interface->MacAddress) == CHIP_NO_ERROR)
    {
        interface->hardwareAddress = ByteSpan(interface->MacAddress, OT_EXT_ADDRESS_SIZE);
    }

    uint8_t ipv6_addr_count = 0;

    ThreadStackMgr().LockThreadStack();
    otInstance * instance = ThreadStackMgrImpl().OTInstance();
    bool is_interface_up  = instance != nullptr && otIp6IsEnabled(instance);
    if (is_interface_up)
    {
        for (const otNetifAddress * address                                     = otIp6GetUnicastAddresses(instance);
             address != nullptr && ipv6_addr_count < kMaxIPv6AddrCount; address = address->mNext)
        {
            if (!address->mValid)
            {
                continue;
            }

            memcpy(interface->Ipv6AddressesBuffer[ipv6_addr_count], address->mAddress.mFields.m8, kMaxIPv6AddrSize);
            interface->Ipv6AddressSpans[ipv6_addr_count] =
                ByteSpan(interface->Ipv6AddressesBuffer[ipv6_addr_count], kMaxIPv6AddrSize);
            ++ipv6_addr_count;
        }
    }
    ThreadStackMgr().UnlockThreadStack();

    interface->isOperational = is_interface_up;
    interface->IPv6Addresses = app::DataModel::List<ByteSpan>(interface->Ipv6AddressSpans, ipv6_addr_count);

    *netifpp = interface;
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
