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
 *          Utilities for accessing parameters of the network interface and the wireless
 *          statistics(extracted from /proc/net/wireless) on Linux platforms.
 */

#pragma once

#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static constexpr uint16_t kWiFi_BAND_2_4_GHZ = 2400;
static constexpr uint16_t kWiFi_BAND_5_0_GHZ = 5000;

namespace ConnectivityUtils {

uint16_t MapChannelToFrequency(const uint16_t inBand, const uint8_t inChannel);
uint8_t MapFrequencyToChannel(const uint16_t frequency);
app::Clusters::GeneralDiagnostics::InterfaceTypeEnum GetInterfaceConnectionType(const char * ifname);
CHIP_ERROR GetInterfaceHardwareAddrs(const char * ifname, uint8_t * buf, size_t bufSize);
CHIP_ERROR GetInterfaceIPv4Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp);
CHIP_ERROR GetInterfaceIPv6Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp);
CHIP_ERROR GetWiFiInterfaceName(char * ifname, size_t bufSize);
CHIP_ERROR GetWiFiChannelNumber(const char * ifname, uint16_t & channelNumber);
CHIP_ERROR GetWiFiRssi(const char * ifname, int8_t & rssi);
CHIP_ERROR GetWiFiBeaconLostCount(const char * ifname, uint32_t & beaconLostCount);
CHIP_ERROR GetWiFiCurrentMaxRate(const char * ifname, uint64_t & currentMaxRate);
CHIP_ERROR GetEthInterfaceName(char * ifname, size_t bufSize);
CHIP_ERROR GetEthPHYRate(const char * ifname, app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & pHYRate);
CHIP_ERROR GetEthFullDuplex(const char * ifname, bool & fullDuplex);

} // namespace ConnectivityUtils

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
