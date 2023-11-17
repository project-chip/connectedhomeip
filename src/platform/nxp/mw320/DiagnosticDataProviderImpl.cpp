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

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for k32w0 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>
#include <inet/InetInterface.h>
#include <lwip/tcpip.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/nxp/mw320/ConnectivityUtils.h>
#include <platform/nxp/mw320/DiagnosticDataProviderImpl.h>

extern "C" {
#include "wlan.h"
}

// #include <openthread/platform/entropy.h>
using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize;

    freeHeapSize    = xPortGetFreeHeapSize();
    currentHeapFree = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

// ToDo: Find a suitable for HEAP_SIZE
#define HEAP_SIZE 0x200

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    /*
        size_t freeHeapSize;
        size_t usedHeapSize;

        freeHeapSize = xPortGetFreeHeapSize();
        usedHeapSize = HEAP_SIZE - freeHeapSize;

        currentHeapUsed = static_cast<uint64_t>(usedHeapSize);
    */
    currentHeapUsed = HEAP_SIZE - xPortGetFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    /*
        size_t highWatermarkHeapSize;

        highWatermarkHeapSize    = HEAP_SIZE - xPortGetMinimumEverFreeHeapSize();
        currentHeapHighWatermark = static_cast<uint64_t>(highWatermarkHeapSize);
    */
    currentHeapHighWatermark = HEAP_SIZE - xPortGetMinimumEverFreeHeapSize();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
    // If implemented, the server SHALL set the value of the CurrentHeapHighWatermark attribute to the
    // value of the CurrentHeapUsed.

    // On Linux, the write operation is non-op since we always rely on the mallinfo system
    // function to get the current heap memory.

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
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    return CHIP_NO_ERROR;
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

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * ifp = new NetworkInterface();
    uint8_t size           = 0;

    strncpy(ifp->Name, "mlan0", Inet::InterfaceId::kMaxIfNameLength);
    ifp->Name[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';
    ifp->name                                          = CharSpan::fromCharString(ifp->Name);
    ifp->isOperational                                 = true;
    ifp->type                                          = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    if (ConnectivityUtils::GetInterfaceIPv4Addrs("", size, ifp) == CHIP_NO_ERROR)
    {
        if (size > 0)
        {
            ifp->IPv4Addresses = DataModel::List<const chip::ByteSpan>(ifp->Ipv4AddressSpans, size);
        }
    }
    if (ConnectivityUtils::GetInterfaceIPv6Addrs("", size, ifp) == CHIP_NO_ERROR)
    {
        if (size > 0)
        {
            ifp->IPv6Addresses = DataModel::List<const chip::ByteSpan>(ifp->Ipv6AddressSpans, size);
        }
    }
    ifp->Next = nullptr;
    *netifpp  = ifp;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    return ConnectivityUtils::GetWiFiChannelNumber(ConnectivityMgrImpl().GetWiFiIfName(), channelNumber);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    return ConnectivityUtils::GetWiFiRssi(ConnectivityMgrImpl().GetWiFiIfName(), rssi);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    return ConnectivityUtils::GetWiFiBeaconLostCount(ConnectivityMgrImpl().GetWiFiIfName(), beaconRxCount);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    return ConnectivityUtils::GetWiFiBeaconLostCount(ConnectivityMgrImpl().GetWiFiIfName(), beaconLostCount);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
#ifdef GET_FROM_SDK
    int ret;
    wifi_pkt_stats_t stats;

    ret = wifi_get_log(&stats);
    if (ret != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "wifi_get_log failed ");
    }
    packetMulticastRxCount = stats.mcast_rx_frame;
#else
    packetMulticastRxCount = 0;
#endif // GET_FROM_SDK
    ChipLogProgress(DeviceLayer, "GetWiFiPacketMulticastRxCount: %lu ", packetMulticastRxCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
#ifdef GET_FROM_SDK
    int ret;
    wifi_pkt_stats_t stats;

    ret = wifi_get_log(&stats);
    if (ret != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "wifi_get_log failed ");
    }

    packetMulticastTxCount = stats.mcast_tx_frame;
#else
    packetMulticastTxCount = 0;
#endif // GET_FROM_SDK
    ChipLogProgress(DeviceLayer, "GetWiFiPacketMulticastTxCount: %lu ", packetMulticastTxCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    // => Not support in SDK yet
    packetUnicastRxCount = 0;
    ChipLogProgress(DeviceLayer, "GetWiFiPacketUnicastRxCount: %lu (ToDo)", packetUnicastRxCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
#ifdef GET_FROM_SDK
    int ret;
    wifi_pkt_stats_t stats;

    ret = wifi_get_log(&stats);
    if (ret != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "wifi_get_log failed ");
    }

    packetUnicastTxCount = stats.tx_frame;
#else
    packetUnicastTxCount   = 0;
#endif // #ifdef GET_FROM_SDK
    ChipLogProgress(DeviceLayer, "GetWiFiPacketUnicastTxCount: %lu", packetUnicastTxCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    return ConnectivityUtils::GetWiFiCurrentMaxRate(ConnectivityMgrImpl().GetWiFiIfName(), currentMaxRate);
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

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion)
{
    return ConnectivityMgrImpl().GetWiFiVersion(wiFiVersion);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & value)
{
    return ConnectivityMgrImpl().GetWiFiBssId(value);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    return ConnectivityMgrImpl().GetWiFiSecurityType(securityType);
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
