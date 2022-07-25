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
 *          for Linux platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <app-common/zap-generated/enums.h>
#include <app/data-model/List.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/Linux/ConnectivityUtils.h>
#include <platform/Linux/DiagnosticDataProviderImpl.h>

#include <arpa/inet.h>
#include <dirent.h>
#include <ifaddrs.h>
#include <linux/ethtool.h>
#include <linux/if_link.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <malloc.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::TLV;
using namespace ::chip::DeviceLayer;
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
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET)
            {
                ChipLogProgress(DeviceLayer, "Found the primary Ethernet interface:%s", ifa->ifa_name);
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
    }

    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR GetWiFiStatsCount(WiFiStatsCountType type, uint64_t & count)
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

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI)
            {
                ChipLogProgress(DeviceLayer, "Found the primary WiFi interface:%s", ifa->ifa_name);
                break;
            }
        }

        if (ifa != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_PACKET && ifa->ifa_data != nullptr)
            {
                // The usecase of this function is embedded devices,on which we can interact with the WiFi
                // driver to get the accurate number of muticast and unicast packets accurately.
                // On Linux simulation, we can only get the total packets received, the total bytes transmitted,
                // the multicast packets received and receiver ring buff overflow.

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
    }

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
#ifndef __GLIBC__
    return CHIP_ERROR_NOT_IMPLEMENTED;
#else
    struct mallinfo mallocInfo = mallinfo();

    // Get the current amount of heap memory, in bytes, that are not being utilized
    // by the current running program.
    currentHeapFree = mallocInfo.fordblks;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
#ifndef __GLIBC__
    return CHIP_ERROR_NOT_IMPLEMENTED;
#else
    struct mallinfo mallocInfo = mallinfo();

    // Get the current amount of heap memory, in bytes, that are being used by
    // the current running program.
    currentHeapUsed = mallocInfo.uordblks;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
#ifndef __GLIBC__
    return CHIP_ERROR_NOT_IMPLEMENTED;
