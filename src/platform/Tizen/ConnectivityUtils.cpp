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

#include "ConnectivityUtils.h"

#include <errno.h>
#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <linux/wireless.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>

#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/Tizen/ConnectivityUtils.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

using namespace ::chip::app::Clusters::GeneralDiagnostics;
using namespace ::chip::app::Clusters::EthernetNetworkDiagnostics;

namespace {

constexpr uint16_t Map2400MHz(const uint8_t inChannel)
{
    if (inChannel >= 1 && inChannel <= 13)
        return static_cast<uint16_t>(2412 + ((inChannel - 1) * 5));
    if (inChannel == 14)
        return 2484;
    return 0;
}

constexpr uint8_t MapFrequencyToChannel(const uint16_t frequency)
{
    if (frequency < 2412)
        return 0;
    if (frequency < 2484)
        return static_cast<uint8_t>((frequency - 2407) / 5);
    if (frequency == 2484)
        return 14;
    return static_cast<uint8_t>(frequency / 5 - 1000);
}

constexpr uint16_t Map5000MHz(const uint8_t inChannel)
{
    switch (inChannel)
    {
    case 183:
        return 4915;
    case 184:
        return 4920;
    case 185:
        return 4925;
    case 187:
        return 4935;
    case 188:
        return 4940;
    case 189:
        return 4945;
    case 192:
        return 4960;
    case 196:
        return 4980;
    case 7:
        return 5035;
    case 8:
        return 5040;
    case 9:
        return 5045;
    case 11:
        return 5055;
    case 12:
        return 5060;
    case 16:
        return 5080;
    case 34:
        return 5170;
    case 36:
        return 5180;
    case 38:
        return 5190;
    case 40:
        return 5200;
    case 42:
        return 5210;
    case 44:
        return 5220;
    case 46:
        return 5230;
    case 48:
        return 5240;
    case 52:
        return 5260;
    case 56:
        return 5280;
    case 60:
        return 5300;
    case 64:
        return 5320;
    case 100:
        return 5500;
    case 104:
        return 5520;
    case 108:
        return 5540;
    case 112:
        return 5560;
    case 116:
        return 5580;
    case 120:
        return 5600;
    case 124:
        return 5620;
    case 128:
        return 5640;
    case 132:
        return 5660;
    case 136:
        return 5680;
    case 140:
        return 5700;
    case 149:
        return 5745;
    case 153:
        return 5765;
    case 157:
        return 5785;
    case 161:
        return 5805;
    case 165:
        return 5825;
    default:
        return 0;
    }
}

constexpr double ConvertFrequencyToFloat(const iw_freq * in)
{
    double result = (double) in->m;
    for (int i = 0; i < in->e; i++)
        result *= 10;
    return result;
}

CHIP_ERROR GetWiFiParameter(int sock,            /* Socket to the kernel */
                            const char * ifname, /* Device name */
                            int request,         /* WE ID */
                            struct iwreq * pwrq) /* Fixed part of the request */
{
    chip::Platform::CopyString(pwrq->ifr_name, ifname);
    if (ioctl(sock, request, pwrq) == -1)
        return CHIP_ERROR_POSIX(errno);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GetWiFiStats(int sock, const char * ifname, struct iw_statistics * stats)
{
    struct iwreq wrq   = {};
    wrq.u.data.pointer = (caddr_t) stats;
    wrq.u.data.length  = sizeof(*stats);
    wrq.u.data.flags   = 1; /* Clear updated flag */
    return GetWiFiParameter(sock, ifname, SIOCGIWSTATS, &wrq);
}

} // namespace

namespace chip {
namespace DeviceLayer {
namespace Internal {
namespace ConnectivityUtils {

InterfaceTypeEnum GetInterfaceConnectionType(const char * ifname)
{
    InterfaceTypeEnum ret = InterfaceTypeEnum::kUnspecified;
    int sock              = -1;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno));
        return ret;
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

CHIP_ERROR GetInterfaceHardwareAddrs(const char * ifname, uint8_t * buf, size_t bufSize)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    struct ifreq req;
    Platform::CopyString(req.ifr_name, ifname);
    VerifyOrExit(ioctl(sock, SIOCGIFHWADDR, &req) != -1, err = CHIP_ERROR_POSIX(errno);
                 ChipLogError(DeviceLayer, "Failed to get hardware address: %s", strerror(errno)));

