/*
 *
 *    Copyright (c) 2022-2025 Project CHIP Authors
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
 *          for Zephyr platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMemString.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/Zephyr/DiagnosticDataProviderImpl.h>
#include <platform/Zephyr/SysHeapMalloc.h>

#include <zephyr/drivers/hwinfo.h>
#include <zephyr/sys/util.h>

#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#include <platform/nrfconnect/Reboot.h>
#elif CHIP_DEVICE_LAYER_TARGET_TELINK
#include <platform/telink/Reboot.h>
#elif defined(CONFIG_MCUBOOT_IMG_MANAGER)
#include <zephyr/dfu/mcuboot.h>
#endif

#ifdef CONFIG_WIFI
#include <platform/Zephyr/InetUtils.h>
#include <platform/Zephyr/wifi/WiFiManager.h>
#endif

#include <malloc.h>

#if CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO

#ifdef CONFIG_NEWLIB_LIBC_ALIGNED_HEAP_SIZE
const size_t kMaxHeapSize = CONFIG_NEWLIB_LIBC_ALIGNED_HEAP_SIZE;
#elif defined(CONFIG_NEWLIB_LIBC)
extern char _end[];
const size_t kMaxHeapSize = CONFIG_SRAM_BASE_ADDRESS + KB(CONFIG_SRAM_SIZE) - POINTER_TO_UINT(_end);
#else
#pragma error "Maximum heap size is required but unknown"
#endif

#endif

namespace chip {
namespace DeviceLayer {

namespace {

static void GetThreadInfo(const struct k_thread * thread, void * user_data)
{
    size_t unusedStackSize;
    ThreadMetrics ** threadMetricsListHead = static_cast<ThreadMetrics **>(user_data);
    ThreadMetrics * threadMetrics          = Platform::New<ThreadMetrics>();

    VerifyOrReturn(threadMetrics != NULL, ChipLogError(DeviceLayer, "Failed to allocate ThreadMetrics"));

#if defined(CONFIG_THREAD_NAME)
    Platform::CopyString(threadMetrics->NameBuf, k_thread_name_get((k_tid_t) thread));
    threadMetrics->name.Emplace(CharSpan::fromCharString(threadMetrics->NameBuf));
#endif

    threadMetrics->id = (uint64_t) thread;
    threadMetrics->stackFreeCurrent.ClearValue(); // unsupported metric
    threadMetrics->stackFreeMinimum.ClearValue();

#if defined(CONFIG_THREAD_STACK_INFO)
    threadMetrics->stackSize.Emplace(static_cast<uint32_t>(thread->stack_info.size));

    if (k_thread_stack_space_get(thread, &unusedStackSize) == 0)
    {
        threadMetrics->stackFreeMinimum.Emplace(static_cast<uint32_t>(unusedStackSize));
    }
#else
    (void) unusedStackSize;
#endif

    threadMetrics->Next    = *threadMetricsListHead;
    *threadMetricsListHead = threadMetrics;
}

BootReasonType DetermineBootReason()
{
#ifdef CONFIG_HWINFO
    uint32_t reason;

    if (hwinfo_get_reset_cause(&reason) != 0)
    {
        return BootReasonType::kUnspecified;
    }

    // Bits returned by hwinfo_get_reset_cause() are accumulated between subsequent resets, so
    // the reset cause must be cleared after reading in order to make sure it always contains
    // information about the most recent boot only.
    (void) hwinfo_clear_reset_cause();

    // If no reset cause is provided, it indicates a power-on-reset.
    if (reason == 0 || reason & (RESET_POR | RESET_PIN))
    {
        return BootReasonType::kPowerOnReboot;
    }

    if (reason & RESET_WATCHDOG)
    {
        return BootReasonType::kHardwareWatchdogReset;
    }

    if (reason & RESET_BROWNOUT)
    {
        return BootReasonType::kBrownOutReset;
    }

    if (reason & RESET_SOFTWARE)
    {
#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT || CHIP_DEVICE_LAYER_TARGET_TELINK
        if (GetSoftwareRebootReason() == SoftwareRebootReason::kSoftwareUpdate)
        {
            return BootReasonType::kSoftwareUpdateCompleted;
        }
#elif defined(CONFIG_MCUBOOT_IMG_MANAGER)
        if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT)
        {
            return BootReasonType::kSoftwareUpdateCompleted;
        }
#endif
        return BootReasonType::kSoftwareReset;
    }
#endif

    return BootReasonType::kUnspecified;
}

} // namespace

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

DiagnosticDataProviderImpl::DiagnosticDataProviderImpl() : mBootReason(DetermineBootReason())
{
    ChipLogDetail(DeviceLayer, "Boot reason: %u", static_cast<uint16_t>(mBootReason));
}

bool DiagnosticDataProviderImpl::SupportsWatermarks()
{
#if defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_CHIP_MALLOC_SYS_HEAP_WATERMARKS_SUPPORT)
    return true;
#else
    return false;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
#ifdef CONFIG_CHIP_MALLOC_SYS_HEAP
    Malloc::Stats stats;
    ReturnErrorOnFailure(Malloc::GetStats(stats));

    currentHeapFree = stats.free;
    return CHIP_NO_ERROR;
#elif CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO
    const auto stats = mallinfo();
    currentHeapFree  = kMaxHeapSize - stats.arena + stats.fordblks;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
#ifdef CONFIG_CHIP_MALLOC_SYS_HEAP
    Malloc::Stats stats;
    ReturnErrorOnFailure(Malloc::GetStats(stats));

    currentHeapUsed = stats.used;
    return CHIP_NO_ERROR;
#elif CHIP_DEVICE_CONFIG_HEAP_STATISTICS_MALLINFO
    currentHeapUsed = mallinfo().uordblks;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
#if defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_CHIP_MALLOC_SYS_HEAP_WATERMARKS_SUPPORT)
    Malloc::Stats stats;
    ReturnErrorOnFailure(Malloc::GetStats(stats));

    currentHeapHighWatermark = stats.maxUsed;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
#if defined(CONFIG_CHIP_MALLOC_SYS_HEAP) && defined(CONFIG_CHIP_MALLOC_SYS_HEAP_WATERMARKS_SUPPORT)
    Malloc::ResetMaxStats();
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
#if defined(CONFIG_THREAD_MONITOR)
    *threadMetricsOut = NULL;
    k_thread_foreach((k_thread_user_cb_t) GetThreadInfo, threadMetricsOut);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics)
{
    while (threadMetrics)
    {
        ThreadMetrics * thread = threadMetrics;
        threadMetrics          = threadMetrics->Next;
        Platform::Delete<ThreadMetrics>(thread);
    }
}

CHIP_ERROR DiagnosticDataProviderImpl::GetRebootCount(uint16_t & rebootCount)
{
    uint32_t count = 0;
    CHIP_ERROR err = ConfigurationMgr().GetRebootCount(count);

    if (err == CHIP_NO_ERROR)
    {
        // If the value overflows, return UINT16 max value to provide best-effort number.
        rebootCount = static_cast<uint16_t>(count <= UINT16_MAX ? count : UINT16_MAX);
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
    // Update the total operational hours and get the most recent value.
    return PlatformMgrImpl().UpdateOperationalHours(&totalOperationalHours);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
#if CONFIG_HWINFO
    bootReason = mBootReason;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * head = NULL;

    for (Inet::InterfaceIterator interfaceIterator; interfaceIterator.HasCurrent(); interfaceIterator.Next())
    {
        NetworkInterface * ifp = new NetworkInterface();

        TEMPORARY_RETURN_IGNORED interfaceIterator.GetInterfaceName(ifp->Name, Inet::InterfaceId::kMaxIfNameLength);
        ifp->name = CharSpan::fromCharString(ifp->Name);
        Inet::InterfaceType interfaceType;
        if (interfaceIterator.GetInterfaceType(interfaceType) == CHIP_NO_ERROR)
        {
            switch (interfaceType)
            {
            case Inet::InterfaceType::Unknown:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kUnspecified;
                break;
            case Inet::InterfaceType::WiFi:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
                break;
            case Inet::InterfaceType::Ethernet:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kEthernet;
                break;
            case Inet::InterfaceType::Thread:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;
                break;
            case Inet::InterfaceType::Cellular:
                ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kCellular;
                break;
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get interface type");
        }

        ifp->offPremiseServicesReachableIPv4.SetNull();
        ifp->offPremiseServicesReachableIPv6.SetNull();

        CHIP_ERROR error;
        uint8_t addressSize;

#ifdef CONFIG_WIFI
        if (interfaceType == Inet::InterfaceType::WiFi)
        {
            ifp->isOperational = ConnectivityMgr().IsWiFiStationConnected();
            error              = interfaceIterator.GetHardwareAddress(ifp->MacAddress, addressSize, sizeof(ifp->MacAddress));
        }
        else
#endif // CONFIG_WIFI
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
            if (interfaceType == Inet::InterfaceType::Thread)
        {
            ifp->isOperational = ConnectivityMgr().IsThreadAttached();

            static_assert(OT_EXT_ADDRESS_SIZE <= sizeof(ifp->MacAddress), "Unexpected extended address size");
            error       = ThreadStackMgr().GetPrimary802154MACAddress(ifp->MacAddress);
            addressSize = OT_EXT_ADDRESS_SIZE;
        }
        else
#endif
        {
            ifp->isOperational = true;
            error              = interfaceIterator.GetHardwareAddress(ifp->MacAddress, addressSize, sizeof(ifp->MacAddress));
        }

        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to get network hardware address");
        }
        else
        {
            ifp->hardwareAddress = ByteSpan(ifp->MacAddress, addressSize);
        }

        // Assuming IPv6-only support
        Inet::InterfaceAddressIterator interfaceAddressIterator;
        uint8_t ipv6AddressesCount = 0;
        while (interfaceAddressIterator.HasCurrent() && ipv6AddressesCount < kMaxIPv6AddrCount)
        {
            if (interfaceAddressIterator.GetInterfaceId() == interfaceIterator.GetInterfaceId())
            {
                chip::Inet::IPAddress ipv6Address;
                if (interfaceAddressIterator.GetAddress(ipv6Address) == CHIP_NO_ERROR)
                {
                    memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], ipv6Address.Addr, kMaxIPv6AddrSize);
                    ifp->Ipv6AddressSpans[ipv6AddressesCount] = ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount]);
                    ipv6AddressesCount++;
                }
            }
            interfaceAddressIterator.Next();
        }

        ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);
        ifp->Next          = head;
        head               = ifp;
    }

    *netifpp = head;
    return CHIP_NO_ERROR;
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

#ifdef CONFIG_WIFI
CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBssId(MutableByteSpan & value)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    VerifyOrReturnError(sizeof(info.mBssId) >= value.size(), CHIP_ERROR_BUFFER_TOO_SMALL);

    memcpy(value.data(), info.mBssId, sizeof(info.mBssId));
    value.reduce_size(sizeof(info.mBssId));

    return CHIP_NO_ERROR;
}

CHIP_ERROR
DiagnosticDataProviderImpl::GetWiFiSecurityType(app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum & securityType)
{
    using app::Clusters::WiFiNetworkDiagnostics::SecurityTypeEnum;

    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    securityType   = info.mSecurityType;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiVersion(app::Clusters::WiFiNetworkDiagnostics::WiFiVersionEnum & wiFiVersion)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    wiFiVersion    = info.mWiFiVersion;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiChannelNumber(uint16_t & channelNumber)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    channelNumber  = info.mChannel;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiRssi(int8_t & rssi)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    rssi           = info.mRssi;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiCurrentMaxRate(uint64_t & currentMaxRate)
{
    WiFiManager::WiFiInfo info;
    ReturnErrorOnFailure(WiFiManager::Instance().GetWiFiInfo(info));
    // mCurrentPhyRate Value in MB
    currentMaxRate = info.mCurrentPhyRate * 1000000;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconLostCount(uint32_t & beaconLostCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    beaconLostCount = stats.mBeaconsLostCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiBeaconRxCount(uint32_t & beaconRxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    beaconRxCount  = stats.mBeaconsSuccessCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastRxCount(uint32_t & packetMulticastRxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetMulticastRxCount = stats.mPacketMulticastRxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketMulticastTxCount(uint32_t & packetMulticastTxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetMulticastTxCount = stats.mPacketMulticastTxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastRxCount(uint32_t & packetUnicastRxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetUnicastRxCount = stats.mPacketUnicastRxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiPacketUnicastTxCount(uint32_t & packetUnicastTxCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    packetUnicastTxCount = stats.mPacketUnicastTxCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetWiFiOverrunCount(uint64_t & overrunCount)
{
    WiFiManager::NetworkStatistics stats;
    ReturnErrorOnFailure(WiFiManager::Instance().GetNetworkStatistics(stats));
    overrunCount   = static_cast<uint64_t>(stats.mOverRunCount);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWiFiNetworkDiagnosticsCounts()
{
    net_if * iface = InetUtils::GetWiFiInterface();
    VerifyOrReturnError(iface != nullptr, INET_ERROR_UNKNOWN_INTERFACE);

    if (net_mgmt(NET_REQUEST_STATS_RESET_WIFI, iface, NULL, 0))
    {
        ChipLogError(DeviceLayer, "WiFi statistics reset failed");
        return CHIP_ERROR_INTERNAL;
    }

    return CHIP_NO_ERROR;
}
#endif // CONFIG_WIFI

} // namespace DeviceLayer
} // namespace chip
