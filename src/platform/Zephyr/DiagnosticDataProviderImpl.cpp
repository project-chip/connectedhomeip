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
 *          for Zephy platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <lib/support/logging/CHIPLogging.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/Zephyr/DiagnosticDataProviderImpl.h>

#include <drivers/hwinfo.h>

#include <malloc.h>

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
#ifdef CONFIG_NEWLIB_LIBC
    // This will return the amount of memory which has been allocated from the system, but is not
    // used right now. Ideally, this value should be increased by the amount of memory which can
    // be allocated from the system, but Zephyr does not expose that number.
    currentHeapFree = mallinfo().fordblks;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
#ifdef CONFIG_NEWLIB_LIBC
    currentHeapUsed = mallinfo().uordblks;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
#ifdef CONFIG_NEWLIB_LIBC
    // ARM newlib does not provide a way to obtain the peak heap usage, so for now just return
    // the amount of memory allocated from the system which should be an upper bound of the peak
    // usage provided that the heap is not very fragmented.
    currentHeapHighWatermark = mallinfo().arena;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
#endif
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

CHIP_ERROR DiagnosticDataProviderImpl::GetBootReason(uint8_t & bootReason)
{
#if CONFIG_HWINFO
    uint32_t reason = 0;

    CHIP_ERROR err = CHIP_NO_ERROR;

    if (hwinfo_get_reset_cause(&reason) != 0)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    if (reason & (RESET_POR | RESET_PIN))
    {
        bootReason = BootReasonType::PowerOnReboot;
    }
    else if (reason & RESET_WATCHDOG)
    {
        bootReason = BootReasonType::SoftwareWatchdogReset;
    }
    else if (reason & RESET_BROWNOUT)
    {
        bootReason = BootReasonType::BrownOutReset;
    }
    else if (reason & (RESET_SOFTWARE | RESET_CPU_LOCKUP | RESET_DEBUG))
    {
        bootReason = BootReasonType::SoftwareReset;
    }
    else
    {
        bootReason = BootReasonType::Unspecified;
    }

    return err;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
#if CHIP_SYSTEM_CONFIG_USE_ZEPHYR_NET_IF
    NetworkInterface * head = NULL;

    for (Inet::InterfaceIterator interfaceIterator; interfaceIterator.HasCurrent(); interfaceIterator.Next())
    {
        NetworkInterface * ifp = new NetworkInterface();

        interfaceIterator.GetInterfaceName(ifp->Name, Inet::InterfaceId::kMaxIfNameLength);
        ifp->name            = CharSpan(ifp->Name, strlen(ifp->Name));
        ifp->fabricConnected = true;
        Inet::InterfaceType interfaceType;
        if (interfaceIterator.GetInterfaceType(interfaceType) == CHIP_NO_ERROR)
        {
            switch (interfaceType)
            {
            case Inet::InterfaceType::Unknown:
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_UNSPECIFIED;
                break;
            case Inet::InterfaceType::WiFi:
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_WI_FI;
                break;
            case Inet::InterfaceType::Ethernet:
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_ETHERNET;
                break;
            case Inet::InterfaceType::Thread:
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_THREAD;
                break;
            case Inet::InterfaceType::Cellular:
                ifp->type = EMBER_ZCL_INTERFACE_TYPE_CELLULAR;
                break;
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to get interface type");
        }

        ifp->offPremiseServicesReachableIPv4 = false;
        ifp->offPremiseServicesReachableIPv6 = false;

        uint8_t addressSize;
        if (interfaceIterator.GetHardwareAddress(ifp->MacAddress, addressSize, sizeof(ifp->MacAddress)) != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Failed to get network hardware address");
        }
        else
        {
            ifp->hardwareAddress = ByteSpan(ifp->MacAddress, addressSize);
        }

        head = ifp;
    }

    *netifpp = head;
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
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