    // Copy 48-bit IEEE MAC Address
    VerifyOrExit(bufSize >= 6, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    memset(buf, 0, bufSize);
    memcpy(buf, req.ifr_ifru.ifru_hwaddr.sa_data, 6);

exit:
    close(sock);
    return err;
}

CHIP_ERROR GetInterfaceIPv4Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    uint8_t index = 0;
    for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET)
        {
            if (strcmp(ifname, ifa->ifa_name) == 0)
            {
                void * addPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;

                memcpy(ifp->Ipv4AddressesBuffer[index], addPtr, kMaxIPv4AddrSize);
                ifp->Ipv4AddressSpans[index] = ByteSpan(ifp->Ipv4AddressesBuffer[index], kMaxIPv4AddrSize);
                index++;

                if (index >= kMaxIPv4AddrCount)
                {
                    break;
                }
            }
        }
    }

    if (index > 0)
    {
        err  = CHIP_NO_ERROR;
        size = index;
    }

    freeifaddrs(ifaddr);
    return err;
}

CHIP_ERROR GetInterfaceIPv6Addrs(const char * ifname, uint8_t & size, NetworkInterface * ifp)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    uint8_t index = 0;
    for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET6)
        {
            if (strcmp(ifname, ifa->ifa_name) == 0)
            {
                void * addPtr = &((struct sockaddr_in6 *) ifa->ifa_addr)->sin6_addr;

                memcpy(ifp->Ipv6AddressesBuffer[index], addPtr, kMaxIPv6AddrSize);
                ifp->Ipv6AddressSpans[index] = ByteSpan(ifp->Ipv6AddressesBuffer[index], kMaxIPv6AddrSize);
                index++;

                if (index >= kMaxIPv6AddrCount)
                {
                    break;
                }
            }
        }
    }

    if (index > 0)
    {
        err  = CHIP_NO_ERROR;
        size = index;
    }

    freeifaddrs(ifaddr);
    return err;
}

CHIP_ERROR GetWiFiInterfaceName(char * ifname, size_t bufSize)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    struct ifaddrs * ifa = nullptr;
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
    return err;
}

CHIP_ERROR GetWiFiChannelNumber(const char * ifname, uint16_t & channelNumber)
{
    CHIP_ERROR err;
    struct iwreq wrq;
    double freq;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    err = GetWiFiParameter(sock, ifname, SIOCGIWFREQ, &wrq);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(DeviceLayer, "Failed to get channel/frequency: %" CHIP_ERROR_FORMAT, err.Format()));

    freq = ConvertFrequencyToFloat(&(wrq.u.freq));
    VerifyOrExit((freq / 1000000) <= UINT16_MAX, err = CHIP_ERROR_INVALID_INTEGER_VALUE);
    channelNumber = MapFrequencyToChannel(static_cast<uint16_t>(freq / 1000000));

exit:
    close(sock);
    return err;
}

