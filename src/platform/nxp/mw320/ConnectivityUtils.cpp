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

#include "ConnectivityUtils.h"
#include <app-common/zap-generated/enums.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>

extern "C" {
#include "wlan.h"
#include "wm_net.h"
static struct wlan_network sta_network;
}

using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace chip {
namespace DeviceLayer {
namespace Internal {

uint16_t ConnectivityUtils::Map2400MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inChannel >= 1 && inChannel <= 13)
    {
        frequency = static_cast<uint16_t>(2412 + ((inChannel - 1) * 5));
    }
    else if (inChannel == 14)
    {
        frequency = 2484;
    }

    return frequency;
}

uint16_t ConnectivityUtils::Map5000MHz(const uint8_t inChannel)
{
    uint16_t frequency = 0;

    switch (inChannel)
    {
    case 183:
        frequency = 4915;
        break;
    case 184:
        frequency = 4920;
        break;
    case 185:
        frequency = 4925;
        break;
    case 187:
        frequency = 4935;
        break;
    case 188:
        frequency = 4940;
        break;
    case 189:
        frequency = 4945;
        break;
    case 192:
        frequency = 4960;
        break;
    case 196:
        frequency = 4980;
        break;
    case 7:
        frequency = 5035;
        break;
    case 8:
        frequency = 5040;
        break;
    case 9:
        frequency = 5045;
        break;
    case 11:
        frequency = 5055;
        break;
    case 12:
        frequency = 5060;
        break;
    case 16:
        frequency = 5080;
        break;
    case 34:
        frequency = 5170;
        break;
    case 36:
        frequency = 5180;
        break;
    case 38:
        frequency = 5190;
        break;
    case 40:
        frequency = 5200;
        break;
    case 42:
        frequency = 5210;
        break;
    case 44:
        frequency = 5220;
        break;
    case 46:
        frequency = 5230;
        break;
    case 48:
        frequency = 5240;
        break;
    case 52:
        frequency = 5260;
        break;
    case 56:
        frequency = 5280;
        break;
    case 60:
        frequency = 5300;
        break;
    case 64:
        frequency = 5320;
        break;
    case 100:
        frequency = 5500;
        break;
    case 104:
        frequency = 5520;
        break;
    case 108:
        frequency = 5540;
        break;
    case 112:
        frequency = 5560;
        break;
    case 116:
        frequency = 5580;
        break;
    case 120:
        frequency = 5600;
        break;
    case 124:
        frequency = 5620;
        break;
    case 128:
        frequency = 5640;
        break;
    case 132:
        frequency = 5660;
        break;
    case 136:
        frequency = 5680;
        break;
    case 140:
        frequency = 5700;
        break;
    case 149:
        frequency = 5745;
        break;
    case 153:
        frequency = 5765;
        break;
    case 157:
        frequency = 5785;
        break;
    case 161:
        frequency = 5805;
        break;
    case 165:
        frequency = 5825;
        break;
    }

    return frequency;
}

uint16_t ConnectivityUtils::MapChannelToFrequency(const uint16_t inBand, const uint8_t inChannel)
{
    uint16_t frequency = 0;

    if (inBand == kWiFi_BAND_2_4_GHZ)
    {
        frequency = Map2400MHz(inChannel);
    }
    else if (inBand == kWiFi_BAND_5_0_GHZ)
    {
        frequency = Map5000MHz(inChannel);
    }

    return frequency;
}

uint8_t ConnectivityUtils::MapFrequencyToChannel(const uint16_t frequency)
{
    if (frequency < 2412)
        return 0;

    if (frequency < 2484)
        return (frequency - 2407) / 5;

    if (frequency == 2484)
        return 14;

    return frequency / 5 - 1000;
}

/*
double ConnectivityUtils::ConvertFrequenceToFloat(const iw_freq * in)
{
    double result = (double) in->m;

    for (int i = 0; i < in->e; i++)
        result *= 10;

    return result;
}
*/
InterfaceType ConnectivityUtils::GetInterfaceConnectionType(const char * ifname)
{
    // MW320 only has the wifi interface
    InterfaceType ret = InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI;
    return ret;
}

CHIP_ERROR ConnectivityUtils::GetInterfaceHardwareAddrs(const char * ifname, uint8_t * buf, size_t bufSize)
{
    CHIP_ERROR err;
    wifi_mac_addr_t mac_addr;

    VerifyOrReturnError(bufSize >= 6, CHIP_ERROR_BUFFER_TOO_SMALL);
    wifi_get_device_mac_addr(&mac_addr);
    memcpy(buf, mac_addr.mac, 6);
    ChipLogProgress(DeviceLayer, "GetInterfaceHardwareAddrs: [%02x:%02x:%02x:%02x:%02x:%02x]", buf[0], buf[1], buf[2], buf[3],
                    buf[4], buf[5]);
    err = CHIP_NO_ERROR;
    return err;
}

CHIP_ERROR ConnectivityUtils::GetInterfaceIPv4Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp)
{
    CHIP_ERROR err;
    uint8_t index = 0;
    struct wlan_ip_config addr;
    uint8_t * pipv4;

    wlan_get_address(&addr);
    memcpy(ifp->Ipv4AddressesBuffer[index], &addr.ipv4.address, kMaxIPv4AddrSize);
    ifp->Ipv4AddressSpans[index] = ByteSpan(ifp->Ipv4AddressesBuffer[index], kMaxIPv4AddrSize);

    pipv4 = (uint8_t *) &addr.ipv4.address;
    ChipLogProgress(DeviceLayer, "GetInterfaceIPv4Addrs: [%u.%u.%u.%u]", pipv4[0], pipv4[1], pipv4[2], pipv4[3]);

    index++;
    err  = CHIP_NO_ERROR;
    size = index;

    return err;
}

