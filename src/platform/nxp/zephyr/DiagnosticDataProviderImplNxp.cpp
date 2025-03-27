/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
 *          for NXP Zephyr platform.
 */

#include "DiagnosticDataProviderImplNxp.h"

#ifdef CONFIG_WIFI_NXP
#include <platform/Zephyr/wifi/WiFiManager.h>
#endif

namespace chip {
namespace DeviceLayer {

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImplNxp::GetDefaultInstance();
}

DiagnosticDataProviderImplNxp & DiagnosticDataProviderImplNxp::GetDefaultInstance()
{
    static DiagnosticDataProviderImplNxp sInstance;
    return sInstance;
}

#ifdef CONFIG_WIFI_NXP
CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiBssId(MutableByteSpan & value)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    VerifyOrReturnError(sizeof(info.mBssId) < value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(value.data(), info.mBssId, sizeof(info.mBssId));
    value.reduce_size(sizeof(info.mBssId));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DiagnosticDataProviderImplNxp::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;

    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    securityType   = info.mSecurityType;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    wiFiVersion    = info.mWiFiVersion;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    channelNumber  = info.mChannel;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiRssi(int8_t & rssi)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    rssi           = info.mRssi;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    // mCurrentPhyRate Value in MB
    currentMaxRate = info.mCurrentPhyRate * 1000000;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err  = WiFiManager::Instance().GetNetworkStatistics(stats);
    beaconLostCount = stats.mBeaconsLostCount - mOldStats.beaconLostCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err = WiFiManager::Instance().GetNetworkStatistics(stats);
    beaconRxCount  = stats.mBeaconsSuccessCount - mOldStats.beaconRxCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err         = WiFiManager::Instance().GetNetworkStatistics(stats);
    packetMulticastRxCount = stats.mPacketMulticastRxCount - mOldStats.packetMulticastRxCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err         = WiFiManager::Instance().GetNetworkStatistics(stats);
    packetMulticastTxCount = stats.mPacketMulticastTxCount - mOldStats.packetMulticastTxCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err       = WiFiManager::Instance().GetNetworkStatistics(stats);
    packetUnicastRxCount = stats.mPacketUnicastRxCount - mOldStats.packetUnicastRxCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err       = WiFiManager::Instance().GetNetworkStatistics(stats);
    packetUnicastTxCount = stats.mPacketUnicastTxCount - mOldStats.packetUnicastTxCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err = WiFiManager::Instance().GetNetworkStatistics(stats);
    overrunCount   = static_cast<uint64_t>(stats.mOverRunCount) - mOldStats.overrunCount;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNxp::ResetWiFiNetworkDiagnosticsCounts()
{
    /* NET_REQUEST_STATS_RESET_WIFI can be used with net_mgmt API to achieve this.
     * NXP WiFi Driver doesn't support this command yet.
     * Workaround to reset the statistics manually in the Matter layer.
     */
    WiFiManager::NetworkStatistics stats;
    CHIP_ERROR err = WiFiManager::Instance().GetNetworkStatistics(stats);
    if (err == CHIP_NO_ERROR)
    {
        mOldStats.beaconLostCount        = stats.mBeaconsLostCount;
        mOldStats.beaconRxCount          = stats.mBeaconsSuccessCount;
        mOldStats.packetMulticastRxCount = stats.mPacketMulticastRxCount;
        mOldStats.packetMulticastTxCount = stats.mPacketMulticastTxCount;
        mOldStats.packetUnicastRxCount   = stats.mPacketUnicastRxCount;
        mOldStats.packetUnicastTxCount   = stats.mPacketUnicastTxCount;
        mOldStats.overrunCount           = static_cast<uint64_t>(stats.mOverRunCount);
    }
    return err;
}
#endif

} // namespace DeviceLayer
} // namespace chip