CHIP_ERROR GetWiFiRssi(const char * ifname, int8_t & rssi)
{
    CHIP_ERROR err;
    struct iw_statistics stats;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    err = GetWiFiStats(sock, ifname, &stats);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to get Wi-Fi stats: %" CHIP_ERROR_FORMAT, err.Format()));

    {
        struct iw_quality * qual = &stats.qual;
        if (qual->updated & IW_QUAL_RCPI)
        {
            /* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
            if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
            {
                double rcpilevel = (qual->level / 2.0) - 110.0;
                VerifyOrExit(rcpilevel <= INT8_MAX, err = CHIP_ERROR_INVALID_INTEGER_VALUE);
                rssi = static_cast<int8_t>(rcpilevel);
            }
        }
        else
        {
            if (qual->updated & IW_QUAL_DBM)
            {
                if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
                {
                    int dblevel = qual->level;
                    /* dBm[-192; 63] */
                    if (qual->level >= 64)
                        dblevel -= 0x100;

                    VerifyOrExit(dblevel <= INT8_MAX, err = CHIP_ERROR_INVALID_INTEGER_VALUE);
                    rssi = static_cast<int8_t>(dblevel);
                }
            }
            else
            {
                if (!(qual->updated & IW_QUAL_LEVEL_INVALID))
                {
                    VerifyOrExit(qual->level <= INT8_MAX, err = CHIP_ERROR_INVALID_INTEGER_VALUE);
                    rssi = static_cast<int8_t>(qual->level);
                }
            }
        }
    }

exit:
    close(sock);
    return err;
}

CHIP_ERROR GetWiFiBeaconLostCount(const char * ifname, uint32_t & beaconLostCount)
{
    CHIP_ERROR err;
    struct iw_statistics stats;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    err = GetWiFiStats(sock, ifname, &stats);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to get Wi-Fi stats: %" CHIP_ERROR_FORMAT, err.Format()));

    beaconLostCount = stats.miss.beacon;

exit:
    close(sock);
    return err;
}

CHIP_ERROR GetWiFiCurrentMaxRate(const char * ifname, uint64_t & currentMaxRate)
{
    CHIP_ERROR err;
    struct iwreq wrq;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    err = GetWiFiParameter(sock, ifname, SIOCGIWRATE, &wrq);
    VerifyOrExit(err == CHIP_NO_ERROR,
                 ChipLogError(DeviceLayer, "Failed to get channel/frequency: %" CHIP_ERROR_FORMAT, err.Format()));

    currentMaxRate = wrq.u.bitrate.value;

exit:
    close(sock);
    return err;
}

CHIP_ERROR GetEthInterfaceName(char * ifname, size_t bufSize)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    struct ifaddrs * ifa = nullptr;
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
    return err;
}

CHIP_ERROR GetEthPHYRate(const char * ifname, PHYRateEnum & pHYRate)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    uint32_t speed          = 0;
    struct ethtool_cmd ecmd = {};
    ecmd.cmd                = ETHTOOL_GSET;
    struct ifreq ifr        = {};

    ifr.ifr_data = reinterpret_cast<char *>(&ecmd);
    Platform::CopyString(ifr.ifr_name, ifname);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    VerifyOrExit(ioctl(sock, SIOCETHTOOL, &ifr) != -1, err = CHIP_ERROR_POSIX(errno);
                 ChipLogError(DeviceLayer, "Cannot get device settings: %s", strerror(errno)));

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

exit:
    close(sock);
    return err;
}

CHIP_ERROR GetEthFullDuplex(const char * ifname, bool & fullDuplex)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    struct ethtool_cmd ecmd = {};
    ecmd.cmd                = ETHTOOL_GSET;
    struct ifreq ifr        = {};

    ifr.ifr_data = reinterpret_cast<char *>(&ecmd);
    Platform::CopyString(ifr.ifr_name, ifname);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    VerifyOrReturnError(sock != -1, CHIP_ERROR_POSIX(errno),
                        ChipLogError(DeviceLayer, "Failed to create INET socket: %s", strerror(errno)));

    VerifyOrExit(ioctl(sock, SIOCETHTOOL, &ifr) != -1, err = CHIP_ERROR_POSIX(errno);
                 ChipLogError(DeviceLayer, "Cannot get device settings: %s", strerror(errno)));

    fullDuplex = ecmd.duplex == DUPLEX_FULL;

exit:
    close(sock);
    return err;
}

} // namespace ConnectivityUtils
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
