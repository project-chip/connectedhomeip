/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ASR/DiagnosticDataProviderImpl.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#ifdef CFG_PLF_RV32
#include "asr_alto_boot.h"
#define duet_get_boot_type asr_get_boot_type
#elif defined CFG_PLF_DUET
#include "duet_boot.h"
#else
#include "lega_boot.h"
#define duet_get_boot_type lega_get_boot_type
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern struct netif * lwip_get_netif(void);
#ifdef __cplusplus
}
#endif

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    currentHeapFree = static_cast<uint64_t>(lega_rtos_get_free_heap_size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    currentHeapUsed = static_cast<uint64_t>(lega_rtos_get_total_size() - lega_rtos_get_free_heap_size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    currentHeapHighWatermark = static_cast<uint64_t>(lega_rtos_get_total_size() - lega_rtos_get_minimum_free_heap_size());
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetUpTime(uint64_t & upTime)
{
    System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp startTime   = PlatformMgrImpl().GetStartTime();

    if (currentTime >= startTime)
    {
        upTime = std::chrono::duration_cast<System::Clock::Seconds64>(currentTime - startTime).count();
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    uint64_t upTime = 0;

    if (GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalHours = 0;
        if (ConfigurationMgr().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            /* uptime is terms of seconds and dividing it by 3600 to calculate
             * totalOperationalHours in hours.
             */
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = duet_get_boot_type();

    bootReason = BootReasonType::kUnspecified;

    if (reason == UNKNOWN_RST)
    {
        bootReason = BootReasonType::kUnspecified;
    }
    else if (reason == PWR_ON_RST)
    {
        bootReason = BootReasonType::kPowerOnReboot;
    }
    else if (reason == HARDWARE_PIN_RST)
    {
        bootReason = BootReasonType::kBrownOutReset;
    }
    else if (reason == SOFTWARE_RST)
    {
        bootReason = BootReasonType::kSoftwareReset;
    }
    /*
    else if (reason == WDG_RST)
    {
        bootReason = BootReasonType::kSoftwareWatchdogReset;
    }
    */
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * ifp = new NetworkInterface();
    struct netif * ifa;

    ifa = lwip_get_netif();

    if (ifa == NULL)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        Platform::CopyString(ifp->Name, ifa->name);

        ifp->name          = CharSpan::fromCharString(ifp->Name);
        ifp->isOperational = true;
        if ((ifa->flags) & NETIF_FLAG_ETHERNET)
            ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kEthernet;
        else
            ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
        ifp->offPremiseServicesReachableIPv4.SetNull();
        ifp->offPremiseServicesReachableIPv6.SetNull();

        memcpy(ifp->MacAddress, ifa->hwaddr, sizeof(ifa->hwaddr));

        if (0)
        {
            ChipLogError(DeviceLayer, "Failed to get network hardware address");
        }
        else
        {
            // Set 48-bit IEEE MAC Address
            ifp->hardwareAddress = ByteSpan(ifp->MacAddress, 6);
        }

        if (ifa->ip_addr.u_addr.ip4.addr != 0)
        {
            memcpy(ifp->Ipv4AddressesBuffer[0], &(ifa->ip_addr.u_addr.ip4.addr), kMaxIPv4AddrSize);
            ifp->Ipv4AddressSpans[0] = ByteSpan(ifp->Ipv4AddressesBuffer[0], kMaxIPv4AddrSize);
            ifp->IPv4Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv4AddressSpans, 1);
        }

        if (ifa->ip6_addr->u_addr.ip6.addr != 0)
        {
            memcpy(ifp->Ipv6AddressesBuffer[0], &(ifa->ip6_addr->u_addr.ip6.addr), kMaxIPv6AddrSize);
            ifp->Ipv6AddressSpans[0] = ByteSpan(ifp->Ipv6AddressesBuffer[0], kMaxIPv6AddrSize);
            ifp->IPv6Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, 1);
        }
    }

    *netifpp = ifp;
    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface * netifp)
{
    while (netifp)
    {
        NetworkInterface * del = netifp;
        netifp                 = netifp->Next;
        delete del;
    }
}

/* Wi-Fi Diagnostics Cluster Support */

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & BssId)
{
    constexpr size_t bssIdSize = 6;
    VerifyOrReturnError(BssId.size() >= bssIdSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    lega_wlan_link_stat_t link_stat;

    if (lega_wlan_get_link_status(&link_stat) == 0)
    {
        memcpy(BssId.data(), link_stat.bssid, bssIdSize);
        BssId.reduce_size(bssIdSize);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;

    lega_wlan_ap_info_adv_t * apinfo = lega_wlan_get_associated_apinfo();

    switch (apinfo->security)
    {
    case WLAN_SECURITY_OPEN:
        securityType = SecurityTypeEnum::kNone;
        break;
    case WLAN_SECURITY_WEP:
        securityType = SecurityTypeEnum::kWep;
        break;
    case WLAN_SECURITY_WPA:
        securityType = SecurityTypeEnum::kWpa;
        break;
    case WLAN_SECURITY_WPA2:
    case WLAN_SECURITY_AUTO:
        securityType = SecurityTypeEnum::kWpa2;
        break;
    default:
        securityType = SecurityTypeEnum::kUnspecified;
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    lega_wlan_link_stat_t link_stat;
    channelNumber = 0;

    if (lega_wlan_get_link_status(&link_stat) == 0)
    {
        channelNumber = link_stat.channel;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    lega_wlan_link_stat_t link_stat;
    rssi = 0;

    if (lega_wlan_get_link_status(&link_stat) == 0)
    {
        rssi = link_stat.wifi_strength;
    }

    return CHIP_NO_ERROR;
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
