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
 *          for Realtek platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/CHIPMemString.h>
#include <platform/PlatformManager.h>
#include <platform/Realtek_bee/DiagnosticDataProviderImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#endif

#include "FreeRTOS.h"
#include "mem_config.h"
#include "os_mem.h"

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    size_t freeHeapSize = os_mem_peek(RAM_TYPE_DATA_ON);
    currentHeapFree     = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    size_t usedHeapSize = NS_HEAP_SIZE - os_mem_peek(RAM_TYPE_DATA_ON);
    currentHeapUsed = static_cast<uint64_t>(usedHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    size_t highestHeapUsageRecorded = NS_HEAP_SIZE - xPortGetMinimumEverFreeHeapSize(RAM_TYPE_DATA_ON);
    currentHeapHighWatermark = static_cast<uint64_t>(highestHeapUsageRecorded);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::ResetWatermarks()
{
    // If implemented, the server SHALL set the value of the CurrentHeapHighWatermark attribute to the
    // value of the CurrentHeapUsed.

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetThreadMetrics(ThreadMetrics ** threadMetricsOut)
{
    /* Obtain all available task information */
    TaskStatus_t * taskStatusArray;
    ThreadMetrics * head = nullptr;
    uint32_t arraySize, x, dummy;

    arraySize = uxTaskGetNumberOfTasks();

    taskStatusArray = (TaskStatus_t *)os_mem_alloc(RAM_TYPE_DATA_ON, arraySize * sizeof(TaskStatus_t));

    if (taskStatusArray != NULL)
    {
        /* Generate raw status information about each task. */
        arraySize = uxTaskGetSystemState(taskStatusArray, arraySize, &dummy);
        /* For each populated position in the taskStatusArray array,
           format the raw data as human readable ASCII data. */

        for (x = 0; x < arraySize; x++)
        {
            ThreadMetrics * thread = (ThreadMetrics *)os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(ThreadMetrics));

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
        os_mem_free(taskStatusArray);
    }

    return CHIP_NO_ERROR;
}

void DiagnosticDataProviderImpl::ReleaseThreadMetrics(ThreadMetrics * threadMetrics)
{
    while (threadMetrics)
    {
        ThreadMetrics * del = threadMetrics;
        threadMetrics       = threadMetrics->Next;
        os_mem_free(del);
    }
}

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
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

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(BootReasonType & bootReason)
{
    uint32_t reason = 0;

    CHIP_ERROR err  = ConfigurationMgr().GetBootReason(reason);

    if (err == CHIP_NO_ERROR)
    {
        VerifyOrReturnError(reason <= UINT8_MAX, CHIP_ERROR_INVALID_INTEGER_VALUE);
        bootReason = static_cast<BootReasonType>(reason);
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
    ChipLogProgress(DeviceLayer, "GetActiveHardwareFaults");

#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_ENUM_RADIO));
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_ENUM_SENSOR));
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_ENUM_POWER_SOURCE));
    ReturnErrorOnFailure(hardwareFaults.add(EMBER_ZCL_HARDWARE_FAULT_ENUM_USER_INTERFACE_FAULT));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveRadioFaults(GeneralFaults<kMaxRadioFaults> & radioFaults)
{
    ChipLogProgress(DeviceLayer, "GetActiveRadioFaults");
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_ENUM_THREAD_FAULT));
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_ENUM_BLE_FAULT));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
    ChipLogProgress(DeviceLayer, "GetActiveNetworkFaults");
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_ENUM_HARDWARE_FAILURE));
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_ENUM_NETWORK_JAMMED));
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_ENUM_CONNECTION_FAILED));
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
    ifp->type = app::Clusters::GeneralDiagnostics::InterfaceTypeEnum::kThread;;

    otExtAddress extAddr;
    ThreadStackMgrImpl().GetExtAddress(extAddr);
    ifp->hardwareAddress = ByteSpan(extAddr.m8, OT_EXT_ADDRESS_SIZE);

    /* Thread only support IPv6 */
    uint8_t ipv6AddressesCount = 0;
    for (Inet::InterfaceAddressIterator iterator; iterator.Next() && ipv6AddressesCount < kMaxIPv6AddrCount;)
    {
        chip::Inet::IPAddress ipv6Address;
        if (iterator.GetAddress(ipv6Address) == CHIP_NO_ERROR)
        {
            memcpy(ifp->Ipv6AddressesBuffer[ipv6AddressesCount], ipv6Address.Addr, kMaxIPv6AddrSize);
            ifp->Ipv6AddressSpans[ipv6AddressesCount] = ByteSpan(ifp->Ipv6AddressesBuffer[ipv6AddressesCount]);
            ipv6AddressesCount++;
        }
    }
    ifp->IPv6Addresses = app::DataModel::List<const ByteSpan>(ifp->Ipv6AddressSpans, ipv6AddressesCount);

    *netifpp = ifp;
#endif

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

} // namespace DeviceLayer
} // namespace chip
