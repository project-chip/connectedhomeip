/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *          Provides an implementation of Wi-Fi network diagnostics methods of the DiagnosticDataProvider object.
 */

#include <platform/Zephyr/wifi/DiagnosticDataProviderImplWiFi.h>
#include <platform/Zephyr/InetUtils.h>
#include <platform/Zephyr/wifi/WiFiManager.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImplWiFi::GetDefaultInstance();
}

DiagnosticDataProviderImplWiFi & DiagnosticDataProviderImplWiFi::GetDefaultInstance() {
    static DiagnosticDataProviderImplWiFi sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & value)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    VerifyOrReturnError(sizeof(info.mBssId) >= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(value.data(), info.mBssId, sizeof(info.mBssId));
    value.reduce_size(sizeof(info.mBssId));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;

    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    securityType = info.mSecurityType;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    wiFiVersion = info.mWiFiVersion;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    channelNumber = info.mChannel;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    rssi = info.mRssi;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    // mCurrentPhyRate Value in MB
    currentMaxRate = info.mCurrentPhyRate * 1000000;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    beaconLostCount = stats.mBeaconsLostCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    beaconRxCount = stats.mBeaconsSuccessCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetMulticastRxCount = stats.mPacketMulticastRxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetMulticastTxCount = stats.mPacketMulticastTxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetUnicastRxCount = stats.mPacketUnicastRxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetUnicastTxCount = stats.mPacketUnicastTxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    overrunCount = static_cast<uint64_t>(stats.mOverRunCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    net_if * iface = InetUtils::GetWiFiInterface();
    VerifyOrReturnError(iface != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

    if (net_mgmt(NET_REQUEST_STATS_RESET_WIFI, iface, NULL, 0))
    {
        ChipLogError(DeviceLayer, "WiFi statistics reset failed");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
