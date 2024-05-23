/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
 *          statistics(extracted from /proc/net/wireless) on webOS platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/webos/ConnectivityUtils.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/if_link.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <lib/core/CHIPEncoding.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>

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

double ConnectivityUtils::ConvertFrequenceToFloat(const iw_freq * in)
{
    double result = (double) in->m;

    for (int i = 0; i < in->e; i++)
        result *= 10;

    return result;
}

InterfaceTypeEnum ConnectivityUtils::GetInterfaceConnectionType(const char * ifname)
{
    InterfaceTypeEnum ret = InterfaceTypeEnum::kUnspecified;
    int sock              = -1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to open socket");
        return InterfaceTypeEnum::kUnspecified;
    }

    // Test wireless extensions for CONNECTION_WIFI
    struct iwreq pwrq = {};
    Platform::CopyString(pwrq.ifr_name, ifname);

    if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1)
    {
        ret = InterfaceTypeEnum::kWiFi;
    }
    else if ((strncmp(ifname, "en", 2) == 0) || (strncmp(ifname, "eth", 3) == 0))
    {
        struct ethtool_cmd ecmd = {};
        ecmd.cmd                = ETHTOOL_GSET;
        struct ifreq ifr        = {};
        ifr.ifr_data            = reinterpret_cast<char *>(&ecmd);
        Platform::CopyString(ifr.ifr_name, ifname);

        if (ioctl(sock, SIOCETHTOOL, &ifr) != -1)
            ret = InterfaceTypeEnum::kEthernet;
    }

    close(sock);

    return ret;
}

