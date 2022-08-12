/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 *          for Beken platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <platform/Beken/DiagnosticDataProviderImpl.h>
#include <platform/DiagnosticDataProvider.h>

#include "matter_pal.h"

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    currentHeapFree = xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    currentHeapUsed = prvHeapGetTotalSize() - xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    currentHeapHighWatermark = prvHeapGetTotalSize() - xPortGetMinimumEverFreeHeapSize();
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
    struct netif * netif;

    netif = (struct netif *) net_get_sta_handle(); // assume only on station mode
    if (netif == NULL || ifp == NULL)
    {
        ChipLogError(DeviceLayer, "Can't get the netif instance");
        *netifpp = NULL;
        return CHIP_ERROR_INTERNAL;
    }

    strncpy(ifp->Name, netif->hostname, Inet::InterfaceId::kMaxIfNameLength);
    ifp->Name[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';
    ifp->name                                          = CharSpan::fromCharString(ifp->Name);
    ifp->type                                          = EMBER_ZCL_INTERFACE_TYPE_WI_FI;
    ifp->offPremiseServicesReachableIPv4.SetNonNull(false);
    ifp->offPremiseServicesReachableIPv6.SetNonNull(false);
    memcpy(ifp->MacAddress, netif->hwaddr, sizeof(netif->hwaddr));
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

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(ByteSpan & BssId)
{
    LinkStatusTypeDef linkStatus;

    memset(&linkStatus, 0x0, sizeof(LinkStatusTypeDef));
    if (0 == bk_wlan_get_link_status(&linkStatus))
    {
        BssId = ByteSpan(linkStatus.bssid, 6);
    }
    else
    {
        ChipLogError(DeviceLayer, "GetWiFiBssId Not Supported");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(uint8_t & wifiVersion)
{
    // Support 802.11a/n Wi-Fi in Beken chipset
    wifiVersion = EMBER_ZCL_WI_FI_VERSION_TYPE_802__11N;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(uint8_t & securityType)
{
    int cipher_type;
    cipher_type = bk_sta_cipher_type();
    switch (cipher_type)
    {
    case BK_SECURITY_TYPE_NONE:
        securityType = EMBER_ZCL_SECURITY_TYPE_NONE;
        break;
    case BK_SECURITY_TYPE_WEP:
        securityType = EMBER_ZCL_SECURITY_TYPE_WEP;
        break;
    case BK_SECURITY_TYPE_WPA_TKIP:
    case BK_SECURITY_TYPE_WPA_AES:
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA;
        break;
    case BK_SECURITY_TYPE_WPA2_AES:
    case BK_SECURITY_TYPE_WPA2_TKIP:
    case BK_SECURITY_TYPE_WPA2_MIXED:
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA2;
        break;
    case BK_SECURITY_TYPE_WPA3_SAE:
    case BK_SECURITY_TYPE_WPA3_WPA2_MIXED:
        securityType = EMBER_ZCL_SECURITY_TYPE_WPA3;
        break;
    case BK_SECURITY_TYPE_AUTO:
    default:
        securityType = EMBER_ZCL_SECURITY_TYPE_UNSPECIFIED;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    LinkStatusTypeDef linkStatus;

    memset(&linkStatus, 0x0, sizeof(LinkStatusTypeDef));
    if (0 == bk_wlan_get_link_status(&linkStatus))
    {
        channelNumber = linkStatus.channel;
    }
    else
    {
        ChipLogError(DeviceLayer, "GetWiFiChannelNumber Not Supported");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    LinkStatusTypeDef linkStatus;

    memset(&linkStatus, 0x0, sizeof(LinkStatusTypeDef));
    if (0 == bk_wlan_get_link_status(&linkStatus))
    {
        rssi = linkStatus.wifi_strength;
    }
    else
    {
        ChipLogError(DeviceLayer, "GetWiFiRssi Not Supported");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    beaconLostCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    currentMaxRate = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    packetMulticastRxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    packetMulticastTxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    packetUnicastRxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    packetUnicastTxCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    overrunCount = 0;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    return CHIP_NO_ERROR;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
