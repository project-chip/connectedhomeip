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
 *          for nrfconnect platform.
 */

#include "DiagnosticDataProviderImplNrf.h"

#ifdef CONFIG_WIFI_NRF700X
#include <platform/nrfconnect/wifi/WiFiManager.h>
#endif

namespace chip {
namespace DeviceLayer {

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImplNrf::GetDefaultInstance();
}

DiagnosticDataProviderImplNrf & DiagnosticDataProviderImplNrf::GetDefaultInstance()
{
    static DiagnosticDataProviderImplNrf sInstance;
    return sInstance;
}

#ifdef CONFIG_WIFI_NRF700X
CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiBssId(ByteSpan & value)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    value          = info.mBssId;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiSecurityType(uint8_t & securityType)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    securityType   = info.mSecurityType;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiVersion(uint8_t & wiFiVersion)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    wiFiVersion    = info.mWiFiVersion;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    channelNumber  = info.mChannel;
    return err;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiRssi(int8_t & rssi)
{
    WiFiManager::WiFiInfo info;
    CHIP_ERROR err = WiFiManager::Instance().GetWiFiInfo(info);
    rssi           = info.mRssi;
    return err;
}

// below will be implemented when the WiFi driver exposes Zephyr NET_STATISTICS API
CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}

CHIP_ERROR DiagnosticDataProviderImplNrf::ResetWiFiNetworkDiagnosticsCounts()
{
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
}
#endif

} // namespace DeviceLayer
} // namespace chip
