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
 *          Provides an implementation of the DiagnosticDataProvider object.
 */

#pragma once

#include <memory>

#include <platform/DiagnosticDataProvider.h>

/**
 * DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts() is
 * currently not supported due to missing reset feature from SDK's wifi driver.
 * For this reason, the PKTCNT (beaconRxCount, packetMulticastRxCount,
 * packetMulticastTxCount, packetUnicastRxCount, packetUnicastTxCount)
 * and ERRCNT (beaconLostCount, overrunCount) features of DGWIFI cluster
 * are currently not supported.
 */
#define DGWIFI_RESET_COUNTS_SUPPORTED 0

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Linux platforms.
 */
class DiagnosticDataProviderImpl : public DiagnosticDataProvider
{
public:
    static DiagnosticDataProviderImpl & GetDefaultInstance();

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;

    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;
    void ReleaseNetworkInterfaces(NetworkInterface * netifp) override;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & value) override;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType) override;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion) override;
    CHIP_ERROR GetWiFiChannelNumber(uint16_t & channelNumber) override;
    CHIP_ERROR GetWiFiRssi(int8_t & rssi) override;
#if DGWIFI_RESET_COUNTS_SUPPORTED
    CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & beaconLostCount) override;
    CHIP_ERROR GetWiFiBeaconRxCount(uint32_t & beaconRxCount) override;
    CHIP_ERROR GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount) override;
    CHIP_ERROR GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount) override;
    /**
     * TODO : This should reset BeaconLostCount, BeaconRxCount, PacketMulticastRxCount,
     * PacketMulticastTxCount, PacketUnicastRxCount, PacketUnicastTxCount
     */
    // CHIP_ERROR ResetWiFiNetworkDiagnosticsCounts() override;
#endif /* DGWIFI_RESET_COUNTS_SUPPORTED */
#endif /* CHIP_DEVICE_CONFIG_ENABLE_WPA */
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
