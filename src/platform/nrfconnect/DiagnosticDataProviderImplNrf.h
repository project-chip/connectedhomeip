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
 *           for nrfconnect platform.
 */

#pragma once

#include <platform/Zephyr/DiagnosticDataProviderImpl.h>

namespace chip {
namespace DeviceLayer {

class DiagnosticDataProviderImplNrf : public DiagnosticDataProviderImpl
{
public:
#ifdef CONFIG_WIFI_NRF70
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & value) override;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType) override;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion) override;
    CHIP_ERROR GetWiFiChannelNumber(uint16_t & channelNumber) override;
    CHIP_ERROR GetWiFiRssi(int8_t & rssi) override;
    CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & beaconLostCount) override;
    CHIP_ERROR GetWiFiBeaconRxCount(uint32_t & beaconRxCount) override;
    CHIP_ERROR GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount) override;
    CHIP_ERROR GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount) override;
    CHIP_ERROR GetWiFiCurrentMaxRate(uint64_t & currentMaxRate) override;
    CHIP_ERROR GetWiFiOverrunCount(uint64_t & overrunCount) override;
    CHIP_ERROR ResetWiFiNetworkDiagnosticsCounts() override;
#endif

    static DiagnosticDataProviderImplNrf & GetDefaultInstance();

private:
    DiagnosticDataProviderImplNrf() = default;
};

DiagnosticDataProvider & GetDiagnosticDataProviderImpl();

} // namespace DeviceLayer
} // namespace chip
