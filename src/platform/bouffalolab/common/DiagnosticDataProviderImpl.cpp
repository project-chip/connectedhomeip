/*
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/CHIPMemString.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>

#include <FreeRTOS.h>
#if CHIP_DEVICE_LAYER_TARGET_BL616
#include <mem.h>
#endif
namespace chip {
namespace DeviceLayer {

extern "C" size_t get_heap_size(void);
#ifdef CFG_USE_PSRAM
extern "C" size_t get_heap3_size(void);
#endif
extern "C" struct netif * deviceInterface_getNetif(void);

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
#if CHIP_DEVICE_LAYER_TARGET_BL616
    struct meminfo info;
    bflb_mem_usage(KMEM_HEAP, &info);
    currentHeapFree = info.free_size;
#else
#ifdef CFG_USE_PSRAM
    currentHeapFree = xPortGetFreeHeapSize() + xPortGetFreeHeapSizePsram();
#else
    currentHeapFree          = xPortGetFreeHeapSize();
#endif
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
#if CHIP_DEVICE_LAYER_TARGET_BL616
    struct meminfo info;
    bflb_mem_usage(KMEM_HEAP, &info);
    currentHeapUsed = info.total_size - info.free_size;
#else
#ifdef CFG_USE_PSRAM
    currentHeapUsed = (get_heap_size() + get_heap3_size() - xPortGetFreeHeapSize() - xPortGetFreeHeapSizePsram());
#else
    currentHeapUsed          = (get_heap_size() - xPortGetFreeHeapSize());
#endif
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
#if CHIP_DEVICE_LAYER_TARGET_BL616
    struct meminfo info;
    bflb_mem_usage(KMEM_HEAP, &info);
    currentHeapHighWatermark = info.total_size - info.max_free_size;

#else
#ifdef CFG_USE_PSRAM
    currentHeapHighWatermark =
        get_heap_size() + get_heap3_size() - xPortGetMinimumEverFreeHeapSize() - xPortGetMinimumEverFreeHeapSizePsram();
#else
    currentHeapHighWatermark = get_heap_size() - xPortGetMinimumEverFreeHeapSize();
#endif
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    /* Obtain all available task information */
    TaskStatus_t * taskStatusArray;
    ThreadMetrics * head = nullptr;
    uint32_t arraySize, x, dummy;

    arraySize = uxTaskGetNumberOfTasks();

    taskStatusArray = static_cast<TaskStatus_t *>(chip::Platform::MemoryCalloc(arraySize, sizeof(TaskStatus_t)));

    if (taskStatusArray != NULL)
    {
        /* Generate raw status information about each task. */
        arraySize = uxTaskGetSystemState(taskStatusArray, arraySize, &dummy);
        /* For each populated position in the taskStatusArray array,
           format the raw data as human readable ASCII data. */

        for (x = 0; x < arraySize; x++)
        {
            ThreadMetrics * thread = new ThreadMetrics();
            if (thread)
            {
                Platform::CopyString(thread->NameBuf, taskStatusArray[x].pcTaskName);
                thread->name.Emplace(CharSpan::fromCharString(thread->NameBuf));
                thread->id = taskStatusArray[x].xTaskNumber;
                thread->stackFreeMinimum.Emplace(taskStatusArray[x].usStackHighWaterMark);

                /* Unsupported metrics */
                // thread->stackSize
                // thread->stackFreeCurrent

                thread->Next = head;
                head         = thread;
            }
        }

        *threadMetricsOut = head;
        /* The array is no longer needed, free the memory it consumes. */
        chip::Platform::MemoryFree(taskStatusArray);
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveHardwareFaults(GeneralFaults<kMaxHardwareFaults> & hardwareFaults)
{
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kRadio)));
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kSensor)));
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kPowerSource)));
    ReturnErrorOnFailure(hardwareFaults.add(to_underlying(HardwareFaultEnum::kUserInterfaceFault)));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(radioFaults.add(to_underlying(RadioFaultEnum::kThreadFault)));
    ReturnErrorOnFailure(radioFaults.add(to_underlying(RadioFaultEnum::kBLEFault)));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(networkFaults.add(to_underlying(NetworkFaultEnum::kHardwareFailure)));
    ReturnErrorOnFailure(networkFaults.add(to_underlying(NetworkFaultEnum::kNetworkJammed)));
    ReturnErrorOnFailure(networkFaults.add(to_underlying(NetworkFaultEnum::kConnectionFailed)));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * ifp = new NetworkInterface();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    const char * threadNetworkName = otThreadGetNetworkName(ThreadStackMgrImpl().OTInstance());
    ifp->name                      = Span<const char>(threadNetworkName, strlen(threadNetworkName));
    ifp->isOperational             = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;
    uint8_t macBuffer[ConfigurationManager::kPrimaryMACAddressLength];
    ConfigurationMgr().GetPrimary802154MACAddress(macBuffer);
    ifp->hardwareAddress = ByteSpan(macBuffer, ConfigurationManager::kPrimaryMACAddressLength);
#else

    struct netif * netif = deviceInterface_getNetif();

    Platform::CopyString(ifp->Name, netif->name);
    ifp->name          = CharSpan::fromCharString(ifp->Name);
    ifp->isOperational = true;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ifp->type          = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kWiFi;
#else
    ifp->type                = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kEthernet;
#endif
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();

    memcpy(ifp->MacAddress, netif->hwaddr, sizeof(netif->hwaddr));
    ifp->hardwareAddress = ByteSpan(ifp->MacAddress, sizeof(netif->hwaddr));

    memcpy(ifp->Ipv4AddressesBuffer[0], netif_ip_addr4(netif), kMaxIPv4AddrSize);
    ifp->Ipv4AddressSpans[0] = ByteSpan(ifp->Ipv4AddressesBuffer[0], kMaxIPv4AddrSize);
    ifp->IPv4Addresses       = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv4AddressSpans, 1);

    int addr_count = 0;
    for (size_t i = 0; (i < LWIP_IPV6_NUM_ADDRESSES) && (i < kMaxIPv6AddrCount); i++)
    {
        if (!ip6_addr_isany(&(netif->ip6_addr[i].u_addr.ip6)))
        {
            memcpy(ifp->Ipv6AddressesBuffer[addr_count], &(netif->ip6_addr[i].u_addr.ip6), sizeof(ip6_addr_t));
            ifp->Ipv6AddressSpans[addr_count] = ByteSpan(ifp->Ipv6AddressesBuffer[addr_count], kMaxIPv6AddrSize);
        }
    }
    ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, addr_count);
#endif

    *netifpp = ifp;

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

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
