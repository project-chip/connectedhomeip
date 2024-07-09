/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2023 NXP
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
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for nxp platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "DiagnosticDataProviderImpl.h"
#include <crypto/CHIPCryptoPAL.h>
#include <platform/DiagnosticDataProvider.h>

#include <inet/InetInterface.h>
#include <lwip/tcpip.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
extern "C" {
#include <wm_net.h>
}
#endif

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
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
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgr().GetBootReason(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * ifp = new NetworkInterface();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    const char * threadNetworkName = otThreadGetNetworkName(ThreadStackMgrImpl().OTInstance());
    ifp->name                      = CharSpan(threadNetworkName, strlen(threadNetworkName));
    ifp->isOperational             = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;
    ConfigurationMgr().GetPrimary802154MACAddress(ifp->MacAddress);
    ifp->hardwareAddress = ByteSpan(ifp->MacAddress, kMaxHardwareAddrSize);
#elif CHIP_DEVICE_CONFIG_ENABLE_WPA
    struct netif * netif = nullptr;
    netif                = static_cast<struct netif *>(net_get_mlan_handle());
    strncpy(ifp->Name, "wlan0", Inet::InterfaceId::kMaxIfNameLength);
    ifp->name          = CharSpan(ifp->Name, strlen(ifp->Name));
    ifp->isOperational = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->type            = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
    ifp->hardwareAddress = ByteSpan(netif->hwaddr, netif->hwaddr_len);
#endif

    // IPv6-only support
    Inet::InterfaceAddressIterator interfAddrIterator;
    uint8_t ipv6AddressesCount = 0;
    while (interfAddrIterator.HasCurrent() && ipv6AddressesCount < kMaxIPv6AddrCount)
    {
        chip::Inet::IPAddress ipv6Address;
        if (interfAddrIterator.GetAddress(ipv6Address) == CHIP_NO_ERROR)
        {
            memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], ipv6Address.Addr, kMaxIPv6AddrSize);
            ifp->Ipv6AddressSpans[ipv6AddressesCount] = ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount]);
            ipv6AddressesCount++;
        }
        interfAddrIterator.Next();
    }
    ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);

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

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & BssId)
{
    constexpr size_t bssIdSize = 6;
    struct wlan_network current_network;

    VerifyOrReturnError(BssId.size() >= bssIdSize, CHIP_ERROR_BUFFER_TOO_SMALL);

    int ret = wlan_get_current_network(&current_network);
    if (ret == WM_SUCCESS)
    {
        memcpy(BssId.data(), current_network.bssid, bssIdSize);
        BssId.reduce_size(bssIdSize);
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;
    struct wlan_network current_network;
    int ret = wlan_get_current_network(&current_network);

    if (ret != WM_SUCCESS)
    {
        // Set as no security by default
        securityType = SecurityTypeEnum::kNone;
        return CHIP_NO_ERROR;
    }
    switch (current_network.security.type)
    {
    case WLAN_SECURITY_WEP_OPEN:
    case WLAN_SECURITY_WEP_SHARED:
        securityType = SecurityTypeEnum::kWep;
        break;
    case WLAN_SECURITY_WPA:
        securityType = SecurityTypeEnum::kWpa;
        break;
    case WLAN_SECURITY_WPA2:
        securityType = SecurityTypeEnum::kWpa2;
        break;
    case WLAN_SECURITY_WPA3_SAE:
        securityType = SecurityTypeEnum::kWpa3;
        break;
    case WLAN_SECURITY_NONE:
    default: // Default: No_security
        securityType = SecurityTypeEnum::kNone;
    }

    ChipLogProgress(DeviceLayer, "GetWiFiSecurityType: %u", to_underlying(securityType));
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion)
{
#if defined(CONFIG_11AX)
    wifiVersion = app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum::kAx;
#elif defined(CONFIG_11AC)
    wifiVersion = app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum::kAc;
#elif defined(CONFIG_11N)
    wifiVersion = app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum::kN;
#endif
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    channelNumber = wlan_get_current_channel();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    short w_rssi;
    int err = wlan_get_current_rssi(&w_rssi);
    if (err != 0)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    rssi = static_cast<int8_t>(w_rssi);
    return CHIP_NO_ERROR;
}

#if DGWIFI_RESET_COUNTS_SUPPORTED
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
#ifdef CONFIG_WIFI_GET_LOG
    wifi_pkt_stats_t stats;
    int ret = wifi_get_log(&stats, MLAN_BSS_TYPE_STA);
    if (ret == WM_SUCCESS)
    {
        beaconLostCount = stats.bcn_miss_cnt;
        return CHIP_NO_ERROR;
    }
#endif /* CONFIG_WIFI_GET_LOG */
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
#ifdef CONFIG_WIFI_GET_LOG
    wifi_pkt_stats_t stats;
    int ret = wifi_get_log(&stats, MLAN_BSS_TYPE_STA);
    if (ret == WM_SUCCESS)
    {
        beaconRxCount = stats.bcn_rcv_cnt;
        return CHIP_NO_ERROR;
    }
#endif /* CONFIG_WIFI_GET_LOG */
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
#ifdef CONFIG_WIFI_GET_LOG
    wifi_pkt_stats_t stats;
    int ret = wifi_get_log(&stats, MLAN_BSS_TYPE_STA);
    if (ret == WM_SUCCESS)
    {
        packetMulticastRxCount = stats.mcast_rx_frame;
        return CHIP_NO_ERROR;
    }
#endif /* CONFIG_WIFI_GET_LOG */
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
#ifdef CONFIG_WIFI_GET_LOG
    wifi_pkt_stats_t stats;
    int ret = wifi_get_log(&stats, MLAN_BSS_TYPE_STA);
    if (ret == WM_SUCCESS)
    {
        packetMulticastTxCount = stats.mcast_tx_frame;
        return CHIP_NO_ERROR;
    }
#endif /* CONFIG_WIFI_GET_LOG */
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
#ifdef CONFIG_WIFI_GET_LOG
    wifi_pkt_stats_t stats;
    int ret = wifi_get_log(&stats, MLAN_BSS_TYPE_STA);
    if (ret == WM_SUCCESS)
    {
        packetUnicastTxCount = stats.tx_frame;
        return CHIP_NO_ERROR;
    }
#endif /* CONFIG_WIFI_GET_LOG */
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

#endif /* DGWIFI_RESET_COUNTS_SUPPORTED */

#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
