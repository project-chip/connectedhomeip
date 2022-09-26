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
 *          for CYW30739 platform.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/DiagnosticDataProvider.h>
#include <platform/Infineon/CYW30739/DiagnosticDataProviderImpl.h>

#include <hal/wiced_memory.h>
#include <malloc.h>

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#endif

namespace chip {
namespace DeviceLayer {

DiagnosticDataProviderImpl & DiagnosticDataProviderImpl::GetDefaultInstance()
{
    static DiagnosticDataProviderImpl sInstance;
    return sInstance;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapFree(uint64_t & currentHeapFree)
{
    const struct mallinfo mallocInfo = mallinfo();

    currentHeapFree = wiced_memory_get_free_bytes() + mallocInfo.fordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapUsed(uint64_t & currentHeapUsed)
{
    const struct mallinfo mallocInfo = mallinfo();

    currentHeapUsed = mallocInfo.uordblks;

    return CHIP_NO_ERROR;
}

CHIP_ERROR DiagnosticDataProviderImpl::GetCurrentHeapHighWatermark(uint64_t & currentHeapHighWatermark)
{
    const struct mallinfo mallocInfo = mallinfo();

    currentHeapHighWatermark = mallocInfo.arena;

    return CHIP_NO_ERROR;
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

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
CHIP_ERROR DiagnosticDataProviderImpl::GetNetworkInterfaces(NetworkInterface ** netifpp)
{
    auto ifp = Platform::New<NetworkInterface>();
    VerifyOrReturnError(ifp != nullptr, CHIP_ERROR_NO_MEMORY);

    const char * threadNetworkName = otThreadGetNetworkName(ThreadStackMgrImpl().OTInstance());
    ifp->name                      = Span<const char>(threadNetworkName, strlen(threadNetworkName));
    ifp->isOperational             = true;
    ifp->offPremiseServicesReachableIPv4.SetNull();
    ifp->offPremiseServicesReachableIPv6.SetNull();
    ifp->hardwareAddress = ByteSpan(ifp->MacAddress);
    ifp->type            = app::Clusters::GeneralDiagnostics::InterfaceType::EMBER_ZCL_INTERFACE_TYPE_THREAD;

    static_assert(sizeof(ifp->MacAddress) >= ConfigurationManager::kPrimaryMACAddressLength, "Invalid MacAddress buffer size");
    ConfigurationMgr().GetPrimary802154MACAddress(ifp->MacAddress);

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
        Platform::Delete(del);
    }
}
#endif /* CHIP_DEVICE_CONFIG_ENABLE_THREAD */

DiagnosticDataProvider & GetDiagnosticDataProviderImpl()
{
    return DiagnosticDataProviderImpl::GetDefaultInstance();
}

} // namespace DeviceLayer
} // namespace chip
