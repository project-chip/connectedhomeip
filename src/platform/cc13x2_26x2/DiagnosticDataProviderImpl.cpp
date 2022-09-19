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
 *          for cc13x2 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <bget.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/cc13x2_26x2/DiagnosticDataProviderImpl.h>

#include <driverlib/sys_ctrl.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

/*
 * The following Heap stats are compiled values done by the BGET heap implementation.
 * See https://www.fourmilab.ch/bget/
 */
CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    long freeHeapSize, dummy;

    bstats(&dummy, &freeHeapSize, &dummy, &dummy, &dummy);

    currentHeapFree = static_cast<uint64_t>(freeHeapSize);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    long heapUsed, dummy;

    bstats(&heapUsed, &dummy, &dummy, &dummy, &dummy);

    VerifyOrReturnError(heapUsed >= 0, CHIP_ERROR_INVALID_INTEGER_VALUE);
    currentHeapUsed = static_cast<uint64_t>(heapUsed);

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

            strncpy(thread->NameBuf, taskStatusArray[x].pcTaskName, kMaxThreadNameLength - 1);
            thread->NameBuf[kMaxThreadNameLength] = '\0';
            thread->name.Emplace(CharSpan::fromCharString(thread->NameBuf));
            thread->id = taskStatusArray[x].xTaskNumber;

            thread->stackFreeMinimum.Emplace(taskStatusArray[x].usStackHighWaterMark);
            /* Unsupported metrics */
            // thread->stackSize
            // thread->stackFreeCurrent

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

// General Diagnostics Getters

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
    switch (SysCtrlResetSourceGet())
    {
    case RSTSRC_PWR_ON:
    case RSTSRC_WAKEUP_FROM_SHUTDOWN:
        bootReason = BootReasonType::kPowerOnReboot;
        break;

    case RSTSRC_PIN_RESET:
    case RSTSRC_WAKEUP_FROM_TCK_NOISE:
        bootReason = BootReasonType::kHardwareWatchdogReset;
        break;

    case RSTSRC_VDDS_LOSS:
    case RSTSRC_VDDR_LOSS:
    case RSTSRC_CLK_LOSS:
        bootReason = BootReasonType::kBrownOutReset;
        break;

    case RSTSRC_SYSRESET:
    case RSTSRC_WARMRESET:
        bootReason = BootReasonType::kSoftwareReset;
        // We do not have a clean way to differentiate between a software reset
        // and a completed software update.
        // bootReason = kSoftwareUpdateCompleted;
        break;

    default:
        bootReason = BootReasonType::kUnknownEnumValue;
        break;
    }

    return CHIP_NO_ERROR;
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
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_TYPE_THREAD_FAULT));
    ReturnErrorOnFailure(radioFaults.add(EMBER_ZCL_RADIO_FAULT_TYPE_BLE_FAULT));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetActiveNetworkFaults(GeneralFaults<kMaxNetworkFaults> & networkFaults)
{
#if CHIP_CONFIG_TEST
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_TYPE_HARDWARE_FAILURE));
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_TYPE_NETWORK_JAMMED));
    ReturnErrorOnFailure(networkFaults.add(EMBER_ZCL_NETWORK_FAULT_TYPE_CONNECTION_FAILED));
#endif

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    NetworkInterface * ifp = new NetworkInterface();

    const char * threadNetworkName = otThreadGetNetworkName(ThreadStackMgrImpl().OTInstance());
    ifp->name                      = Span<const char>(threadNetworkName, strlen(threadNetworkName));
    ifp->isOperational             = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->type = EMBER_ZCL_INTERFACE_TYPE_THREAD;

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