#else
    struct mallinfo mallocInfo = mallinfo();

    // The usecase of this function is embedded devices,on which we would need to intercept
    // malloc/calloc/free and then record the maximum amount of heap memory,in bytes, that
    // has been used by the Node.
    // On Linux, since it uses virtual memory, whereby a page of memory could be copied to
    // the hard disk, called swap space, and free up that page of memory. So it is impossible
    // to know accurately peak physical memory it use. We just return the current heap memory
    // being used by the current running program.
    currentHeapHighWatermark = mallocInfo.uordblks;

    return CHIP_NO_ERROR;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
    // If implemented, the server SHALL set the value of the CurrentHeapHighWatermark attribute to the
    // value of the CurrentHeapUsed.

    // On Linux, the write operation is non-op since we always rely on the mallinfo system
    // function to get the current heap memory.

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    CHIP_ERROR err = CHIP_ERROR_READ_FAILED;
    DIR * proc_dir = opendir("/proc/self/task");

    if (proc_dir == nullptr)
    {
        ChipLogError(DeviceLayer, "Failed to open current process task directory");
    }
    else
    {
        ThreadMetrics * head = nullptr;
        struct dirent * entry;

        /* proc available, iterate through tasks... */
        while ((entry = readdir(proc_dir)) != nullptr)
        {
            if (entry->d_name[0] == '.')
                continue;

            ThreadMetrics * thread = new ThreadMetrics();

            strncpy(thread->NameBuf, entry->d_name, kMaxThreadNameLength);
            thread->NameBuf[kMaxThreadNameLength] = '\0';
            thread->name.Emplace(CharSpan::fromCharString(thread->NameBuf));
            thread->id = atoi(entry->d_name);

            // TODO: Get stack info of each thread: thread->stackFreeCurrent,
            // thread->stackFreeMinimum, thread->stackSize.

            thread->Next = head;
            head         = thread;
        }

        closedir(proc_dir);

        *threadMetricsOut = head;
        err               = CHIP_NO_ERROR;
    }

    return err;
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics)
{
    while (threadMetrics)
    {
        ThreadMetrics * del = threadMetrics;
        threadMetrics       = threadMetrics->Next;
        delete del;
    }
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;

    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(count <= UINT16_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        rebootCount = static_cast<uint16_t>(count);
    }

    return err;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetUpTime(uint64_t & upTime)
{
    System::Clock::Timestamp currentTime = System::SystemClock().GetMonotonicTimestamp();
    System::Clock::Timestamp startTime   = PlatformMgrImpl().GetStartTime();

    if (currentTime >= startTime)
    {
        upTime = std::chrono::duration_cast<System::Clock::Seconds64>(currentTime - startTime).count();
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_INVALID_TIME;
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
#if CHIP_CONFIG_TEST
    // On Linux Simulation, set following hardware faults statically.
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_RADIO));
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_SENSOR));
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_POWER_SOURCE));
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_TYPE_USER_INTERFACE_FAULT));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
#if CHIP_CONFIG_TEST
    // On Linux Simulation, set following radio faults statically.
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_TYPE_WI_FI_FAULT));
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_TYPE_CELLULAR_FAULT));
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_TYPE_NFC_FAULT));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
#if CHIP_CONFIG_TEST
    // On Linux Simulation, set following radio faults statically.
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_TYPE_CONNECTION_FAILED));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    CHIP_ERROR err          = CHIP_ERROR_READ_FAILED;
    struct ifaddrs * ifaddr = nullptr;

    if (getifaddrs(&ifaddr) == -1)
    {
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        NetworkInterface * head = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (struct ifaddrs * ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET)
            {
                uint8_t size           = 0;
                NetworkInterface * ifp = new NetworkInterface();

                strncpy(ifp->Name, ifa->ifa_name, Inet::InterfaceId::kMaxIfNameLength);
                ifp->Name[Inet::InterfaceId::kMaxIfNameLength - 1] = '\0';

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

                if (ConnectivityUtils::GetInterfaceHardwareAddrs(ifa->ifa_name, ifp->MacAddress, kMaxHardwareAddrSize) !=
                    CHIP_NO_ERROR)
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

        *netifpp = head;
        err      = CHIP_NO_ERROR;

        freeifaddrs(ifaddr);
    }

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

CHIP_ERROR DiagnosticDataProviderImpl::GetEthPHYRate(app::Clusters::EthernetNetworkDiagnostics::PHYRateType & pHYRate)
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
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_ETHERNET)
            {
                ChipLogProgress(DeviceLayer, "Found the primary Ethernet interface:%s", ifa->ifa_name);
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
    }

    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetWiFiChannelNumber(ConnectivityMgrImpl().GetWiFiIfName(), channelNumber);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    return ConnectivityUtils::GetWiFiRssi(ConnectivityMgrImpl().GetWiFiIfName(), rssi);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    uint32_t count;

    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
    {
        return CHIP_ERROR_READ_FAILED;
    }

    ReturnErrorOnFailure(ConnectivityUtils::GetWiFiBeaconLostCount(ConnectivityMgrImpl().GetWiFiIfName(), count));
    VerifyOrReturnError(count >= mBeaconLostCount, CHIP_ERROR_INVALID_INTEGER_VALUE);
    beaconLostCount = count - mBeaconLostCount;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    if (ConnectivityMgrImpl().GetWiFiIfName() == nullptr)
    {
        return CHIP_ERROR_READ_FAILED;
    }

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
        ChipLogError(DeviceLayer, "Failed to get network interfaces");
    }
    else
    {
        struct ifaddrs * ifa = nullptr;

        // Walk through linked list, maintaining head pointer so we can free list later.
        for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
        {
            if (ConnectivityUtils::GetInterfaceConnectionType(ifa->ifa_name) == InterfaceType::EMBER_ZCL_INTERFACE_TYPE_WI_FI)
            {
                ChipLogProgress(DeviceLayer, "Found the primary WiFi interface:%s", ifa->ifa_name);
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
    }

    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(uint8_t & wiFiVersion)
{
    return ConnectivityMgrImpl().GetWiFiVersion(wiFiVersion);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(ByteSpan & value)
{
    return ConnectivityMgrImpl().GetWiFiBssId(value);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiSecurityType(uint8_t & securityType)
{
    return ConnectivityMgrImpl().GetWiFiSecurityType(securityType);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
