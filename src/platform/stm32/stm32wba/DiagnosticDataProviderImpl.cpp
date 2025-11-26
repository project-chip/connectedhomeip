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
 *          for stm32 platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <platform/stm32/stm32wba/DiagnosticDataProviderImpl.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
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

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
	HeapStats_t heapStats;
	vPortGetHeapStats(&heapStats);
	currentHeapFree = static_cast<uint64_t>(heapStats.xAvailableHeapSpaceInBytes);
	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
	HeapStats_t heapStats;
	vPortGetHeapStats(&heapStats);
	//using the linker defined heap size
	uint64_t totalHeapSize = static_cast<uint64_t>(configTOTAL_HEAP_SIZE /*(50*1024)*/);
	// Calculate used heap space
	currentHeapUsed = static_cast<uint64_t>(totalHeapSize - heapStats.xAvailableHeapSpaceInBytes);
	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    HeapStats_t heapStats;
    vPortGetHeapStats(&heapStats);
	uint64_t totalHeapSize = static_cast<uint64_t>(configTOTAL_HEAP_SIZE);
    currentHeapHighWatermark = static_cast<uint64_t>(totalHeapSize - heapStats.xMinimumEverFreeBytesRemaining);
    return CHIP_NO_ERROR;
}


CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
	HeapStats_t heapStats;
	vPortGetHeapStats(&heapStats);

	heapStats.xAvailableHeapSpaceInBytes = 0;
	heapStats.xSizeOfLargestFreeBlockInBytes = 0;
	heapStats.xSizeOfSmallestFreeBlockInBytes = 0;
	heapStats.xNumberOfFreeBlocks = 0;
	heapStats.xMinimumEverFreeBytesRemaining = 0;
	heapStats.xNumberOfSuccessfulAllocations = 0;
	heapStats.xNumberOfSuccessfulFrees = 0;
	return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    /* Obtain all available task information */
    TaskStatus_t * taskStatusArray;
    ThreadMetrics * head = nullptr;
    unsigned long arraySize, x, dummy;

    arraySize = uxTaskGetNumberOfTasks();

    taskStatusArray = (TaskStatus_t *) pvPortMalloc(arraySize * sizeof(TaskStatus_t));

    if (taskStatusArray != NULL)
    {
        /* Generate raw status information about each task. */
        arraySize = uxTaskGetSystemState(taskStatusArray, arraySize, &dummy);
        /* For each populated position in the taskStatusArray array,
           format the raw data as human readable ASCII data. */

        for (x = 0; x < arraySize; x++)
        {
            ThreadMetrics * thread = (ThreadMetrics *) pvPortMalloc(sizeof(ThreadMetrics));

            Platform::CopyString(thread->NameBuf, taskStatusArray[x].pcTaskName);
            thread->name.Emplace(CharSpan::fromCharString(thread->NameBuf));
            thread->id = taskStatusArray[x].xTaskNumber;

            thread->stackFreeMinimum.Emplace(taskStatusArray[x].usStackHighWaterMark);

            /* Unsupported metrics */
            thread->stackFreeCurrent.ClearValue();
            thread->stackSize.ClearValue();

            thread->Next = head;
            head         = thread;
        }

        *threadMetricsOut = head;
        /* The array is no longer needed, free the memory it consumes. */
        vPortFree(taskStatusArray);
    }

    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics)
{
    while (threadMetrics)
    {
        ThreadMetrics * del = threadMetrics;
        threadMetrics       = threadMetrics->Next;
        vPortFree(del);
    }
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
    return ConfigurationMgr().GetTotalOperationalHours(totalOperationalHours);
}

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;

    CHIP_ERROR err = ConfigurationMgr().GetBootReason(reason);
    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
        ChipLogProgress(DeviceLayer, "GetBootReasonType: %ld", static_cast<uint32_t>(bootReason));
    }

    return err;
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
    ifp->isOperational = false;
    ifp->type          = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kUnspecified;
#endif

    // IPv6-only support
    Inet::InterfaceAddressIterator interfAddrIterator;
    uint8_t ipv6AddressesCount = 0;
    while (interfAddrIterator.HasCurrent() && ipv6AddressesCount < kMaxIPv6AddrCount)
    {
        chip::Inet::IPAddress ipv6Address;
        if (interfAddrIterator.GetAddress(ipv6Address) == CHIP_NO_ERROR)
        {
            memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], ipv6Address.Addr, kMaxIPv6AddrSize);
            ifp->Ipv6AddressSpans[ipv6AddressesCount] = ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount]);
            ipv6AddressesCount++;
        }
        interfAddrIterator.Next();
    }
    ifp->IPv6Addresses = chip::app::DataModel::List<chip::ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);

    *netifpp = ifp;
    return CHIP_NO_ERROR;
}


DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
