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
#include <platform/ESP32/DiagnosticDataProviderImpl.h>

#include <algorithm>
#include <cstring>

#include "esp_log.h"
#include "esp_netif.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <openthread/link.h>
#include <platform/ThreadStackManager.h>
#endif

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace {

InterfaceTypeEnum GetInterfaceType(const char * if_desc)
{
    if (if_desc == nullptr)
    {
        return InterfaceTypeEnum::kUnspecified;
    }

    if (strncmp(if_desc, "ap", 2) == 0 || strncmp(if_desc, "sta", 3) == 0)
    {
        return InterfaceTypeEnum::kWiFi;
    }
    if (strncmp(if_desc, "openthread", 10) == 0)
    {
        return InterfaceTypeEnum::kThread;
    }
    if (strncmp(if_desc, "eth", 3) == 0)
    {
        return InterfaceTypeEnum::kEthernet;
    }
    return InterfaceTypeEnum::kUnspecified;
}

} // namespace

namespace chip {
namespace DeviceLayer {

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    // TODO: safely iterate over netifs
    esp_netif_t * netif     = esp_netif_next_unsafe(NULL);
    NetworkInterface * head = NULL;
    uint8_t ipv6_addr_count = 0;
    esp_ip6_addr_t ip6_addr[LWIP_IPV6_NUM_ADDRESSES];
    if (netif == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        for (esp_netif_t * ifa = netif; ifa != NULL; ifa = esp_netif_next_unsafe(ifa))
        {
            NetworkInterface * ifp = new NetworkInterface();
            uint8_t addressSize    = 0;
            Platform::CopyString(ifp->Name, esp_netif_get_ifkey(ifa));
            ifp->name          = CharSpan::fromCharString(ifp->Name);
            ifp->isOperational = true;
            ifp->type          = GetInterfaceType(esp_netif_get_desc(ifa));
            ifp->offPremiseServicesReachableIPv4.SetNull();
            ifp->offPremiseServicesReachableIPv6.SetNull();
            bool useEspNetifMacAddress = true;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
            if (ifp->type == InterfaceTypeEnum::kThread)
            {
                static_assert(OT_EXT_ADDRESS_SIZE <= sizeof(ifp->MacAddress), "Unexpected extended address size");
                if (ThreadStackMgr().GetPrimary802154MACAddress(ifp->MacAddress) == CHIP_NO_ERROR)
                {
                    addressSize = OT_EXT_ADDRESS_SIZE;
                }
                useEspNetifMacAddress = false;
            }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
            if (useEspNetifMacAddress && esp_netif_get_mac(ifa, ifp->MacAddress) == ESP_OK)
            {
                // For Wi-Fi or Ethernet interface, the MAC address size should be 6.
                addressSize = 6;
            }
            if (addressSize != 0)
            {
                ifp->hardwareAddress = ByteSpan(ifp->MacAddress, addressSize);
            }
            else
            {
                ChipLogError(DeviceLayer, "Failed to get network hardware address");
            }
#ifndef CONFIG_DISABLE_IPV4
            esp_netif_ip_info_t ipv4_info;
            if (esp_netif_get_ip_info(ifa, &ipv4_info) == ESP_OK)
            {
                memcpy(ifp->Ipv4AddressesBuffer[0], &(ipv4_info.ip.addr), kMaxIPv4AddrSize);
                ifp->Ipv4AddressSpans[0] = ByteSpan(ifp->Ipv4AddressesBuffer[0], kMaxIPv4AddrSize);
                ifp->IPv4Addresses       = app::DataModel::List<ByteSpan>(ifp->Ipv4AddressSpans, 1);
            }
#endif // !defined(CONFIG_DISABLE_IPV4)

            static_assert(kMaxIPv6AddrCount <= UINT8_MAX, "Count might not fit in ipv6_addr_count");
            auto addr_count = esp_netif_get_all_ip6(ifa, ip6_addr);
            if (addr_count < 0)
            {
                ipv6_addr_count = 0;
            }
            else
            {
                ipv6_addr_count = static_cast<uint8_t>(std::min(addr_count, static_cast<int>(kMaxIPv6AddrCount)));
            }
            for (uint8_t idx = 0; idx < ipv6_addr_count; ++idx)
            {
                memcpy(ifp->Ipv6AddressesBuffer[idx], ip6_addr[idx].addr, kMaxIPv6AddrSize);
                ifp->Ipv6AddressSpans[idx] = ByteSpan(ifp->Ipv6AddressesBuffer[idx], kMaxIPv6AddrSize);
            }
            ifp->IPv6Addresses = app::DataModel::List<ByteSpan>(ifp->Ipv6AddressSpans, ipv6_addr_count);

            ifp->Next = head;
            head      = ifp;
        }
    }
    *netifpp = head;
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
