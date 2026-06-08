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
#include <mm.h>
#endif

namespace chip {
namespace DeviceLayer {

extern "C" struct netif * deviceInterface_getNetif(void);

#if CHIP_DEVICE_LAYER_TARGET_BL616
void get_usage_info(uint64_t & used_size, uint64_t & free_size)
{
    uintptr_t irq_flags     = mm_lock_save();
    mem_manager_t * manager = &g_mem_manager;
    mm_heap_t * heap;
    const mm_allocator_t * allocator;
    struct mm_usage_info usage;
    int heap_id = 0;

    used_size = free_size = 0;
    for (heap_id = 0; heap_id < CONFIG_MM_HEAP_COUNT; heap_id++)
    {
        if (!manager->initialized)
        {
            break;
        }

        heap = manager->heaps[heap_id];
        if (!heap || !heap->is_active)
        {
            continue;
        }

        allocator = manager->allocators[heap->allocator_id];
        if (!allocator || !allocator->get_usage_info)
        {
            continue;
        }

        allocator->get_usage_info(heap, &usage);

        used_size += usage.used_size;
        free_size += usage.free_size;
    }

    mm_unlock_restore(irq_flags);
}

#else
extern "C" size_t get_heap_size(void);
#ifdef CFG_USE_PSRAM
extern "C" size_t get_heap3_size(void);
#endif
#endif

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
#if CHIP_DEVICE_LAYER_TARGET_BL616
    uint64_t used_size;
    get_usage_info(used_size, currentHeapFree);
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
    uint64_t free_size;
    get_usage_info(currentHeapUsed, free_size);
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
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#else
#ifdef CFG_USE_PSRAM
    currentHeapHighWatermark =
        get_heap_size() + get_heap3_size() - xPortGetMinimumEverFreeHeapSize() - xPortGetMinimumEverFreeHeapSizePsram();
#else
    currentHeapHighWatermark = get_heap_size() - xPortGetMinimumEverFreeHeapSize();
#endif
    return CHIP_NO_ERROR;
#endif
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
    auto ifp = std::make_unique<NetworkInterface>();

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    const char * threadNetworkName = otThreadGetNetworkName(ThreadStackMgrImpl().OTInstance());
    ifp->name                      = Span<const char>(threadNetworkName, strlen(threadNetworkName));
    ifp->isOperational             = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;
    TEMPORARY_RETURN_IGNORED ConfigurationMgr().GetPrimary802154MACAddress(ifp->MacAddress);
    ifp->hardwareAddress = ByteSpan(ifp->MacAddress, sizeof(ifp->MacAddress));
#else

    struct netif * netif = deviceInterface_getNetif();
    if (netif == nullptr)
    {
        *netifpp = nullptr;
        return CHIP_ERROR_NOT_FOUND;
    }

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
#endif

    // IPv6-only support
    Inet::InterfaceAddressIterator interfAddrIterator;
    uint32_t ipv6AddressesCount = 0, ipv4AddressesCount = 0;
    chip::Inet::IPAddress ipAddress;
    while (interfAddrIterator.HasCurrent())
    {
        if (interfAddrIterator.GetAddress(ipAddress) == CHIP_NO_ERROR)
        {
            if (ipAddress.IsIPv4())
            {
                if (ipv4AddressesCount < kMaxIPv4AddrCount)
                {
                    memcpy(ifp->Ipv4AddressesBuffer[ipv4AddressesCount], ipAddress.Addr, kMaxIPv4AddrSize);
                    ifp->Ipv4AddressSpans[ipv4AddressesCount] = ByteSpan(ifp->Ipv4AddressesBuffer[ipv4AddressesCount]);
                    ipv4AddressesCount++;
                }
            }
            else
            {
                if (ipv6AddressesCount < kMaxIPv6AddrCount)
                {
                    memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], ipAddress.Addr, kMaxIPv6AddrSize);
                    ifp->Ipv6AddressSpans[ipv6AddressesCount] = ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount]);
                    ipv6AddressesCount++;
                }
            }
        }
        interfAddrIterator.Next();
    }
    ifp->IPv4Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv4AddressSpans, ipv4AddressesCount);
    ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);

    *netifpp = ifp.release();

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
