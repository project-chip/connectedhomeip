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

#include <memory>

#include "whd.h"
#include "whd_buffer_api.h"
#include "whd_network_types.h"
#include "whd_wifi_api.h"
#include "whd_wlioctl.h"
#include <cy_wcm.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/Infineon/PSOC6/P6Utils.h>

#define PHYRATE_KPBS_BYTES_PER_SEC 125

/* WiFi Counters */
enum class WiFiStatsCountType
{
    kWiFiBeaconLostCount,
    kWiFiBeaconRxCount,
    kWiFiUnicastPacketRxCount,
    kWiFiUnicastPacketTxCount,
    kWiFiMulticastPacketRxCount,
    kWiFiMulticastPacketTxCount,
    kWiFiOverrunCount,
    kWiFiResetCount
};

#define MAX_WHD_INTERFACE (2)
extern whd_interface_t whd_ifs[MAX_WHD_INTERFACE];

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Infineon PSoC6 platforms.
 */
class DiagnosticDataProviderImpl : public DiagnosticDataProvider
{
public:
    static DiagnosticDataProviderImpl & GetDefaultInstance();

    // ===== Methods that implement the PlatformManager abstract interface.

    CHIP_ERROR GetCurrentHeapFree(uint64_t & currentHeapFree) override;
    CHIP_ERROR GetCurrentHeapUsed(uint64_t & currentHeapUsed) override;
    CHIP_ERROR GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark) override;

    CHIP_ERROR GetRebootCount(uint16_t & rebootCount) override;
    CHIP_ERROR GetUpTime(uint64_t & upTime) override;
    CHIP_ERROR GetTotalOperationalHours(uint32_t & totalOperationalHours) override;
    CHIP_ERROR GetBootReason(BootReasonType & bootReason) override;
    CHIP_ERROR GetNetworkInterfaces(NetworkInterface ** netifpp) override;
    void ReleaseNetworkInterfaces(NetworkInterface * netifp) override;

    CHIP_ERROR GetWiFiBssId(MutableByteSpan & BssId) override;
    CHIP_ERROR GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType) override;
    CHIP_ERROR GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wifiVersion) override;
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
    CHIP_ERROR WiFiCounters(WiFiStatsCountType type, uint64_t & count);
    void xtlv_buffer_parsing(const uint8_t * tlv_buf, uint16_t buflen, WiFiStatsCountType Counttype, uint64_t & count);
    void ReadCounters(WiFiStatsCountType Counttype, uint64_t & count, wl_cnt_ver_30_t * cnt, wl_cnt_ge40mcst_v1_t * cnt_ge40);
    /* Function to update ipv4 and ipv6 off premise service capability bit */
    void UpdateoffPremiseService(bool ipv4service, bool ipv6service);
    CHIP_ERROR GetThreadMetrics(ThreadMetrics ** threadMetricsOut) override;
    void ReleaseThreadMetrics(ThreadMetrics * threadMetrics) override;
    /* These variables will be set to 0 during start up and will be updated when reset-counts
     * zcl command is received.
     * These are considered as base for below attributes of WiFi Diagnostics Cluster:
     * BeaconLostCount, BeaconRxCount, PacketMulticastRxCount, PacketMulticastTxCount,
     * PacketUnicastRxCount, PacketUnicastTxCount, OverrunCount.
     * Each attributes read will be difference of WLAN counters minus equivalent variables below.
     */
    uint32_t mBeaconRxCount          = 0;
    uint32_t mBeaconLostCount        = 0;
    uint32_t mPacketMulticastRxCount = 0;
    uint32_t mPacketMulticastTxCount = 0;
    uint32_t mPacketUnicastRxCount   = 0;
    uint32_t mPacketUnicastTxCount   = 0;
    uint64_t mOverrunCount           = 0;
    app::DataModel::Nullable<bool> mipv4_offpremise;
    app::DataModel::Nullable<bool> mipv6_offpremise;
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
