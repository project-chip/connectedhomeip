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

#pragma once

#include <platform/DiagnosticDataProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

// XXX: This is a workaround for a bug in the Tizen SDK header files. It is not
//      possible to include both <net/if.h> and <linux/if.h> at the same time.
//      This will cause warning that struct ifmap is redefined. On Linux, this
//      is not a problem, because in <linux/if.h> the struct is guarded with
//      ifdef. To prevent this, we will define _LINUX_IF_H, so the <linux/if.h>
//      will not be included.
#define _LINUX_IF_H

#include <linux/types.h>
#include <linux/wireless.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static constexpr uint16_t kWiFi_BAND_2_4_GHZ = 2400;
static constexpr uint16_t kWiFi_BAND_5_0_GHZ = 5000;

class ConnectivityUtils
{
public:
    static uint16_t MapChannelToFrequency(const uint16_t inBand, const uint8_t inChannel);
    static uint8_t MapFrequencyToChannel(const uint16_t frequency);
    static app::Clusters::GeneralDiagnostics::InterfaceTypeEnum GetInterfaceConnectionType(const char * ifname);
    static CHIP_ERROR GetInterfaceHardwareAddrs(const char * ifname, uint8_t * buf, size_t bufSize);
    static CHIP_ERROR GetInterfaceIPv4Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp);
    static CHIP_ERROR GetInterfaceIPv6Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp);
    static CHIP_ERROR GetWiFiInterfaceName(char * ifname, size_t bufSize);
    static CHIP_ERROR GetWiFiChannelNumber(const char * ifname, uint16_t & channelNumber);
    static CHIP_ERROR GetWiFiRssi(const char * ifname, int8_t & rssi);
    static CHIP_ERROR GetWiFiBeaconLostCount(const char * ifname, uint32_t & beaconLostCount);
    static CHIP_ERROR GetWiFiCurrentMaxRate(const char * ifname, uint64_t & currentMaxRate);
    static CHIP_ERROR GetEthInterfaceName(char * ifname, size_t bufSize);
    static CHIP_ERROR GetEthPHYRate(const char * ifname, app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & pHYRate);
    static CHIP_ERROR GetEthFullDuplex(const char * ifname, bool & fullDuplex);

private:
    static uint16_t Map2400MHz(const uint8_t inChannel);
    static uint16_t Map5000MHz(const uint8_t inChannel);
    static double ConvertFrequenceToFloat(const iw_freq * in);
    static CHIP_ERROR GetWiFiParameter(int skfd, const char * ifname, int request, struct iwreq * pwrq);
    static CHIP_ERROR GetWiFiStats(int skfd, const char * ifname, struct iw_statistics * stats);
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