CHIP_ERROR ConnectivityUtils::GetInterfaceIPv6Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp)
{
    CHIP_ERROR err;
    uint8_t i;
    uint8_t index = 0;
    int ret;

    ret = wlan_get_current_network(&sta_network);
    if (ret != WM_SUCCESS)
    {
        return CHIP_NO_ERROR;
    }

    for (i = 0; i < MAX_IPV6_ADDRESSES; i++)
    {
        if (sta_network.ip.ipv6[i].addr_state == IP6_ADDR_INVALID)
        {
            continue;
        }
        ChipLogProgress(DeviceLayer, "\t%-13s:\t%s (%s)", ipv6_addr_type_to_desc(&(sta_network.ip.ipv6[i])),
                        inet6_ntoa(sta_network.ip.ipv6[i].address), ipv6_addr_state_to_desc(sta_network.ip.ipv6[i].addr_state));
        memcpy(ifp->Ipv6AddressesBuffer[index], &sta_network.ip.ipv6[index].address, kMaxIPv6AddrSize);
        ifp->Ipv6AddressSpans[index] = ByteSpan(ifp->Ipv6AddressesBuffer[index], kMaxIPv6AddrSize);
        index++;
    }

    err  = CHIP_NO_ERROR;
    size = index;
    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiInterfaceName(char * ifname, size_t bufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    strncpy(ifname, "mlan0", bufSize);
    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiChannelNumber(const char * ifname, uint16_t & channelNumber)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    channelNumber  = wlan_get_current_channel();
    ChipLogProgress(DeviceLayer, "GetWiFiChannelNumber: [%u]", channelNumber);
    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiRssi(const char * ifname, int8_t & rssi)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    short dblevel;

    wlan_get_current_rssi(&dblevel);

    VerifyOrReturnError(dblevel <= INT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
    rssi = static_cast<int8_t>(dblevel);
    ChipLogProgress(DeviceLayer, "GetWiFiRssi: [%d]", rssi);
    err = CHIP_NO_ERROR;
    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiBeaconRxCount(const char * ifname, uint32_t & beaconRxCount)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret;
    wifi_pkt_stats_t stats;

    ret = wifi_get_log(&stats);
    if (ret != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "wifi_get_log failed ");
    }

    beaconRxCount = stats.bcn_rcv_cnt;
    ChipLogProgress(DeviceLayer, "GetWiFiBeaconRxCount [%ld] -> working in sdk", beaconRxCount);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiBeaconLostCount(const char * ifname, uint32_t & beaconLostCount)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret;
    wifi_pkt_stats_t stats;

    ret = wifi_get_log(&stats);
    if (ret != WM_SUCCESS)
    {
        ChipLogError(DeviceLayer, "wifi_get_log failed ");
    }

    beaconLostCount = stats.bcn_miss_cnt;
    ChipLogProgress(DeviceLayer, "GetWiFiBeaconLostCount [%ld] -> working in sdk", beaconLostCount);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiCurrentMaxRate(const char * ifname, uint64_t & currentMaxRate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    currentMaxRate = 24000000;
    ChipLogProgress(DeviceLayer, "GetWiFiCurrentMaxRate: %llu", currentMaxRate);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetEthInterfaceName(char * ifname, size_t bufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    strncpy(ifname, "mlan0", bufSize);
    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
