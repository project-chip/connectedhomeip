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
 *          Provides an implementation of the DiagnosticDataProvider object
 *          for Tizen platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMemString.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/Tizen/ConnectivityUtils.h>
#include <platform/Tizen/DiagnosticDataProviderImpl.h>

#include <errno.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <malloc.h>
#include <string.h>

using namespace ::chip::app;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::app::Clusters::GeneralDiagnostics;

namespace {

enum class EthernetStatsCountType
{
    kEthPacketRxCount,
    kEthPacketTxCount,
    kEthTxErrCount,
    kEthCollisionCount,
    kEthOverrunCount
};

enum class WiFiStatsCountType
{
    kWiFiUnicastPacketRxCount,
    kWiFiUnicastPacketTxCount,
    kWiFiMulticastPacketRxCount,
    kWiFiMulticastPacketTxCount,
    kWiFiOverrunCount
};

CHIP_ERROR GetEthernetStatsCount(EthernetStatsCountType type, uint64_t & count)
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
        if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceTypeEnum::kEthernet)
        {
            ChipLogProgress(DeviceLayer, "Found the primary Ethernet interface: %s", StringOrNullMarker(ifa->ifa_name));
            break;
        }
    }

    if (ifa != nullptr)
    {
        if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
        {
            struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;
            switch (type)
            {
            case EthernetStatsCountType::kEthPacketRxCount:
                count = stats->rx_packets;
                err   = CHIP_NO_ERROR;
                break;
            case EthernetStatsCountType::kEthPacketTxCount:
                count = stats->tx_packets;
                err   = CHIP_NO_ERROR;
                break;
            case EthernetStatsCountType::kEthTxErrCount:
                count = stats->tx_errors;
                err   = CHIP_NO_ERROR;
                break;
            case EthernetStatsCountType::kEthCollisionCount:
                count = stats->collisions;
                err   = CHIP_NO_ERROR;
                break;
            case EthernetStatsCountType::kEthOverrunCount:
                count = stats->rx_over_errors;
                err   = CHIP_NO_ERROR;
                break;
            default:
                ChipLogError(DeviceLayer, "Unknown Ethernet statistic metric type");
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR GetWiFiStatsCount(WiFiStatsCountType type, uint64_t & count)
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
        if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceTypeEnum::kWiFi)
        {
            ChipLogProgress(DeviceLayer, "Found the primary WiFi interface:%s", StringOrNullMarker(ifa->ifa_name));
            break;
        }
    }

    if (ifa != nullptr)
    {
        if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
        {
            struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;
            switch (type)
            {
            case WiFiStatsCountType::kWiFiUnicastPacketRxCount:
                count = stats->rx_packets;
                err   = CHIP_NO_ERROR;
                break;
            case WiFiStatsCountType::kWiFiUnicastPacketTxCount:
                count = stats->tx_packets;
                err   = CHIP_NO_ERROR;
                break;
            case WiFiStatsCountType::kWiFiMulticastPacketRxCount:
                count = stats->multicast;
                err   = CHIP_NO_ERROR;
                break;
            case WiFiStatsCountType::kWiFiMulticastPacketTxCount:
                count = 0;
                err   = CHIP_NO_ERROR;
                break;
            case WiFiStatsCountType::kWiFiOverrunCount:
                count = stats->rx_over_errors;
                err   = CHIP_NO_ERROR;
                break;
            default:
                ChipLogError(DeviceLayer, "Unknown WiFi statistic metric type");
                break;
            }
        }
    }

    freeifaddrs(ifaddr);

    return err;
}
#endif // #if CHIP_DEVICE_CONFIG_ENABLE_WIFI

} // namespace

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    struct mallinfo mallocInfo = mallinfo();
    currentHeapFree            = mallocInfo.fordblks;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    struct mallinfo mallocInfo = mallinfo();
    currentHeapUsed            = mallocInfo.uordblks;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics) {}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetUpTime(uint64_t & upTime)
{
    System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp startTime   = PlatformMgrImpl().GetStartTime();

    if (currentTime < startTime)
        return CHIP_ERROR_INVALID_TIME;

    upTime = std::chrono::duration_cast<System::Clock::Seconds64>(currentTime - startTime).count();
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetTotalOperationalHours(uint32_t & totalOperationalHours)
{
    uint64_t upTime = 0;

    if (GetUpTime(upTime) == CHIP_NO_ERROR)
    {
        uint32_t totalHours = 0;
        if (ConfigurationMgr().GetTotalOperationalHours(totalHours) == CHIP_NO_ERROR)
        {
            VerifyOrReturnError(upTime / 3600 <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
            totalOperationalHours = totalHours + static_cast<uint32_t>(upTime / 3600);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_INVALID_TIME;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgr().GetBootReason(reason);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    NetworkInterface * head = nullptr;
    for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET)
        {
            uint8_t size           = 0;
            NetworkInterface * ifp = new NetworkInterface();

            Platform::CopyString(ifp->Name, ifa->ifa_name);

            ifp->name          = CharSpan::fromCharString(ifp->Name);
            ifp->isOperational = ifa->ifa_flags & IFF_RUNNING;
            ifp->type          = ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name);
            ifp->offPremiseServicesReachableIPv4.SetNull();
            ifp->offPremiseServicesReachableIPv6.SetNull();

            if (ConnectivityUtils::GetInterfaceIPv4Addrs(ifa->ifa_name, size, ifp) == CHIP_NO_ERROR)
            {
                if (size > 0)
                {
                    ifp->IPv4Addresses = DataModel::List<const chip::ByteSpan>(ifp->Ipv4AddressSpans, size);
                }
            }

            if (ConnectivityUtils::GetInterfaceIPv6Addrs(ifa->ifa_name, size, ifp) == CHIP_NO_ERROR)
            {
                if (size > 0)
                {
                    ifp->IPv6Addresses = DataModel::List<const chip::ByteSpan>(ifp->Ipv6AddressSpans, size);
                }
            }

            if (ConnectivityUtils::GetInterfaceHardwareAddrs(ifa->ifa_name, ifp->MacAddress, kMaxHardwareAddrSize) != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Failed to get network hardware address");
            }
            else
            {
                // Set 48-bit IEEE MAC Address
                ifp->hardwareAddress = ByteSpan(ifp->MacAddress, 6);
            }

            ifp->Next = head;
            head      = ifp;
        }
    }

    if (head != nullptr)
    {
        *netifpp = head;
        err      = CHIP_NO_ERROR;
    }

    freeifaddrs(ifaddr);
    return err;
}

void DiagnosticDataProviderImpl::ReleaseNetworkInterfaces(NetworkInterface * netifp)
{
    while (netifp)
    {
        NetworkInterface * del = netifp;
        netifp                 = netifp->Next;
        delete del;
    }
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthPHYRate(app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum & pHYRate)
{
    if (ConnectivityMgrImpl().GetEthernetIfName() == nullptr)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetEthPHYRate(ConnectivityMgrImpl().GetEthernetIfName(), pHYRate);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthFullDuplex(bool & fullDuplex)
{
    if (ConnectivityMgrImpl().GetEthernetIfName() == nullptr)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetEthFullDuplex(ConnectivityMgrImpl().GetEthernetIfName(), fullDuplex);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthTimeSinceReset(uint64_t & timeSinceReset)
{
    return GetDiagnosticDataProvider().GetUpTime(timeSinceReset);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthPacketRxCount(uint64_t & packetRxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthPacketRxCount, count));
    VerifyOrReturnError(count >= mEthPacketRxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetRxCount = count - mEthPacketRxCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthPacketTxCount(uint64_t & packetTxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthPacketTxCount, count));
    VerifyOrReturnError(count >= mEthPacketTxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetTxCount = count - mEthPacketTxCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthTxErrCount(uint64_t & txErrCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthTxErrCount, count));
    VerifyOrReturnError(count >= mEthTxErrCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    txErrCount = count - mEthTxErrCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthCollisionCount(uint64_t & collisionCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthCollisionCount, count));
    VerifyOrReturnError(count >= mEthCollisionCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    collisionCount = count - mEthCollisionCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetEthOverrunCount(uint64_t & overrunCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetEthernetStatsCount(EthernetStatsCountType::kEthOverrunCount, count));
    VerifyOrReturnError(count >= mEthOverrunCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    overrunCount = count - mEthOverrunCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetEthNetworkDiagnosticsCounts()
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
        if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceTypeEnum::kEthernet)
        {
            ChipLogProgress(DeviceLayer, "Found the primary Ethernet interface:%s", StringOrNullMarker(ifa->ifa_name));
            break;
        }
    }

    if (ifa != nullptr)
    {
        if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
        {
            struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;

            mEthPacketRxCount  = stats->rx_packets;
            mEthPacketTxCount  = stats->tx_packets;
            mEthTxErrCount     = stats->tx_errors;
            mEthCollisionCount = stats->collisions;
            mEthOverrunCount   = stats->rx_over_errors;
            err                = CHIP_NO_ERROR;
        }
    }

    freeifaddrs(ifaddr);
    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & bssId)
{
    return ConnectivityMgrImpl().GetWiFiBssId(bssId);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    return ConnectivityMgrImpl().GetWiFiSecurityType(securityType);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion)
{
    return ConnectivityMgrImpl().GetWiFiVersion(wiFiVersion);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
        return CHIP_ERROR_READ_FAILED;

    return ConnectivityUtils::GetWiFiChannelNumber(ConnectivityMgrImpl().GetWiFiIfName(), channelNumber);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
        return CHIP_ERROR_READ_FAILED;

    return ConnectivityUtils::GetWiFiRssi(ConnectivityMgrImpl().GetWiFiIfName(), rssi);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    uint32_t count;

    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
        return CHIP_ERROR_READ_FAILED;

    ReturnErrorOnFailure(ConnectivityUtils::GetWiFiBeaconLostCount(ConnectivityMgrImpl().GetWiFiIfName(), count));
    VerifyOrReturnError(count >= mBeaconLostCount, CHIP_ERROR_INVALID_INTEGER_VALUE);
    beaconLostCount = count - mBeaconLostCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
        return CHIP_ERROR_READ_FAILED;

    return ConnectivityUtils::GetWiFiCurrentMaxRate(ConnectivityMgrImpl().GetWiFiIfName(), currentMaxRate);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiMulticastPacketRxCount, count));
    VerifyOrReturnError(count >= mPacketMulticastRxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketMulticastRxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetMulticastRxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiMulticastPacketTxCount, count));
    VerifyOrReturnError(count >= mPacketMulticastTxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketMulticastTxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetMulticastTxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiUnicastPacketRxCount, count));
    VerifyOrReturnError(count >= mPacketUnicastRxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketUnicastRxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetUnicastRxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiUnicastPacketTxCount, count));
    VerifyOrReturnError(count >= mPacketUnicastTxCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    count -= mPacketUnicastTxCount;
    VerifyOrReturnError(count <= UINT32_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);

    packetUnicastTxCount = static_cast<uint32_t>(count);

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    uint64_t count;

    ReturnErrorOnFailure(GetWiFiStatsCount(WiFiStatsCountType::kWiFiOverrunCount, count));
    VerifyOrReturnError(count >= mOverrunCount, CHIP_ERROR_INVALID_INTEGER_VALUE);

    overrunCount = count - mOverrunCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    ReturnErrorOnFailure(GetWiFiBeaconLostCount(mBeaconLostCount));

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces: %s", strerror(errno));
        return CHIP_ERROR_POSIX(errno);
    }

    struct ifaddrs * ifa = nullptr;
    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceTypeEnum::kWiFi)
        {
            ChipLogProgress(DeviceLayer, "Found the primary WiFi interface:%s", StringOrNullMarker(ifa->ifa_name));
            break;
        }
    }

    if (ifa != nullptr)
    {
        if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
        {
            struct rtnl_link_stats * stats = (struct rtnl_link_stats *) ifa->ifa_data;

            mPacketMulticastRxCount = stats->multicast;
            mPacketMulticastTxCount = 0;
            mPacketUnicastRxCount   = stats->rx_packets;
            mPacketUnicastTxCount   = stats->tx_packets;
            mOverrunCount           = stats->rx_over_errors;

            err = CHIP_NO_ERROR;
        }
    }

    freeifaddrs(ifaddr);
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
