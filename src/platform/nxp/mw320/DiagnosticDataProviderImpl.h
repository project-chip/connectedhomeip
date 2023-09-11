/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *          Provides an implementation of the DiagnosticDataProvider object.
 */

#pragma once

#include <memory>

#include <platform/DiagnosticDataProvider.h>

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

    bool SupportsWatermarks() override { return true; }
    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;
    CHIP_ERROR ResetWatermarks() override;

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;

    CHIP_ERROR GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults) override;
    CHIP_ERROR GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults) override;
    CHIP_ERROR GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults) override;

    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;

    // Wi-Fi Diagnostics
    CHIP_ERROR GetWiFiChannelNumber(uint16_t & channelNumber) override;
    CHIP_ERROR GetWiFiRssi(int8_t & rssi) override;
    CHIP_ERROR GetWiFiBeaconRxCount(uint32_t & beaconRxCount) override;
    CHIP_ERROR GetWiFiBeaconLostCount(uint32_t & beaconLostCount) override;
    CHIP_ERROR GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount) override;
    CHIP_ERROR GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount) override;
    CHIP_ERROR GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount) override;
    CHIP_ERROR GetWiFiCurrentMaxRate(uint64_t & currentMaxRate) override;
    CHIP_ERROR GetWiFiOverrunCount(uint64_t & overrunCount) override;
    CHIP_ERROR ResetWiFiNetworkDiagnosticsCounts() override;

    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion) override;
    CHIP_ERROR GetWiFiBssId(MutableByteSpan & value) override;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType) override;
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