CHIP_ERROR ConnectivityUtils::GetInterfaceHardwareAddrs(const char * ifname, uint8_t * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;
    int skfd;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (ifname[0] != '\0')
    {
        struct ifreq req;

        strcpy(req.ifr_name, ifname);
        if (ioctl(skfd, SIOCGIFHWADDR, &req) != -1)
        {
            // Copy 48-bit IEEE MAC Address
            VerifyOrReturnError(bufSize >= 6, CHIP_ERROR_BUFFER_TOO_SMALL);

            memset(buf, 0, bufSize);
            memcpy(buf, req.ifr_ifru.ifru_hwaddr.sa_data, 6);
            err = CHIP_NO_ERROR;
        }
    }

    close(skfd);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiInterfaceName(char * ifname, size_t bufSize)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        /* Walk through linked list, maintaining head pointer so we
          can free list later */
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (GetInterfaceConnectionType(ifa->ifa_name) == InterfaceTypeEnum::kWiFi)
            {
                Platform::CopyString(ifname, bufSize, ifa->ifa_name);
                err = CHIP_NO_ERROR;
                break;
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiParameter(int skfd,            /* Socket to the kernel */
                                               const char * ifname, /* Device name */
                                               int request,         /* WE ID */
                                               struct iwreq * pwrq) /* Fixed part of the request */
{
    /* Set device name */
    Platform::CopyString(pwrq->ifr_name, ifname);

    /* Do the request */
    if (ioctl(skfd, request, pwrq) < 0)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ConnectivityUtils::GetWiFiStats(int skfd, const char * ifname, struct iw_statistics * stats)
{
    struct iwreq wrq;

    wrq.u.data.pointer = (caddr_t) stats;
    wrq.u.data.length  = sizeof(struct iw_statistics);
    wrq.u.data.flags   = 1; /*Clear updated flag */
    Platform::CopyString(wrq.ifr_name, ifname);

    return GetWiFiParameter(skfd, ifname, SIOCGIWSTATS, &wrq);
}

CHIP_ERROR ConnectivityUtils::GetWiFiChannelNumber(const char * ifname, uint16_t & channelNumber)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;

    struct iwreq wrq;
    int skfd;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (GetWiFiParameter(skfd, ifname, SIOCGIWFREQ, &wrq) == CHIP_NO_ERROR)
    {
        double freq = ConvertFrequenceToFloat(&(wrq.u.freq));
        VerifyOrReturnError((freq / 1000000) <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        channelNumber = MapFrequencyToChannel(static_cast<uint16_t>(freq / 1000000));

        err = CHIP_NO_ERROR;
    }

    close(skfd);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiRssi(const char * ifname, int8_t & rssi)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;
    struct iw_statistics stats;
    int skfd;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (GetWiFiStats(skfd, ifname, &stats) == CHIP_NO_ERROR)
    {
        struct iw_quality * qual = &stats.qual;

        /* Check if the statistics are in RCPI (IEEE 802.11k) */
        if (qual->updated & IW_QUAL_RCPI)
        { /* Deal with signal level in RCPI */
            /* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
            if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
            {
                double rcpilevel = (qual->level / 2.0) - 110.0;
                VerifyOrReturnError(rcpilevel <= INT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
                rssi = static_cast<int8_t>(rcpilevel);
                err  = CHIP_NO_ERROR;
            }
        }
        else
        { /* Check if the statistics are in dBm */
            if (qual->updated & IW_QUAL_DBM)
            { /* Deal with signal level in dBm  (absolute power measurement) */
                if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
                {
                    int dblevel = qual->level;
                    /* Implement a range for dBm[-192; 63] */
                    if (qual->level >= 64)
                        dblevel -= 0x100;

                    VerifyOrReturnError(dblevel <= INT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
                    rssi = static_cast<int8_t>(dblevel);
                    err  = CHIP_NO_ERROR;
                }
            }
            else
            { /* Deal with signal level as relative value (0 -> max) */
                if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
                {
                    VerifyOrReturnError(qual->level <= INT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
                    rssi = static_cast<int8_t>(qual->level);
                    err  = CHIP_NO_ERROR;
                }
            }
        }
    }

    close(skfd);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiBeaconLostCount(const char * ifname, uint32_t & beaconLostCount)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;
    struct iw_statistics stats;
    int skfd;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (GetWiFiStats(skfd, ifname, &stats) == CHIP_NO_ERROR)
    {
        beaconLostCount = stats.miss.beacon;
        err             = CHIP_NO_ERROR;
    }

    close(skfd);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetWiFiCurrentMaxRate(const char * ifname, uint64_t & currentMaxRate)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;
    struct iwreq wrq;
    int skfd;

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (GetWiFiParameter(skfd, ifname, SIOCGIWRATE, &wrq) == CHIP_NO_ERROR)
    {
        currentMaxRate = wrq.u.bitrate.value;
        err            = CHIP_NO_ERROR;
    }

    close(skfd);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetEthInterfaceName(char * ifname, size_t bufSize)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        /* Walk through linked list, maintaining head pointer so we
          can free list later */
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (GetInterfaceConnectionType(ifa->ifa_name) == InterfaceTypeEnum::kEthernet)
            {
                Platform::CopyString(ifname, bufSize, ifa->ifa_name);
                err = CHIP_NO_ERROR;
                break;
            }
        }

        freeifaddrs(ifaddr);
    }

    return err;
}

CHIP_ERROR ConnectivityUtils::GetEthPHYRate(const char * ifname, app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & pHYRate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    int skfd;
    uint32_t speed          = 0;
    struct ethtool_cmd ecmd = {};
    ecmd.cmd                = ETHTOOL_GSET;
    struct ifreq ifr        = {};

    ifr.ifr_data = reinterpret_cast<char *>(&ecmd);
    Platform::CopyString(ifr.ifr_name, ifname);

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1)
    {
        ChipLogError(DeviceLayer, "Cannot get device settings");
        close(skfd);
        return CHIP_ERROR_READ_FAILED;
    }

    speed = (ecmd.speed_hi << 16) | ecmd.speed;
    switch (speed)
    {
    case 10:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;
        break;
    case 100:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate100M;
        break;
    case 1000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate1G;
        break;
    case 25000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate25g;
        break;
    case 5000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate5G;
        break;
    case 10000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate10G;
        break;
    case 40000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate40G;
        break;
    case 100000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate100G;
        break;
    case 200000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate200G;
        break;
    case 400000:
        pHYRate = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate400G;
        break;
    default:
        ChipLogError(DeviceLayer, "Undefined speed! (%d)\n", speed);
        err = CHIP_ERROR_READ_FAILED;
        break;
    };

    close(skfd);

    return err;
}

CHIP_ERROR ConnectivityUtils::GetEthFullDuplex(const char * ifname, bool & fullDuplex)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;

    int skfd;
    struct ethtool_cmd ecmd = {};
    ecmd.cmd                = ETHTOOL_GSET;
    struct ifreq ifr        = {};

    ifr.ifr_data = reinterpret_cast<char *>(&ecmd);
    Platform::CopyString(ifr.ifr_name, ifname);

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create a channel to the NET kernel.");
        return CHIP_ERROR_OPEN_FAILED;
    }

    if (ioctl(skfd, SIOCETHTOOL, &ifr) == -1)
    {
        ChipLogError(DeviceLayer, "Cannot get device settings");
        err = CHIP_ERROR_READ_FAILED;
    }
    else
    {
        fullDuplex = ecmd.duplex == DUPLEX_FULL;
        err        = CHIP_NO_ERROR;
    }

    close(skfd);

    return err;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
