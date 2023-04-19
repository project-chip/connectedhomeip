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
 *          Provides an implementation of the DiagnosticDataProvider object.
 */

#pragma once

#include <platform/DiagnosticDataProvider.h>

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the DiagnosticDataProvider singleton object for Tizen platforms.
 */
class DiagnosticDataProviderImpl : public DiagnosticDataProvider
{
public:
    static DiagnosticDataProviderImpl & GetDefaultInstance();

    // ===== Methods that implement the DiagnosticDataProvider abstract interface.

    bool SupportsWatermarks() override { return true; }
    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;
    CHIP_ERROR GetThreadMetrics(ThreadMetrics ** threadMetricsOut) override;
    CHIP_ERROR ResetWatermarks() override;
    void ReleaseThreadMetrics(ThreadMetrics * threadMetrics) override;

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;

    CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults) override;
    CHIP_ERROR GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults) override;
    CHIP_ERROR GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults) override;

    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;
    void ReleaseNetworkInterfaces(NetworkInterface * netifp) override;

    CHIP_ERROR GetEthPHYRate(app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & pHYRate) override;
    CHIP_ERROR GetEthFullDuplex(bool & fullDuplex) override;
    CHIP_ERROR GetEthTimeSinceReset(uint64_t & timeSinceReset) override;
    CHIP_ERROR GetEthPacketRxCount(uint64_t & packetRxCount) override;
    CHIP_ERROR GetEthPacketTxCount(uint64_t & packetTxCount) override;
    CHIP_ERROR GetEthTxErrCount(uint64_t & txErrCount) override;
    CHIP_ERROR GetEthCollisionCount(uint64_t & collisionCount) override;
    CHIP_ERROR GetEthOverrunCount(uint64_t & overrunCount) override;
    CHIP_ERROR ResetEthNetworkDiagnosticsCounts() override;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & bssId) override;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType) override;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion) override;
    CHIP_ERROR GetWiFiChannelNumber(uint16_t & channelNumber) override;
    CHIP_ERROR GetWiFiRssi(int8_t & rssi) override;
    CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & beaconLostCount) override;
    CHIP_ERROR GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount) override;
    CHIP_ERROR GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount) override;
    CHIP_ERROR GetWiFiCurrentMaxRate(uint64_t & currentMaxRate) override;
    CHIP_ERROR GetWiFiOverrunCount(uint64_t & overrunCount) override;
    CHIP_ERROR ResetWiFiNetworkDiagnosticsCounts() override;
#endif

private:
    uint64_t mEthPacketRxCount  = 0;
    uint64_t mEthPacketTxCount  = 0;
    uint64_t mEthTxErrCount     = 0;
    uint64_t mEthCollisionCount = 0;
    uint64_t mEthOverrunCount   = 0;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    uint32_t mBeaconLostCount        = 0;
    uint32_t mPacketMulticastRxCount = 0;
    uint32_t mPacketMulticastTxCount = 0;
    uint32_t mPacketUnicastRxCount   = 0;
    uint32_t mPacketUnicastTxCount   = 0;
    uint64_t mOverrunCount           = 0;
#endif
};

/**
 * Returns the platform-specific implementation of the DiagnosticDataProvider singleton object.
 *
 * Applications can use this to gain access to features of the DiagnosticDataProvider
 * that are specific to the selected platform.
 */
DiagnosticDataProvider & GetDiagnosticDataProviderImpl();

} // namespace DeviceLayer
} // namespace chip
